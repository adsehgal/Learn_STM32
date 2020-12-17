<!-- ![](https://github.com/adsehgal/adsehgal/blob/master/LOGO.png) -->
![](LOGO_V1.1.png)
# *Learning about the STM 32 world!*

This repo contains projects to learn the basics of embedded systems with STM32 MCUs.

## *Projects:*
* **[DMA Byte Replication](https://github.com/adsehgal/Learn_STM32/tree/master/DMA_Byte_Replication)** - This is a simple intro to using the DMA on the **STM32F401RE** MCU/Board to complete a M2M transfer of 4096 bytes from one location in RAM to another. When complete the destination array is printed out to the terminal using the HAL library with USART2. As of now (12/17/2020) I am polling to check for transfer completion, however, I would like to generate an interrupt when the transfer is complete. 


## *Built With:*
* Devices / Boards:
  * [Nucleo-F401RE](https://www.st.com/en/evaluation-tools/nucleo-f401re.html)
  * [Nucleo-F042K6](https://www.st.com/en/evaluation-tools/nucleo-f042k6.html)
* [STM32CubeIDE](https://www.st.com/en/development-tools/stm32cubeide.html) - Firmware Development
* [VS Code](https://code.visualstudio.com/) - Firmware Development / Text Edit

## *Up Next:*
- [x] DMA Byte Replication (12/17/20)
- [ ] DMA Endian Swap
- [ ] ADC to DMA to UART
- [ ] AND MORE...

<!-- ### *Notes:* -->
<!-- - The PCB uses a 5/5mil DRC constraint
- The top of the PCB acts as the product face plate
- Non-standard footprint 3-D models have been provided in the STEP file format -->

## *Authors:*

* **Aditya Sehgal** 
  * *Firmware* - [Adsehgal](https://github.com/adsehgal)