
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

void setup() {
  Serial.begin(9600);
  setupReceiver();
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

void loop() {
  transmitter_active = checkActivity();
  String statusString = getStatusString();
  Serial.print("Active: ");
  Serial.println(transmitter_active);
  Serial.println(statusString);
  delay(1000);
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

