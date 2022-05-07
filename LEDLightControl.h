/***********************************************************************
* @file      LEDLightControl.h
*
*    Class encapsulation for controlling LED1 (D3, PA_0) on MultiTech 
*    Dragonfly Nano development board with 4G LTE Cellular messages.
* 
*    For ease of use, power, flexibility and readability of the code,  
*    the LightControl protocol has been written in a modern C++ (C++20) 
*    metaprogrammed and templatized class-encapsulated idiom. 
* 
*    A goal of the design is to encourage and promote zero-cost 
*    abstractions, even and especially so, in the embedded realms. Note
*    that care has been taken to deduce and glean these abstractions 
*    from the problem-domain itself. And in the doing so, care has also 
*    been taken to shape the abstractions logically.
* 
* @brief   
* 
* @note    
*
* @warning  
*
* @author  Nuertey Odzeyem
* 
* @date    May 7th, 2022
*
* @copyright Copyright (c) 2022 Nuertey Odzeyem. All Rights Reserved.
***********************************************************************/
#pragma once

#include "mbed.h"
#include "mbed_events.h"
#include "randLIB.h"
#include "Utilities.h"

// Primary usecase:
#define MTS_DRAGONFLY_L471QG 

// To allow for potential debug testing on the only MCU that I do have 
// available:
//#define NUCLEO-F767ZI

// TBD Nuertey Odzeyem; confirm if the below holds for both 
// MTS_DRAGONFLY_L471QG and the NUCLEO-F767ZI targets:
#define LED_ON  1
#define LED_OFF 0

// Choice of socket type configuration parsed from mbed_app.json:
#define UDP 0
#define TCP 1
#define NONIP 2

enum class TransportScheme_t : uint8_t
{
    CELLULAR_4G_LTE,          // Primary usecase for MTS_DRAGONFLY_L471QG target (LTE Cat M1 Cellular).
    ETHERNET,                 // To potentially allow for debug testing on my available NUCLEO-F767ZI target.  
    MESH_NETWORK_6LoWPAN_ND,  // Design room for future enhancements.
    MESH_NETWORK_Wi_SUNMODE_4 // Design room for future enhancements.
};

enum class TransportSocket_t : uint8_t
{
    TCP,
    UDP,
    CELLULAR_NON_IP
};

#if MBED_CONF_APP_SOCK_TYPE == TCP
    static constexpr char SOCKET_TYPE[] = "TCP";
#elif MBED_CONF_APP_SOCK_TYPE == UDP
    static constexpr char SOCKET_TYPE[] = "UDP";
#elif MBED_CONF_APP_SOCK_TYPE == NONIP
    static constexpr char SOCKET_TYPE[] = "CellularNonIP";
#endif

static constexpr char ECHO_HOSTNAME[] = MBED_CONF_APP_ECHO_SERVER_HOSTNAME;
static constexpr int ECHO_PORT = MBED_CONF_APP_ECHO_SERVER_PORT; // Same value holds for TCP and UDP.

using namespace std::chrono_literals;

static constexpr auto LED_BLINKING_RATE = 500ms;

// Intrinsically enforce our requirements with C++20 Concepts.
template <TransportScheme_t transport, TransportSocket_t socket>
concept IsValidTransportType = (((transport == TransportScheme_t::CELLULAR_4G_LTE)
    && ((socket == TransportSocket_t::TCP) || (socket == TransportSocket_t::UDP) || (socket == TransportSocket_t::CELLULAR_NON_IP)))
                             || ((transport == TransportScheme_t::ETHERNET)
    && ((socket == TransportSocket_t::TCP) || (socket == TransportSocket_t::UDP)))
                             || ((transport == TransportScheme_t::MESH_NETWORK_6LoWPAN_ND)
    && (socket == TransportSocket_t::UDP))
                             || ((transport == TransportScheme_t::MESH_NETWORK_Wi_SUNMODE_4)
    && (socket == TransportSocket_t::UDP)));


// Per both potential MCU specs, common LED 'in situ' on the MCU:        
// Target = MTS_DRAGONFLY_L471QG: UNO pin D3 (i.e. STM32 pin PA_0).
// Target = NUCLEO-F767ZI: Green LED
DigitalOut g_UserLED(LED1);

#ifdef MTS_DRAGONFLY_L471QG

#elseif NUCLEO-F767ZI

#endif

class LEDLightControl
{   
    static constexpr uint8_t  MAXIMUM_NUMBER_OF_CONNECT_RETRIES{3};
    
    static constexpr auto     STARTUP_STATUS_CHECK_DELAY    {10ms};
    static constexpr auto     CHANGE_MEASUREMENT_MODE_DELAY {50ms};
    
public:
    LEDLightControl(NetworkInterface * pInterface);

    LEDLightControl(const LEDLightControl&) = delete;
    LEDLightControl& operator=(const LEDLightControl&) = delete;

    virtual ~LEDLightControl();

    template <TransportScheme_t transport, TransportSocket_t socket>
        requires IsValidTransportType<transport, socket>
    [[nodiscard]] bool Connect();
    
    template <TransportScheme_t transport, TransportSocket_t socket>
        requires IsValidTransportType<transport, socket>
    void Run();
        
    void ChangeOperationMode(const IsOperationModeType& mode);

protected:
    template <TransportScheme_t transport, TransportSocket_t socket>
        requires IsValidTransportType<transport, socket>
    [[nodiscard]] bool Send();

    template <TransportScheme_t transport, TransportSocket_t socket>
        requires IsValidTransportType<transport, socket>
    [[nodiscard]] bool Receive();
    
private:
    NetworkInterface *         m_pNetworkInterface;
    std::optional<std::string> m_EchoServerDomainName; // Domain name might not always necessarily exist...
    std::string                m_EchoServerAddress;    // However IP Address always would.
    uint16_t                   m_EchoServerPort;
    IsOperationModeType        m_OperationMode;
    bool                       m_IsConnected;
};

LEDLightControl::LEDLightControl(NetworkInterface * pInterface)
    : m_pNetworkInterface(pInterface)
    , m_EchoServerDomainName(std::nullopt)
    , m_EchoServerAddress(ECHO_HOSTNAME)
    , m_EchoServerPort(ECHO_PORT) 
    , m_OperationMode(standbyMode)
    , m_IsConnected(false)
{

}

LEDLightControl::~LEDLightControl()
{
}

template <TransportScheme_t transport, TransportSocket_t socket>
    requires IsValidTransportType<transport, socket>
bool LEDLightControl::Connect()
{    
    bool result = false;
    
    auto [ipAddress, domainName] = Utility::ResolveAddressIfDomainName(m_EchoServerAddress, m_pNetworkInterface);
    m_EchoServerAddress = ipAddress;
    
    if (domainName)
    {
        m_EchoServerDomainName = std::move(domainName);
    }

    printf("\r\nConnecting to : \"%s:%d\" ...", m_EchoServerAddress.c_str(), m_EchoServerPort);
    nsapi_error_t rc = m_pNetworkInterface.connect();
    
    // TBD Nuertey Odzeyem; perform both NetworkInterface->connect() and socket.open() here.
    
    
    if (rc != NSAPI_ERROR_OK)
    {
        printf("\r\n\r\nError! NetworkInterface.connect() to EchoServer returned: [%d] -> %s\n", rc, ToString(rc).c_str());
    }
    else
    {   
        if constexpr (transport == TransportScheme_t::CELLULAR_4G_LTE) 
        {

        }
        else if constexpr (transport == TransportScheme_t::ETHERNET)
        {

        }
        else
        {
            // Mesh Network branch deliberately unimplemented as it is out of scope.
        }
    }
}

template <TransportScheme_t transport, TransportSocket_t socket>
    requires IsValidTransportType<transport, socket>
void LEDLightControl::Run()
{
    while (true)
    {
        if (Send<transport, socket>())
        {
            if (Receive<transport, socket>())
            {
                // Update LED here?
                continue;
            }
            else
            {
                break;
            }
        }
        else
        {
            break;
        }
    }
}

template <TransportScheme_t transport, TransportSocket_t socket>
    requires IsValidTransportType<transport, socket>
bool LEDLightControl::Send()
{
    if constexpr (transport == TransportScheme_t::CELLULAR_4G_LTE) 
    {

    }
    else if constexpr (transport == TransportScheme_t::ETHERNET)
    {

    }
    else
    {
        // Mesh Network branch deliberately unimplemented as it is out of scope.
    }
}

template <TransportScheme_t transport, TransportSocket_t socket>
    requires IsValidTransportType<transport, socket>
bool LEDLightControl::Receive()
{
    if constexpr (transport == TransportScheme_t::CELLULAR_4G_LTE) 
    {

    }
    else if constexpr (transport == TransportScheme_t::ETHERNET)
    {

    }
    else
    {
        // Mesh Network branch deliberately unimplemented as it is out of scope.
    }
}

void LEDLightControl::ChangeOperationMode(const IsOperationModeType& mode)
{
    // \" In order to avoid a real time error, it is strongly recommended
    // to verify that the DRDY signal is low (no new data) before 
    // activating new mode. \"
    if (Utilities::g_ExternalDataReadyPin.read() == 0)
    {
        if (mode == highResolutionMeasurementMode) 
        {
            WriteToDirectRegister<RegisterAddress_t::OPERATION>(Operation_t::INITIATE_HIGH_RESOLUTION_ACQUISITION_MODE);
        }
        else if (mode == highSpeedMeasurementMode) 
        {
            WriteToDirectRegister<RegisterAddress_t::OPERATION>(Operation_t::INITIATE_HIGH_SPEED_ACQUISITION_MODE);
        }    
        else if (mode == ultraLowPowerMeasurementMode) 
        {
            WriteToDirectRegister<RegisterAddress_t::OPERATION>(Operation_t::INITIATE_ULTRA_LOW_POWER_ACQUISITION_MODE);
        }    
        else if (mode == lowPowerMeasurementWithExternalTriggerMode) 
        {
            WriteToDirectRegister<RegisterAddress_t::OPERATION>(Operation_t::INITIATE_LOW_POWER_ACQUISITION_MODE);
        }    
        else if ((mode == standbyMode) 
              || (mode == powerDownMode))
        {
            WriteToDirectRegister<RegisterAddress_t::OPERATION>(Operation_t::CANCEL_CURRENT_OPERATION_MODE);
        }                
        m_OperationMode = mode;
    }
    else if (Utilities::g_ExternalDataReadyPin.read() == 1)
    {
        // \" If DRDY is high it is necessary to read the output data 
        // before activating new measurement mode. \"
        
        // Post the requested operation mode change on the shared event
        // queue so that the already enqueued Data Ready signal therefore
        // ::AcquireSensorData() method can be scheduled and complete 
        // first before actually changing the operation mode.
        auto event1 = make_user_allocated_event(&this, 
                      &LEDLightControl::ChangeOperationMode, mode);
        
        // bind & post
        event1.call_on(Utilities::g_pSharedEventQueue);
        
        // Note that the EventQueue has no concept of event priority. 
        // If you schedule events to run at the same time, the order in
        // which the events run relative to one another is undefined. 
        // The EventQueue only schedules events based on time.
    }   
}

void LEDLightControl::AcquireSensorData()
{
    if ((m_OperationMode == highResolutionMeasurementMode)
     || (m_OperationMode == highSpeedMeasurementMode)
     || (m_OperationMode == ultraLowPowerMeasurementMode))
    {
        // \"
        // 2.2.1.1 Continuous measurement modes
        //
        // In continuous measurement mode the output data is refreshed after
        // each measurement and the availability of the updated pressure and
        // temperature data is signaled through the assertion of the DRDY 
        // pin and a DRDY bit is set to ‘1’ in the STATUS register. \"
        
        // \" 
        // 2.2.3 Reading the pressure and temperature
        //
        // After the DRDY pin has signaled the availability of new measurement
        // data, it is recommended that the output data is read immediately
        // in the following order:
        //
        // o read the TEMPOUT register (temperature data in bits [13:0] – in
        // case the temperature data is not needed this step can be omitted).
        //
        // o read the DATARD8 register (bits [2:0] contain the MSB of the 
        //   pressure data)
        //
        // o read the DATARD16 register (contains the 16 LSB of the pressure
        //   data) \"
        
        // \" 2.4 DRDY – data ready pin
        //
        // Availability of updated pressure and temperature data is signaled
        // through the assertion of the DRDY pin. DRDY is cleared after the
        // DATARD16 register is read, see section 2.2.1 for more detailed
        // information. \"

        // Always read temperature first and then pressure sensor data so as
        // to clear the DRDY signal pin:
        printf("On-chip temperature sensor:\n\t-> %s °C\n", 
            TruncateAndToString<double>(GetTemperature<Celsius_t>()).c_str());
        
        printf("On-chip temperature sensor:\n\t-> %s °F\n", 
            TruncateAndToString<double>(GetTemperature<Fahrenheit_t>()).c_str());
        
        printf("On-chip temperature sensor:\n\t-> %s K\n",  
            TruncateAndToString<double>(GetTemperature<Kelvin_t>()).c_str());
            
        printf("Barometric pressure:\n\t->%s Pa\n\n", 
            TruncateAndToString<double>(GetPressure()).c_str());
    }
}
