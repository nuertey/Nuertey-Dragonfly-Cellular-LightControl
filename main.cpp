/***********************************************************************
* @file      main.cpp
*
*    ARM Mbed application that blinks LED1 (D3, PA_0) on MultiTech 
*    Dragonfly Nano development board by sending and receiving 4G LTE 
*    Cellular LightControl messages.
*
* @brief   Input: None
*          Output: LED1 begins to blink, continuously, once 4G LTE 
*                  connection is established and messages begin to flow.
* 
* @note   
*  
* Key features
*   
*   "Key features of the Mbed OS cellular APIs include:
*   
*       Compatible with 3GPP TS 27.007 and 27.005.
*       Implements core functionality for OMA Lightweight M2M Client.
*       Supports many Mbed Enabled cellular boards.
*   
*   3GPP TS 27.007 and 27.005 are standards to provide consistent cellular
*   network data connectivity and SMS over an AT command interface for 
*   cellular devices, regardless of the underlying cellular network.
*   
*   OMA Lightweight M2M is a protocol from the Open Mobile Alliance for 
*   IoT device management. The Mbed OS cellular API provides core 
*   functionality to implement the LWM2M Client. For more information, 
*   please see OMA LightweightM2M and OMA LWM2M Object Connectivity 
*   Management at OMA Specifications.
*   
*   Many Mbed Enabled boards already support cellular connectivity. 
*   Because Mbed OS is an open source platform, developers can enable 
*   support for new cellular boards with our adaptation framework. Please
*   see our cellular porting guide for more information."
* 
*   https://os.mbed.com/docs/mbed-os/v6.15/apis/cellular-networking.html
* 
* 
*   Dragonfly Nano Key Benefits
* 
*   End device certified by leading carriers
*   Low power modes extend life of battery powered devices
*   Extended coverage means assets deep inside buildings are now reachable
*   Long solution lifecycle reduces redesign time and cost
*   LTE, Cat M1 
* 
*   Dragonfly Nano Features
*
*   3GPP Release 13 Extended Discontinuous Reception (eDRX)
*   3GPP Release 13 Extended Coverage
*   3GPP Release 12 Power Saving Mode (PSM)
*   Arm® Mbed™ OS enabled
*   Multiple I/O interfaces for connecting almost any “Thing” 
*
* @warning   
*
* @author    Nuertey Odzeyem
* 
* @date      May 7th, 2022
*
* @copyright Copyright (c) 2022 Nuertey Odzeyem. All Rights Reserved.
***********************************************************************/
#include "LEDLightControl.h"

// Set the board that you want to target/test here:
//MCUTarget_t g_MCUTarget{MCUTarget_t::MTS_DRAGONFLY_L471QG};
MCUTarget_t g_MCUTarget{MCUTarget_t::NUCLEO_F767ZI};

std::unique_ptr<LEDLightControl> g_pLEDLightControl = std::make_unique<LEDLightControl>();

// Alternate to the above global variable approach, would be to use a
// local shared_ptr variable within main() as elucidated below, but perhaps
// such an approach might be overkill on Embedded, perhaps:
//
    // "Also, always ensure to assign the shared_ptr--which must of necessity
    // be declared as inheriting from:
    //
    // LEDLightControl : public std::enable_shared_from_this<LEDLightControl>
    //
    // --to a temporary otherwise it will run out of scope and the socket(s)
    // will segfault with:
    //
    // [WARN] Error in reading from TCP socket connection:
    // 127.0.0.1:5000
    // Value := "Code: 125
    //  Category: system
    //  Message: Operation canceled
    //
    //auto theSessionManager = std::make_shared<LEDLightControl>();
    //theSessionManager->Setup();"

int main()
{
    printf("\r\n\r\nNuertey-Dragonfly-Cellular-LightControl Application - Beginning... \r\n\r\n");
#ifdef MBED_MAJOR_VERSION
    printf("Mbed OS version: %d.%d.%d\n\n", MBED_MAJOR_VERSION, MBED_MINOR_VERSION, MBED_PATCH_VERSION);
#endif
    printf("Built: %s, %s\n\n", __DATE__, __TIME__);
#ifdef MBED_CONF_NSAPI_DEFAULT_CELLULAR_PLMN
    printf("[MAIN], CELLULAR_PLMN: %s\n\n", (MBED_CONF_NSAPI_DEFAULT_CELLULAR_PLMN ? MBED_CONF_NSAPI_DEFAULT_CELLULAR_PLMN : "NULL"));
#endif

    // Note that the MBED_ASSERT macro is only available in the Debug 
    // and Development build profiles and not in the Release build profile. 
    MBED_ASSERT(g_pLEDLightControl);

    if (g_MCUTarget == MCUTarget_t::MTS_DRAGONFLY_L471QG)
    {
        // This call will never return as it encapsulates an EventQueue's
        // ::dispatch_forever() method.
        g_pLEDLightControl->Setup<TransportScheme_t::CELLULAR_4G_LTE, TransportSocket_t::TCP>();
    }
    else if (g_MCUTarget == MCUTarget_t::NUCLEO_F767ZI)
    {
        // This call will never return as it encapsulates an EventQueue's
        // ::dispatch_forever() method.
        g_pLEDLightControl->Setup<TransportScheme_t::ETHERNET, TransportSocket_t::TCP>();
    }
     
    // As per design, we will NEVER get to this statement. Great! Helps with debug...
    printf("\r\n\r\nNuertey-Dragonfly-Cellular-LightControl Application - Exiting.\r\n\r\n");
}
