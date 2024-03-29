<!-- ![](https://github.com/adsehgal/adsehgal/blob/master/LOGO.png) -->

![](LOGO_V1.1.png)

# _Learning about the STM 32 world!_

This repo contains projects to learn the basics of embedded systems with STM32 MCUs.

## _Projects:_

- [x] **[DMA Byte Replication (12/17/20)](https://github.com/adsehgal/Learn_STM32/tree/master/DMA_Byte_Replication)** - This is a simple intro to using the DMA on the **STM32F401RE Nucleo Board** to complete a M2M transfer of 4096 bytes from one location in RAM to another. When complete the destination array is printed out to the terminal using the HAL library with USART2. As of now (12/17/2020) I am polling to check for transfer completion, however, I would like to generate an interrupt when the transfer is complete.

- [x] **[Bootloader Intro (04/12/21)](https://github.com/adsehgal/Learn_STM32/tree/master/Learning_Bootloader)** - This is an introduction on the **STM32F401RE Nucleo Board** to using a custom bootloader to check for a user application at 0x8020000000 when the user holds down a button at boot and then jump to the application. Once the user application has been jumped to, it simply flashes an LED every 100ms and prints out a confirmation every second, otherwise the bootloader goes into a "panic state" where it rapidly flashes an LED every 40ms.

- [x] **[SPI Non-Volatile Flash (10/10/21)](https://github.com/adsehgal/Learn_STM32/tree/master/SPI_FLASH_F767ZI_V2)** - This is an introduction on the **STM32F767ZI Nucleo Board** to utilizing SPI with a non-volatile flash storage. This project simply allows the user to read, write and erase any addressed bytes on the flash as well as read chip ID and other chip parameters. Further I utilized "[Little FS](https://github.com/littlefs-project/littlefs)" to further abstract the communication and enable file writing, reading, etc on the SPI flash

## _Built With:_

- Devices / Boards:
  - [Nucleo-F401RE](https://www.st.com/en/evaluation-tools/nucleo-f401re.html)
  - [Nucleo-F042K6](https://www.st.com/en/evaluation-tools/nucleo-f042k6.html)
  - [Nucleo-F767ZI](https://www.st.com/en/evaluation-tools/nucleo-f767zi.html)
  - [STM32F469I-DISCO](https://www.st.com/en/evaluation-tools/32f469idiscovery.html#overview)
- [STM32CubeIDE](https://www.st.com/en/development-tools/stm32cubeide.html) - Firmware Development
- [VS Code](https://code.visualstudio.com/) - Firmware Development / Text Edit

## _Up Next:_

- [ ] External QSPI Boot
- [ ] LWIP with FreeRTOS

<!-- ### *Notes:* -->
<!-- - The PCB uses a 5/5mil DRC constraint
- The top of the PCB acts as the product face plate
- Non-standard footprint 3-D models have been provided in the STEP file format -->

## _Authors:_

- **Aditya Sehgal**
  - _Firmware_ - [Adsehgal](https://github.com/adsehgal)
