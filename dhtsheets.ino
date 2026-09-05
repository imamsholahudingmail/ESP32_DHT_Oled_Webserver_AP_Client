#include <WiFi.h>
#include <HTTPClient.h>

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>

// =========================
// WIFI
// =========================
const char* ssid = "DigPart 4G";
const char* password = "digpart4g";

// =========================
// GOOGLE SHEETS
// =========================
// GANTI DENGAN URL WEB APP KAMU
const char* googleScriptURL =
"https://script.google.com/macros/s/AKfycbytLD_1yJ_K4AqBtDJVDzmdONbES77P6Onim4i45dQsV13XVpcUrhDyXxibBVhYK2_wCw/exec";

// =========================
// DHT22
// =========================
#define DHTPIN 14
#define DHTTYPE DHT22

DHT dht(DHTPIN, DHTTYPE);

// =========================
// OLED
// =========================
#define OLED_SDA 21
#define OLED_SCL 22

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

Adafruit_SSD1306 display(
  SCREEN_WIDTH,
  SCREEN_HEIGHT,
  &Wire,
  -1
);

// =========================
// SETUP
// =========================
void setup() {

  Serial.begin(115200);

  // =========================
  // DHT22
  // =========================
  dht.begin();

  // =========================
  // OLED
  // =========================
  Wire.begin(OLED_SDA, OLED_SCL);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {

    Serial.println("OLED TIDAK TERDETEKSI!");

    while (true) {
      delay(1000);
    }
  }

  // =========================
  // TAMPILAN AWAL OLED
  // =========================
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);

  display.setTextSize(1);
  display.setCursor(0, 0);

  display.println("MONITORING DHT22");
  display.println();
  display.println("Menghubungkan WiFi...");

  display.display();

  // =========================
  // WIFI
  // =========================
  WiFi.begin(ssid, password);

  Serial.print("Menghubungkan WiFi");

  while (WiFi.status() != WL_CONNECTED) {

    delay(500);

    Serial.print(".");
  }

  Serial.println();
  Serial.println("WiFi TERHUBUNG!");

  Serial.print("IP ESP32: ");
  Serial.println(WiFi.localIP());

  // =========================
  // OLED WIFI TERHUBUNG
  // =========================
  display.clearDisplay();

  display.setTextSize(1);
  display.setCursor(0, 0);

  display.println("MONITORING DHT22");
  display.println();
  display.println("WiFi TERHUBUNG");

  display.display();

  delay(2000);
}

// =========================
// LOOP
// =========================
void loop() {

  // =========================
  // BACA DHT22
  // =========================
  float suhu = dht.readTemperature();
  float kelembapan = dht.readHumidity();

  Serial.println("========================");

  // =========================
  // CEK SENSOR
  // =========================
  if (isnan(suhu) || isnan(kelembapan)) {

    Serial.println("GAGAL MEMBACA DHT22!");

    display.clearDisplay();

    display.setTextSize(1);
    display.setCursor(0, 0);

    display.println("DHT22 ERROR!");
    display.println();
    display.println("Cek kabel:");
    display.println("DATA -> GPIO 26");

    display.display();

    delay(2000);

    return;
  }

  // =========================
  // SERIAL MONITOR
  // =========================
  Serial.print("Suhu       : ");
  Serial.print(suhu, 1);
  Serial.println(" C");

  Serial.print("Kelembapan : ");
  Serial.print(kelembapan, 1);
  Serial.println(" %");

  // =========================
  // OLED
  // =========================
  display.clearDisplay();

  display.setTextColor(SSD1306_WHITE);

  display.setTextSize(1);
  display.setCursor(0, 0);

  display.println("MONITORING DHT22");

  display.setTextSize(2);

  display.setCursor(0, 20);
  display.print(suhu, 1);
  display.println(" C");

  display.setCursor(0, 45);
  display.print(kelembapan, 1);
  display.println(" %");

  display.display();

  // =========================
  // KIRIM KE GOOGLE SHEETS
  // =========================
  if (WiFi.status() == WL_CONNECTED) {

    HTTPClient http;

    String url = String(googleScriptURL);

    url += "?suhu=";
    url += String(suhu, 1);

    url += "&kelembapan=";
    url += String(kelembapan, 1);

    Serial.println("Mengirim data ke Google Sheets...");

    http.begin(url);

    int httpCode = http.GET();

    if (httpCode > 0) {

      Serial.print("HTTP Code: ");
      Serial.println(httpCode);

      String response = http.getString();

      Serial.print("Response: ");
      Serial.println(response);

    } else {

      Serial.print("Gagal mengirim data. Error: ");
      Serial.println(http.errorToString(httpCode));
    }

    http.end();

  } else {

    Serial.println("WiFi terputus!");
  }

  // =========================
  // INTERVAL
  // =========================
  delay(5000);
}