# Nuertey-Dragonfly-Cellular-LightControl - LED LightControl Through 4G LTE Cellular Transport On MultiTech Dragonfly™ Nano Development Board 

ARM Mbed application that blinks LED1 (D3, PA_0) on MultiTech Dragonfly™ Nano development board by sending and receiving 4G LTE Cellular LightControl messages.

## Nuertey-Dragonfly-Cellular-LightControl UML Class Diagram - main branch (i.e. Asynchronous Design Version)

![Nuertey-Dragonfly-Cellular-LightControl Class Diagram](https://github.com/nuertey/RandomArtifacts/blob/master/Nuertey-Dragonfly-Cellular-LightControl_asynchronous.png?raw=true)

## Nuertey-Dragonfly-Cellular-LightControl UML Sequence Diagram - main branch (i.e. Asynchronous Design Version)

<placeholder>

## DEPENDENCIES - CODING LANGUAGE/OS/COMPILATION TARGET/COMPILER:
  - C++20
  - mbed-os-6.15.1
  - MTS_DRAGONFLY_L471QG
  - GCC ARM 10.3.1
    - arm-none-eabi-g++ (GNU Arm Embedded Toolchain 10.3-2021.10) 10.3.1 20210824 (release)
    - gcc-arm-none-eabi-10.3-2021.10/bin

```console 
(py37-venv) nuertey@nuertey-PC:/.../Nuertey-Dragonfly-Cellular-LightControl$ mbed config -L

[mbed] Global config:
GCC_ARM_PATH=/.../gcc-arm-none-eabi-10.3-2021.10/bin

[mbed] Local config (/.../Workspace/Nuertey-Dragonfly-Cellular-LightControl):
TARGET=nucleo_f767zi
TOOLCHAIN=GCC_ARM


(py37-venv) nuertey@nuertey-PC:/.../Nuertey-Dragonfly-Cellular-LightControl$ mbed ls

[mbed] Working path "/.../Nuertey-Dragonfly-Cellular-LightControl" (library)
[mbed] Program path "/.../Nuertey-Dragonfly-Cellular-LightControl"
Nuertey-Dragonfly-Cellular-LightControl ( revision in the current branch)
`- mbed-os (#2eb06e762085, tags: mbed-os-6.15.1, mbed-os-6.15.1-rc1)

```
 
## Compilation Output (Mbed CLI 1)

```shell-session
...
Compile [ 99.6%]: stm32f7xx_hal_dac.c
Compile [ 99.7%]: stm32f7xx_ll_gpio.c
Compile [ 99.8%]: MBRBlockDevice.cpp
Compile [ 99.9%]: i2c_api.c
Compile [100.0%]: stm32f7xx_hal_pwr.c
Link: Nuertey-Dragonfly-Cellular-LightControl
Elf2Bin: Nuertey-Dragonfly-Cellular-LightControl
| Module               |           .text |       .data |          .bss |
|----------------------|-----------------|-------------|---------------|
| [fill]               |         90(+90) |     11(+11) |       78(+78) |
| [lib]/c.a            |   13584(+13584) | 2472(+2472) |       58(+58) |
| [lib]/gcc.a          |     4740(+4740) |       0(+0) |         0(+0) |
| [lib]/misc           |       188(+188) |       4(+4) |       28(+28) |
| [lib]/nosys.a        |         32(+32) |       0(+0) |         0(+0) |
| [lib]/stdc++.a       |     7532(+7532) |       8(+8) |       44(+44) |
| main.o               |   55396(+55396) |       1(+1) |       98(+98) |
| mbed-os/cmsis        |   26232(+26232) |   168(+168) |   6336(+6336) |
| mbed-os/connectivity | 175932(+175932) |   107(+107) | 23695(+23695) |
| mbed-os/drivers      |     5060(+5060) |       0(+0) |         0(+0) |
| mbed-os/events       |     4272(+4272) |       0(+0) |     864(+864) |
| mbed-os/hal          |     4240(+4240) |       8(+8) |     114(+114) |
| mbed-os/platform     |   28712(+28712) |   260(+260) |     381(+381) |
| mbed-os/rtos         |     1924(+1924) |       0(+0) |         8(+8) |
| mbed-os/targets      |   27998(+27998) |       9(+9) |   1304(+1304) |
| Subtotals            | 355932(+355932) | 3048(+3048) | 33008(+33008) |
Total Static RAM memory (data + bss): 36056(+36056) bytes
Total Flash memory (text + data): 358980(+358980) bytes

Image: ./BUILD/NUCLEO_F767ZI/GCC_ARM-MY_PROFILE/Nuertey-Dragonfly-Cellular-LightControl.bin

```

## Tested Target (and Peripheral):

Lacking an actual MultiTech Dragonfly Nano dev board and associated cellular modem, SIM Card, etc., on my workbench for testing, I reconfigured and built the same Nuertey-Dragonfly-Cellular-LightControl application for my NUCLEO_F767ZI board and tested it via Ethernet transport protocol and with TCP sockets. Relevant application code segment like so:

```c++
    if (g_MCUTarget == MCUTarget_t::MTS_DRAGONFLY_L471QG)
    {
        // This call will never return as it encapsulates an EventQueue's dispatch_forever() method.
        g_pLEDLightControlManager->Setup<TransportScheme_t::CELLULAR_4G_LTE, TransportSocket_t::TCP>();
    }
    else if (g_MCUTarget == MCUTarget_t::NUCLEO_F767ZI)
    {
        // This call will never return as it encapsulates an EventQueue's dispatch_forever() method.
        g_pLEDLightControlManager->Setup<TransportScheme_t::ETHERNET, TransportSocket_t::TCP>();
    }
```

## Execution Output Snippet:

```shell-session
Nuertey-Dragonfly-Cellular-LightControl Application - Beginning... 

Mbed OS version: 6.15.1

Built: Jun  7 2022, 08:43:36

Running LEDLightControl::Setup() ... 
Connecting to network ...
Global IP address set!
Running LEDLightControl::ConnectToSocket() ... 
Particular Network Interface IP address: 10.42.0.58
Particular Network Interface Netmask: 255.255.255.0
Particular Network Interface Gateway: 10.42.0.1
Particular Network Interface MAC Address: 00:80:e1:37:00:25
Performing DNS lookup for : "echo.mbedcloudtesting.com" ...
Connecting to "echo.mbedcloudtesting.com" as resolved to: "52.215.34.155:7" ...
Success! Connected to EchoServer at "echo.mbedcloudtesting.com" as resolved to: "52.215.34.155:7"
Running LEDLightControl::Run() ... 
Success! m_pTheSocket->recv() returned:                [20] -> t:lights;g:001;s:1;
Successfully parsed LightControl message. Turning LED ON ... 
Success! m_pTheSocket->recv() returned:                [20] -> t:lights;g:001;s:0;
Successfully parsed LightControl message. Turning LED OFF ... 
Success! m_pTheSocket->recv() returned:                [20] -> t:lights;g:001;s:1;
Successfully parsed LightControl message. Turning LED ON ... 
Success! m_pTheSocket->recv() returned:                [20] -> t:lights;g:001;s:0;
Successfully parsed LightControl message. Turning LED OFF ... 
Success! m_pTheSocket->recv() returned:                [20] -> t:lights;g:001;s:1;
Successfully parsed LightControl message. Turning LED ON ... 
Success! m_pTheSocket->recv() returned:                [20] -> t:lights;g:001;s:0;
Successfully parsed LightControl message. Turning LED OFF ... 
Success! m_pTheSocket->recv() returned:                [20] -> t:lights;g:001;s:1;
Successfully parsed LightControl message. Turning LED ON ... 
Success! m_pTheSocket->recv() returned:                [20] -> t:lights;g:001;s:0;
Successfully parsed LightControl message. Turning LED OFF ... 
Success! m_pTheSocket->recv() returned:                [20] -> t:lights;g:001;s:1;
Successfully parsed LightControl message. Turning LED ON ... 
Success! m_pTheSocket->recv() returned:                [20] -> t:lights;g:001;s:0;
Successfully parsed LightControl message. Turning LED OFF ... 
Success! m_pTheSocket->recv() returned:                [20] -> t:lights;g:001;s:1;
Successfully parsed LightControl message. Turning LED ON ... 
Success! m_pTheSocket->recv() returned:                [20] -> t:lights;g:001;s:0;
Successfully parsed LightControl message. Turning LED OFF ... 
Success! m_pTheSocket->recv() returned:                [20] -> t:lights;g:001;s:1;
Successfully parsed LightControl message. Turning LED ON ... 
Success! m_pTheSocket->recv() returned:                [20] -> t:lights;g:001;s:0;
Successfully parsed LightControl message. Turning LED OFF ... 
Success! m_pTheSocket->recv() returned:                [20] -> t:lights;g:001;s:1;
Successfully parsed LightControl message. Turning LED ON ... 
Success! m_pTheSocket->recv() returned:                [20] -> t:lights;g:001;s:0;
Successfully parsed LightControl message. Turning LED OFF ... 
Success! m_pTheSocket->recv() returned:                [20] -> t:lights;g:001;s:1;
Successfully parsed LightControl message. Turning LED ON ... 
Success! m_pTheSocket->recv() returned:                [20] -> t:lights;g:001;s:0;
Successfully parsed LightControl message. Turning LED OFF ... 
Success! m_pTheSocket->recv() returned:                [20] -> t:lights;g:001;s:1;
Successfully parsed LightControl message. Turning LED ON ... 
Success! m_pTheSocket->recv() returned:                [20] -> t:lights;g:001;s:0;
Successfully parsed LightControl message. Turning LED OFF ... 
Success! m_pTheSocket->recv() returned:                [20] -> t:lights;g:001;s:1;
Successfully parsed LightControl message. Turning LED ON ... 
Success! m_pTheSocket->recv() returned:                [20] -> t:lights;g:001;s:0;
Successfully parsed LightControl message. Turning LED OFF ... 
Success! m_pTheSocket->recv() returned:                [20] -> t:lights;g:001;s:1;
Successfully parsed LightControl message. Turning LED ON ... 
Success! m_pTheSocket->recv() returned:                [20] -> t:lights;g:001;s:0;
Successfully parsed LightControl message. Turning LED OFF ... 
Success! m_pTheSocket->recv() returned:                [20] -> t:lights;g:001;s:1;
Successfully parsed LightControl message. Turning LED ON ... 
Success! m_pTheSocket->recv() returned:                [20] -> t:lights;g:001;s:0;
Successfully parsed LightControl message. Turning LED OFF ... 
Success! m_pTheSocket->recv() returned:                [20] -> t:lights;g:001;s:1;
Successfully parsed LightControl message. Turning LED ON ... 
Success! m_pTheSocket->recv() returned:                [20] -> t:lights;g:001;s:0;
Successfully parsed LightControl message. Turning LED OFF ... 
Success! m_pTheSocket->recv() returned:                [20] -> t:lights;g:001;s:1;
Successfully parsed LightControl message. Turning LED ON ... 
Success! m_pTheSocket->recv() returned:                [20] -> t:lights;g:001;s:0;
Successfully parsed LightControl message. Turning LED OFF ... 
Success! m_pTheSocket->recv() returned:                [20] -> t:lights;g:001;s:1;
Successfully parsed LightControl message. Turning LED ON ... 
Success! m_pTheSocket->recv() returned:                [20] -> t:lights;g:001;s:0;
Successfully parsed LightControl message. Turning LED OFF ... 
Success! m_pTheSocket->recv() returned:                [20] -> t:lights;g:001;s:1;
Successfully parsed LightControl message. Turning LED ON ... 
Success! m_pTheSocket->recv() returned:                [20] -> t:lights;g:001;s:0;
Successfully parsed LightControl message. Turning LED OFF ... 
Success! m_pTheSocket->recv() returned:                [20] -> t:lights;g:001;s:1;
Successfully parsed LightControl message. Turning LED ON ... 

```

## License
MIT License

Copyright (c) 2022 Nuertey Odzeyem

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
