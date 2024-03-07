/*
Elation Sports Technologies LLC
Austin Allen
27 Feb 2024

Single Axis Joystick Demo Unit

Copyright (c) 2024 Elation Sports Technologies LLC

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.

*/

#include <AccelStepper.h>
#include <Servo.h>
#include <movingAvg.h>
#include <SoftPWM.h>

const int moving_avg_window = 5;
movingAvg joy_1(moving_avg_window);
movingAvg joy_2(moving_avg_window);
movingAvg joy_3(moving_avg_window);
 
Servo myservo;

const int pin_LED_R = 2;
const int pin_LED_G = 3;
const int pin_LED_B = 4;

int value_LED_R = 0;
int value_LED_G = 0;
int value_LED_B = 0;

const int pin_STEP = 7;
const int pin_DIR = 8;

const int pin_SERVO = 5;

const int pin_JOY_1 = A0;
const int pin_JOY_2 = A1;
const int pin_JOY_3 = A2;

int value_JOY1 = 0;
int value_JOY2 = 0;
int value_JOY3 = 0;

//Value read limits based on mechanical constraints
int JOY1_min = 0;
int JOY1_max = 1050;
int JOY2_min = 25;
int JOY2_max = 995;
int JOY3_min = 160;
int JOY3_max = 835;

int JOY1_initial = 0;
int JOY2_initial = 0;
int JOY3_initial = 0;

int JOY1_deadzone = 10;
int JOY2_deadzone = 5;
int JOY3_deadzone = 5;

AccelStepper stepper(AccelStepper::DRIVER, pin_STEP, pin_DIR);

int servo_pos = 0;

bool serialPrintBool = false;

int stepper_max_speed = 0;
int stepper_max_speed_min = 0;
int stepper_max_speed_max = 1000;

void setup() {

  if (serialPrintBool == true){
    Serial.begin(9600);
    while (!Serial) {
      ; // Wait for serial port to connect to laptop via USB cable, just used for debugging if desired.
    }
  }

  //Set the RGB LED pins as digital outputs
  pinMode(pin_LED_R,OUTPUT);
  pinMode(pin_LED_G,OUTPUT);
  pinMode(pin_LED_B,OUTPUT);

  //Soft PWM library is used to set the brightness of the red and green LEDs inside the RGB LED
  SoftPWMBegin();
  SoftPWMSet(pin_LED_R, 0);
  SoftPWMSet(pin_LED_G, 0);

  //Set fade time for each pin to 200 ms fade-up time, and 500 ms fade-down time
  SoftPWMSetFadeTime(pin_LED_R, 200, 500);
  SoftPWMSetFadeTime(pin_LED_G, 200, 500);

  //Stepper motor controller pins
  pinMode(pin_STEP,OUTPUT);
  pinMode(pin_DIR,OUTPUT);

  //pinMode(pin_SERVO,OUTPUT);
  myservo.attach(pin_SERVO);

  //Set the single-axis joystick connected pins as analog inputs
  pinMode(pin_JOY_1,INPUT);
  pinMode(pin_JOY_2,INPUT);
  pinMode(pin_JOY_3,INPUT);

  //Take initial readings of the 3 x single-axis joysticks
  JOY1_initial = analogRead(pin_JOY_1);
  JOY2_initial = analogRead(pin_JOY_2);
  JOY3_initial = analogRead(pin_JOY_3);

  //Begin rolling average value collection for the 3 x joysticks
  joy_1.begin();
  joy_2.begin();
  joy_3.begin();

  //Set some initial characteristsic of the stepper motor controller object
  stepper.move(100000);
  stepper.setMaxSpeed(1000);
  stepper.setSpeed(0);
  stepper.setAcceleration(250.0);

}

void loop() {

  //Update the rolling averages for the 3 x joysticks
  value_JOY1 = joy_1.reading(analogRead(pin_JOY_1));
  value_JOY2 = joy_2.reading(analogRead(pin_JOY_2));
  value_JOY3 = joy_3.reading(analogRead(pin_JOY_3));

  //Set the servo position
  if (value_JOY2 >= (JOY2_initial + JOY2_deadzone)){
    servo_pos = map(value_JOY2,(JOY2_initial + JOY2_deadzone),JOY2_max,93,180);
    myservo.write(servo_pos); 
  }
  else if (value_JOY2 <= (JOY2_initial - JOY2_deadzone)){
    servo_pos = map(value_JOY2,(JOY2_initial - JOY2_deadzone),JOY2_min,93,0);
    myservo.write(servo_pos); 
  }
  else{
    myservo.write(93); 
  }

  //Set the RGB LED color and brightness
  if (value_JOY1 >= (JOY1_initial + JOY1_deadzone)){
    value_LED_R = map(value_JOY1,(JOY1_initial + JOY1_deadzone),JOY1_max,0,255);
    SoftPWMSet(pin_LED_R, value_LED_R);
    SoftPWMSet(pin_LED_G, 0);
  }
  else if (value_JOY1 <= (JOY1_initial - JOY1_deadzone)){
    value_LED_G = map(value_JOY1,(JOY1_initial - JOY1_deadzone),JOY1_min,0,255);
    SoftPWMSet(pin_LED_G, value_LED_G);
    SoftPWMSet(pin_LED_R, 0);
  }
  else{
    SoftPWMSet(pin_LED_G, 0);
    SoftPWMSet(pin_LED_R, 0);
  }

  //Set the jogging speed of the stepper motor
  if (value_JOY3 >= (JOY3_initial + JOY3_deadzone)){
    stepper.move(100000);
    stepper_max_speed = map(value_JOY3,(JOY3_initial + JOY3_deadzone),JOY3_max,stepper_max_speed_min,stepper_max_speed_max);
    stepper.setMaxSpeed(stepper_max_speed);
    stepper.run();
  }
  else if (value_JOY3 <= (JOY3_initial - JOY3_deadzone)){
    stepper.move(-100000);
    stepper_max_speed = map(value_JOY3,(JOY3_initial - JOY3_deadzone),JOY3_min,stepper_max_speed_min,stepper_max_speed_max);
    stepper.setMaxSpeed(stepper_max_speed);
    stepper.run();
  }
  else{
    stepper.move(0);
    stepper.run();
  }


}
