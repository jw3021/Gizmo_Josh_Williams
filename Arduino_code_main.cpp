#include <Arduino.h>

// Josh Williams 2023, Gizmo, GigaSketch

// This code makes the workings of a giant etch a sketch.
// It uses two rotary encoders, two stepper motors and four
// limit switches to controll the interface.
// It then has a built in game function which allows the user
// to find a randomly selected point on the table.

// This code can be sampled and edited, but any work must be
// credited to Josh Williams.

// NOTE TO HELPFUL SOURCES:
// How to Mecatronics webpage 'Stepper motors and arduino' was
// very helpful to get me off and running with controlling stepper motors.
// link: https://howtomechatronics.com/tutorials/arduino/stepper-motors-and-arduino-the-ultimate-guide/

#include <LiquidCrystal.h>
// LCD libruary is required to make the lcd screen dispay
// setting the LCD pins
const int rs = 42, en = 43, d4 = 45, d5 = 47, d6 = 49, d7 = 51;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

// setting the stepper pins for X axis
#define dirPinX = 22
#define stepPinX = 23
// and for Y axis
#define dirPinY = 29
#define stepPinY = 28

// setting the led pin
#define ledPin 3
#define ledgreen 2
// setting the rotary encoder pins, as well as the
// step pins for each of the stepper motors
#define CLK_X 8
#define DT_X 7
#define ST_X 9
#define ms1X 26
#define ms2X 25
#define ms3X 24

#define CLK_Y 5
#define DT_Y 6
#define ST_Y 34
#define ms1Y 32
#define ms2Y 31
#define ms3Y 30
// setting the limit switch pins
#define switch1_X 10
#define switch2_X 11
#define switch1_Y 12
#define switch2_Y 13
// initiating millis for the led blink and the counter
unsigned long previousMillis = 0;
// making the distance to mine a global variable
float distanceToMine;
// setting the target values as global variables to be changed by the ramdon genarator later
int targetX;
int targetY;
const int movesY = 62; // the number of clicks per Y axis
const int movesX = 41; // the number of clicks per X axis

int MineCount = 0; // tracking how many mines have been found per game

// setting stepper settings, these are changed depending on how much
// and how quick you want the ball to move per click of the rotary encoder
const int stepsPerRevolution = 760;
int speed = 200;
// setting the rotory encoder last press value
unsigned long lastButtonPress = 0;
unsigned long lastButtonPressY = 0;

// setting the counters for the x and y position
int counterX = 0;
int currentStateCLK_X;
int lastStateCLK_X;
String currentDir_X = "";

int counterY = 0;
int currentStateCLK_Y;
int lastStateCLK_Y;
String currentDir_Y = "";

// setting the value for how close the ball gets before it
// is counted as being found
const int ballrange = 2;

// functions to home the position of the ball at the start
//  and when asked to.
void HomeX()
{
    while (digitalRead(switch1_X) == HIGH)
    {
        rotateCCW_X();
    }
    rotateCW_X();
    counterX = 0;
}

void HomeY()
{
    while (digitalRead(switch1_Y) == HIGH)
    {
        rotateCCW_Y();
    }
    rotateCW_Y();
    counterY = 0;
}
// functions for 1/16 stepping for the stepper motors.
void OneSixteenthStep_X()
{
    digitalWrite(ms1X, HIGH);
    digitalWrite(ms2X, HIGH);
    digitalWrite(ms3X, HIGH);
}

void OneSixteenthStep_Y()
{
    digitalWrite(ms1Y, HIGH);
    digitalWrite(ms2Y, HIGH);
    digitalWrite(ms3Y, HIGH);
}
// setup code
void setup()
{
    // Declare stepper pins as Outputs
    pinMode(stepPinX, OUTPUT);
    pinMode(dirPinX, OUTPUT);
    pinMode(stepPinY, OUTPUT);
    pinMode(dirPinY, OUTPUT);
    // setting the rotary encoder as input
    pinMode(CLK_X, INPUT);
    pinMode(DT_X, INPUT);
    pinMode(ST_X, INPUT_PULLUP); // pullu
    pinMode(CLK_Y, INPUT);
    pinMode(DT_Y, INPUT);
    pinMode(ST_Y, INPUT_PULLUP);
    // setting the LED pin to output
    pinMode(ledPin, OUTPUT);
    pinMode(ledgreen, OUTPUT);
    // set up the LCD's number of columns and rows:
    lcd.begin(16, 2);
    // Print a message to the LCD. This will remain constant
    // throughout the whole code
    lcd.print("Mines Found:");

    // declaring the limit switch pins as input
    pinMode(switch1_X, INPUT_PULLUP); // Set switch 1 as an input, using pullup to give a high logic level
    pinMode(switch2_X, INPUT_PULLUP);
    pinMode(switch1_Y, INPUT_PULLUP);
    pinMode(switch2_Y, INPUT_PULLUP);

    Serial.begin(9600);
    // Reads the initial state of the outputA
    lastStateCLK_X = digitalRead(CLK_X);
    lastStateCLK_Y = digitalRead(CLK_Y);
    // setting the motors to 1/16 step using the functions created
    OneSixteenthStep_X();
    OneSixteenthStep_Y();
    // changing the speed during homing so that it dostn't take too long
    // this could be made faster depending on your size of board
    speed = 100;
    HomeX();
    HomeY();
    speed = 200;
    delay(1000);

    // randomly selecting the location of the first mine
    // Note that the randonseed is used to create a different
    // set of random numbers each time the code is run.
    randomSeed(analogRead(0));
    targetY = random(0, 62);
    targetX = random(0, 41);

    // Print the randomly generated coordinates for debugging
    Serial.print("Randomly generated coordinates: ");
    Serial.print("targetX = ");
    Serial.print(targetX);
    Serial.print(", targetY = ");
    Serial.println(targetY);
}
// Function to calculate the straight line distance to the Mine point.
// This uses the Eclidean distance formula.
void calculateAndPrintDistance(int x1, int y1, int x2, int y2)
{
    // Boundary check to keep the ball within the grid.
    x1 = constrain(x1, 0, movesX);
    y1 = constrain(y1, 0, movesY);

    // Euclidean distance formula
    distanceToMine = sqrt(pow(x2 - x1, 2) + pow(y2 - y1, 2));
    // Print the distance (this is only for de-bugging):
    Serial.print("Distance is: ");
    Serial.println(distanceToMine);
    // Checking if the ball is within the specified range.
    // If the ball is, then add one to the counter and re-assign a new
    // random variable for the Mine position.
    if (distanceToMine < ballrange)
    {
        Serial.println("Mine found");
        MineCount = MineCount + 1;
        Serial.println(MineCount);
        Serial.print("is now your MineCount");
        // turn the green led on when the Mine has been found
        digitalWrite(ledgreen, HIGH);
        delay(1500);
        digitalWrite(ledgreen, LOW);

        targetY = random(0, 62);
        targetX = random(0, 41);
        Serial.print("targetX = ");
        Serial.print(targetX);
        Serial.print(", targetY = ");
        Serial.println(targetY);
    }
}
// These are the four functions which make the motor spin either way
void rotateCW_X()
{
    digitalWrite(dirPinX, LOW);
    for (int x = 0; x < stepsPerRevolution; x++)
    {
        digitalWrite(stepPinX, HIGH);
        delayMicroseconds(speed);
        digitalWrite(stepPinX, LOW);
        delayMicroseconds(speed);
    }
}
void rotateCCW_X()
{
    digitalWrite(dirPinX, HIGH);
    for (int x = 0; x < stepsPerRevolution; x++)
    {
        digitalWrite(stepPinX, HIGH);
        delayMicroseconds(speed);
        digitalWrite(stepPinX, LOW);
        delayMicroseconds(speed);
    }
}
void rotateCW_Y()
{
    digitalWrite(dirPinY, LOW);
    for (int x = 0; x < stepsPerRevolution; x++)
    {
        digitalWrite(stepPinY, HIGH);
        delayMicroseconds(speed);
        digitalWrite(stepPinY, LOW);
        delayMicroseconds(speed);
    }
}
void rotateCCW_Y()
{
    digitalWrite(dirPinY, HIGH);
    for (int x = 0; x < stepsPerRevolution; x++)
    {
        digitalWrite(stepPinY, HIGH);
        delayMicroseconds(speed);
        digitalWrite(stepPinY, LOW);
        delayMicroseconds(speed);
    }
}
// These are the two main functions to be run which allow the
// motors to be spun in either direction.
void MoveBothWays_Y()
{
    currentStateCLK_X = digitalRead(CLK_X); // Reads the state of the outputA.
    // If the previous and the current state of the outputA are different, that means a Pulse has occured
    if (currentStateCLK_X != lastStateCLK_X && currentStateCLK_X == 1)
    {

        // If the DT state is different than the CLK state then
        // the encoder is rotating CCW so decrement
        if (digitalRead(DT_X) != currentStateCLK_X)
        {
            counterX--;
            currentDir_X = "CCW";
            rotateCCW_X();
            counterX = constrain(counterX, 0, movesX);
            calculateAndPrintDistance(counterX, counterY, targetX, targetY);
        }
        else
        {
            // Encoder is rotating CW so increment
            counterX++;
            currentDir_X = "CW";
            rotateCW_X();
            counterX = constrain(counterX, 0, movesX);
            calculateAndPrintDistance(counterX, counterY, targetX, targetY);
        }

        Serial.print("X-Axis - Direction: ");
        Serial.print(currentDir_X);
        Serial.print(" | Counter: ");
        Serial.println(counterX);
    }

    // Remember last CLK state
    lastStateCLK_X = currentStateCLK_X;
    delay(1);
}

void MoveBothWays_X()
{
    currentStateCLK_Y = digitalRead(CLK_Y); // Reads the state of the outputA
    // If the previous and the current state of the outputA are different, that means a Pulse has occured
    if (currentStateCLK_Y != lastStateCLK_Y && currentStateCLK_Y == 1)
    {

        // If the DT state is different than the CLK state then
        // the encoder is rotating CCW so decrement
        if (digitalRead(DT_Y) != currentStateCLK_Y)
        {
            counterY--;
            currentDir_Y = "CCW";
            rotateCCW_Y();
            counterY = constrain(counterY, 0, movesY);

            calculateAndPrintDistance(counterX, counterY, targetX, targetY);
        }
        else
        {
            // Encoder is rotating CW so increment
            counterY++;
            currentDir_Y = "CW";
            rotateCW_Y();
            counterY = constrain(counterY, 0, movesY);

            calculateAndPrintDistance(counterX, counterY, targetX, targetY);
        }

        Serial.print("Y-Axis - Direction: ");
        Serial.print(currentDir_Y);
        Serial.print(" | Counter: ");
        Serial.println(counterY);
    }

    // Remember last CLK state
    lastStateCLK_Y = currentStateCLK_Y;
    delay(1);
}
// the main code is placed here, however note that is it made largly
// of functions that have already been created.
void loop()
{
    // reading the current values of the stopper switches.

    int switchState1_X = digitalRead(switch1_X); // Read switch 1
    int switchState2_X = digitalRead(switch2_X); // Read switch 2
    int switchState1_Y = digitalRead(switch1_Y); // Read switch 1
    int switchState2_Y = digitalRead(switch2_Y); // Read switch 2
    // setting the bounce when the ball hits the limits of the board.
    if (switchState1_X == LOW)
    {
        // switch x1 is pressed
        Serial.println("Switch1 X is pressed, turn CW");
        // MoveCW();
        rotateCW_X();
        delay(1);
    }
    else if (switchState2_X == LOW)
    {
        // switch x2 is pressed
        Serial.println("Switch2 X is pressed, turn CCW");
        // MoveCCW();
        rotateCCW_X();
        delay(1);
    }
    else if (switchState1_Y == LOW)
    {
        // switch Y1 is pressed
        Serial.println("Switch1 Y is pressed, turn CW");
        // MoveCW();
        rotateCW_Y();
        delay(1);
    }
    else if (switchState2_Y == LOW)
    {
        // switch Y2 is pressed
        Serial.println("Switch2 X is pressed, turn CCW");
        // MoveCCW();
        rotateCCW_Y();
        delay(1);
    }
    // if none of the switches are pressed, then run the functions to allow the
    // ball to move both ways.
    else
    {
        MoveBothWays_X();
        MoveBothWays_Y();
    }

    int btnStateX = digitalRead(ST_X);

    // If we detect LOW signal, button is pressed. When this happens
    // the game returns the ball to 0,0.
    if (btnStateX == LOW)
    {
        // if 50ms have passed since last LOW pulse, it means that the
        // button has been pressed, released and pressed again
        if (millis() - lastButtonPress > 50)
        {
            Serial.println("Button X pressed!");
            speed = 100;
            HomeX();
            HomeY();
            speed = 200;
        }

        // Remember last button press event
        lastButtonPress = millis();
    }
    int btnStateY = digitalRead(ST_Y);

    // If we detect LOW signal, button is pressed
    if (btnStateY == LOW)
    {
        // if 50ms have passed since last LOW pulse, it means that the
        // button has been pressed, released and pressed again
        if (millis() - lastButtonPressY > 50)
        {
            Serial.println("Button Y pressed!");
            MineCount = 0;
            lcd.setCursor(0, 1);
            lcd.print("Reset game!");
            delay(5000);
            lcd.setCursor(0, 1);
            lcd.print("              ");
        }

        // Remember last button press event
        lastButtonPressY = millis();
    }
    // this is calculating appopriate flash rate using a map function
    int flashRate = map(distanceToMine, 0, 70, 100, 2000);

    // Check if it's time to update the LED.
    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= flashRate)
    {
        // Save the current time
        previousMillis = currentMillis;
        // Toggle the LED state
        digitalWrite(ledPin, !digitalRead(ledPin));
    }
    lcd.setCursor(0, 1);
    // print the number of seconds since reset:
    lcd.print(MineCount);
}
