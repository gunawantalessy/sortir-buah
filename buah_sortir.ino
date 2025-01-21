#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Servo.h>  // Library untuk mengontrol servo motor

// Inisialisasi LCD I2C dengan alamat 0x27 dan ukuran 20x4
LiquidCrystal_I2C lcd(0x27, 20, 4);

// Deklarasi Servo
#define SERVO1_PIN 10  // Servo 1: Untuk turun saat warna merah
#define SERVO2_PIN 11  // Servo 2: Untuk mendeteksi warna hijau

Servo servoMotor1;  // Objek servo 1
Servo servoMotor2;  // Objek servo 2

#define S0 4        // S0 ke pin 4
#define S1 5        // S1 ke pin 5
#define S2 6        // S2 ke pin 6
#define S3 7        // S3 ke pin 7
#define salidaTCS 8 // salidaTCS ke pin 8

int countMasak = 0;
int countMentah = 0;

void setup() {
  // Inisialisasi pin untuk sensor warna
  pinMode(S0, OUTPUT);
  pinMode(S1, OUTPUT);
  pinMode(S2, OUTPUT);
  pinMode(S3, OUTPUT);
  pinMode(salidaTCS, INPUT);

  digitalWrite(S0, HIGH);  // Tetapkan frekuensi output
  digitalWrite(S1, LOW);   // Modul ke 20 persen

  Serial.begin(9600);

  // Inisialisasi LCD
  lcd.begin();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Sortir Buah Naga");
  lcd.setCursor(0, 1);
  lcd.print("Masak : ");
  lcd.setCursor(0, 2);
  lcd.print("Mentah : ");

  // Inisialisasi Servo
  servoMotor1.attach(SERVO1_PIN);
  servoMotor2.attach(SERVO2_PIN);

  // Servo mulai dari posisi awal
  servoMotor1.write(0); // Posisi awal Servo 1
  servoMotor2.write(0); // Posisi awal Servo 2
}

void loop() {
  // Baca nilai warna
  digitalWrite(S2, LOW);
  digitalWrite(S3, LOW);
  int merah = pulseIn(salidaTCS, LOW, 100000);
  delay(100);

  digitalWrite(S2, HIGH);
  digitalWrite(S3, HIGH);
  int hijau = pulseIn(salidaTCS, LOW, 100000);
  delay(100);

  Serial.print("R:");
  Serial.print(merah);
  Serial.print("\tV:");
  Serial.print(hijau);
  Serial.println();

  // Logika untuk mendeteksi warna
  if (merah < 150 && hijau > 100) { // Buah masak
    Serial.println("MASAK (Merah Terdeteksi)");
    countMasak++;
    lcd.setCursor(8, 1);
    lcd.print(countMasak);

    // Servo 1 turun
    servoMotor1.write(90);  // Gerakkan servo 1 turun ke 90 derajat
    delay(1000);            // Jeda 1 detik
    servoMotor1.write(0);   // Kembali ke posisi awal
  } 
  else if (hijau < 200 && merah > 180) { // Buah mentah (hijau terdeteksi)
    Serial.println("MENTAH (Hijau Terdeteksi)");
    countMentah++;
    lcd.setCursor(8, 2);
    lcd.print(countMentah);

    // Servo 2 mendeteksi warna hijau
    servoMotor2.write(45);  // Gerakkan servo 2 ke 45 derajat
    delay(1000);            // Jeda 1 detik
    servoMotor2.write(0);   // Kembali ke posisi awal
  } 
  else {
    Serial.println("TIDAK TERDETEKSI");
  }

  delay(1000); // Jeda sebelum membaca sensor lagi
}
