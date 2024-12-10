#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <Keypad.h>
#include "DHT.h"
//konfigurasi pin dht yang digunakan dan tipe dht yang digunakan
#define DHTPIN 12      // DHT PIN 2
#define DHTTYPE DHT22  // DHT 22  (AM2302), AM2321
DHT dht(DHTPIN, DHTTYPE);
// Konfigurasi LCD
LiquidCrystal_I2C lcd(0x27, 16, 2);
//variabel yang digunakan pada dht
// Konfigurasi keypad
const byte ROWS = 4;  // Jumlah baris
const byte COLS = 4;  // Jumlah kolom
char keys[ROWS][COLS] = {
  { '1', '2', '3', 'A' },
  { '4', '5', '6', 'B' },
  { '7', '8', '9', 'C' },
  { '*', '0', '#', 'D' }
};
byte rowPins[ROWS] = { 9, 8, 7, 6 };  // Pin untuk baris
byte colPins[COLS] = { 5, 4, 3, 2 };  // Pin untuk kolom

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

const int buzzerPin = 11;  // Pin untuk buzzer
double h, t;
float batasAtas = 0.0;      // Batas atas
float batasBawah = 0.0;     // Batas bawah
bool setBatasBawah = true;  // Menentukan apakah sedang set batas bawah
bool Mulai;      // Mode pengaturan atau pembacaan
String input = "";          // String sementara untuk menyimpan input keypad

void setup() {
  Serial.begin(9600);
  lcd.init();
  dht.begin();
  lcd.backlight();
  pinMode(buzzerPin, OUTPUT);
  digitalWrite(buzzerPin, LOW);

  lcd.setCursor(0, 0);
  lcd.print("Set batas sensor");
  lcd.setCursor(2, 1);
  lcd.print("DHT22");
  delay(2000);
  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("A: Setpoint");
  lcd.setCursor(0, 1);
  lcd.print("B: Sensor");
  
}

void loop() {
  // Membaca input dari keypad
  char key = keypad.getKey();
  if (key) {
    if (key == 'A') {  // Masuk ke mode pengaturan
      Mulai = true;
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Atur batas:");
    } else if (key == 'B') {  // Masuk ke mode pembacaan sensor
      Mulai = false;
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Baca sensor:");
    } else if (key == 'C') {
      digitalWrite(buzzerPin, LOW);
    }
  }
  // Mode pengaturan (Set batas)
  if (Mulai) {
    digitalWrite(buzzerPin, LOW);
    if (key >= '0' && key <= '9') {  // Jika tombol angka ditekan
      input += key;                  // Tambahkan angka ke input
      lcd.setCursor(0, 1);
      lcd.print("Input: " + input);
    } else if (key == '#') {  // Tombol '#' untuk konfirmasi input
      if (setBatasBawah) {
        batasBawah = input.toFloat();  // Konversi input ke integer
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Bawah: " + String(batasBawah));
        setBatasBawah = false;  // Pindah ke set batas atas
      } else {
        batasAtas = input.toFloat();  // Konversi input ke integer
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Atas: " + String(batasAtas));
        lcd.setCursor(0, 1);
        lcd.print("Selesai!");
        delay(2000);
        lcd.clear();
        lcd.print("B: Sensor Mode");
        setBatasBawah = true;  // Reset ke pengaturan bawah untuk selanjutnya
        Mulai = false;         // Kembali ke mode pembacaan
      }
      input = "";             // Reset input
    } else if (key == '*') {  // Tombol '*' untuk reset
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Reset setpoint");
      setBatasBawah = true;
      input = "";
      delay(2000);
      lcd.clear();
      lcd.print("A: Setpoint");
      lcd.setCursor(0, 1);
      lcd.print("B: Sensor");
    }
  } else {  // Mode pembacaan sensor
    h = dht.readHumidity();
    // Read temperature as Celsius (the default)
    t = dht.readTemperature();

    // Check if any reads failed and exit early (to try again).
    if (isnan(h) || isnan(t)) {
      Serial.println("Failed to read from DHT sensor!");
      digitalWrite(buzzerPin, HIGH);  // Buzzer menyala untuk memberi tahu masalah
      delay(1000);
      digitalWrite(buzzerPin, LOW);
      return;
    }
    lcd.setCursor(0, 1);
    lcd.print("t:");
    lcd.print(t);
    lcd.print("  ");
    lcd.print("h:");
    lcd.print(h);

    Serial.print("Temperature: ");
    Serial.print(t);
    Serial.print(" *C\t");
    Serial.print("Humidity: ");
    Serial.print(h);
    Serial.println(" %");
    digitalWrite(buzzerPin, LOW);
    if (t > batasAtas || t < batasBawah) {
      digitalWrite(buzzerPin, HIGH);  // Nyalakan buzzer
      lcd.setCursor(0, 0);
      lcd.print("ALERT!          ");
    } else {
      digitalWrite(buzzerPin, LOW);  // Matikan buzzer
      lcd.setCursor(0, 0);
      lcd.print("Normal          ");
    }
    delay(500);  // Delay untuk pembaruan pembacaan
  }
}