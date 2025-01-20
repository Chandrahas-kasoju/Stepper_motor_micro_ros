# Stepper_motor_micro_ros
This repository intends to control stepper motor using ROS(Robot Operating System) with the help of MicroROS.

## PlatformIO setup
ESP32 S2 mini board was used in this project which is also supported in platformIO. When creating a new project in PlatformIO select a board(in this case its WEMOS LOLIN S2 MINI)

![Screenshot from 2025-01-20 10-59-14](https://github.com/user-attachments/assets/0aa059e0-3496-4a60-b9c4-3273d2c1c2a8)

Once the project is created, the platformio.ini and main.cpp files can be found inside the newly created project folder. Copy the contents of these files from this repository and build the project.

*NOTE: This project is built on the Arduino framework and is dependent on the micro_ros_platformio library but not on micro_ros_arduino!*

## Micro_ros_agent setup
In order to control the stepper motor from the Host PC, a micro_ros agent should be running on the Host PC. To set up the micro_ros follow the steps in this [link](https://micro.ros.org/docs/tutorials/core/first_application_linux/).

*Note: You need to have ROS installed in your Host PC already and using Docker is optional*

Since the communication was established using  the serial port between the microcontroller and Host PC, the follwing command was used to launch the micro_ros_agent


ros2 run micro_ros_agent micro_ros_agent serial --dev /dev/ttyACM0
