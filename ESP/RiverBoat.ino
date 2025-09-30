#include <OneWire.h>
#include <DallasTemperature.h>
#include "thingProperties.h"

// Direction management variables
enum Direction
{
    STOP = 0,
    FORWARD = 1,
    BACKWARD = 2,
    LEFT = 3,
    RIGHT = 4
};

Direction currentDirection = STOP;

// GPIO pins for 3-bit direction output (ESP32 compatible)
#define DIR_PIN_0 12 // Bit 0
#define DIR_PIN_1 13 // Bit 1
#define DIR_PIN_2 14 // Bit 2
#define TEMP_PIN 32  // Temperature

OneWire ourWire(TEMP_PIN);
DallasTemperature sensor(&ourWire);

void setup()
{
    // Initialize serial and wait for port to open:
    Serial.begin(9600);

    // Initialize GPIO pins for direction output
    pinMode(DIR_PIN_0, OUTPUT);
    pinMode(DIR_PIN_1, OUTPUT);
    pinMode(DIR_PIN_2, OUTPUT);

    pinMode(TEMP_PIN, INPUT);

    // Set initial state to STOP (000)
    digitalWrite(DIR_PIN_0, LOW);
    digitalWrite(DIR_PIN_1, LOW);
    digitalWrite(DIR_PIN_2, LOW);

    // This delay gives the chance to wait for a Serial Monitor without blocking if none is found
    delay(1500);
    // Defined in thingProperties.h
    initProperties();
    // Connect to Arduino IoT Cloud
    ArduinoCloud.begin(ArduinoIoTPreferredConnection);

    /*
       The following function allows you to obtain more information
       related to the state of network and IoT Cloud connection and errors
       the higher number the more granular information you'll get.
       The default is 0 (only errors).
       Maximum is 4
   */
    setDebugMessageLevel(2);
    ArduinoCloud.printDebugInfo();
    sensor.begin();
    Serial.println("I'm ready!");
}

void loop()
{
    ArduinoCloud.update();
    // Your code here

    // Execute movement based on current direction
    executeMovement();

    // Temperature Sections
    sensor.requestTemperatures();
    temperature = sensor.getTempCByIndex(0);
}

void outputDirection(Direction dir)
{
    // Output 3-bit direction code to GPIO pins
    switch (dir)
    {
    case STOP: // 000
        digitalWrite(DIR_PIN_0, LOW);
        digitalWrite(DIR_PIN_1, LOW);
        digitalWrite(DIR_PIN_2, LOW);
        ui_movement = "000 (STOP)";
        Serial.println("000 (STOP)");
        break;
    case FORWARD: // 001
        digitalWrite(DIR_PIN_0, HIGH);
        digitalWrite(DIR_PIN_1, LOW);
        digitalWrite(DIR_PIN_2, LOW);
        ui_movement = "001 (FORWARD)";
        Serial.println("001 (FORWARD)");
        break;
    case BACKWARD: // 010
        digitalWrite(DIR_PIN_0, LOW);
        digitalWrite(DIR_PIN_1, HIGH);
        digitalWrite(DIR_PIN_2, LOW);
        ui_movement = "010 (BACKWARD)";
        Serial.println("010 (BACKWARD)");
        break;
    case LEFT: // 011
        digitalWrite(DIR_PIN_0, HIGH);
        digitalWrite(DIR_PIN_1, HIGH);
        digitalWrite(DIR_PIN_2, LOW);
        ui_movement = "011 (LEFT)";
        Serial.println("011 (LEFT)");
        break;
    case RIGHT: // 100
        digitalWrite(DIR_PIN_0, LOW);
        digitalWrite(DIR_PIN_1, LOW);
        digitalWrite(DIR_PIN_2, HIGH);
        ui_movement = "100 (RIGHT)";
        Serial.println("100 (RIGHT)");
        break;
    }
}

void setDirection(Direction newDirection)
{
    // Clear all movement flags first
    move_F = false;
    move_B = false;
    move_L = false;
    move_R = false;

    // Set the new direction
    currentDirection = newDirection;

    // Output direction to GPIO pins
    outputDirection(currentDirection);

    // Set the appropriate flag based on direction
    switch (currentDirection)
    {
    case FORWARD:
        move_F = true;
        break;
    case BACKWARD:
        move_B = true;
        break;
    case LEFT:
        move_L = true;
        break;
    case RIGHT:
        move_R = true;
        break;
    case STOP:
    default:
        break;
    }
}

void executeMovement()
{
    // Add your motor control code here based on currentDirection
    switch (currentDirection)
    {
    case FORWARD:
        // Add forward movement code
        break;
    case BACKWARD:
        // Add backward movement code
        break;
    case LEFT:
        // Add left turn code
        break;
    case RIGHT:
        // Add right turn code
        break;
    case STOP:
    default:
        // Stop all motors
        break;
    }
}

/*
  Since Temperature is READ_WRITE variable, onTemperatureChange() is
  executed every time a new value is received from IoT Cloud.
*/
void onTemperatureChange()
{
    // Add your code here to act upon Temperature change
}

/*
  Since MoveF is READ_WRITE variable, onMoveFChange() is
  executed every time a new value is received from IoT Cloud.
*/
void onMoveFChange()
{
    if (move_F)
    {
        setDirection(FORWARD);
    }
    else if (currentDirection == FORWARD)
    {
        setDirection(STOP);
    }
}

/*
  Since MoveB is READ_WRITE variable, onMoveBChange() is
  executed every time a new value is received from IoT Cloud.
*/
void onMoveBChange()
{
    if (move_B)
    {
        setDirection(BACKWARD);
    }
    else if (currentDirection == BACKWARD)
    {
        setDirection(STOP);
    }
}

/*
  Since MoveL is READ_WRITE variable, onMoveLChange() is
  executed every time a new value is received from IoT Cloud.
*/
void onMoveLChange()
{
    if (move_L)
    {
        setDirection(LEFT);
    }
    else if (currentDirection == LEFT)
    {
        setDirection(STOP);
    }
}

/*
  Since MoveR is READ_WRITE variable, onMoveRChange() is
  executed every time a new value is received from IoT Cloud.
*/
void onMoveRChange()
{
    if (move_R)
    {
        setDirection(RIGHT);
    }
    else if (currentDirection == RIGHT)
    {
        setDirection(STOP);
    }
}

/*
  Since UiMovement is READ_WRITE variable, onUiMovementChange() is
  executed every time a new value is received from IoT Cloud.
*/
void onUiMovementChange()
{
}
/*
  Since UiMessenger is READ_WRITE variable, onUiMessengerChange() is
  executed every time a new value is received from IoT Cloud.
*/
void onUiMessengerChange()
{
    // Add your code here to act upon UiMessenger change
}