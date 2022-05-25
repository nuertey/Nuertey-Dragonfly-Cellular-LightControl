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

// Choice of socket type configuration parsed from mbed_app.json:
#define NUERTEY_UDP 0
#define NUERTEY_TCP 1
#define NUERTEY_NONIP 2

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

// TBD Nuertey Odzeyem; I dont think these are any longer needed. Remove if confirmed.
//#if MBED_CONF_APP_SOCK_TYPE == NUERTEY_TCP
//    static constexpr char SOCKET_TYPE[] = "TCP";
//#elif MBED_CONF_APP_SOCK_TYPE == NUERTEY_UDP
//    static constexpr char SOCKET_TYPE[] = "UDP";
//#elif MBED_CONF_APP_SOCK_TYPE == NUERTEY_NONIP
//    static constexpr char SOCKET_TYPE[] = "CellularNonIP";
//#endif

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
    // 1 minute of failing to exchange packets with the EchoServer ought
    // to be enough to tell us that there is something wrong with the socket.
    static constexpr int32_t BLOCKING_SOCKET_TIMEOUT_MILLISECONDS{60000};
    static constexpr uint8_t MASTER_LIGHT_CONTROL_GROUP{0};
    static constexpr uint8_t     MY_LIGHT_CONTROL_GROUP{1};
    static constexpr uint32_t STANDARD_BUFFER_SIZE{1024}; // 1 K ought to cover all our cases.
    
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

protected:
    template <TransportScheme_t transport, TransportSocket_t socket>
        requires IsValidTransportType<transport, socket>
    void ConnectToNetworkInterface();

    void Run();
    [[nodiscard]] bool Send();
    [[nodiscard]] bool Receive();
    
    void ParseAndConsumeLightControlMesage(std::string& s, const std::string& delimiter);
    
private:
    TransportScheme_t          m_TheTransportSchemeType;
    TransportSocket_t          m_TheTransportSocketType;
    NetworkInterface *         m_pNetworkInterface;
    
    // To enable soft_power_off/on(), shutdown(), hard_power_on/off(), and such functions.
    CellularDevice *           m_pTheCellularDevice;   
    
    std::string                m_EchoServerDomainName; // Domain name will always exist.
    std::optional<std::string> m_EchoServerAddress;    // However IP Address might not always exist...
    uint16_t                   m_EchoServerPort;
    bool                       m_IsConnected;
    
    // Conditional class members based upon configuration parsed from 
    // mbed_app.json follow:
    
    // "The Socket class defines the Mbed OS Socket API and \'LOOSELY\' 
    // follows the POSIX standard (IEEE Std 1003.1).
    //
    // You can use this interface when designing \'PORTABLE APPLICATION\' 
    // interfaces that do not require specific protocol to be defined. 
    // For example, instead of using TCPSocket* in methods, the application
    // can use Socket* to allow either UDP or TCP to work, or even TLS."
    //
    // https://os.mbed.com/docs/mbed-os/v6.15/apis/socket.html
    
    // TBD Nuertey Odzeyem; enhance this LEDLightControl class encapsulation
    // to, instead of the below, rather use the portable Socket class interface
    // in handling all the 3 possible socket types. Ergo:
    //
    // Socket *                 m_pTheSocket;
    // SocketAddress            m_TheSocketAddress;
    //
    // And then create the requisite derived socket types based on the
    // templatized TransportSocket_t as you are already doing in the rest
    // of the code.
    
    #if MBED_CONF_APP_SOCK_TYPE == NUERTEY_TCP
        TCPSocket               m_TheSocket;
        SocketAddress           m_TheSocketAddress;
    #elif MBED_CONF_APP_SOCK_TYPE == NUERTEY_UDP
        UDPSocket               m_TheSocket;
        SocketAddress           m_TheSocketAddress;
    #elif MBED_CONF_APP_SOCK_TYPE == NUERTEY_NONIP
        // Send and receive 3GPP non-IP datagrams (NIDD) using the cellular IoT feature.
        CellularNonIPSocket     m_TheSocket;
    #endif
};

// Note that:
//
// m_TheTransportSchemeType is deliberately 'self-initialized' by its enum composition.
// m_TheTransportSocketType is deliberately 'self-initialized' by its enum composition.
// m_pNetworkInterface is deliberately 'self-initialized' by its requisite default constructor.
// m_pTheCellularDevice is deliberately 'self-initialized' by its requisite default constructor.
LEDLightControl::LEDLightControl()
    : m_EchoServerDomainName(ECHO_HOSTNAME)
    , m_EchoServerAddress(std::nullopt)
    , m_EchoServerPort(ECHO_PORT) 
    , m_IsConnected(false)
    // Deliberately letting the conditional class member variables 'self-initialize'
    // by default-constructing. 
{
}

LEDLightControl::~LEDLightControl()
{
    trace_close(); // For the internal cellular stack I believe.
}

template <TransportScheme_t transport, TransportSocket_t socket>
    requires IsValidTransportType<transport, socket>
void LEDLightControl::Setup()
{
    randLIB_seed_random();
    trace_open(); // For the internal cellular stack I believe.
    
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
        {
            printf("Local IP address set!\r\n");
            g_STDIOMutex.unlock();
            break;
        }
        case NSAPI_STATUS_GLOBAL_UP:
        {
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
        }
        case NSAPI_STATUS_DISCONNECTED:
        {
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
        }
        case NSAPI_STATUS_CONNECTING:
        {
            printf("Connecting to network!\r\n");
            g_STDIOMutex.unlock();
            break;
        }
        default:
        {
            printf("Perhaps New Cellular Pointer Data Codes Have Asynchronously Arrived:\r\n");
            g_STDIOMutex.unlock();
            
            if (statusEvent >= NSAPI_EVENT_CELLULAR_STATUS_BASE && statusEvent <= NSAPI_EVENT_CELLULAR_STATUS_END)
            {
                cell_callback_data_t *ptr_data = (cell_callback_data_t *)parameterPointerData;
                
                tr_debug("Network Status Event Callback: %d, \t\r\nptr_data->error: %d, \t\r\nptr_data->status_data: %d", \
                    statusEvent, ptr_data->error, ptr_data->status_data);
                
                cellular_connection_status_t cellEvent = static_cast<cellular_connection_status_t>(statusEvent);
                
                //if (cellEvent == CellularRegistrationStatusChanged)
                //{
                //    // broadcast only network registration changes to state machine
                //    //_state_machine->cellular_event_changed(ev, ptr);
                //}

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
                //else if (cellEvent == CellularSIMStatusChanged && ptr_data->error == NSAPI_ERROR_OK &&
                //         ptr_data->status_data == CellularSIM::SimStatePinNeeded)
                //{
                //    // if (strlen(_sim_pin))
                //    // {
                //    //     _state_machine->set_sim_pin(_sim_pin);
                //    // }
                //}
            }
            break;
        }
    }
}

void LEDLightControl::ConnectToSocket()
{
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
    if (m_TheTransportSocketType == TransportSocket_t::TCP)
    {
        nsapi_error_t rc = m_TheSocket.open(m_pNetworkInterface);
        if (rc != NSAPI_ERROR_OK)
        {
            printf("\r\n\r\nError! TCPSocket.open() returned: \
                [%d] -> %s\r\n", rc, ToString(rc).c_str());

            // Abandon attempting to connect to the socket. Subsequent 
            // NetworkStatusCallbacks() will dispatch the ConnectToSocket()
            // event again should network conditions become better favorable.                
            return;
        }
    }
    else if (m_TheTransportSocketType == TransportSocket_t::UDP)
    {
        nsapi_error_t rc = m_TheSocket.open(m_pNetworkInterface);
        if (rc != NSAPI_ERROR_OK)
        {
            printf("\r\n\r\nError! UDPSocket.open() returned: \
                [%d] -> %s\r\n", rc, ToString(rc).c_str());

            // Abandon attempting to connect to the socket. Subsequent 
            // NetworkStatusCallbacks() will dispatch the ConnectToSocket()
            // event again should network conditions become better favorable.                
            return;
        }
    }
    else if (m_TheTransportSocketType == TransportSocket_t::CELLULAR_NON_IP)
    {
        nsapi_error_t rc = m_TheSocket.open(static_cast<CellularContext *>(m_pNetworkInterface));
        if (rc != NSAPI_ERROR_OK)
        {
            printf("\r\n\r\nError! CellularNonIPSocket.open() returned: \
                [%d] -> %s\r\n", rc, ToString(rc).c_str());

            // Abandon attempting to connect to the socket. Subsequent 
            // NetworkStatusCallbacks() will dispatch the ConnectToSocket()
            // event again should network conditions become better favorable.                
            return;
        }
    }  
    
    // Set timeout on blocking socket operations.
    //
    // Initially all sockets have unbounded timeouts. NSAPI_ERROR_WOULD_BLOCK
    // is returned if a blocking operation takes longer than the specified timeout.
    m_TheSocket.set_timeout(BLOCKING_SOCKET_TIMEOUT_MILLISECONDS);
    
    if (m_TheTransportSocketType != TransportSocket_t::CELLULAR_NON_IP)
    {
        auto ipAddress = Utilities::ResolveAddressIfDomainName(*m_EchoServerAddress
                                                             , m_pNetworkInterface
                                                             , &m_TheSocketAddress);
        
        if (ipAddress)
        {
            std::swap(m_EchoServerAddress, ipAddress);
        }
        else
        {
            printf("\r\n\r\nError! Utility::ResolveAddressIfDomainName() failed.\r\n");

            // Abandon attempting to connect to the socket. Subsequent 
            // NetworkStatusCallbacks() will dispatch the ConnectToSocket()
            // event again should network conditions become better favorable.                
            return; 
        }
        m_TheSocketAddress.set_port(m_EchoServerPort);

        if (m_TheTransportSocketType == TransportSocket_t::TCP)
        {
            printf("\r\nConnecting to : \"%s:%d\" ...", \
                (*m_EchoServerAddress).c_str(), m_EchoServerPort);
                
            nsapi_error_t rc = m_TheSocket.connect(m_TheSocketAddress);

            if (rc != NSAPI_ERROR_OK)
            {
                printf("\r\n\r\nError! TCPSocket.connect() to EchoServer returned:\
                    [%d] -> %s\n", rc, ToString(rc).c_str());
                    
                // Abandon attempting to connect to the socket. Subsequent 
                // NetworkStatusCallbacks() will dispatch the ConnectToSocket()
                // event again should network conditions become better favorable.                
                return;
            }
            else
            {   
                printf("\r\nSuccess! Connected to EchoServer at: \"%s:%d\"", \
                    (*m_EchoServerAddress).c_str(), m_EchoServerPort);
            }
        }
    }
    
    Run();
}

void LEDLightControl::Run()
{
    while (m_IsConnected)
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
    
    // Abandon exchanging packets with the EchoServer. Subsequent 
    // NetworkStatusCallbacks() will dispatch the ConnectToSocket()
    // event again should network conditions become better favorable. 
}

bool LEDLightControl::Send()
{
    auto result = false;
    char rawBuffer[STANDARD_BUFFER_SIZE];
    int lengthWritten{0};    
    
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
                                  (g_UserLEDState ? "1" : "0")) + 1; // Ensure to account for terminating NUL.
    
    MBED_ASSERT(lengthWritten > 0);
    
    if (m_TheTransportSocketType != TransportSocket_t::UDP)
    {
        nsapi_error_t rc = m_TheSocket.send(rawBuffer, lengthWritten);
        
        if (rc != NSAPI_ERROR_OK)
        {
            printf("\r\n\r\nError! m_TheSocket.send() to EchoServer returned:\
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
        
        if (rc != NSAPI_ERROR_OK)
        {
            printf("\r\n\r\nError! m_TheSocket.sendto() to EchoServer returned:\
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
                        
            ParseAndConsumeLightControlMesage(s, delimiter);
        }
        else if (rc < 0)
        {
            printf("\r\n\r\nError! m_TheSocket.recv() returned:\
                [%d] -> %s\n", rc, ToString(rc).c_str());
        }
        else
        {
            printf("\r\n\r\nError! m_TheSocket.recv() indicated :\n\t\
                \"No data available to be received and the peer has performed an orderly shutdown.\"\n");
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
                        
            ParseAndConsumeLightControlMesage(s, delimiter);
        }
        else if (rc < 0)
        {
            printf("\r\n\r\nError! m_TheSocket.recvfrom() returned:\
                [%d] -> %s\n", rc, ToString(rc).c_str());
        }
        else
        {
            printf("\r\n\r\nError! m_TheSocket.recvfrom() indicated :\n\t\
                \"No data available to be received and the peer has performed an orderly shutdown.\"\n");
        }
    }
    
    return result;
}

void LEDLightControl::ParseAndConsumeLightControlMesage(std::string& s, const std::string& delimiter)
{
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
                            g_UserLED = LED_OFF;
                        }
                        else if (!token.compare("s:1"))
                        {
                            g_UserLED = LED_ON;
                        }
                        else
                        {
                            printf("\r\n\r\nError! \"s:<1|0>\" comparison failed. \
                                We rather parsed: \"%s\"\r\n", token.c_str());
                        }
                    }
                    else
                    {
                        printf("\r\n\r\nError! 3rd occurrence of LightControl \
                            message delimiter parsing failed.\r\n");
                    }
                }
                else
                {
                    printf("\r\n\r\nError! \"g:001\" comparison failed. \
                        We rather parsed: \"%s\"\r\n", token.c_str());
                }
            }
            else
            {
                printf("\r\n\r\nError! 2nd occurrence of LightControl \
                    message delimiter parsing failed.\r\n");
            }
        }
        else
        {
            printf("\r\n\r\nError! \"t:lights\" comparison failed. \
                We rather parsed: \"%s\"\r\n", token.c_str());
        }
    }
    else
    {
        printf("\r\n\r\nError! 1st occurrence of LightControl \
            message delimiter parsing failed.\r\n");
    }
}
