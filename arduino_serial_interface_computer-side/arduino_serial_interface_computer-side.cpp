/*
  Reads and sends serial port data from/to an Arduino microcontroller.
  Run this program on the computer.
  Hold escape to end program.
  
  Basic Windows functions involved in serial port communication:
  CreateFile() - to open the port
  SetCommState() - to configure baud rate, stop bits, etc.
  WriteFile() - to write to the port
  ReadFile() - to read from the port
*/


#include <windows.h>
#include <stdio.h>
#include <ctype.h>   // for isspace() etc
#include <time.h>    // for time(0)


// Function declarations:
HANDLE connecttoPort(int portnr);
int sendtoPort(HANDLE serialPort, const char *sendText);
char readfromPort(HANDLE serialPort);
void closePort(HANDLE serialPort);

// Helper functions:
bool keyPressed(int k) {return GetAsyncKeyState(k);}
void showWarning(const char *text) {MessageBox(NULL, text, "error", MB_SETFOREGROUND | MB_OK);}



HANDLE connecttoPort(int portnr) {
/*
 Connects to specified serial port number, e.g. "COM4".
 (Do not leave the Arduino software's serial monitor open as well, it will prevent access)
*/
char portname[10] = "";  
snprintf(portname, 10, "COM%i", portnr);

// Open a filestream to the specified serial port for reading and writing:
HANDLE hSerial = CreateFile(portname, GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
if(hSerial == INVALID_HANDLE_VALUE) {return NULL;}

// Check the port device's path to see if it is connected to an external device, not to e.g. a modem. 
// The Arduino's serial port will return a path like "\Device\Serial2"
char devicePath[MAX_PATH] = "";
QueryDosDevice(portname, devicePath, MAX_PATH);
if(!strstr(devicePath, "Device\\Serial")) {CloseHandle(hSerial);  return NULL;}

// Prepare serial port configuration.
DCB dcb = {0};
dcb.DCBlength = sizeof(dcb);
// Load current control settings:
GetCommState(hSerial, &dcb);
// Change some settings to match Arduino:
dcb.BaudRate = 9600;
dcb.ByteSize = 8;
dcb.StopBits = ONESTOPBIT;
dcb.Parity = NOPARITY;
// Set 'DTR_CONTROL_DISABLE' to prevent resetting the Arduino every time SetCommState() reconnects.
dcb.fDtrControl = DTR_CONTROL_DISABLE;

// Configure the serial port for communication:
if(!SetCommState(hSerial, &dcb)) {
  CloseHandle(hSerial);
  showWarning("could not set serial port parameters");
  return NULL;
}
// If successful, purge the serial port buffer and start with a clean slate:
PurgeComm(hSerial, PURGE_RXCLEAR);
return hSerial;
}



int sendtoPort(HANDLE serialPort, const char *sendText) {
/*
 Feeds text characters to the connected serial port for the Arduino to read.
 The characters will be cued in a buffer until the Arduino program has read them all.
 Windows serial ports can cue a maximum of 64 unread characters.
*/
if(!serialPort) {return 0;}
DWORD bytesSent = 0;  
if(!WriteFile(serialPort, sendText, strlen(sendText), &bytesSent, NULL)) {return 0;}
// If you are using Serial.parseInt() in the Arduino's program,
// send a non-digit at the end to tell parseInt() where the serial data ends,
// otherwise its standard timeout will wait one second longer than necessary.
return WriteFile(serialPort, "\n", 1, &bytesSent, NULL);
}



char readfromPort(HANDLE serialPort) {
/*
 Reads one character from the connected serial port and returns it.
*/
if(!serialPort) {return 0;}
// Pre-check if there is any data waiting in the serial port's buffer.
COMSTAT serialStatus = {0};
DWORD errors = 0;
ClearCommError(serialPort, &errors, &serialStatus);
// If there is no data waiting, forego ReadFile(), 
// or it would freeze this program until data is received.
if(serialStatus.cbInQue) {
  char readBuffer[1] = "";   // buffer to store character(s) from the serial port
  DWORD bytesRead = 0;       // will be set by ReadFile()
  // Read 1 character into our buffer and return the buffer's first (and only) character:
  if(ReadFile(serialPort, readBuffer, 1, &bytesRead, NULL)) {return readBuffer[0];}
}
return 0;
}



void closePort(HANDLE serialPort) {
if(!serialPort) {return;}
// Transmit all remaining buffered output before closing the serial port.
FlushFileBuffers(serialPort);
CloseHandle(serialPort);
}



int main() {
// Open the Arduino's serial port for reading and writing. The port number may vary.
// The last plugged-in device typically has the highest port number,
// so try each port from highest to 0 until we find one we can connect to.
HANDLE Arduino = NULL;
for(int nr = 20; nr >= 0; nr--) {
  if(Arduino = connecttoPort(nr)) {break;}
}
if(!Arduino) {showWarning("could not connect to serial port"); return 0;}
// Mind that if this is the first time we connect to the serial port, 
// the Arduino will spend several seconds resetting and clears the serial port,
// so we can not send it a serial message right away.

// Just for testing, end this program after now + x seconds.
time_t endTime = time(0) + 7;

// Buffer to accumulate received text:
const unsigned MAX_LENGTH = 100;
char readtext[MAX_LENGTH+1] = "";   //+1 for the closing '\0'
unsigned textlength = 0;

while(!keyPressed(VK_ESCAPE) && time(0) < endTime) {
  Sleep(100);  // (Arbitrary delay inbetween reads)

  // Read characters from the serial port one by one.
  // Have the Arduino send us characters with e.g. "Serial.println("123text");".
  // Note that 'Serial.println("text")' in Arduino sends an additional 
  // newline character ('\n') and carriage return ('\r') at the end of each message.
  if(char readchar = readfromPort(Arduino)) {
    // Add each received character to our text buffer:
    if(isprint(readchar) && textlength < MAX_LENGTH) {
      readtext[textlength++] = readchar;
    }

    // You can trigger things in response to individual characters.
    // e.g. make the computer beep upon receiving a certain character:
    if(readchar == 'b') {Beep(500,100);}

    // Or you can do something with the accumulated text upon reading a newline character:
    else if(readchar == '\n') {
      // Do something with the text:
      if(textlength > 0) {
        MessageBox(NULL, readtext, "message received from Arduino", MB_SETFOREGROUND | MB_OK);
      }

      // Convert the received text to a number (sets 0 if not a number).
      // (Note: atoi() can cause bugs if the number exceeds max int value. strtol() is safe)
      long int nr = strtol(readtext, NULL, 10);
      // Do something if we received a number:
      if(nr != 0 || readtext[0] == '0') {
        // doSomethingWith(nr);
      }

      // and/or send a message back to the Arduino:
      sendtoPort(Arduino, "message received!!!");

      // Prepare to read the next message:
      strcpy(readtext, "");
      textlength = 0;
    }
  }  // read a character from the serial port.

}  // end of while loop

// Close the serial port connection and exit the program.
closePort(Arduino);
return 1;
}


