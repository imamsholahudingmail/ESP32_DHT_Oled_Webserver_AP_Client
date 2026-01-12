#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <DHT.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128  // OLED display width, in pixels
#define SCREEN_HEIGHT 64  // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

const char *ssid = "DigPart";
const char *password = "12443411";

// Pengaturan untuk mode AP (jika mode STA gagal)
const char* ssid_ap = "ESP32_AP";
const char* password_ap = "1234567890"; // Password minimal 8 karakter
const int connection_timeout = 20; // Batas waktu koneksi dalam detik


WebServer server(80);
DHT dht(14, DHT22);

void handleRoot() {
  char msg[1500];

  snprintf(msg, 1500,
           "<html>\
  <head>\
    <meta http-equiv='refresh' content='4'/>\
    <meta name='viewport' content='width=device-width, initial-scale=1'>\
    <link rel='stylesheet' href='https://use.fontawesome.com/releases/v5.7.2/css/all.css' integrity='sha384-fnmOCqbTlWIlj8LyTjo7mOUStjsKC4pOpQbqyi7RrhN7udi9RwhKkMHpvLbHG9Sr' crossorigin='anonymous'>\
    <title>ESP32 DHT Server</title>\
    <style>\
    html { font-family: Arial; display: inline-block; margin: 0px auto; text-align: center;}\
    h2 { font-size: 3.0rem; }\
    p { font-size: 3.0rem; }\
    .units { font-size: 1.2rem; }\
    .dht-labels{ font-size: 1.5rem; vertical-align:middle; padding-bottom: 15px;}\
    </style>\
  </head>\
  <body>\
      <h2>ESP32 DHT Server!</h2>\
      <p>\
        <i class='fas fa-thermometer-half' style='color:#ca3517;'></i>\
        <span class='dht-labels'>Temperature</span>\
        <span>%.2f</span>\
        <sup class='units'>&deg;C</sup>\
      </p>\
      <p>\
        <i class='fas fa-tint' style='color:#00add6;'></i>\
        <span class='dht-labels'>Humidity</span>\
        <span>%.2f</span>\
        <sup class='units'>&percnt;</sup>\
      </p>\
  </body>\
</html>",
           readDHTTemperature(), readDHTHumidity()
          );
  server.send(200, "text/html", msg);
}

void setup(void) {

  Serial.begin(115200);
  dht.begin();

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ;
  }
  delay(500);
  display.clearDisplay();
  display.setTextColor(WHITE);
  
  // Inisialisasi WiFi ke mode STA
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");

  int attempts = 0;
  // Tunggu koneksi STA atau sampai batas waktu tercapai
  while (WiFi.status() != WL_CONNECTED && attempts < connection_timeout) {
    delay(1000);
    Serial.print(".");
    display.print(".");
    attempts++;
  }

  // Periksa status koneksi setelah loop
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nKoneksi STA berhasil!");
    Serial.print("Alamat IP STA: ");
    Serial.println(WiFi.localIP());
    // Lanjutkan dengan logika mode STA Anda di sini
  } else {
    Serial.println("\nKoneksi STA gagal. Beralih ke mode Access Point (AP)...");
    activateAPMode();
  }


  if (MDNS.begin("esp32")) {
    Serial.println("MDNS responder started");
  }
  server.on("/", handleRoot);

  server.begin();
  Serial.println("HTTP server started");
}

void activateAPMode() {
  // Matikan mode STA terlebih dahulu
  WiFi.disconnect(); 
  
  // Aktifkan mode AP
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid_ap, password_ap);

  Serial.print("Mode AP aktif. SSID: ");
  Serial.println(ssid_ap);
  Serial.print("Alamat IP AP: ");
  Serial.println(WiFi.softAPIP());
  // Lanjutkan dengan logika mode AP Anda di sini (misalnya, memulai server web)
}

void loop(void) {
  server.handleClient();

  //read temperature and humidity
  float t = dht.readTemperature();
  float h = dht.readHumidity();
  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
  }
  // clear display
  display.clearDisplay();

  // display IP
  display.setTextSize(1);
  display.setCursor(0, 0);
  printWiFiMode(WiFi.getMode());

  // display temperature
  display.setCursor(0, 10);
  display.print("Suhu: ");
  display.setTextSize(2);
  display.setCursor(0, 20);
  display.print(t);
  display.print(" ");
  display.setTextSize(1);
  display.cp437(true);
  display.write(167);
  display.setTextSize(2);
  display.print("C");

  // display humidity
  display.setTextSize(1);
  display.setCursor(0, 35);
  display.print("Kelembaban: ");
  display.setTextSize(2);
  display.setCursor(0, 45);
  display.print(h);
  display.print(" %");

  display.display();
  delay(20);//allow the cpu to switch to other tasks
}


void printWiFiMode(WiFiMode_t mode) {
  switch (mode) {
    case WIFI_MODE_STA:
      display.print("STA: ");
      display.print(WiFi.localIP());
      break;
    case WIFI_MODE_AP:
      display.print("AP: ");
      display.print(WiFi.softAPIP());
      break;
    case WIFI_MODE_APSTA:
      display.print("WIFI_AP_STA (Access Point + Station Mode)");
      break;
    case WIFI_MODE_NULL:
      display.print("WIFI_MODE_NULL (WiFi dinonaktifkan)");
      break;
    default:
      display.print("Mode tidak dikenal");
      break;
  }
}

float readDHTTemperature() {
  // Sensor readings may also be up to 2 seconds
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();
  if (isnan(t)) {    
    Serial.println("Failed to read from DHT sensor!");
    return -1;
  }
  else {
    //Serial.println(t);
    return t;
  }
}

float readDHTHumidity() {
  // Sensor readings may also be up to 2 seconds
  float h = dht.readHumidity();
  if (isnan(h)) {
    Serial.println("Failed to read from DHT sensor!");
    return -1;
  }
  else {
    //Serial.println(h);
    return h;
  }
}