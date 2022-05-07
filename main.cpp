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



// =======================================================================
// 10mm LED connections to NUCLEO-F767ZI output pins are documented below:
//
// Employing 100Ω resistors for these next three per my actual breadboard
// wiring.
// =======================================================================

// Connector: CN10
// Pin      : 12 
// Pin Name : D2
// STM32 Pin: PF15 ***
// Signal   : I/O
DigitalOut g_External10mmLEDGreen(PF_15); // LED Current = 18mA; Voltage Drop = 2.1V; Calculated Resistance = 66.67Ω

// *** CAUTION!!!
// PA7 is used as D11 and connected to CN7 pin 14 by default. If JP6 is ON,
// it is also connected to both Ethernet PHY as RMII_DV and CN9 pin 15. 
// In this case only one function of the Ethernet or D11 must be used.
//
// So choose some other pin here other than CN9, pin 15 (PA7).

// Connector: CN9 
// Pin      : 30 
// Pin Name : D64
// STM32 Pin: PG1
// Signal   : I/O
DigitalOut g_External10mmLEDYellow(PG_1);// LED Current = 18mA; Voltage Drop = 2.1V; Calculated Resistance = 66.67Ω

// Connector: CN10 
// Pin      : 28 
// Pin Name : D38
// STM32 Pin: PE14
// Signal   : I/O
DigitalOut g_External10mmLEDRed(PE_14);   // LED Current = 18mA; Voltage Drop = 2.0V; Calculated Resistance = 72.22Ω

// ====================================================================
// 10mm LED connections to NUCLEO-F767ZI PWM pins are documented below:
//
// Employing 47Ω resistors for these next three per my actual breadboard
// wiring.
// ====================================================================

// Connector: CN7 
// Pin      : 18 
// Pin Name : D9
// STM32 Pin: PD15
// Signal   : TIMER_B_PWM2
PwmOut g_ExternalPWMLEDGreen(PD_15);

// Connector: CN10 
// Pin      : 29 
// Pin Name : D32
// STM32 Pin: PA0
// Signal   : TIMER_C_PWM1
PwmOut g_ExternalPWMLEDYellow(PA_0);

// Connector: CN10 
// Pin      : 31 
// Pin Name : D33
// STM32 Pin: PB0
// Signal   : TIMER_D_PWM1
PwmOut g_ExternalPWMLEDRed(PB_0);
      
// Let's ride on the bleeding edge with chrono_literals:
using namespace std::chrono_literals;

// The LowPowerTicker class has the same methods as the Ticker class but
// operates in deep sleep mode and has less resolution. Use the LowPowerTicker
// interface to set up a recurring interrupt when you only need millisecond 
// accuracy; it calls a function repeatedly and at a specified rate. Because
// the LowPowerTicker class can operate in deep sleep mode, it does not 
// block deep sleep when active.
//
// You can create any number of LowPowerTicker objects, allowing multiple
// outstanding interrupts at the same time. The function can be a static
// function, a member function of a particular object or a Callback object.
LowPowerTicker ticker0_0;
LowPowerTicker ticker0_1;

LowPowerTicker ticker1;
LowPowerTicker ticker2;
LowPowerTicker ticker3;

// Leverage the LowPowerTicker mode to convert the previously self-contained
// triangular wave loop into a global loop that will advance per sample.
auto     g_MaxTriangular = std::max_element(g_TriangleWaveform, g_TriangleWaveform + NUMBER_OF_TRIANGULAR_SAMPLES);
uint32_t g_TriangularCount = 0;

void LEDGreenBlinker()
{
    g_External10mmLEDGreen = !g_External10mmLEDGreen;
}

void LEDYellowBlinker()
{
    g_External10mmLEDYellow = !g_External10mmLEDYellow;
}

void LEDRedBlinker()
{
    g_External10mmLEDRed = !g_External10mmLEDRed;
}

void LEDSawToothWave()
{
    // Gradually change the intensity of the LED according to the
    // saw-tooth waveform pattern.
    g_ExternalPWMLEDGreen = g_ExternalPWMLEDGreen + 0.01;

    // Set the output duty-cycle, specified as a percentage (float)
    //
    // Parameters
    //    value A floating-point value representing the output duty-cycle, 
    //    specified as a percentage. The value should lie between 0.0f 
    //    (representing on 0%) and 1.0f (representing on 100%). Values 
    //    outside this range will be saturated to 0.0f or 1.0f.
    if (g_ExternalPWMLEDGreen == 1.0)
    {
        g_ExternalPWMLEDGreen = 0;
    }
}

void LEDTriangularWave()
{
    // Warnings and notes
    //
    // No blocking code in ISR: avoid any calls to wait, infinite while 
    // loops or blocking calls in general.
    //
    // No printf, malloc or new in ISR: avoid any calls to bulky library
    // functions. In particular, certain library functions (such as printf,
    // malloc and new) are not re-entrant, and their behavior could be 
    // corrupted when called from an ISR.

    auto dutyCycle = g_TriangleWaveform[g_TriangularCount];

    // Set the output duty-cycle, specified as a percentage (float)
    //
    // Parameters
    //    value A floating-point value representing the output duty-cycle, 
    //    specified as a percentage. The value should lie between 0.0f 
    //    (representing on 0%) and 1.0f (representing on 100%). Values 
    //    outside this range will be saturated to 0.0f or 1.0f.
    float scaledDutyCycle = (dutyCycle/(*g_MaxTriangular));
    g_ExternalPWMLEDYellow = scaledDutyCycle;
    
    ++g_TriangularCount;
    if (g_TriangularCount >= NUMBER_OF_TRIANGULAR_SAMPLES)
    {
        g_TriangularCount = 0;
    }
}

void LEDSinusoidalWave(PwmOut * pExternalLEDPin)
{
    auto result = std::max_element(g_SineWaveform, g_SineWaveform + NUMBER_OF_SINUSOID_SAMPLES);

    while (1)
    {
        for (auto & dutyCycle : g_SineWaveform) 
        {
            // Set the output duty-cycle, specified as a percentage (float)
            //
            // Parameters
            //    value A floating-point value representing the output duty-cycle, 
            //    specified as a percentage. The value should lie between 0.0f 
            //    (representing on 0%) and 1.0f (representing on 100%). Values 
            //    outside this range will be saturated to 0.0f or 1.0f.
            float scaledDutyCycle = (dutyCycle/(*result));
            *pExternalLEDPin = scaledDutyCycle;
            
            // Power Management (sleep)
            //
            // Sleep mode
            //
            // The system clock to the core stops until a reset or an interrupt 
            // occurs. This eliminates dynamic power that the processor, 
            // memory systems and buses use. This mode maintains the processor,
            // peripheral and memory state, and the peripherals continue to work
            // and can generate interrupts.
            //
            // You can wake up the processor by any internal peripheral interrupt
            // or external pin interrupt.
            ThisThread::sleep_for(40ms);
        }
    }
}

int main()
{
    printf("\r\n\r\nNuertey-ConcurrentLEDs-Baremetal - Beginning... \r\n\r\n");

    // By default, enough buffer space for 32 Callbacks, i.e. 32*EVENTS_EVENT_SIZE
    // Reduce this amount if the target device has severely limited RAM.
    EventQueue theMasterEventQueue;

    // Events are simple callbacks:
    theMasterEventQueue.call(LEDSinusoidalWave, &g_ExternalPWMLEDRed);
    
    // The address of the function to be attached and the interval (in seconds).
    // Have these interrupt the sinusoidal wave event processing as that can sleep:
    ticker0_0.attach(&LEDSawToothWave, 200ms);
    ticker0_1.attach(&LEDTriangularWave, 200ms);
    
    ticker1.attach(&LEDGreenBlinker, 100ms);
    ticker2.attach(&LEDYellowBlinker, 200ms);
    ticker3.attach(&LEDRedBlinker, 500ms);

    // We will never return from the call below, as events are executed by 
    // the dispatch_forever method. And this is precisely what we want as,    
    // to be compatible with ARM microlib, a bare metal application MUST
    // NEVER return from main(). A crash will occur otherwise!
    theMasterEventQueue.dispatch_forever();

    // So we will NEVER get to this statement. Great! It is per design.
    printf("\r\n\r\nNuertey-ConcurrentLEDs-Baremetal Application - Exiting.\r\n\r\n");
}
