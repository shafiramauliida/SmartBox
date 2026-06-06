#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <DHT.h>

/* KONFIGURASI PIN */

#define DHTPIN 2
#define DHTTYPE DHT11

#define LDR_PIN A1
#define MIC_PIN A0

/* OBJECT */

DHT dht(DHTPIN, DHTTYPE);


LiquidCrystal_I2C lcd(0x27, 16, 2);

const int sampleWindow = 50; // Lebar sampling 50 ms (20Hz)
unsigned int sample;
const double vRef = 0.0044; 
const double dbOffset = 30.0; // Kalibrasi offset untuk menyamakan dengan dB nyata

/* SETUP */

void setup() {
  
  Serial.begin(9600);
  pinMode(LDR_PIN, INPUT);
  pinMode(MIC_PIN, INPUT);
  dht.begin();

  lcd.init();
  lcd.backlight();

  lcd.setCursor(0,0);
  lcd.print("SYSTEM READY");

  delay(2000);

  lcd.clear();
}

/* LOOP */

void loop() {

  /* BACA SENSOR */

  float suhu = dht.readTemperature();

  int mentah = analogRead(LDR_PIN);
  int cahaya = map(mentah, 1023, 0, 0, 100);
  
  // Membatasi nilai agar tetap di rentang 0-100%
  cahaya = constrain(cahaya, 0, 100);

  unsigned long startMillis = millis();  
  unsigned int peakToPeak = 0;   

  unsigned int signalMax = 0;
  unsigned int signalMin = 1024;

   // 1. Proses Sampling Gelombang AC Audio
   while (millis() - startMillis < sampleWindow) {
      sample = analogRead(MIC_PIN);
      if (sample < 1024) {  
         if (sample > signalMax) {
            signalMax = sample;
         }
         else if (sample < signalMin) {
            signalMin = sample;  
         }
      }
   }
   
   // 2. Hitung Amplitudo Puncak-ke-Puncak
   peakToPeak = signalMax - signalMin;  
   
   // 3. Konversi ADC ke Volt (Nilai minimum dibatasi 1 agar log10 tidak minus tak terhingga)
   if (peakToPeak < 1) peakToPeak = 1; 
   double volts = (peakToPeak * 5.0) / 1023.0;  

   // 4. Rumus Logaritma Desibel (dB)
   // Menghitung rasio logaritmik dari tegangan yang terbaca
   double suara = 20.0 * log10(volts / vRef) + dbOffset;

   // Pembatasan logika agar nilai tidak minus jika ruangan sangat hening
   if (suara < 0) suara = 0;

  /* PARAMETER SUHU */

  String kondisiSuhu;

  if (suhu < 25) {
    kondisiSuhu = "DINGIN";
  }
  else if (suhu <= 32) {
    kondisiSuhu = "NORMAL";
  }
  else {
    kondisiSuhu = "PANAS";
  }

  /* PARAMETER CAHAYA */

  String kondisiCahaya;

  if (cahaya < 40) {
    kondisiCahaya = "GELAP";
  }
  else if (cahaya <= 70) {
    kondisiCahaya = "CUKUP";
  }
  else {
    kondisiCahaya = "TERANG";
  }

  /* PARAMETER SUARA */

  String kondisiSuara;

  if (suara < 50) {
    kondisiSuara = "SUNYI";
  }
  else if (suara <= 80) {
    kondisiSuara = "NORMAL";
  }
  else {
    kondisiSuara = "BISING";
  }

  /* KESIMPULAN */

  String kesimpulan;

  if (
      kondisiSuhu == "NORMAL" &&
      kondisiCahaya != "GELAP" &&
      kondisiSuara != "BISING"
     )
  {
    kesimpulan = "LAYAK";
  }
  else {
    kesimpulan = "TIDAK LAYAK";
  }

  /* TAMPIL SUHU */

  lcd.clear();

  lcd.setCursor(0,0);
  lcd.print("SUHU:");

  lcd.print(suhu);

  lcd.print("C");

  lcd.setCursor(0,1);
  lcd.print(kondisiSuhu);

  delay(4000);

  /* TAMPIL CAHAYA */

  lcd.clear();

  lcd.setCursor(0,0);
  lcd.print("CAHAYA:");

  lcd.print(cahaya);

  lcd.setCursor(0,1);
  lcd.print(kondisiCahaya);

  delay(4000);

  /* TAMPIL SUARA */

  lcd.clear();

  lcd.setCursor(0,0);
  lcd.print("SUARA:");

  lcd.print(suara);

  lcd.setCursor(0,1);
  lcd.print(kondisiSuara);

  delay(4000);

  /* TAMPIL KESIMPULAN */

  lcd.clear();

  lcd.setCursor(0,0);
  lcd.print("KESIMPULAN");

  lcd.setCursor(0,1);
  lcd.print(kesimpulan);

  delay(4000);
}