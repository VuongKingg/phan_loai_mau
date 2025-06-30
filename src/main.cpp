#include <Arduino.h>
#include <Servo.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// TCS3200
#define S0 7
#define S1 6
#define S2 A1
#define S3 A2
#define sensorOut A0

// Servo
#define SERVO1_PIN 4
#define SERVO2_PIN 5

// Motor
#define IN1 8
#define IN2 9

// IR sensors
#define IR_RED 2
#define IR_BLUE 3

// LCD I2C
LiquidCrystal_I2C lcd(0x27, 16, 2);

Servo servo1;
Servo servo2;

int countRed = 0;
int countGreen = 0;
int countBlue = 0;

// Hàng đợi màu (dùng mảng đơn giản)
const int MAX_QUEUE = 10;
String colorQueue[MAX_QUEUE];
int queueStart = 0;
int queueEnd = 0;

String currentColor = "unknown";
String lastColor = "unknown";

// Hàm push vào hàng đợi
void enqueueColor(String color) {
  if ((queueEnd + 1) % MAX_QUEUE != queueStart) {
    colorQueue[queueEnd] = color;
    queueEnd = (queueEnd + 1) % MAX_QUEUE;
  }
}

// Hàm lấy màu khỏi hàng đợi
String dequeueColor() {
  if (queueStart == queueEnd) return "empty";
  String color = colorQueue[queueStart];
  queueStart = (queueStart + 1) % MAX_QUEUE;
  return color;
}

// Kiểm tra hàng đợi có trống không
bool isQueueEmpty() {
  return queueStart == queueEnd;
}

void setup() {
  Serial.begin(9600);
  lcd.init();
  lcd.backlight();

  pinMode(IR_RED, INPUT);
  pinMode(IR_BLUE, INPUT);

  servo1.attach(SERVO1_PIN);
  servo2.attach(SERVO2_PIN);
  servo1.write(0);
  servo2.write(0);

  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  digitalWrite(IN1, HIGH);  // Motor quay
  digitalWrite(IN2, LOW);

  pinMode(S0, OUTPUT);
  pinMode(S1, OUTPUT);
  pinMode(S2, OUTPUT);
  pinMode(S3, OUTPUT);
  pinMode(sensorOut, INPUT);
  digitalWrite(S0, HIGH);
  digitalWrite(S1, LOW);

  lcd.setCursor(0, 0);
  lcd.print("Color: UNKNOWN");
  lcd.setCursor(0, 1);
  lcd.print("R:0 G:0 B:0");

  delay(2000);
}

void loop() {
  unsigned int red, green, blue;
  readRGB(red, green, blue);
  detectColorWithRatio(red, green, blue);

  // Nếu phát hiện màu mới, lưu vào hàng đợi (KHÔNG đếm ở đây nữa)
  if (currentColor != lastColor && currentColor != "unknown") {
    enqueueColor(currentColor);

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Color: ");
    lcd.print(currentColor);

    lcd.setCursor(0, 1);
    lcd.print("R:");
    lcd.print(countRed);
    lcd.print(" G:");
    lcd.print(countGreen);
    lcd.print(" B:");
    lcd.print(countBlue);

    delay(300);
  }

  lastColor = currentColor;

  // IR_RED phát hiện và màu đầu là red
  if (digitalRead(IR_RED) == LOW && !isQueueEmpty()) {
    String c = dequeueColor();
    if (c == "red") {
      servo1.write(75);
      delay(1400);
      servo1.write(0);
      countRed++;
      updateLCD();  // cập nhật LCD sau khi đẩy
    } else {
      // Nếu không phải red, vẫn cần đưa lại vào queue
      enqueueColor(c);
    }
  }

  // IR_BLUE phát hiện và màu đầu là blue
  if (digitalRead(IR_BLUE) == LOW && !isQueueEmpty()) {
    String c = dequeueColor();
    if (c == "blue") {
      servo2.write(75);
      delay(1400);
      servo2.write(0);
      countBlue++;
      updateLCD();  // cập nhật LCD sau khi đẩy
    } else if (c == "green") {
      countGreen++;  // green đi thẳng, vẫn tính
      updateLCD();
    } else {
      enqueueColor(c);
    }
  }

  // Nếu cả hai cảm biến IR không phát hiện gì, kiểm tra đầu hàng đợi là GREEN
  if (digitalRead(IR_RED) == HIGH && digitalRead(IR_BLUE) == HIGH && !isQueueEmpty()) {
    String c = dequeueColor();
    if (c == "green") {
      countGreen++;
      updateLCD();
    } else {
      enqueueColor(c);  // nếu không phải green thì đưa lại vào queue
    }
  }

  delay(100);
}

// Cập nhật lại thông tin LCD
void updateLCD() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Color: ");
  lcd.print(currentColor);

  lcd.setCursor(0, 1);
  lcd.print("R:");
  lcd.print(countRed);
  lcd.print(" G:");
  lcd.print(countGreen);
  lcd.print(" B:");
  lcd.print(countBlue);
}


// Đọc RGB từ cảm biến màu
void readRGB(unsigned int &red, unsigned int &green, unsigned int &blue) {
  const int samples = 5;
  unsigned long r = 0, g = 0, b = 0;
  for (int i = 0; i < samples; i++) {
    r += readColorFrequency(LOW, LOW);    // RED
    g += readColorFrequency(HIGH, HIGH);  // GREEN
    b += readColorFrequency(LOW, HIGH);   // BLUE
  }
  red = r / samples;
  green = g / samples;
  blue = b / samples;
}

unsigned int readColorFrequency(int s2_val, int s3_val) {
  digitalWrite(S2, s2_val);
  digitalWrite(S3, s3_val);
  delay(2);
  return pulseIn(sensorOut, LOW);
}

// Phát hiện màu theo tỉ lệ
void detectColorWithRatio(unsigned int red, unsigned int green, unsigned int blue) {
  unsigned int minValue = min(red, min(green, blue));

  if (minValue == red && red < green * 0.8 && red < blue * 0.8) {
    currentColor = "red";
    Serial.println("Detected: RED");
  } else if (minValue == green && green < red * 0.8 && green < blue * 0.8) {
    currentColor = "green";
    Serial.println("Detected: GREEN");
  } else if (minValue == blue && blue < red * 0.8 && blue < green * 0.8) {
    currentColor = "blue";
    Serial.println("Detected: BLUE");
  } else {
    currentColor = "unknown";
    Serial.println("Detected: UNKNOWN");
  }
}