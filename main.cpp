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
*   
*   3GPP TS 27.007 and 27.005 are standards to provide consistent cellular
*   network data connectivity and SMS over an AT command interface for 
*   cellular devices, regardless of the underlying cellular network."
* 
*   https://os.mbed.com/docs/mbed-os/v6.15/apis/cellular-networking.html
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

LEDLightControl * g_pLEDLightControlManager = new LEDLightControl();

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

    mbed_trace_init();       // initialize the trace library
        
    if (g_MCUTarget == MCUTarget_t::MTS_DRAGONFLY_L471QG)
    {
        // This call will never return as it encapsulates an EventQueue's dispatch_forever() method.
        g_pLEDLightControlManager->Setup<TransportScheme_t::CELLULAR_4G_LTE, TransportSocket_t::TCP>();
    }
    else if (g_MCUTarget == MCUTarget_t::NUCLEO_F767ZI)
    {
        // This call will never return as it encapsulates an EventQueue's dispatch_forever() method.
        g_pLEDLightControlManager->Setup<TransportScheme_t::ETHERNET, TransportSocket_t::UDP>();
    }
    
    // It is envisioned that the application will execute forever so, the
    // following statement will never be reached, as indeed we do not want it to. 
    delete g_pLEDLightControlManager;
    
    // As per design, we will NEVER get to this statement. Great! Helps with debug...
    printf("\r\n\r\nNuertey-Dragonfly-Cellular-LightControl Application - Exiting.\r\n\r\n");
}
