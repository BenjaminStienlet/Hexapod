#include <Servo.h> 


// GWS Micro STD: min = 9, max = 166
// GWS PARK: min = 0, max = 180
// Modelcraft MC-1811: min = 25, max = 158
#define SERVO_PIN 3
#define MIN_ANGLE 0
#define MAX_ANGLE 180

Servo servo;

void setup() {
  Serial.begin(9600);
  servo.attach(SERVO_PIN);
  pinMode(2, INPUT);
}

//void loop() {
//  servo.write(140);
//  delay(10);
//}

void loop() {
  int value = digitalRead(2);
  if (value == LOW) {
    for(int i = MIN_ANGLE; i <= MAX_ANGLE; i++) {
      Serial.print("Servo angle: ");
      Serial.println(i);
      servo.write(i);
      delay(200);
    }
    delay(1000);
    for(int i = MAX_ANGLE; i >= MIN_ANGLE; i--) {
      Serial.print("Servo angle: ");
      Serial.println(i);
      servo.write(i);
      delay(20);
    }
  }
  else {
    servo.write(90);
  }
  delay(1000);
}
