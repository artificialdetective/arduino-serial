/*
 Reads and sends serial port data from/to a computer.
 Run this program on an Arduino.
 This program uses standard Arduino functions.
*/

const int ledPin = 13;

void setup() {
 // Start receiving input as soon as a connection has been established.
 // send and receive at 9600 baud, about 1000 characters per second.
 Serial.begin(9600); 
 // Set up pin 13 on the Arduino to output a voltage.
 pinMode(ledPin, OUTPUT);
}


void loop() {

//Read characters from the serial port:
if(Serial.available() > 0) {
  // Read one character at a time.
  int serialInput = Serial.read();

  // Trigger actions depending on which character we received:
  switch(serialInput) {

    case '1': {
      // Turn led on when we receive a "1" character:
      digitalWrite(ledPin, HIGH);
      break;
    }

    case '0': {
      // Turn led off:
      digitalWrite(ledPin, LOW);
      break;
    }

    case '!': {
      // Blink led light:
      digitalWrite(ledPin, HIGH);
      delay(500);
      digitalWrite(ledPin, LOW);
      delay(500);
      break;
    }

  }   //end of switch
 
}   //if incoming serial data

else {
  // When not receiving anything, send a message to the serial port every second.
  // Serial.println() automatically adds a newline '\n' and carriage return '\r' character, Serial.print() does not.
  Serial.println("b-e-e-p");
  delay(1000);
}

}   // end of main loop
