<!-- ![](https://github.com/adsehgal/adsehgal/blob/master/LOGO.png) -->

![](LOGO_V1.1.png)

# _Learning about the STM 32 world!_

This repo contains projects to learn the basics of embedded systems with STM32 MCUs.

## _Projects:_

- [x] **[DMA Byte Replication (12/17/20)](https://github.com/adsehgal/Learn_STM32/tree/master/DMA_Byte_Replication)** - This is a simple intro to using the DMA on the **STM32F401RE Nucleo Board** to complete a M2M transfer of 4096 bytes from one location in RAM to another. When complete the destination array is printed out to the terminal using the HAL library with USART2. As of now (12/17/2020) I am polling to check for transfer completion, however, I would like to generate an interrupt when the transfer is complete.
- [x] **[UART RX Intro (02/08/21)](https://github.com/adsehgal/Learn_STM32/tree/master/DMA_Byte_Replication)** - This is an introduction on the **STM32F042K6 Nucleo Board** to utilizing UART on interrupts to recieve a new-line-terminated user string. Once the string is receieved, the program simply echoes the string back to the user.
- [x] **[Bootloader Intro (04/12/21)](https://github.com/adsehgal/Learn_STM32/tree/master/DMA_Byte_Replication)** - This is an introduction on the **STM32F401RE Nucleo Board** to using a custom bootloader to check for a user application at 0x8020000000 when the user holds down a button at boot and then jump to the application. Once the user application has been jumped to, it simply flashes an LED every 100ms and prints out a confirmation every second, otherwise the bootloader goes into a "panic state" where it rapidly flashes an LED every 40ms.
- [x] **[SPI Non-Volatile Flash (06/02/21)](https://github.com/adsehgal/Learn_STM32/tree/master/DMA_Byte_Replication)** - This is an introduction on the **STM32F767ZI Nucleo Board** to utilizing SPI with a non-volatile flash storage. This project simply allows the user to read, write and erase any addressed bytes on the flash as well as read chip ID and other chip parameters.

- [x] **[USB VCP Intro (06/15/21)](https://github.com/adsehgal/Learn_STM32/tree/master/DMA_Byte_Replication)** - This is an introduction on the **STM32F469ZI Discovery Board** to utilizing the native USB PHY as a virtual COM port (VCP), effectively eliminating the need of USB-TTL converter chips. The program reads the user input through a serial monitor charater by character on an interrupt basis, and when a new-line character is recieved, it process the string and displays the user with the requested information. Type "help" to begin.

## _Built With:_

- Devices / Boards:
  - [Nucleo-F401RE](https://www.st.com/en/evaluation-tools/nucleo-f401re.html)
  - [Nucleo-F042K6](https://www.st.com/en/evaluation-tools/nucleo-f042k6.html)
  - [Nucleo-F767ZI](https://www.st.com/en/evaluation-tools/nucleo-f767zi.html)
  - [STM32F469I-DISCO](https://www.st.com/en/evaluation-tools/32f469idiscovery.html#overview)
- [STM32CubeIDE](https://www.st.com/en/development-tools/stm32cubeide.html) - Firmware Development
- [VS Code](https://code.visualstudio.com/) - Firmware Development / Text Edit

## _Up Next:_

- [ ] LWIP with FreeRTOS
- [ ] USB CDC (Custom Device)

<!-- ### *Notes:* -->
<!-- - The PCB uses a 5/5mil DRC constraint
- The top of the PCB acts as the product face plate
- Non-standard footprint 3-D models have been provided in the STEP file format -->

## _Authors:_

- **Aditya Sehgal**
  - _Firmware_ - [Adsehgal](https://github.com/adsehgal)
