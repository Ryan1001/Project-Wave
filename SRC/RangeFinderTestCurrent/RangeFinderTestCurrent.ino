#include <Filter.h>
#include <firFilter.h>
#include <iirFilter.h>
#include <medianFilter.h>
#include <newFilter.h>

/*
Project Wave
by Ryan Brill and Evan Brill
Last updated June 2, 2018 at home.

    Ultrasonic sensor Pins:
        VCC: +5VDC
        Trig : Trigger (INPUT) - Pin11
        Echo: Echo (OUTPUT) - Pin 12
        GND: GND
*/
//---- In this part, I declare all of the variables that I'm going to use in the loop() function----
  const int numReadings = 30;

  int readings[numReadings];      // the readings from the analog input
  int readIndex = 0;              // the index of the current reading
  int total = 0;                  // the running total

  int upperLimit = 5000;  //the upper admissible signal reading. Helps prevent wild readings from being passed through.
  int lowerLimit = 0; //you get the idea

  int trigPin = 11;    //Trig - green Jumper
  int echoPin = 12;    //Echo - yellow Jumper
  long duration, output; //duration is the output of the sensor
  int average = 0;                // the output of the moving average function 
  int gatedOutput = 0;                 // output of sensor after gating
  int medianFiltered;     //after we gate the output, we put it through a median Filter

  medianFilter thisMedianFilter;


  //MIDI variables
  byte noteON = 144;//note on command
  byte noteOFF = 128;//note off command

//-----In this part, I tell the arduino which pins to listen on (Input) or to send messages out of (Output)
void setup() {
  //Serial Port begin
  Serial.begin (9600);
    thisMedianFilter.begin();

  //Define inputs and outputs
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  // initialize all the readings to 0 for signal smoothing function:
  for (int thisReading = 0; thisReading < numReadings; thisReading++) {
    readings[thisReading] = 0;
  }
}

//---- This loop function repeats (loops) as long as the arduino is powered.
// I basically tell the arduino to read the sensor, then I smooth out the signal and remove values that are too high to be reasonable
void loop()
{
  // TRIGGER SENSOR AND READ DATA IN
    // The sensor is triggered by a HIGH pulse of 10 or more microseconds.
    // Give a short LOW pulse beforehand to ensure a clean HIGH pulse:
    digitalWrite(trigPin, LOW);
    delayMicroseconds(5);
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);

    // Read the signal from the sensor: a HIGH pulse whose
    // duration is the time (in microseconds) from the sending
    // of the ping to the reception of its echo off of an object.
    pinMode(echoPin, INPUT);
    duration = pulseIn(echoPin, HIGH);

    medianFiltered= thisMedianFilter.run(duration);

  // -----SMOOTHING ----
    ///// begin smoothing function
    // subtract the last reading:
    total = total - readings[readIndex];
    // read from the sensor:
    readings[readIndex] = medianFiltered;
    // add the reading to the total:
    total = total + readings[readIndex];
    // advance to the next position in the array:
    readIndex = readIndex + 1;

    // if we're at the end of the array...
    if (readIndex >= numReadings) {
      // ...wrap around to the beginning:
      readIndex = 0;
    }

    // calculate the average:
    average = total / numReadings;
    // send it to the computer as ASCII digits

//    delay(1);        // delay in between reads for stability

    //  SMOOTHING function END ---

  //remove high and low values. 
    //Sensor tends to report crazy values on changes - this and smoothing corrects that.
    if (average < upperLimit && average > lowerLimit) {
      gatedOutput = average;
    };

    
//  gatedOutput = map(gatedOutput, 0, 3300, 0, 127);

  Serial.println(gatedOutput);

  delay(20);

//Send MIDI messages with our sensor data. 
  // Note that you need to comment out the first line if you want to get Serial Plotter to display correctly.

//MIDImessage(noteON, 60, medianFiltered);//turn note on
//    delay(300);//hold note for 300ms
//    MIDImessage(noteOFF, 60, gatedOutput);//turn note off (note on with velocity 0)
//    delay(200);//wait 200ms until triggering next note
}

//send MIDI message. This is a helper method that is referenced in loop()
void MIDImessage(byte command, byte data1, byte data2) {
  Serial.write(command);
  Serial.write(data1);
  Serial.write(data2);
}


