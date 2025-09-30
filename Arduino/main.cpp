#include <avr/wdt.h>

// L298N Motor Driver A (Left Motor)
int IN1 = 9;  // Direction pin 1 for Motor A
int IN2 = 8;  // Direction pin 2 for Motor A
int ENA = 11; // PWM pin for Motor A speed control

// L298N Motor Driver B (Right Motor)
int IN3 = 3;  // Direction pin 1 for Motor B
int IN4 = 2;  // Direction pin 2 for Motor B
int ENB = 10; // PWM pin for Motor B speed control

// Ultrasonic sensor pins
int trigPin = 7;
int echoPin = 4;
long duration;
int distance;

// Conveyor belt motor (separate L298N or additional motor)
int IN5 = 12; // Direction pin 1 for Conveyor Motor
int IN6 = 13; // Direction pin 2 for Conveyor Motor
int ENC = 5;  // PWM pin for Conveyor Motor speed control

// ESP pins (reassigned to avoid conflicts)
int ESP1 = A0;
int ESP2 = A1;
int ESP3 = A2;

int currentSpeed = 80;
unsigned long conveyorStartTime = 0;
bool conveyorRunning = false;
String lastStage = "";

void setup()
{

  wdt_disable();

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  // Set pin modes for Motor A (Left)
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(ENA, OUTPUT);

  // Set pin modes for Motor B (Right)
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  pinMode(ENB, OUTPUT);

  // Set pin modes for Conveyor Belt motor
  pinMode(IN5, OUTPUT);
  pinMode(IN6, OUTPUT);
  pinMode(ENC, OUTPUT);

  pinMode(ESP1, INPUT);
  pinMode(ESP2, INPUT);
  pinMode(ESP3, INPUT);

  Serial.begin(9600);
  delay(500);

  wdt_enable(WDTO_8S);
}

void loop()
{

  int dist = readDistance();
  if (dist < 35)
  {
    wdt_reset();
    conveyorRunning = true;
    conveyorStartTime = millis();
  }
  if (conveyorRunning)
  {
    conveyorBackward();
  }

  if (conveyorRunning && millis() - conveyorStartTime >= 5000)
  {
    conveyorStop();
    conveyorRunning = false;
    Serial.println("Conveyor Stopped after 5s");
  }

  int b1 = digitalRead(ESP1);
  int b2 = digitalRead(ESP2);
  int b3 = digitalRead(ESP3);

  String bits = String(b3) + String(b2) + String(b1);

  // Only update if different from last stage
  if (bits != lastStage)
  {
    lastStage = bits;
    wdt_reset();

    if (bits == "000")
    {
      stopMotors();
      Serial.println("STOP");
    }
    else if (bits == "001")
    {
      forward();
      Serial.println("FORWARD");
    }
    else if (bits == "010")
    {
      backward();
      Serial.println("BACKWARD");
    }
    else if (bits == "011")
    {
      left();
      Serial.println("LEFT");
    }
    else if (bits == "100")
    {
      right();
      Serial.println("RIGHT");
    }
  }

  delay(100);
}

int readDistance()
{
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  duration = pulseIn(echoPin, HIGH, 30000);
  distance = duration * 0.034 / 2; // Convert to cm
  if (duration == 0)
  {
    return 999;
  }
  return distance;
}

void conveyorBackward()
{
  digitalWrite(IN5, LOW);
  digitalWrite(IN6, HIGH);
  analogWrite(ENC, 80); 
}

void conveyorStop()
{
  digitalWrite(IN5, LOW);
  digitalWrite(IN6, LOW);
  analogWrite(ENC, 0); 
}

void forward()
{
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  analogWrite(ENA, currentSpeed); 

  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
  analogWrite(ENB, currentSpeed); 
}

void backward()
{
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  analogWrite(ENA, currentSpeed); 

  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
  analogWrite(ENB, currentSpeed); 
}

void left()
{
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  analogWrite(ENA, currentSpeed);

  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
  analogWrite(ENB, currentSpeed);
}

void right()
{
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  analogWrite(ENA, currentSpeed);

  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
  analogWrite(ENB, currentSpeed);
}

void stopMotors()
{
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  analogWrite(ENA, 0); // Set speed to 0

  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
  analogWrite(ENB, 0); // Set speed to 0
}