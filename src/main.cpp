#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Servo.h>

// LCD I2C
LiquidCrystal_I2C lcd(0x27, 16, 2);

// TCS3200 color sensor
#define S0 7
#define S1 6
#define S2 A1
#define S3 A2
#define sensorOut A0

// Cảm biến quang tại vị trí servo1 (đỏ) và servo2 (xanh dương)
#define IR_SENSOR_RED 2
#define IR_SENSOR_BLUE 3

// Motor L298N
#define IN1 8
#define IN2 9

// Servo
Servo servo1;  // gạt màu đỏ
Servo servo2;  // gạt màu xanh dương
#define SERVO1_PIN 4
#define SERVO2_PIN 5

// Đếm sản phẩm
int countRed = 0;
int countGreen = 0;
int countBlue = 0;

// Biến lưu màu vật thể sau khi phân loại
String currentColor = "";

void setup() {
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Khoi dong...");
  delay(1000);
  lcd.clear();

  pinMode(S0, OUTPUT);
  pinMode(S1, OUTPUT);
  pinMode(S2, OUTPUT);
  pinMode(S3, OUTPUT);
  pinMode(sensorOut, INPUT);

  pinMode(IR_SENSOR_RED, INPUT);
  pinMode(IR_SENSOR_BLUE, INPUT);

  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);

  servo1.write(0);
  servo2.write(0);
  delay(500);  // Đảm bảo servo ổn định trước khi attach
  servo1.attach(SERVO1_PIN);
  servo2.attach(SERVO2_PIN);

  digitalWrite(S0, HIGH);
  digitalWrite(S1, LOW);

  Serial.begin(9600);
}

int readColor(char color) {
  switch (color) {
    case 'R': digitalWrite(S2, LOW); digitalWrite(S3, LOW); break;
    case 'G': digitalWrite(S2, HIGH); digitalWrite(S3, HIGH); break;
    case 'B': digitalWrite(S2, LOW); digitalWrite(S3, HIGH); break;
  }
  delay(100);
  return pulseIn(sensorOut, LOW);
}

void runConveyor() {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
}

void stopConveyor() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
}

void loop() {
  delay(3000);  // Đợi 3 giây mỗi lần quét màu
  int red = readColor('R');
  int green = readColor('G');
  int blue = readColor('B');

  Serial.print("R:"); Serial.print(red);
  Serial.print(" G:"); Serial.print(green);
  Serial.print(" B:"); Serial.println(blue);

  int minValue = min(red, min(green, blue));

  if (minValue == red && red < green * 0.8 && red < blue * 0.8) {
    currentColor = "red";
    Serial.println("Phat hien: DO");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Phat hien: DO");
  } else if (minValue == green && green < red * 0.8 && green < blue * 0.8) {
    currentColor = "green";
    Serial.println("Phat hien: XANH LA");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Phat hien: XANH LA");
  } else if (minValue == blue && blue < red * 0.8 && blue < green * 0.8) {
    currentColor = "blue";
    Serial.println("Phat hien: XANH DUONG");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Phat hien: XANH DUONG");
  } else {
    currentColor = "unknown";
    Serial.println("Khong xac dinh");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Khong xac dinh");
  }

  runConveyor();

  if (digitalRead(IR_SENSOR_RED) == LOW && currentColor == "red") {
    stopConveyor();
    servo1.write(90);
    delay(500);
    servo1.write(0);
    countRed++;
    currentColor = "";
    delay(300);
    runConveyor();
  }

  if (digitalRead(IR_SENSOR_BLUE) == LOW && currentColor == "blue") {
    stopConveyor();
    servo2.write(90);
    delay(500);
    servo2.write(0);
    countBlue++;
    currentColor = "";
    delay(300);
    runConveyor();
  }

  if (currentColor == "green" && digitalRead(IR_SENSOR_BLUE) == LOW) {
    countGreen++;
    currentColor = "";
    delay(300);
  }

  lcd.setCursor(0, 1);
  lcd.print("Do:"); lcd.print(countRed);
  lcd.print(" X: "); lcd.print(countGreen);
  lcd.print(" B:"); lcd.print(countBlue);

  delay(100);
}
