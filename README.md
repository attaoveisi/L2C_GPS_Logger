# L2C_GPS_Logger

In this repo, a GSM/GPRS module is implemented on AttBot2.0 based on the Arduino Mega 2560 board. 

![image](https://user-images.githubusercontent.com/17289954/103784545-5e93c580-503a-11eb-831e-fbee96beae93.png)

## GSM module (SIM808)

The GSM module allows AttBot2.0 to communicate with my smart phone to send it's location and recieve positions to be (if GPS signal is available.)

## GPS module

The gps module is for creating a navsatfix message for the ROS world. The inerface is realized with rosserial. The baudrate of the gps signal is 9600 which is mauch slower than the other sensor on AttBo2.0 (57600).

## The micro SD-card module 

In order to track the GPS location of the robot a micro-SD card module is also implemented:

![image](https://user-images.githubusercontent.com/17289954/103785595-aebf5780-503b-11eb-9379-7e55df0acbc5.png)
