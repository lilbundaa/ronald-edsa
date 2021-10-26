#include <Servo.h>
#define SERVORIGHT   50
#define SERVOCENTRE  100
#define SERVOLEFT    150
#define SERVOPIN     16
#define TRIGPIN      2    
#define ECHOPIN      0    
 
#define RightMotorSpeedPin 4
#define RightMotorDirPin   2 
#define LeftMotorSpeedPin  5
#define LeftMotorDirPin    0
 
int Speed = 100;  // max 1024
int TSpeed = 90;  //Turning Speed
Servo servo;
 
void stoped()
{
  //Apply speed zero for stopping motors
  
    analogWrite(RightMotorSpeedPin, 0);
    analogWrite(LeftMotorSpeedPin, 0);
    Serial.println("Stop");
}
 
void forward()
{
    analogWrite(RightMotorSpeedPin, Speed);
    analogWrite(LeftMotorSpeedPin, Speed);
    digitalWrite(RightMotorDirPin, HIGH);
    digitalWrite(LeftMotorDirPin, HIGH);
    Serial.println("forward");
}
 
void back()
{
    analogWrite(RightMotorSpeedPin, Speed);
    analogWrite(LeftMotorSpeedPin, Speed);
    digitalWrite(RightMotorDirPin, LOW);
    digitalWrite(LeftMotorDirPin, LOW);
    Serial.println("Back");
}
 
void left()
{
    analogWrite(RightMotorSpeedPin, TSpeed);
    analogWrite(LeftMotorSpeedPin, TSpeed);
    digitalWrite(RightMotorDirPin, LOW);
    digitalWrite(LeftMotorDirPin, HIGH);
    Serial.println("Left");
}
 
void right()
{
    analogWrite(RightMotorSpeedPin, TSpeed);
    analogWrite(LeftMotorSpeedPin, TSpeed);
    digitalWrite(RightMotorDirPin, HIGH);
    digitalWrite(LeftMotorDirPin, LOW);
    Serial.println("right");
}
 
int stopCount = 0;
 
int ping()
{
    // pause for 50ms between scans
    delay(50);
 
    // send ping
    digitalWrite(TRIGPIN, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIGPIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIGPIN, LOW);
 
    // read echo
    long duration = pulseIn(ECHOPIN, HIGH);
 
    // convert distance to cm
    unsigned int centimetres = int(duration / 2 / 29.1);
 
    return centimetres;
}
 
char scan()
{
    // ping times in microseconds
    unsigned int left_scan, centre_scan, right_scan;
    char choice;
 
    // scan left
    servo.write(SERVOLEFT);
    delay(300);
    left_scan = ping();
 
    // scan right
    servo.write(SERVORIGHT);
    delay(600);
    right_scan = ping();
 
    // scan straight ahead
    servo.write(SERVOCENTRE);
    delay(300);
    centre_scan = ping();
 
    if (left_scan>right_scan && left_scan>centre_scan)
    {
        choice = 'L';
    }
    else if (right_scan>left_scan && right_scan>centre_scan)
    {
        choice = 'R';
    }
    else {
      choice = 'C';
    }
 
    return choice;
}
 
void setup() {
  // put your setup code here, to run once:
   Serial.begin(9600);
    Serial.println("Obstacle Avoiding Car v1.0");
    // set the servo data pin
    servo.attach(SERVOPIN);
 
    pinMode(RightMotorSpeedPin, OUTPUT);
    pinMode(LeftMotorSpeedPin, OUTPUT);
    pinMode(RightMotorDirPin, OUTPUT);
    pinMode(LeftMotorDirPin, OUTPUT);
    digitalWrite(RightMotorSpeedPin, LOW);
    digitalWrite(LeftMotorSpeedPin, LOW);
    digitalWrite(RightMotorDirPin, HIGH);
    digitalWrite(LeftMotorDirPin, HIGH);
    // set the trig pin to output (send sound waves)
    pinMode(TRIGPIN, OUTPUT);
 
    // set the echo pin to input (receive sound waves)
    pinMode(ECHOPIN, INPUT);
}
 
void loop() {
  // put your main code here, to run repeatedly:
    // get distance from obstacle straight ahead
    unsigned int distance = ping();
    Serial.print("Distance: "); Serial.println(distance);
    if (distance < 30 && distance > 0)
    {
        if (distance < 10)
        {
            // turn around
            Serial.println("Turn around..."); 
           // display.drawString(10, 40, "Turn around...") ;         
            back();
            delay(300);
            left();
            delay(700);
        }
        else
        {
            // stop both motors
            Serial.println("Motor stop...");
            stoped();
            
            // scan for obstacles
            char turn_direction = scan();
 
            // turn left/right or ignore and go straight
            if (turn_direction == 'L')
            {
              Serial.println("Turn left...");
                left();
                delay(500);
            }
            else if (turn_direction == 'R')
            {
              Serial.println("Turn right...");
                right();
                delay(500);
            }
            else if (turn_direction == 'C')
            {
              stopCount++;
              if(stopCount > 3){
                stopCount = 0;
                Serial.println("Turn back...");
                right();
                delay(700);
              }
              
            }
        }
    }
    else
    {
        // no obstacle, keep going forward
        Serial.println("No obstacle, keep going forward...");
        forward();
    }
}
