#include <Servo.h> 

// Channel threshold values for joysticks and toggle switches
#define CH_STICK_MIN 1000
#define CH_STICK_MAX 1950
#define CH_SWITCH_LOW 1200
#define CH_SWITCH_HIGH 1800

// Pin and interrupt numbers for an Arduino Mega 2560
// Channel 1: right horizontal
#define CH1_PIN 2
#define CH1_INTERRUPT 0

// Channel 2: left vertical
#define CH2_PIN 3
#define CH2_INTERRUPT 1

// Channel 3: right vertical
#define CH3_PIN 18
#define CH3_INTERRUPT 5

// Channel 4: left horizontal
#define CH4_PIN 19
#define CH4_INTERRUPT 4

// Channel 5: left switch
#define CH5_PIN 20
#define CH5_INTERRUPT 3

// Channel 6: right switch
#define CH6_PIN 21
#define CH6_INTERRUPT 2

#define SERVO1_PIN 22
#define SERVO1_DEFAULT 90
#define SERVO1_MIN 0
#define SERVO1_MAX 180

#define SERVO2_PIN 23
#define SERVO2_DEFAULT 90
#define SERVO2_MIN 9
#define SERVO2_MAX 166

#define LED1_PIN 52
#define LED1_DEFAULT LOW

#define LED2_PIN 53
#define LED2_DEFAULT LOW

volatile unsigned long lastInterrupt;

volatile unsigned long ch1Timer;
volatile int ch1Status;
volatile unsigned long ch2Timer;
volatile int ch2Status;
volatile unsigned long ch3Timer;
volatile int ch3Status;
volatile unsigned long ch4Timer;
volatile int ch4Status;
volatile unsigned long ch5Timer;
volatile int ch5Status;
volatile unsigned long ch6Timer;
volatile int ch6Status;

boolean transmitter_active = false;
boolean reset = false;

Servo servo1;
Servo servo2;

void setup() {
  Serial.begin(9600);
  setupReceiver();
  setupServos();
  setupLeds();
  resetOutputs();
}

void loop() {
  transmitter_active = checkActivity();
  String statusString = getStatusString();
  Serial.print("Active: ");
  Serial.print(transmitter_active);
  Serial.print(", ");
  Serial.print(statusString);

  if (transmitter_active) {
    reset = false;
    float percentage = (float)(ch2Status - CH_STICK_MIN) / (CH_STICK_MAX - CH_STICK_MIN);
    if (percentage > 100.0) {
      percentage = 100.0;
    }
    if (percentage < 0.0) {
      percentage = 0.0;
    }
    int angle = (int)(SERVO1_MIN + percentage * (SERVO1_MAX - SERVO1_MIN));
    servo1.write(angle);
    Serial.print(", S1: ");
    Serial.print(angle);
    
    percentage = (float)(ch3Status - CH_STICK_MIN) / (CH_STICK_MAX - CH_STICK_MIN);
    if (percentage > 100.0) {
      percentage = 100.0;
    }
    if (percentage < 0.0) {
      percentage = 0.0;
    }
    angle = (int)(SERVO2_MIN + percentage * (SERVO2_MAX - SERVO2_MIN));
    servo2.write(angle);
    Serial.print(", S2: ");
    Serial.print(angle);
  
    if (ch5Status > CH_SWITCH_HIGH) {
      digitalWrite(LED1_PIN, HIGH);
    }
    if (ch5Status < CH_SWITCH_LOW) {
      digitalWrite(LED1_PIN, LOW);
    }
  
    if (ch6Status > CH_SWITCH_HIGH) {
      digitalWrite(LED2_PIN, HIGH);
    }
    if (ch6Status < CH_SWITCH_LOW) {
      digitalWrite(LED2_PIN, LOW);
    }
  }
  else {
    resetOutputs();
  }
  Serial.println("");
}

void resetOutputs() {
  if (! reset) {
    servo1.write(SERVO1_DEFAULT);
    servo2.write(SERVO2_DEFAULT);
  
    digitalWrite(LED1_PIN, LED1_DEFAULT);
    digitalWrite(LED2_PIN, LED2_DEFAULT);

    reset = true;
  }
}

void setupReceiver() {
  pinMode(CH1_PIN, INPUT);
  attachInterrupt(CH1_INTERRUPT, ch1_interrupt, CHANGE);
  pinMode(CH2_PIN, INPUT);
  attachInterrupt(CH2_INTERRUPT, ch2_interrupt, CHANGE);
  pinMode(CH3_PIN, INPUT);
  attachInterrupt(CH3_INTERRUPT, ch3_interrupt, CHANGE);
  pinMode(CH4_PIN, INPUT);
  attachInterrupt(CH4_INTERRUPT, ch4_interrupt, CHANGE);
  pinMode(CH5_PIN, INPUT);
  attachInterrupt(CH5_INTERRUPT, ch5_interrupt, CHANGE);
  pinMode(CH6_PIN, INPUT);
  attachInterrupt(CH6_INTERRUPT, ch6_interrupt, CHANGE);
}

void setupServos() {
  servo1.attach(SERVO1_PIN);
  servo2.attach(SERVO2_PIN);
}

void setupLeds() {
  pinMode(LED1_PIN, OUTPUT);
  pinMode(LED2_PIN, OUTPUT);
}

boolean checkActivity() {
  return !((ch5Status == 0 & ch6Status == 0) |
           (CH_SWITCH_LOW < ch5Status & ch5Status < CH_SWITCH_HIGH & 
            CH_SWITCH_LOW < ch6Status & ch6Status < CH_SWITCH_HIGH));
}

String getStatusString() {
  String ss = "CH1: ";
  ss += ch1Status;
  ss += ", CH2: ";
  ss += ch2Status;
  ss += ", CH3: ";
  ss += ch3Status;
  ss += ", CH4: ";
  ss += ch4Status;
  ss += ", CH5: ";
  ss += ch5Status;
  ss += ", CH6: ";
  ss += ch6Status;
  return ss;
}

void channel_interrupt(int channelPin, volatile unsigned long &channelTimer, volatile int &channelStatus, String channelName) {
  lastInterrupt = micros();
  if (digitalRead(channelPin) == HIGH) {
    channelTimer = lastInterrupt;
  }
  else {
    channelStatus = (volatile int)(lastInterrupt - channelTimer);
  }
}

void ch1_interrupt() {
  channel_interrupt(CH1_PIN, ch1Timer, ch1Status, "CH1");
}

void ch2_interrupt() {
  channel_interrupt(CH2_PIN, ch2Timer, ch2Status, "CH2");
}

void ch3_interrupt() {
  channel_interrupt(CH3_PIN, ch3Timer, ch3Status, "CH3");
}

void ch4_interrupt() {
  channel_interrupt(CH4_PIN, ch4Timer, ch4Status, "CH4");
}

void ch5_interrupt() {
  channel_interrupt(CH5_PIN, ch5Timer, ch5Status, "CH5");
}

void ch6_interrupt() {
  channel_interrupt(CH6_PIN, ch6Timer, ch6Status, "CH6");
}

