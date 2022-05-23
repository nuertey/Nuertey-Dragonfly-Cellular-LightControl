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

[mbed] Local config (/.../Nuertey-Dragonfly-Cellular-LightControl):
No local configuration is set

(py37-venv) nuertey@nuertey-PC:/.../Nuertey-Dragonfly-Cellular-LightControl$ mbed ls

[mbed] Working path "/.../Nuertey-Dragonfly-Cellular-LightControl" (library)
[mbed] Program path "/.../Nuertey-Dragonfly-Cellular-LightControl"
Nuertey-Dragonfly-Cellular-LightControl ( revision in the current branch)
`- mbed-os (#2eb06e762085, tags: mbed-os-6.15.1, mbed-os-6.15.1-rc1)

```
 
## Compilation Output (Mbed CLI 1)

```console
...
Compile [ 99.9%]: mbed_crc_api.c
Compile [100.0%]: stm32f7xx_hal_smbus.c
Compile [100.0%]: gpio_api.c
Link: Nuertey-LDESeries-Mbed
Elf2Bin: Nuertey-LDESeries-Mbed
| Module               |           .text |       .data |          .bss |
|----------------------|-----------------|-------------|---------------|
| NuerteyNTPClient.o   |     4042(+4042) |       4(+4) |     101(+101) |
| Utilities.o          |     9592(+9592) |       4(+4) |     449(+449) |
| [fill]               |       308(+308) |     21(+21) |       82(+82) |
| [lib]/c.a            |   81456(+81456) | 2574(+2574) |       97(+97) |
| [lib]/gcc.a          |     7416(+7416) |       0(+0) |         0(+0) |
| [lib]/m.a            |       264(+264) |       0(+0) |         0(+0) |
| [lib]/misc           |       188(+188) |       4(+4) |       28(+28) |
| [lib]/nosys.a        |         32(+32) |       0(+0) |         0(+0) |
| [lib]/stdc++.a       | 174244(+174244) |   145(+145) |   5720(+5720) |
| main.o               |     3726(+3726) |       4(+4) |     261(+261) |
| mbed-os/cmsis        |     9890(+9890) |   168(+168) | 14400(+14400) |
| mbed-os/connectivity |   54186(+54186) |   103(+103) | 24059(+24059) |
| mbed-os/drivers      |     1146(+1146) |       0(+0) |   1852(+1852) |
| mbed-os/events       |     1776(+1776) |       0(+0) |   3104(+3104) |
| mbed-os/hal          |     1528(+1528) |       8(+8) |     114(+114) |
| mbed-os/platform     |     7166(+7166) |   340(+340) |     493(+493) |
| mbed-os/rtos         |     1280(+1280) |       0(+0) |         8(+8) |
| mbed-os/targets      |   17476(+17476) |       9(+9) |   1352(+1352) |
| Subtotals            | 375716(+375716) | 3384(+3384) | 52120(+52120) |
Total Static RAM memory (data + bss): 55504(+55504) bytes
Total Flash memory (text + data): 379100(+379100) bytes

Image: ./BUILD/NUCLEO_F767ZI/GCC_ARM-MY_PROFILE/Nuertey-LDESeries-Mbed.bin

```

## Tested Target (and Peripheral):

Lacking an actual LDE Series pressure sensor on my workbench for testing, I am left to appeal to you OEMs or better-equipped IoT hobbyists out there. If you have a spare LDE Series pressure sensor dev board that can be connected to my STM32F767-ZI [Nucleo-144], kindly send me an email at nuertey_odzeyem@hotmail.com 


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
