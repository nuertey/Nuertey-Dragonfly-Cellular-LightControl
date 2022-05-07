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
    // \" The MSB of the words are sent first. \"     
    static constexpr uint8_t  DEFAULT_BYTE_ORDER               {0}; // A value of zero indicates MSB-first.
    static constexpr uint32_t DEFAULT_FREQUENCY           {500000}; // \" SPI max 500 kHz. \"
    static constexpr uint8_t  MAXIMUM_NUMBER_OF_STARTUP_RETRIES{6};
    static constexpr auto     STARTUP_STATUS_CHECK_DELAY    {10ms};
    static constexpr auto     CHANGE_MEASUREMENT_MODE_DELAY {50ms};
    
    static constexpr uint8_t RETRY_COUNT{3};
    
public:
    LEDLightControl(NetworkInterface * pInterface);

    LEDLightControl(const LEDLightControl&) = delete;
    LEDLightControl& operator=(const LEDLightControl&) = delete;

    virtual ~LEDLightControl();

    [[nodiscard]] bool Connect();
    std::error_code Send();
    std::error_code Receive();
        
    void ChangeOperationMode(const IsOperationModeType& mode);
    
    void InitiatePeriodicReadings();
    
    // Trigger it on sensor interrupt/event queue
    void AcquireSensorData();
    
    // TBD, Nuertey Odzeyem; when????
    void LowPowerAcquireSensorData();
    
    double GetPressure();
        
    template <IsTemperatureScaleType T>
    double GetTemperature();
    
    template <IsDirectRegisterAccess R>
    void WriteToDirectRegister(const uint8_t& value);

    template <IsIndirectRegisterAccess R>
    void WriteToIndirectRegister(const uint8_t& value);
    
    template <IsEEPROMRegisterAccess R>
    void WriteToEEPROMRegister(const uint8_t& value);
    
    // User must specify expected response SPI frame byte width and 
    // expected class object method return type:
    template <IsDirectRegisterAccess R, std::size_t M, typename Return_t = uint8_t>
    Return_t ReadFromDirectRegister();

    template <IsIndirectRegisterAccess R>
    uint8_t ReadFromIndirectRegister();
    
    template <IsEEPROMRegisterAccess R>
    uint8_t ReadFromEEPROMRegister();

protected:
    template <std::size_t N, std::size_t M>
    bool FullDuplexTransfer(const SPIFrame_t<N>& cBuffer, SPIFrame_t<M>& rBuffer);
    
    double ConvertPressure(const uint32_t& sensorData) const;
    
    double ConvertTemperature(const int16_t& sensorData) const;    
    
    template <IsTemperatureScaleType T>
    double ConvertTemperature(const int16_t& sensorData) const;
    
private:               
    SPI                                m_TheSPIBus;
    uint8_t                            m_Mode;
    uint8_t                            m_ByteOrder;
    uint8_t                            m_BitsPerWord;
    uint32_t                           m_Frequency;
    IsOperationModeType                m_OperationMode;
    bool                               m_PoweredDownMode;
};

LEDLightControl::LEDLightControl(NetworkInterface * pInterface)
    : m_TheSPIBus(mosi, miso, sclk, ssel, mbed::use_gpio_ssel)
    , m_Mode(mode)
    , m_ByteOrder(byteOrder)
    , m_BitsPerWord(bitsPerWord)
    , m_Frequency(frequency)
    , m_OperationMode(standbyMode)
    , m_PoweredDownMode(false)
{
    // The device runs in SPI mode 0, which requires the clock 
    // line SCLK to idle low (CPOL = 0), and for data to be sampled on
    // the leading clock edge (CPHA = 0).
    
    // \" Bits from MOSI line are sampled in on the rising edge of SCK
    // and bits to MISO line are latched out on falling edge of SCK. \"

    // By default, the SPI bus is configured at the Mbed layer with 
    // format set to 8-bits, mode 0, and a clock frequency of 1MHz.
    m_TheSPIBus.format(m_BitsPerWord, m_Mode);
    m_TheSPIBus.frequency(m_Frequency);
}

LEDLightControl::~LEDLightControl()
{
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

void LEDLightControl::LowPowerAcquireSensorData()
{
    // \" In low power mode SCP1000 measures pressure and temperature
    // once after the measurement is triggered. \"
    if (m_OperationMode == lowPowerMeasurementWithExternalTriggerMode)
    {
    }
}

template <std::size_t N, std::size_t M>
bool LEDLightControl::FullDuplexTransfer(const SPIFrame_t<N>& cBuffer,
                                                    SPIFrame_t<M>& rBuffer)
{   
    bool result{true};
    
    // Do not presume that the users of this OS-abstraction are well-behaved.
    rBuffer.fill(0);

    // Assert the Slave Select line, acquiring exclusive access to the
    // SPI bus. Chip select is active low hence cs = 0 here. Note that
    // write already internally mutex locks and selects the SPI bus.
    //m_TheSPIBus.select();
    
    // Write to the SPI Slave and obtain the response.
    //
    // The total number of bytes sent and received will be the maximum
    // of tx_length and rx_length. The bytes written will be padded with
    // the value 0xff. Further note that the number of bytes to either
    // write or read, may be zero, without raising any exceptions.
    std::size_t bytesWritten = m_TheSPIBus.write(reinterpret_cast<const char*>(cBuffer.data()),
                                                 cBuffer.size(),
                                                 reinterpret_cast<char*>(rBuffer.data()), 
                                                 rBuffer.size());
    
    // Deassert the Slave Select line, releasing exclusive access to the
    // SPI bus. Chip select is active low hence cs = 1 here.  Note that
    // write already internally deselects and mutex unlocks the SPI bus.
    //m_TheSPIBus.deselect();   
    
    if (bytesWritten != std::max(cBuffer.size(), rBuffer.size()))
    {
        printf("%s: Error! SPI Command Frame - Incorrect number of bytes \
            transmitted\n",
            __PRETTY_FUNCTION__);
        result = false;
        
        // Enabled to facilitate runtime debugging:
        DisplayFrame(cBuffer);
        DisplayFrame(rBuffer);
    } 
    
    return result;
}

template <IsDirectRegisterAccess R>
void LEDLightControl::WriteToDirectRegister(const uint8_t& value)
{   
    // \" Each SPI communication frame contains two or three 8 bit words:
    // the first word defines the register address (6 bits wide, bits
    // [A5:A0] in Figure 9) followed by the type of access (‘0’ = Read 
    // or ‘1’ = Write) and one zero bit (bit 0, LSB). \"
    uint8_t firstWord = (R << 2) | RegisterAccess_t::SPI_WRITE;
    
    SPIFrame_t commandFrame{firstWord, value};
    SPIFrame_t dummyResponse = {}; // Initialize to zeros.

    // \" The CSB line must stay low during the entire frame accesses, 
    // i.e. between the bytes. If the CSB line state changes to high, 
    // the access is terminated. The CSB has to be pulled up after each
    // communication frame. \"    
    auto status = FullDuplexTransfer<2, 2>(commandFrame, dummyResponse);
    
    if (!status)
    {
        printf("[%s]: Error! Failed to write direct register value. \
            \n\tRegister Address := [%s]\n\tValue := [%s]\n",
            __PRETTY_FUNCTION__, 
            IntegerToHex(R).c_str(), 
            IntegerToHex(value).c_str());
    }
}

template <IsIndirectRegisterAccess R>
void LEDLightControl::WriteToIndirectRegister(const uint8_t& value)
{
    // Write the indirect register address into direct register ADDPTR (0x02).
    WriteToDirectRegister<RegisterAddress_t::INDIRECT_REGISTER_ACCESS_ADDRESS_POINTER>(R);
    
    // Write the value into direct register DATAWR (0x01).
    WriteToDirectRegister<RegisterAddress_t::INDIRECT_REGISTER_ACCESS_DATA>(value);
    
    // Write 0x02 into direct register OPERATION (0x03). Essentially 
    // effecting:
    //
    // \" Write DATAWR contents into the indirect access register 
    // pointed to by ADDPTR. \"
    WriteToDirectRegister<RegisterAddress_t::OPERATION>(Operation_t::WRITE_INDIRECT_REGISTER_ACCESS_DATA);
    
    // \" Wait 50 ms. \"
    ThisThread::sleep_for(50ms);
}

template <IsEEPROMRegisterAccess R>
void LEDLightControl::WriteToEEPROMRegister(const uint8_t& value)
{
    // \" 3.4.1 EEPROM writing
    //
    // Please note that in order to guarantee reliable EEPROM writing
    // operation it is very critical to follow the requirements below.
    // The minimum value for supply voltage at +25 °C temperature is
    // 3.0 V during EEPROM register write operation. The peak current
    // consumption is also significantly higher than in normal operation
    // (~2 mA for 15 ms period per byte). \"
    
    // Write the EEPROM register address into direct register ADDPTR (0x02).
    WriteToDirectRegister<RegisterAddress_t::INDIRECT_REGISTER_ACCESS_ADDRESS_POINTER>(R);
    
    // Write the value into direct register DATAWR (0x01).
    WriteToDirectRegister<RegisterAddress_t::INDIRECT_REGISTER_ACCESS_DATA>(value);
    
    // Write 0x06 into direct register OPERATION (0x03). Essentially 
    // effecting:
    //
    // \" Write DATAWR contents into the EEPROM register pointed by
    // ADDPTR. \"
    WriteToDirectRegister<RegisterAddress_t::OPERATION>(Operation_t::WRITE_EEPROM_REGISTER_DATA);
    
    // \" Wait 50 ms. \"
    ThisThread::sleep_for(50ms);
}

template <IsDirectRegisterAccess R, std::size_t M, typename Return_t = uint8_t>
Return_t LEDLightControl::ReadFromDirectRegister()
{
    Return_t result{0};
           
    // \" Each SPI communication frame contains two or three 8 bit words:
    // the first word defines the register address (6 bits wide, bits
    // [A5:A0] in Figure 9) followed by the type of access (‘0’ = Read 
    // or ‘1’ = Write) and one zero bit (bit 0, LSB). \"
    uint8_t firstWord = (R << 2) | RegisterAccess_t::SPI_READ;
    
    SPIFrame_t<1> commandFrame{firstWord};
    SPIFrame_t<M> responseFrame = {}; // Initialize to zeros.

    // \" The CSB line must stay low during the entire frame accesses, 
    // i.e. between the bytes. If the CSB line state changes to high, 
    // the access is terminated. The CSB has to be pulled up after each
    // communication frame. \"    
    auto status = FullDuplexTransfer<1, M>(commandFrame, responseFrame);
    
    if (!status)
    {
        printf("[%s]: Error! Failed to read from direct register address. \
            \n\tRegister Address := [%s]\n\tM := [%s]\n",
            __PRETTY_FUNCTION__, 
            IntegerToHex(R).c_str(), 
            IntegerToDec(M).c_str());
    }
    
    if constexpr (std::is_same_v<Return_t, uint8_t>)
    {
        if constexpr (M == 1)
        {
            // \" ... the register content is in bits [7:0], see section 3.2
            // for further information. \"
            result = responseFrame.at(0);
        }
        else if constexpr (M == 2)
        {
            // \" ... bits [15:8] should be treated as zeros, the register 
            // content is in bits [7:0], see section 3.2 for further information. \"
            result = responseFrame.at(1);
        }
    }
    else if constexpr (std::is_same_v<Return_t, uint16_t>)
    {
        result = Deserialize(responseFrame);
    }
    
    return result;
}
