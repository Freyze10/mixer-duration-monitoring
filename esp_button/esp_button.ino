#include <WiFi.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <time.h>
#include <SD.h>
#include <SPI.h>

// OLED display settings
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define SDA_PIN 25
#define SCL_PIN 26

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// Button pins
int button1 = 13;
int button2 = 12;

// Flags and time variables
bool startPressed = false;
bool endPressed = false;
time_t startTime, endTime;

// WiFi credentials
const char* ssid = "Deco M5-1";
const char* password = "mbpi_2025";

// NTP settings (Philippines timezone: UTC+8)
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 8 * 3600;
const int daylightOffset_sec = 0;

// File
File file;

void setup() {
  Serial.begin(115200);
  Wire.begin(SDA_PIN, SCL_PIN);

  pinMode(button1, INPUT_PULLUP);
  pinMode(button2, INPUT_PULLUP);

  // Initialize OLED display
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("SSD1306 allocation failed!");
    for (;;);
  }
  displayMessage("Initializing SD...");
  delay(500);

  // Initialize SD card using default VSPI pins
  if (!SD.begin(5)) { // CS = 5
    Serial.println("SD Card initialization failed!");
    displayMessage("SD init failed!");
    delay(1500);
  } else {
    Serial.println("SD Card initialized successfully!");
    displayMessage("SD Card Ready!");
    delay(1000);

    // Test file write
    file = SD.open("/test.txt", FILE_WRITE);
    if (file) {
      file.println("SD card test successful!");
      file.close();
      Serial.println("File written successfully!");
      displayMessage("SD Write OK!");
      delay(1000);
    } else {
      Serial.println("File write failed!");
      displayMessage("Write failed!");
      delay(1500);
    }
  }

  // Connect to Wi-Fi
  displayMessage("Connecting WiFi...");
  WiFi.begin(ssid, password);
  int wifi_attempts = 0;
  while (WiFi.status() != WL_CONNECTED && wifi_attempts < 20) {
    delay(500);
    Serial.print(".");
    wifi_attempts++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi connected!");
    displayMessage("WiFi Connected!");
  } else {
    Serial.println("\nWiFi failed to connect!");
    displayMessage("WiFi Failed!");
  }

  // Sync NTP time
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  displayMessage("Syncing Time...");
  delay(1000);

  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    displayMessage("Time sync failed!");
    Serial.println("Failed to obtain time");
  } else {
    displayMessage("Time synced!");
    Serial.println("NTP time synced!");
  }

  delay(1500);
  header();
  display.display();
}

void header() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(30, 10);
  display.println("Masterbatch");
  display.setCursor(12, 20);
  display.println("Philippines, INC.");
  display.setCursor(0, 30);
  display.println("---------------------");
  display.setCursor(28, 45);
  display.println("Press Start!");
  display.display();
}

void displayMessage(String msg) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(10, 30);
  display.println(msg);
  display.display();
}

String getDateTimeString(time_t t) {
  struct tm *timeinfo = localtime(&t);
  char buffer[25];
  strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", timeinfo);
  return String(buffer);
}

void loop() {
  int state1 = digitalRead(button1);
  int state2 = digitalRead(button2);

  // Start button pressed
  if (state1 == HIGH && !startPressed) {
    startPressed = true;
    endPressed = false;
    time(&startTime);
    Serial.print("Start Time: ");
    Serial.println(getDateTimeString(startTime));

    // Log start to SD
    file = SD.open("/log.txt", FILE_APPEND);
    if (file) {
      file.print("Start: ");
      file.println(getDateTimeString(startTime));
      file.close();
    }

    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(10, 10);
    display.println("Counting...");
    display.display();
  }

  // End button pressed
  if (state2 == HIGH && startPressed && !endPressed) {
    endPressed = true;
    time(&endTime);
    displayEnd();
    startPressed = false;
  }

  // If timer is running, update elapsed time every second
  if (startPressed && !endPressed) {
    static unsigned long lastUpdate = 0;
    unsigned long nowMillis = millis();

    if (nowMillis - lastUpdate >= 1000) { // update every 1 second
      lastUpdate = nowMillis;

      time_t currentTime;
      time(&currentTime);

      double seconds = difftime(currentTime, startTime);
      int hours = seconds / 3600;
      int minutes = ((int)seconds % 3600) / 60;
      int secs = (int)seconds % 60;

      char durationStr[20];
      sprintf(durationStr, "%02d:%02d:%02d", hours, minutes, secs);

      display.clearDisplay();
      display.setTextSize(1);
      display.setCursor(10, 10);
      display.println("Usage Duration:");
      display.setTextSize(2);
      display.setCursor(16, 35);
      display.println(durationStr);
      display.display();

      Serial.print("Elapsed: ");
      Serial.println(durationStr);
    }
  }
}


// void displayStart() {
//   display.clearDisplay();
//   display.setCursor(10, 10);
//   display.println("Total Usage:");
//   display.setCursor(5, 20);
//   display.print("from: ");
//   display.println(getDateTimeString(startTime));
//   display.setTextSize(2);
//   // display.setCursor(16, 35);
//   // display.println(durationStr);
//   display.display();

//   Serial.print("Start Time: ");
//   Serial.println(getDateTimeString(startTime));

//   // Log to SD card
//   file = SD.open("/log.txt", FILE_APPEND);
//   if (file) {
//     file.print("Start: ");
//     file.println(getDateTimeString(startTime));
//     file.close();
//   }
// }

void displayEnd() {
  Serial.print("End Time: ");
  Serial.println(getDateTimeString(endTime));

  double seconds = difftime(endTime, startTime);
  int hours = seconds / 3600;
  int minutes = ((int)seconds % 3600) / 60;
  int secs = (int)seconds % 60;

  Serial.printf("Usage Duration: %02d:%02d:%02d\n", hours, minutes, secs);

  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(10, 0);
  display.println("From:");
  display.setCursor(5, 10);
  display.println(getDateTimeString(startTime));
  display.setCursor(10, 20);
  display.println("To:");
  display.setCursor(6, 30);
  display.println(getDateTimeString(endTime));
  display.setCursor(6, 40);
  display.println("Total Usage:");
  display.setTextSize(2);
  display.setCursor(16, 50);
  char durationStr[20];
  sprintf(durationStr, "%02d:%02d:%02d", hours, minutes, secs);
  display.println(durationStr);
  display.display();

  // Log end and duration to SD
  file = SD.open("/log.txt", FILE_APPEND);
  if (file) {
    file.print("End: ");
    file.println(getDateTimeString(endTime));
    file.print("Duration: ");
    file.println(durationStr);
    file.println("--------------------------");
    file.close();
  }
}
