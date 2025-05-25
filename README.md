
# Alarm Clock using STM32C031C6
**This project is a custom-built STM32-based alarm clock designed to demonstrate foundational embedded systems concepts.**

To use the alarm clock, establish a USART connection using PuTTY or any compatible serial terminal software. Once connected, you can configure the system time and set alarms via a command-line interface.
When an alarm is triggered:
- A passive buzzer plays a melody using PWM signal generation.
- The LCD displays a pop-up message indicating the active alarm.
- Once the alarm is deactivated, the LCD reverts to showing the current time, including seconds.

This project represents the culmination of my self-taught learning in embedded systems, integrating several key concepts:
- USART (Interrupt Mode) – for serial communication and command parsing over a terminal interface.
- RTC (Real-Time Clock) – for accurate timekeeping and interrupt-driven alarm functionality.
- I2C Protocol – to interface with an external LCD display module.
- PWM Generation – to drive the passive buzzer with a time melody.

## Demonstration
[![Demonstration video 1](https://media1.giphy.com/media/v1.Y2lkPTc5MGI3NjExeDBpbXpmaGxtaWxkbHE0MGZkdWxoNDhsODhqeTdmNGxnNmhyY3c2diZlcD12MV9pbnRlcm5hbF9naWZfYnlfaWQmY3Q9Zw/dvGzu9yryBg65F5R4t/giphy.gif)](https://www.youtube.com/watch?v=joikh8UFD_I)


## How to install and run this project?
To install and run this project there are some things you **must have** beforehand:
* NUCLEO-C031C6 developmental board - [you can find it here.](https://www.st.com/en/evaluation-tools/nucleo-c031c6.html#overview)
* STM32CubeIDE (microcontroller specific IDE) - [you can find it here.](https://www.st.com/en/development-tools/stm32cubeide.html)
* STM32CubeMX (graphical tool) - [you can find it here.](https://www.st.com/en/development-tools/stm32cubemx.html) 
* 1602 LCD Display - [you can find it here.](https://www.amazon.com/dp/B0D2LBRXHM/ref=sspa_dk_detail_0?sp_csd=d2lkZ2V0TmFtZT1zcF9kZXRhaWxfdGhlbWF0aWM) 
* Cables and a passive buzzer - [Starter kit with everything.](https://www.amazon.com/dp/B09YRJQRFF) 
* USB-A to Micro USB - [you can find it here.](https://www.amazon.com/Amazon-Basics-Charging-Transfer-Gold-Plated/dp/B071S5NTDR?source=ps-sl-shoppingads-lpcontext&ref_=fplfs&smid=ATVPDKIKX0DER&gQT=0)
* PuTTY (software needed to communicate with microcontroller) - [you can find it here.](https://www.putty.org/)


After you got everything proceed.

### 1. Install project zip as shown in the gif
![hippo](https://media3.giphy.com/media/v1.Y2lkPTc5MGI3NjExanB6dDVrbnJodGRpNTMwbHJ5OXdqdXZneTNiaGlueGN1cGV2YWR4YSZlcD12MV9pbnRlcm5hbF9naWZfYnlfaWQmY3Q9Zw/Al0xIlR44DJClLfU8u/giphy.gif)

### 2. Extract project zip (I am assuming you know how)
![hippo](https://media2.giphy.com/media/v1.Y2lkPTc5MGI3NjExOXhleGdjMnYwenk2djN2ZnBuNnB1Z3hhMm5tZ2dqOHJmM2lmMGdrdiZlcD12MV9pbnRlcm5hbF9naWZfYnlfaWQmY3Q9Zw/I4hSsp4Qmsrkwy0MHv/giphy.gif)

### 3. Open the extracted folder and run the .cproject file
![hippo](https://media0.giphy.com/media/v1.Y2lkPTc5MGI3NjExajR4dWUxemZqczNiMGlwbGRncDk3ejA2Y3pwdTdwY2V1aHYybGJqcSZlcD12MV9pbnRlcm5hbF9naWZfYnlfaWQmY3Q9Zw/YYgUAKQAwdBTJfgQoM/giphy.gif)
### 4. Watch this video for further instructions
[![step 4 instructions](https://media4.giphy.com/media/v1.Y2lkPTc5MGI3NjExcmRndzdhZWR5eDY0b3R2dnZ2eWlqeXVuaGNtdnd6bjVnb2E3czA0ZSZlcD12MV9pbnRlcm5hbF9naWZfYnlfaWQmY3Q9Zw/0pzElAhaY79jqkUOaB/giphy.gif)](https://youtu.be/k6KFNa1gwo8)

## Acknowledgements
https://www.micropeta.com/video61 - module for LCD screen functionality.
