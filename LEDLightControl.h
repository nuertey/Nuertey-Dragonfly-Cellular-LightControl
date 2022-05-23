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
#include "mbed_assert.h"
#include "mbed_events.h"
#include "randLIB.h"

//#include "common_functions.h"
#include "CellularNonIPSocket.h"
#include "UDPSocket.h"
#include "TCPSocket.h"
#include "CellularDevice.h"
#include "cellular_demo_tracing.h"

#include "Utilities.h"

// TBD Nuertey Odzeyem; confirm if the below holds for both 
// MTS_DRAGONFLY_L471QG and the NUCLEO_F767ZI targets:
#define LED_ON  1
#define LED_OFF 0

// Choice of socket type configuration parsed from mbed_app.json:
#define UDP 0
#define TCP 1
#define NONIP 2

enum class TransportScheme_t : uint8_t
{
    CELLULAR_4G_LTE,          // Primary usecase for MTS_DRAGONFLY_L471QG target (LTE Cat M1 Cellular).
    ETHERNET,                 // To potentially allow for debug testing on my available NUCLEO_F767ZI target.  
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
// Target = NUCLEO_F767ZI: Green LED
DigitalOut g_UserLED(LED1);

// Protect the platform STDIO object so it is shared politely between 
// threads, periodic events and periodic callbacks (not hopefully in IRQ context
// but when safely translated into, say, an EventQueue context). Essentially,
// ensure our output does not come out garbled on the serial terminal.
PlatformMutex g_STDIOMutex; 

// Do NOT use std::unique_ptr<> as we must NOT delete the shared
// event queue pointer (a singleton) at any time.       
EventQueue * g_pSharedEventQueue;

class LEDLightControl
{   
    static constexpr uint8_t  MAXIMUM_NUMBER_OF_CONNECT_RETRIES{3};
    
    static constexpr auto     STARTUP_STATUS_CHECK_DELAY    {10ms};
    static constexpr auto     CHANGE_MEASUREMENT_MODE_DELAY {50ms};
    
public:
    LEDLightControl();

    LEDLightControl(const LEDLightControl&) = delete;
    LEDLightControl& operator=(const LEDLightControl&) = delete;

    virtual ~LEDLightControl();

    template <TransportScheme_t transport, TransportSocket_t socket>
        requires IsValidTransportType<transport, socket>
    void Setup();
    
    void NetworkStatusCallback(nsapi_event_t status, intptr_t param);
    
    void ConnectToSocket();
        
    void ChangeOperationMode(const IsOperationModeType& mode);

protected:
    template <TransportScheme_t transport, TransportSocket_t socket>
        requires IsValidTransportType<transport, socket>
    void ConnectToNetworkInterface();

    void Run();
    [[nodiscard]] bool Send();
    [[nodiscard]] bool Receive();
    
private:
    TransportScheme_t              m_TheTransportSchemeType;
    TransportSocket_t              m_TheTransportSocketType;
    NetworkInterface *             m_pNetworkInterface;
    CellularDevice *               m_pTheCellularDevice;
    std::optional<std::string>     m_EchoServerDomainName; // Domain name might not always necessarily exist...
    std::string                    m_EchoServerAddress;    // However IP Address always would.
    uint16_t                       m_EchoServerPort;
    IsOperationModeType            m_OperationMode;
    bool                           m_IsConnected;
    
    // Conditional class members based upon configuration parsed from mbed_app.json:
    #if MBED_CONF_APP_SOCK_TYPE == TCP
        TCPSocket                  m_TheSocket;
        SocketAddress              m_TheSocketAddress;
    #elif MBED_CONF_APP_SOCK_TYPE == UDP
        UDPSocket                  m_TheSocket;
        SocketAddress              m_TheSocketAddress;
    #elif MBED_CONF_APP_SOCK_TYPE == NONIP
        // Send and receive 3GPP non-IP datagrams (NIDD) using the cellular IoT feature.
        CellularNonIPSocket        m_TheSocket;
    #endif
};

// Note that:
//
// m_TheTransportSchemeType is deliberately 'self-initialized' by its enum composition.
// m_TheTransportSocketType is deliberately 'self-initialized' by its enum composition.
// m_pNetworkInterface is deliberately 'self-initialized' by its requisite default constructor.
// m_pTheCellularDevice is deliberately 'self-initialized' by its requisite default constructor.
LEDLightControl::LEDLightControl()
    : m_EchoServerDomainName(std::nullopt)
    , m_EchoServerAddress(ECHO_HOSTNAME)
    , m_EchoServerPort(ECHO_PORT) 
    , m_OperationMode(standbyMode)
    , m_IsConnected(false)
    // Deliberately letting the conditional class member variables 'self-initialize'
    // by default-constructing. 
{
}

LEDLightControl::~LEDLightControl()
{
}

template <TransportScheme_t transport, TransportSocket_t socket>
    requires IsValidTransportType<transport, socket>
void LEDLightControl::Setup()
{
    randLIB_seed_random();
    
    if constexpr (transport == TransportScheme_t::CELLULAR_4G_LTE) 
    {
        // "Non-IP cellular socket: Send and receive 3GPP non-IP datagrams (NIDD)
        // using the cellular IoT feature."
        //
        // https://os.mbed.com/docs/mbed-os/v6.15/apis/connectivity.html
        //
        // https://os.mbed.com/docs/mbed-os/v6.15/apis/network-interface-apis.html
        if constexpr (socket == TransportSocket_t::CELLULAR_NON_IP) 
        {
            m_pNetworkInterface = CellularContext::get_default_nonip_instance();
        }
        else
        {
            m_pNetworkInterface = CellularContext::get_default_instance();
        }
        
        // SIM PIN, APN, credentials and possible PLMN are extracted automagically
        // from the mbed_app.json when using NetworkInterface::set_default_parameters():
        m_pNetworkInterface->set_default_parameters();
        
        // Runtime Mbed OS assertion as opposed to the compile-time MBED_STATIC_ASSERT
        // assertion as below:
        //
        // MBED_STATIC_ASSERT(sizeof(equeue_timer) >= sizeof(Timer),"The equeue_timer buffer must fit the class Timer");
        //
        // Note that the MBED_ASSERT macro is only available in the Debug 
        // and Development build profiles and not in the Release build profile. 
        MBED_ASSERT(m_pNetworkInterface);

        m_pTheCellularDevice = CellularDevice::get_target_default_instance();
        MBED_ASSERT(m_pTheCellularDevice);
    }
    else if constexpr (transport == TransportScheme_t::ETHERNET)
    {
        m_pNetworkInterface = NetworkInterface::get_default_instance();
        MBED_ASSERT(m_pNetworkInterface);
    }
    else
    {
        // static_assert(false, "...");
        //
        // ... if had been rather used for the below would have NOT compiled here.
        // This particular nuance is documented at this link:
        //
        // https://devblogs.microsoft.com/oldnewthing/20200311-00/?p=103553
        
        // Mesh Network branch deliberately unimplemented as it is out of scope.
        //
        // m_pNetworkInterface = MeshInterface::get_default_instance();
         static_assert(((transport == TransportScheme_t::CELLULAR_4G_LTE) || (transport == TransportScheme_t::ETHERNET)), 
             "Hey! Mesh Network branch DELIBERATELY unimplemented as it is out of scope!!!");
    }
    
    // Asynchronously monitor for Network Status events:
    m_pNetworkInterface->attach(callback(this, &LEDLightControl::NetworkStatusCallback));  // callback using member function
    
    // "Attach to network so we can get update status from the network"
    // _nw->attach(callback(this, &CellularDevice::cellular_callback));
    
    // Apropos "gedit ./mbed-os/connectivity/cellular/include/cellular/framework/API/CellularDevice.h":
    //
    // 
    // /** Cellular callback to be attached to Network and CellularStateMachine classes.
    //  *  CellularContext calls this when in PPP mode to provide network changes.
    //  *  This method will broadcast to every interested classes:
    //  *  CellularContext (might be many) and CellularStateMachine if available.
    //  */
    // virtual void cellular_callback(nsapi_event_t ev, intptr_t ptr, CellularContext *ctx = NULL);
    
    // TBD Nuertey Odzeyem; remove all superfluous comments and clean up
    // the implementation once class is tested and proven to be working
    // like the below:
    
    // LEDLightControl::ConnectToNetworkInterface() should happen here 
    // and should be called by the class itself hence private or protected member:
    ConnectToNetworkInterface<transport, socket>();
}

template <TransportScheme_t transport, TransportSocket_t socket>
    requires IsValidTransportType<transport, socket>
void LEDLightControl::ConnectToNetworkInterface()
{
    // At this juncture, save and encapsulate template parameter variables
    // within the class itself for later ::NetworkStatusCallbacks() to operate on.
    m_TheTransportSchemeType = transport;
    m_TheTransportSocketType = socket;    

    // "Asynchronous operation
    // 
    // NetworkInterface::connect() and NetworkInterface::disconnect() are blocking
    // by default. When an application prefers asynchronous operation, it can set
    // the interface into nonblocking mode by calling NetworkInterface::set_blocking(false).
    // This has to be done for each interface separately.
    // 
    // When an interface operates in asynchronous mode, the return values of connect()
    // and disconnect() have slightly different meanings. Calling connect() starts the
    // asynchronous operation, which puts the device in the GLOBAL_UP state. Calling
    // disconnect() puts the target in the DISCONNECTED state. Return code in 
    // asynchronous mode does not reflect the connection status. The most common 
    // return codes in asynchronous mode is NSAPI_ERROR_OK, which means that operation
    // just started. Please refer to the Doxygen documentation of NetworkInterface::connect()
    // and NetworkInterface::disconnect() for return values of these functions.
    // 
    // To check whether the interface is connected, the application needs to register 
    // the status callback for the interface. Please refer to the Network status API
    // for information on how to do so."
    //
    // https://os.mbed.com/docs/mbed-os/v6.15/apis/network-interface.html
    m_pNetworkInterface->set_blocking(false);
    [[maybe_unused]] auto asynchronous_connect_return_perhaps_can_be_safely_ignored \
                                           = m_pNetworkInterface->connect();
      
    // Setup complete, so we can now dispatch the shared event queue forever:
    
    // We will never return from the call below, as events are executed by 
    // the dispatch_forever method.
    if (g_pSharedEventQueue)
    {
        g_pSharedEventQueue->dispatch_forever();
    }
    else
    {
        assert(((void)"Such a thing ought to never happen but unfortunately (whom knows why?), g_pSharedEventQueue is nullptr!!", \
            (g_pSharedEventQueue != nullptr)));
    }
}

void LEDLightControl::NetworkStatusCallback(nsapi_event_t statusEvent, intptr_t parameterPointerData)
{
    // TBD Nuertey Odzeyem; verify with testing whether this assertion 
    // is needed, and if it is here, will it negatively affect the "workings"
    // of the Cellular network.
    //assert(statusEvent == NSAPI_EVENT_CONNECTION_STATUS_CHANGE);

    g_STDIOMutex.lock();
    printf("Network Connection status changed!\r\n");

    switch (parameterPointerData)
    {
        case NSAPI_STATUS_LOCAL_UP:
            printf("Local IP address set!\r\n");
            g_STDIOMutex.unlock();
            break;
        case NSAPI_STATUS_GLOBAL_UP:
            printf("Global IP address set!\r\n");
            m_IsConnected = true;
            g_STDIOMutex.unlock();
            
            // Post the asynchronously notified network status change on the shared event
            // queue so that its actions can be scheduled and complete in synchronous
            // thread mode instead of in interrupt (i.e. callback) mode.
            auto event1 = make_user_allocated_event(this, &LEDLightControl::ConnectToSocket);
        
            // bind & post
            event1.call_on(g_pSharedEventQueue);
            
            // Note that the EventQueue has no concept of event priority. 
            // If you schedule events to run at the same time, the order in
            // which the events run relative to one another is undefined. 
            // The EventQueue only schedules events based on time.
            
            break;
        case NSAPI_STATUS_DISCONNECTED:
            printf("NetworkInterface disconnected!\r\n");
            m_IsConnected = false;
            g_STDIOMutex.unlock();
            
            tr_debug("Network Status Event Callback: %d, \t\r\nparameterPointerData: %d", \
                statusEvent, parameterPointerData);
                
            if (statusEvent == NSAPI_EVENT_CONNECTION_STATUS_CHANGE) //&& parameterPointerData == NSAPI_STATUS_DISCONNECTED)
            {
                // we have been disconnected, reset state machine so that application can start connect sequence again
                // if (_state_machine)
                // {
                //  _state_machine->reset();
                // }
            }
            break;
        case NSAPI_STATUS_CONNECTING:
            printf("Connecting to network!\r\n");
            g_STDIOMutex.unlock();
            break;
        default:
            printf("Perhaps New Cellular Pointer Data Codes Have Asynchronously Arrived:\r\n");
            g_STDIOMutex.unlock();
            
            if (statusEvent >= NSAPI_EVENT_CELLULAR_STATUS_BASE && statusEvent <= NSAPI_EVENT_CELLULAR_STATUS_END)
            {
                cell_callback_data_t *ptr_data = static_cast<cell_callback_data_t *>(parameterPointerData);
                
                tr_debug("Network Status Event Callback: %d, \t\r\nptr_data->error: %d, \t\r\nptr_data->status_data: %d", \
                    statusEvent, ptr_data->error, ptr_data->status_data);
                
                cellular_connection_status_t cellEvent = static_cast<cellular_connection_status_t>(statusEvent);
                
                if (cellEvent == CellularRegistrationStatusChanged)
                {
                    // broadcast only network registration changes to state machine
                    //_state_machine->cellular_event_changed(ev, ptr);
                }

                if (cellEvent == CellularDeviceReady && ptr_data->error == NSAPI_ERROR_OK)
                {
                    // // Here we can create mux and give new filehandles as mux reserves the one what was in use.
                    // // if mux we would need to set new filehandle:_state_machine->set_filehandle( get fh from mux);
                    // _nw = open_network(_fh);
                    // // Attach to network so we can get update status from the network
                    // _nw->attach(callback(this, &CellularDevice::cellular_callback));
                    // if (strlen(_plmn))
                    // {
                    //     _state_machine->set_plmn(_plmn);
                    // }

                    // TBD Nuertey Odzeyem; should it be like this in this
                    // state of the Cellular state machine? 
                    // Confirm with testing...:
                    g_STDIOMutex.lock();
                    m_IsConnected = true;
                    g_STDIOMutex.unlock();

                    // Post the asynchronously notified network status change on the shared event
                    // queue so that its actions can be scheduled and complete in synchronous
                    // thread mode instead of in interrupt (callback) mode.
                    auto event1 = make_user_allocated_event(this, &LEDLightControl::ConnectToSocket);
                
                    // bind & post
                    event1.call_on(g_pSharedEventQueue);
                    
                    // Note that the EventQueue has no concept of event priority. 
                    // If you schedule events to run at the same time, the order in
                    // which the events run relative to one another is undefined. 
                    // The EventQueue only schedules events based on time.
                }
                else if (cellEvent == CellularSIMStatusChanged && ptr_data->error == NSAPI_ERROR_OK &&
                         ptr_data->status_data == CellularSIM::SimStatePinNeeded)
                {
                    // if (strlen(_sim_pin))
                    // {
                    //     _state_machine->set_sim_pin(_sim_pin);
                    // }
                }
            }
            break;
    }
}

void LEDLightControl::ConnectToSocket()
{
    // Show the particular NetworkInterface addresses to encourage Debug. 
    // Don't forget that this class object is being designed to handle 
    // several NetworkInterfaces--primarily Cellular, yes?, but also Ethernet
    // to aid debug and testing, and can even be extended in the future 
    // for Mesh Networks... as already documented in the above preliminaries: 
    const char *ip = m_pNetworkInterface->get_ip_address();
    const char *netmask = m_pNetworkInterface->get_netmask();
    const char *gateway = m_pNetworkInterface->get_gateway();
    const char *mac = m_pNetworkInterface->get_mac_address();
    
    printf("Particular Network Interface IP address: %s\n", ip ? ip : "None");
    printf("Particular Network Interface Netmask: %s\n", netmask ? netmask : "None");
    printf("Particular Network Interface Gateway: %s\n", gateway ? gateway : "None");
    printf("Particular Network Interface MAC Address: %s\n", mac ? mac : "None");
    
    // Opens:
    // - UDP or TCP socket with the given echo server and performs an echo
    //   transaction retrieving current message.
    //
    // - Cellular Non-IP socket for which the data delivery path is decided
    //   by network's control plane CIoT optimisation setup, for the given APN.
    if ((m_TheTransportSocketType == TransportSocket_t::TCP)
     || (m_TheTransportSocketType == TransportSocket_t::UDP))
    {
        nsapi_error_t rc = m_TheSocket.open(m_pNetworkInterface);
        if (rc != NSAPI_ERROR_OK)
        {
            printf("\r\n\r\nError! TCPSocket or UDPSocket.open() returned: \
                [%d] -> %s\r\n", rc, ToString(rc).c_str());
                
            return;
        }
    }
    else if (m_TheTransportSocketType == TransportSocket_t::CELLULAR_NON_IP)
    {
        nsapi_error_t rc = m_TheSocket.open(static_cast<CellularContext*>(m_pNetworkInterface));
        if (rc != NSAPI_ERROR_OK)
        {
            printf("\r\n\r\nError! CellularNonIPSocket.open() returned: \
                [%d] -> %s\r\n", rc, ToString(rc).c_str());
                
            return;
        }
    }  
    
      
    
    // TBD Nuertey Odzeyem; resolve hostname here??? ...
    auto [ipAddress, domainName] = Utility::ResolveAddressIfDomainName(m_EchoServerAddress, m_pNetworkInterface);
    m_EchoServerAddress = ipAddress;
    
    if (domainName)
    {
        m_EchoServerDomainName = std::move(domainName);
    }

    printf("\r\nConnecting to : \"%s:%d\" ...", m_EchoServerAddress.c_str(), m_EchoServerPort);
    // nsapi_error_t rc = m_pNetworkInterface.connect();
    // 
    // if (rc != NSAPI_ERROR_OK)
    // {
    //     printf("\r\n\r\nError! NetworkInterface.connect() to EchoServer returned: [%d] -> %s\n", rc, ToString(rc).c_str());
    // }
    // else
    // {   
    //     if constexpr (transport == TransportScheme_t::CELLULAR_4G_LTE) 
    //     {
    // 
    //     }
    //     else if constexpr (transport == TransportScheme_t::ETHERNET)
    //     {
    // 
    //     }
    //     else
    //     {
    //         // Mesh Network branch deliberately unimplemented as it is out of scope.
    //     }
    // }
    
    // "Open a socket on the network interface, and create a TCP connection to mbed.org..."
    // // TBD Nuertey Odzeyem; perform socket.open() and connect() here???? Do so if it seems logical....
    // 
    // 
    
    // /**
    //  * For UDP or TCP it opens a socket with the given echo server and performs an echo transaction.
    //  * For Cellular Non-IP it opens a socket for which the data delivery path is decided
    //  * by network's control plane CIoT optimisation setup, for the given APN.
    //  */
    // bool test_send_and_receive()
    
    // TBD Nuertey Odzeyem; call non-returning function ::Run() here to do the
    // actual sending and receiving if m_IsConnected is true, and forget
    // not to actual exit and drop out of ::Run() if m_IsConnected is false.
    // Subsequent NetworkStatusCallbacks() will handle dispatching the
    // ConnectToSocket() event again.
}

void LEDLightControl::Run()
{
    while (true)
    {
        if (Send())
        {
            if (Receive())
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
