#include <digitalWriteFast.h>
#include <eRCaGuy_Timer2_Counter.h>
#include <Servo.h> 

// ==============================
//            CONSTANTS
// ==============================

#define DEBUG 1
#define NR_CHANNEL_LIST 5
#define MIN_ANGLE_DIFF 2

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

// Output pins
// Servo 1
#define SERVO1_PIN 22
#define SERVO1_DEFAULT 90
#define SERVO1_MIN 0
#define SERVO1_MAX 180
// Servo 2
#define SERVO2_PIN 23
#define SERVO2_DEFAULT 90
#define SERVO2_MIN 9
#define SERVO2_MAX 163
// LED 1
#define LED1_PIN 52
#define LED1_DEFAULT LOW
// LED 2
#define LED2_PIN 53
#define LED2_DEFAULT LOW


// ==============================
//           VARIABLES
// ==============================

// Channel interrupt variables
volatile unsigned long lastInterrupt;
volatile unsigned long ch1Timer;
volatile int ch1Status[NR_CHANNEL_LIST];
volatile int ch1Counter;
volatile unsigned long ch2Timer;
volatile int ch2Status[NR_CHANNEL_LIST];
volatile int ch2Counter;
volatile unsigned long ch3Timer;
volatile int ch3Status[NR_CHANNEL_LIST];
volatile int ch3Counter;
volatile unsigned long ch4Timer;
volatile int ch4Status[NR_CHANNEL_LIST];
volatile int ch4Counter;
volatile unsigned long ch5Timer;
volatile int ch5Status[NR_CHANNEL_LIST];
volatile int ch5Counter;
volatile unsigned long ch6Timer;
volatile int ch6Status[NR_CHANNEL_LIST];
volatile int ch6Counter;

boolean hasResetOutputs = false;

// Servos
Servo servo1;
Servo servo2;


// ==============================
//              MAIN
// ==============================

void setup() {
  // Setup eRCaGuy_Timer2_Counter
  timer2.setup();
  #if DEBUG > 0
  Serial.begin(9600);
  #endif
  setupReceiver();
  setupServos();
  setupLeds();
  resetOutputs();
}

void loop() {
  // Check if transmitter is active
  boolean transmitter_active = checkActivity();
  
  #if DEBUG > 0
  String statusString = getStatusString();
  Serial.print("Active: ");
  Serial.print(transmitter_active);
  Serial.print(", ");
  Serial.print(statusString);
  #endif

  if (transmitter_active) {
    hasResetOutputs = false;
    // Translate the current value to a percentage and change it to a valid value (>= 0 & <= 100)
    float percentage = (float)(getChannelValue(ch2Status, ch2Counter) - CH_STICK_MIN) / (CH_STICK_MAX - CH_STICK_MIN);
    if (percentage > 100.0) { percentage = 100.0; }
    if (percentage < 0.0) { percentage = 0.0; }
    // Calculate angle based on percentage
    int angle = (int)round(SERVO1_MIN + percentage * (SERVO1_MAX - SERVO1_MIN));
    servoWrite(servo1, angle);
    
    #if DEBUG > 0
    Serial.print(", S1: ");
    Serial.print(angle);
    Serial.print(", ");
    Serial.print(percentage);
    #endif
    
    // Translate the current value to a percentage and change it to a valid value (>= 0 & <= 100)
    percentage = (float)(getChannelValue(ch3Status, ch3Counter) - CH_STICK_MIN) / (CH_STICK_MAX - CH_STICK_MIN);
    if (percentage > 100.0) { percentage = 100.0; }
    if (percentage < 0.0) { percentage = 0.0; }
    // Calculate angle based on percentage
    angle = (int)round(SERVO2_MIN + percentage * (SERVO2_MAX - SERVO2_MIN));
    servoWrite(servo2, angle);
    
    #if DEBUG > 0
    Serial.print(", S2: ");
    Serial.print(angle);
    Serial.print(", ");
    Serial.print(percentage);
    #endif

    int ch5Value = getChannelValue(ch5Status, ch5Counter);
    if (ch5Value > CH_SWITCH_HIGH) {
      digitalWrite(LED1_PIN, HIGH);
    }
    if (ch5Value < CH_SWITCH_LOW) {
      digitalWrite(LED1_PIN, LOW);
    }

    int ch6Value = getChannelValue(ch6Status, ch6Counter);
    if (ch6Value > CH_SWITCH_HIGH) {
      digitalWrite(LED2_PIN, HIGH);
    }
    if (ch6Value < CH_SWITCH_LOW) {
      digitalWrite(LED2_PIN, LOW);
    }
  }
  else {
    resetOutputs();
  }
  
  #if DEBUG > 0
  Serial.println("");
  #endif
}


// ==============================
//              SETUP
// ==============================

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


// ==============================
//              RESET
// ==============================

void resetOutputs() {
  if (! hasResetOutputs) {
    servo1.write(SERVO1_DEFAULT);
    servo2.write(SERVO2_DEFAULT);
  
    digitalWrite(LED1_PIN, LED1_DEFAULT);
    digitalWrite(LED2_PIN, LED2_DEFAULT);

    hasResetOutputs = true;
  }
}


// ==============================
//         HELP FUNCTIONS
// ==============================

boolean checkActivity() {
  // Check the value of the channels which send the status of the switches 
  // to see if the transmitter is active
  int ch5Value = getChannelValue(ch5Status, ch5Counter);
  int ch6Value = getChannelValue(ch6Status, ch6Counter);
  return !((ch5Status == 0 & ch6Status == 0) |
           (CH_SWITCH_LOW < ch5Value & ch5Value < CH_SWITCH_HIGH & 
            CH_SWITCH_LOW < ch6Value & ch6Value < CH_SWITCH_HIGH));
}

int getChannelValue(volatile int* channelStatus, volatile int& channelCounter) {
  // Calculates the value of a channel by taking the weighted average of the NR_CHANNEL_LIST 
  // last measurements, giving a higher weight to more recent measurements
  int value = 0;
  // Don't allow interrupts
  uint8_t SREG_old = SREG;
  cli();
  // Iterate over the status list
  for (int i = 0; i < NR_CHANNEL_LIST; i++) {
    value += (i + 1) * channelStatus[(channelCounter + i) % NR_CHANNEL_LIST];
  }
  // Allow interrupts again
  SREG = SREG_old;
  // Return the average
  return (2 * value) / (NR_CHANNEL_LIST * (NR_CHANNEL_LIST + 1));
}

String getStatusString() {
  String ss = "CH1: ";
  ss += getChannelValue(ch1Status, ch1Counter);
  ss += ", CH2: ";
  ss += getChannelValue(ch2Status, ch2Counter);
  ss += ", CH3: ";
  ss += getChannelValue(ch3Status, ch3Counter);
  ss += ", CH4: ";
  ss += getChannelValue(ch4Status, ch4Counter);
  ss += ", CH5: ";
  ss += getChannelValue(ch5Status, ch5Counter);
  ss += ", CH6: ";
  ss += getChannelValue(ch6Status, ch6Counter);
  return ss;
}

void servoWrite(Servo& servo, int angle) {
  // Write the angle to the given servo if the difference with the current angle is large enough
  if (abs(servo.read() - angle) > MIN_ANGLE_DIFF) {
    servo.write(angle);
  }
}


// ==============================
//               ISR
// ==============================
// Interrupt service routines for reading the status of the channels

void ch1_interrupt() {
  // Use timer2 to get the current time
  // get_count returns the count in 0.5 microseconds
  lastInterrupt = timer2.get_count()/2;
  // digitalReadFast instead of digitalRead for faster ISRs
  // This function needs a constant instead of a variable as parameter
  // => ISR for each channel instead of one function
  if (digitalReadFast(CH1_PIN) == HIGH) {
    // If high, save current time
    ch1Timer = lastInterrupt;
  }
  else {
    // If low, calculate length of high pulse
    ch1Status[ch1Counter] = (volatile int)(lastInterrupt - ch1Timer);
    // Increase counter to always replace the oldest measurement
    ch1Counter = (ch1Counter + 1) % NR_CHANNEL_LIST;
  }
}

void ch2_interrupt() {
  lastInterrupt = timer2.get_count()/2;
  if (digitalReadFast(CH2_PIN) == HIGH) {
    ch2Timer = lastInterrupt;
  }
  else {
    ch2Status[ch2Counter] = (volatile int)(lastInterrupt - ch2Timer);
    ch2Counter = (ch2Counter + 1) % NR_CHANNEL_LIST;
  }
}

void ch3_interrupt() {
  lastInterrupt = timer2.get_count()/2;
  if (digitalReadFast(CH3_PIN) == HIGH) {
    ch3Timer = lastInterrupt;
  }
  else {
    ch3Status[ch3Counter] = (volatile int)(lastInterrupt - ch3Timer);
    ch3Counter = (ch3Counter + 1) % NR_CHANNEL_LIST;
  }
}

void ch4_interrupt() {
  lastInterrupt = timer2.get_count()/2;
  if (digitalReadFast(CH4_PIN) == HIGH) {
    ch4Timer = lastInterrupt;
  }
  else {
    ch4Status[ch4Counter] = (volatile int)(lastInterrupt - ch4Timer);
    ch4Counter = (ch4Counter + 1) % NR_CHANNEL_LIST;
  }
}

void ch5_interrupt() {
  lastInterrupt = timer2.get_count()/2;
  if (digitalReadFast(CH5_PIN) == HIGH) {
    ch5Timer = lastInterrupt;
  }
  else {
    ch5Status[ch5Counter] = (volatile int)(lastInterrupt - ch5Timer);
    ch5Counter = (ch5Counter + 1) % NR_CHANNEL_LIST;
  }
}

void ch6_interrupt() {
  lastInterrupt = timer2.get_count()/2;
  if (digitalReadFast(CH6_PIN) == HIGH) {
    ch6Timer = lastInterrupt;
  }
  else {
    ch6Status[ch6Counter] = (volatile int)(lastInterrupt - ch6Timer);
    ch6Counter = (ch6Counter + 1) % NR_CHANNEL_LIST;
  }
}
