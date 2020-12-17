# arduino-serial
Serial port interface for Arduino

This repository contains a basic interface for communication between a computer and an Arduino microcontroller through a serial port. It consists of two files written in C:

"arduino_serial_interface_arduino-side.ino" should be uploaded to the Arduino using the Arduino's software.
"arduino_serial_interface_computer-side.cpp" can be compiled on the computer and run as an executable, or can be integrated into your own C/C++ project.

Both files contain the minimum necessary functions to send and read data through the serial port that the Arduino is connected to. Edit or add your own messages to be sent and read, bearing in mind that the messages must match in both files.

For a trial run, proceed as follows:
1. Plug an Arduino into the computer.
2. Upload the Arduino's part of the interface though the Arduino software.
   Do not leave the serial monitor open as it would prevent connecting to the serial port.
3. Compile and run the computer-side interface.
   The program will automatically connect to the highest number serial port, typically the last plugged-in device.
4. The computer and Arduino will send a few messages back and forth, causing the on-board led light of the Arduino to blink, and messageboxes to pop up on the computer if successful. The program will end after a few seconds of this.

Edit the source code to suit your purposes.
