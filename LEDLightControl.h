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
//#include "mbed_events.h"
#include "mbed_trace.h"
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

enum class MCUTarget_t : uint8_t
{
    // Primary usecase:
    MTS_DRAGONFLY_L471QG,
    
    // To allow for potential debug testing
    // on the only MCU that I do have available:
    NUCLEO_F767ZI
};

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

static constexpr char ECHO_HOSTNAME[] = MBED_CONF_APP_ECHO_SERVER_HOSTNAME;
static constexpr int ECHO_PORT = MBED_CONF_APP_ECHO_SERVER_PORT; // Same value holds for TCP and UDP.

using namespace std::chrono_literals;

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
bool       g_UserLEDState{false};  // Logically, the board will bootup with the LED off.

class LEDLightControl
{   
    // 1 minute of failing to exchange packets with the EchoServer ought
    // to be enough to tell us that there is something wrong with the socket.
    static constexpr int32_t BLOCKING_SOCKET_TIMEOUT_MILLISECONDS{60000};
    static constexpr uint8_t MASTER_LIGHT_CONTROL_GROUP{0};
    static constexpr uint8_t     MY_LIGHT_CONTROL_GROUP{1};
    static constexpr uint32_t STANDARD_BUFFER_SIZE{40}; // 1K ought to cover all our cases.
    
public:
    LEDLightControl();

    LEDLightControl(const LEDLightControl&) = delete;
    LEDLightControl& operator=(const LEDLightControl&) = delete;

    virtual ~LEDLightControl();

    template <TransportScheme_t transport, TransportSocket_t socket>
        requires IsValidTransportType<transport, socket>
    void Setup();
    
    void ConnectToSocket();

protected:
    template <TransportScheme_t transport, TransportSocket_t socket>
        requires IsValidTransportType<transport, socket>
    void ConnectToNetworkInterface();

    void Run();
    
    [[nodiscard]] bool Send();
    [[nodiscard]] bool Receive();
    
    bool ParseAndConsumeLightControlMessage(std::string& s, const std::string& delimiter);
    
private:
    TransportScheme_t          m_TheTransportSchemeType;
    TransportSocket_t          m_TheTransportSocketType;
    NetworkInterface *         m_pNetworkInterface;
    
    // To enable soft_power_off/on(), shutdown(), hard_power_on/off(), and such functions.
    CellularDevice *           m_pTheCellularDevice;   
    
    std::string                m_EchoServerDomainName; // Domain name will always exist.
    std::optional<std::string> m_EchoServerAddress;    // However IP Address might not always exist...
    uint16_t                   m_EchoServerPort;
    
    // Portable Socket class interface for handling all the 3 possible
    // socket types. Ergo:
    //
    // TCP - Connection-oriented IP.
    // UDP - Connection-less IP.
    // CellularNonIP - 3GPP non-IP datagrams (NIDD) using the cellular IoT feature.
#if MBED_CONF_APP_SOCK_TYPE == TCP
    TCPSocket                  m_TheSocket;
#elif MBED_CONF_APP_SOCK_TYPE == UDP
    UDPSocket                  m_TheSocket;
#elif MBED_CONF_APP_SOCK_TYPE == NONIP
    CellularNonIPSocket        m_TheSocket;
#endif
    SocketAddress              m_TheSocketAddress;
};

LEDLightControl::LEDLightControl()
    : m_EchoServerDomainName(ECHO_HOSTNAME)
    , m_EchoServerAddress(std::nullopt)
    , m_EchoServerPort(ECHO_PORT) 
{
}

LEDLightControl::~LEDLightControl()
{
    // Proper housekeeping...
    if (m_pNetworkInterface)
    {
        [[maybe_unused]] auto unused_return_2 = m_pNetworkInterface->disconnect();
    }
    
    trace_close();
}

template <TransportScheme_t transport, TransportSocket_t socket>
    requires IsValidTransportType<transport, socket>
void LEDLightControl::Setup()
{
    printf("Running LEDLightControl::Setup() ... \r\n");
    
    randLIB_seed_random();
    trace_open();
    
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

    auto rc = m_pNetworkInterface->connect();

    if (rc != NSAPI_ERROR_OK)
    {
        printf("Error! NetworkInterface->connect() returned: \
            [%d] -> %s\r\n", rc, ToString(rc).c_str());
               
        return;
    }      
    
    ConnectToSocket();
}

void LEDLightControl::ConnectToSocket()
{    
    printf("Running LEDLightControl::ConnectToSocket() ... \r\n");
    
    // Show the particular NetworkInterface addresses to encourage Debug. 
    // Don't forget that this class object is being designed to handle 
    // several NetworkInterfaces--primarily Cellular, yes?, but also Ethernet
    // to aid debug and testing, and can even be extended in the future 
    // for Mesh Networks... as already documented in the above preliminaries: 
    auto [ip, netmask, gateway, mac] = Utilities::GetNetworkInterfaceProfile(m_pNetworkInterface);
    
    printf("Particular Network Interface IP address: %s\n", ip.value_or("(null)"));
    printf("Particular Network Interface Netmask: %s\n", netmask.value_or("(null)"));
    printf("Particular Network Interface Gateway: %s\n", gateway.value_or("(null)"));
    printf("Particular Network Interface MAC Address: %s\n", mac.value_or("(null)"));
        
    // Opens:
    // - UDP or TCP socket with the given echo server and performs an echo
    //   transaction retrieving current message.
    //
    // - Cellular Non-IP socket for which the data delivery path is decided
    //   by network's control plane CIoT optimisation setup, for the given APN.
#if MBED_CONF_APP_SOCK_TYPE == TCP        
    nsapi_error_t rc = m_TheSocket.open(m_pNetworkInterface);
    if (rc != NSAPI_ERROR_OK)
    {
        printf("Error! TCPSocket.open() returned: \
            [%d] -> %s\r\n", rc, ToString(rc).c_str());

        // Abandon attempting to connect to the socket. 
        return;
    }
#elif MBED_CONF_APP_SOCK_TYPE == UDP    
    nsapi_error_t rc = m_TheSocket.open(m_pNetworkInterface);
    if (rc != NSAPI_ERROR_OK)
    {
        printf("Error! UDPSocket.open() returned: \
            [%d] -> %s\r\n", rc, ToString(rc).c_str());

        // Abandon attempting to connect to the socket.                
        return;
    }
#elif MBED_CONF_APP_SOCK_TYPE == NONIP        
    nsapi_error_t rc = m_TheSocket.open(m_pNetworkInterface);
    if (rc != NSAPI_ERROR_OK)
    {
        printf("Error! CellularNonIPSocket.open() returned: \
            [%d] -> %s\r\n", rc, ToString(rc).c_str());

        // Abandon attempting to connect to the socket.   
        return;
    }
#endif  
    
    // Set timeout on blocking socket operations.
    //
    // Initially all sockets have unbounded timeouts. NSAPI_ERROR_WOULD_BLOCK
    // is returned if a blocking operation takes longer than the specified timeout.
    //
    // Also, extrapolate from the following rule:
    //
    // "If using network sockets as streams, a timeout should be set to 
    //  stop denial of service attacks."
    m_TheSocket.set_blocking(true);
    m_TheSocket.set_timeout(BLOCKING_SOCKET_TIMEOUT_MILLISECONDS);
    
    if (m_TheTransportSocketType != TransportSocket_t::CELLULAR_NON_IP)
    {
        auto ipAddress = Utilities::ResolveAddressIfDomainName(m_EchoServerDomainName
                                                             , m_pNetworkInterface
                                                             , &m_TheSocketAddress);
        
        if (ipAddress)
        {
            std::swap(m_EchoServerAddress, ipAddress);
        }
        else
        {
            printf("Error! Utility::ResolveAddressIfDomainName() failed.\r\n");
        
            // Abandon attempting to connect to the socket. 
            return; 
        }
        
        m_TheSocketAddress.set_port(m_EchoServerPort);

        if (m_TheTransportSocketType == TransportSocket_t::TCP)
        {
            printf("Connecting to \"%s\" as resolved to: \"%s:%d\" ...\n",
                m_EchoServerDomainName.c_str(), 
                m_EchoServerAddress.value().c_str(), 
                m_EchoServerPort);
                
            nsapi_error_t rc = m_TheSocket.connect(m_TheSocketAddress);

            if (rc != NSAPI_ERROR_OK)
            {
                printf("Error! TCPSocket.connect() to EchoServer returned:\
                    [%d] -> %s\n", rc, ToString(rc).c_str());
                    
                // Abandon attempting to connect to the socket.
                return;
            }
            else
            {   
                printf("Success! Connected to EchoServer at \"%s\" as resolved to: \"%s:%d\"\n", 
                    m_EchoServerDomainName.c_str(), m_EchoServerAddress.value().c_str(), m_EchoServerPort);
            }
        }
    }
    
    Run();
}

void LEDLightControl::Run()
{    
    printf("Running LEDLightControl::Run() ... \r\n");
    
    while (true)
    {
        if (Send())
        {
            if (Receive())
            {
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
    
    // Abandon exchanging packets with the EchoServer. 
}

bool LEDLightControl::Send()
{    
    //printf("Running LEDLightControl::Send() ... \r\n");
    
    auto result = false;
    char rawBuffer[STANDARD_BUFFER_SIZE];
    int lengthWritten{0};    
    
    memset(rawBuffer, 0, sizeof(rawBuffer));
    
    // Simulate LED blinking through LightControl protocol messages sent 
    // on the various supported socket transport protocols:
    g_UserLEDState = !g_UserLEDState;
    
    // Protocol for LightControl message is a NUL terminated string of 
    // semicolon separated <field identifier>:<value> pairs.
    // 
    // LightControl protocol message format:
    //
    // t:lights;g:<group_id>;s:<1|0>;\0
    // 
    lengthWritten = std::snprintf(rawBuffer, 
                                  sizeof(rawBuffer), 
                                  "t:lights;g:%03d;s:%s;", 
                                  MY_LIGHT_CONTROL_GROUP, 
                                  (g_UserLEDState ? "1" : "0")) + 1;
    
    MBED_ASSERT(lengthWritten > 0);
    MBED_ASSERT(lengthWritten < sizeof(rawBuffer));
    
    //printf("After MBED_ASSERT on lengthWritten. lengthWritten = %d\n%s\r\n", lengthWritten, rawBuffer);

    if (m_TheTransportSocketType == TransportSocket_t::TCP)
    {        
        nsapi_error_t rc = m_TheSocket.send(rawBuffer, lengthWritten);
        
        if (rc < 0)
        {
            printf("Error! TCPSocket.send() to EchoServer returned:\
                [%d] -> %s\n", rc, ToString(rc).c_str());
        }
        else
        {
            result = true;
        }
    }
    else if (m_TheTransportSocketType == TransportSocket_t::CELLULAR_NON_IP)
    {
        nsapi_error_t rc = m_TheSocket.send(rawBuffer, lengthWritten);
        
        if (rc < 0)
        {
            printf("Error! CellularNonIPSocket.send() to EchoServer returned:\
                [%d] -> %s\n", rc, ToString(rc).c_str());
        }
        else
        {
            result = true;
        }
    }
    else
    {
        nsapi_error_t rc = m_TheSocket.sendto(m_TheSocketAddress, rawBuffer, lengthWritten);
        
        if (rc < 0)
        {
            printf("Error! UDPSocket.sendto() to EchoServer returned:\
                [%d] -> %s\n", rc, ToString(rc).c_str());
        }
        else
        {
            result = true;
        }
    }
    
    return result;
}

bool LEDLightControl::Receive()
{
    //printf("Running LEDLightControl::Receive() ... \r\n");
    
    auto result = false;
    char receiveBuffer[STANDARD_BUFFER_SIZE];

    memset(receiveBuffer, 0, sizeof(receiveBuffer));
    
    if (m_TheTransportSocketType != TransportSocket_t::UDP)
    {
        nsapi_size_or_error_t rc = m_TheSocket.recv(receiveBuffer, 
                                                    sizeof(receiveBuffer) - 1);
        
        
        if (rc > 0)
        {
            // Some data received of length rc so it is reasonable to
            // presume that the socket is still functioning properly.
            result = true;
                        
            std::string s(receiveBuffer, rc);
            std::string delimiter = ";";
            
            printf("Success! m_TheSocket.recv() returned:\
                [%d] -> %s\n", rc, s.c_str());
                        
            result = ParseAndConsumeLightControlMessage(s, delimiter);
        }
        else if (rc < 0)
        {
            printf("Error! m_TheSocket.recv() returned:\
                [%d] -> %s\n", rc, ToString(rc).c_str());
        }
        else
        {
            printf("Error! m_TheSocket.recv() indicated :\n\t\
                \"No data available to be received and the peer has \
                performed an orderly shutdown.\"\n");
        }
    }
    else
    {
        nsapi_size_or_error_t rc = m_TheSocket.recvfrom(&m_TheSocketAddress, 
                                                        receiveBuffer, 
                                                        sizeof(receiveBuffer) - 1);
        
        if (rc > 0)
        {
            // Some data received of length rc so it is reasonable to
            // presume that the socket is still functioning properly.
            result = true;
                        
            std::string s(receiveBuffer, rc);
            std::string delimiter = ";";
                        
            printf("Success! m_TheSocket.recvfrom() returned:\
                [%d] -> %s\n", rc, s.c_str());
                        
            result = ParseAndConsumeLightControlMessage(s, delimiter);
        }
        else if (rc < 0)
        {
            printf("Error! m_TheSocket.recvfrom() returned:\
                [%d] -> %s\n", rc, ToString(rc).c_str());
        }
        else
        {
            printf("Error! m_TheSocket.recvfrom() indicated :\n\t\
                \"No data available to be received and the peer has \
                performed an orderly shutdown.\"\n");
        }
    }
    
    return result;
}

bool LEDLightControl::ParseAndConsumeLightControlMessage(std::string& s, 
                                           const std::string& delimiter)
{    
    //printf("Running LEDLightControl::ParseAndConsumeLightControlMessage() ... \r\n");
    
    auto result = true;
    size_t pos = 0;
    std::string token;
    if ((pos = s.find(delimiter)) != std::string::npos)
    {
        token = s.substr(0, pos);
        if (!token.compare("t:lights"))
        {
            s.erase(0, pos + delimiter.length());
            
            if ((pos = s.find(delimiter)) != std::string::npos)
            {
                token = s.substr(0, pos);
                if (!token.compare("g:001")) // MY_LIGHT_CONTROL_GROUP
                {
                    s.erase(0, pos + delimiter.length());
                    
                    if ((pos = s.find(delimiter)) != std::string::npos)
                    {
                        token = s.substr(0, pos);
                        if (!token.compare("s:0"))
                        {
                            printf("Successfully parsed LightControl message. Turning LED OFF ... \r\n");
                            g_UserLED = LED_OFF;
                        }
                        else if (!token.compare("s:1"))
                        {
                            printf("Successfully parsed LightControl message. Turning LED ON ... \r\n");
                            g_UserLED = LED_ON;
                        }
                        else
                        {
                            printf("Error! \"s:<1|0>\" comparison failed. \
                                We rather parsed: \"%s\"\r\n", token.c_str());
                            result = false;
                        }
                    }
                    else
                    {
                        printf("Error! 3rd occurrence of LightControl \
                            message delimiter parsing failed.\r\n");
                        result = false;
                    }
                }
                else
                {
                    printf("Error! \"g:001\" comparison failed. \
                        We rather parsed: \"%s\"\r\n", token.c_str());
                    result = false;
                }
            }
            else
            {
                printf("Error! 2nd occurrence of LightControl \
                    message delimiter parsing failed.\r\n");
                result = false;
            }
        }
        else
        {
            printf("Error! \"t:lights\" comparison failed. \
                We rather parsed: \"%s\"\r\n", token.c_str());
            result = false;
        }
    }
    else
    {
        printf("Error! 1st occurrence of LightControl \
            message delimiter parsing failed.\r\n");
        result = false;
    }
    
    return result;
}
