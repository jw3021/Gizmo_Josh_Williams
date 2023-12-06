// Josh Williams 2023, GigaSketch

// This code is used to turn a dc motor on while
// the small etch a sketch is shaken by the user.
// This is stimulated by a piezo disk which is taped
// to the back of the etch a sketch.

// This code can be sampled and edited, but any work must be
// credited to Josh Williams.

// Physical Computing (Gizmo) 2023
// Dyson School of Design Engineering, Imperial College London
// Module leader: Andrew McPherson

#include <Arduino.h>
#include <PdArduino.h>
// setting the pins for the motor and for the piezo input
const int motor1 = 2;
const int motor2 = 3;
const int piezo = 0;
// the threshold for the piezo reading
const float threshold = 0.7;
// the time which the motor spins for each time the threshold is reached
const int duration = 1000;

void setup()
{
    pinMode(motor1, OUTPUT);
    pinMode(motor2, OUTPUT);
    pinMode(piezo, INPUT);
}
// a function to spin a dc motor in one direction for a set duration
void runMotorForDuration(unsigned long duration)
{
    pwmWrite(motor1, 0.0);
    pwmWrite(motor2, 0.9);
    delay(duration);           // Wait for the specified duration
    digitalWrite(motor1, LOW); // Turn off the motor
    pwmWrite(motor1, 0.0);
    pwmWrite(motor2, 0.0);
}

void loop()
{
    // checking to see if the piezo reading has reached the threshold
    // If so, the motor should spin
    // Print to terminal for debugging
    if (analogRead(piezo) > threshold)
    {
        Serial.println("activated");

        runMotorForDuration(duration);
    }
    else
    {
        Serial.println("not activated");
    }
    delay(10);
}
