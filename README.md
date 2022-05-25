# Nuertey-Dragonfly-Cellular-LightControl - LED LightControl Through 4G LTE Cellular Transport On MultiTech Dragonfly™ Nano Development Board 

ARM Mbed application that blinks LED1 (D3, PA_0) on MultiTech Dragonfly™ Nano development board by sending and receiving 4G LTE Cellular LightControl messages.


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

```console
...
Compile [ 99.9%]: sleep.c
Compile [100.0%]: stm32f7xx_hal_smartcard.c
Link: Nuertey-Dragonfly-Cellular-LightControl
Elf2Bin: Nuertey-Dragonfly-Cellular-LightControl
| Module               |         .text |       .data |          .bss |
|----------------------|---------------|-------------|---------------|
| [fill]               |     116(+116) |       3(+3) |       27(+27) |
| [lib]/c.a            |   8840(+8840) | 2108(+2108) |       58(+58) |
| [lib]/gcc.a          |   4740(+4740) |       0(+0) |         0(+0) |
| [lib]/misc           |     188(+188) |       4(+4) |       28(+28) |
| [lib]/nosys.a        |       32(+32) |       0(+0) |         0(+0) |
| [lib]/stdc++.a       |   5196(+5196) |       8(+8) |       44(+44) |
| main.o               |   1642(+1642) |       0(+0) |     128(+128) |
| mbed-os/cmsis        |   9420(+9420) |   168(+168) | 14400(+14400) |
| mbed-os/connectivity |   2516(+2516) |       0(+0) | 12796(+12796) |
| mbed-os/drivers      |     194(+194) |       0(+0) |         0(+0) |
| mbed-os/events       |   1520(+1520) |       0(+0) |   3104(+3104) |
| mbed-os/hal          |   1528(+1528) |       8(+8) |     114(+114) |
| mbed-os/platform     |   7116(+7116) |   324(+324) |     433(+433) |
| mbed-os/rtos         |   1174(+1174) |       0(+0) |         8(+8) |
| mbed-os/targets      | 13650(+13650) |       9(+9) |   1316(+1316) |
| Subtotals            | 57872(+57872) | 2632(+2632) | 32456(+32456) |
Total Static RAM memory (data + bss): 35088(+35088) bytes
Total Flash memory (text + data): 60504(+60504) bytes

Image: ./BUILD/NUCLEO_F767ZI/GCC_ARM-MY_PROFILE/Nuertey-Dragonfly-Cellular-LightControl.bin


```

## Tested Target (and Peripheral):

Lacking an actual MultiTech Dragonfly Nano dev board and associated cellular modem, SIM Card, etc., on my workbench for testing, I reconfigured and built the same Nuertey-Dragonfly-Cellular-LightControl application for my NUCLEO_F767ZI board and tested it via Ethernet transport protocol and with TCP sockets. Relevant application code segment like so:

```c++
#ifdef NUERTEY_MTS_DRAGONFLY_L471QG
    // This call will never return as it encapsulates an EventQueue's
    // ::dispatch_forever() method.
    g_pLEDLightControl->Setup<TransportScheme_t::CELLULAR_4G_LTE, TransportSocket_t::TCP>();
#elseif NUERTEY_NUCLEO_F767ZI
    // This call will never return as it encapsulates an EventQueue's
    // ::dispatch_forever() method.
    g_pLEDLightControl->Setup<TransportScheme_t::ETHERNET, TransportSocket_t::TCP>();
#endif
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
