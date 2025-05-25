#include <Arduino.h>
#include <Servo.h>
#include <LiquidCrystal.h>

// Khai bao LCD (RS, E, D4, D5, D6, D7)
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

// Khai bao cam bien mau TCS3200
#define S0 7
#define S1 6
#define S2 A1
#define S3 A2
#define sensorOut A0

// Khai bao Servo
Servo servoRed;
Servo servoBlue;
#define SERVO_RED_PIN 8
#define SERVO_BLUE_PIN 9

// Khai bao Motor dieu khien boi L298N
#define ENA 10 // PWM dieu chinh toc do
#define IN1 A3
#define IN2 A4

// Bien dem san pham
int countRed = 0;
int countGreen = 0;
int countBlue = 0;

// Toc do dong co (0-255)
int motorSpeed = 150;

void setup() {
  // Khoi dong LCD
  lcd.begin(16, 2);
  lcd.print("Khoi dong...");

  // Cai dat chan cam bien mau
  pinMode(S0, OUTPUT);
  pinMode(S1, OUTPUT);
  pinMode(S2, OUTPUT);
  pinMode(S3, OUTPUT);
  pinMode(sensorOut, INPUT);

  // Cai dat Motor
  pinMode(ENA, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);

  // Gan Servo
  servoRed.attach(SERVO_RED_PIN);
  servoBlue.attach(SERVO_BLUE_PIN);
  servoRed.write(90);
  servoBlue.write(90);

  // Cam bien mau: Tan so cao
  digitalWrite(S0, HIGH);
  digitalWrite(S1, LOW);

  Serial.begin(9600);
}

// Ham doc gia tri mau
int readColor(char color) {
  switch (color) {
    case 'R': digitalWrite(S2, LOW); digitalWrite(S3, LOW); break;
    case 'G': digitalWrite(S2, HIGH); digitalWrite(S3, HIGH); break;
    case 'B': digitalWrite(S2, LOW); digitalWrite(S3, HIGH); break;
  }
  delay(100);
  return pulseIn(sensorOut, LOW);
}

void runConveyor(int speed) {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  analogWrite(ENA, speed);
}

void stopConveyor() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
}

void loop() {
  runConveyor(motorSpeed);

  delay(1000); // Cho san pham den diem doc mau

  int red = readColor('R');
  int green = readColor('G');
  int blue = readColor('B');

  Serial.print(" R:"); Serial.print(red);
  Serial.print(" G:"); Serial.print(green);
  Serial.print(" B:"); Serial.println(blue);

  // Phan loai mau du dua tren tan so do duoc
  if (red < green && red < blue) {
    // Vat mau do
    stopConveyor();
    servoRed.write(0);
    delay(800);
    servoRed.write(90);
    countRed++;
    delay(500);
  } else if (blue < red && blue < green) {
    // Vat mau xanh duong
    stopConveyor();
    servoBlue.write(0);
    delay(800);
    servoBlue.write(90);
    countBlue++;
    delay(500);
  } else {
    // Vat mau xanh la (di thang)
    countGreen++;
  }

  // Cap nhat LCD
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Do:"); lcd.print(countRed);
  lcd.print("XanhLa:"); lcd.print(countGreen);

  lcd.setCursor(0, 1);
  lcd.print("XanhDuong:"); lcd.print(countBlue);

  delay(500);
}
// 