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

int main()
{
    printf("\r\n\r\nNuertey-Dragonfly-Cellular-LightControl Application - Beginning... \r\n\r\n");
#ifdef MBED_MAJOR_VERSION
    printf("Mbed OS version: %d.%d.%d\n\n", MBED_MAJOR_VERSION, MBED_MINOR_VERSION, MBED_PATCH_VERSION);
#endif
    printf("\n\nBuilt: %s, %s\n", __DATE__, __TIME__);
#ifdef MBED_CONF_NSAPI_DEFAULT_CELLULAR_PLMN
    printf("\n\n[MAIN], CELLULAR_PLMN: %s\n", (MBED_CONF_NSAPI_DEFAULT_CELLULAR_PLMN ? MBED_CONF_NSAPI_DEFAULT_CELLULAR_PLMN : "NULL"));
#endif



    // As per design, we will NEVER get to this statement. Great! Helps with debug...
    printf("\r\n\r\nNuertey-Dragonfly-Cellular-LightControl Application - Exiting.\r\n\r\n");
}
