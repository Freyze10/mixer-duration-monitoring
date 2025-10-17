#include <WiFi.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <time.h>

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

void setup() {
  Serial.begin(115200);
  Wire.begin(SDA_PIN, SCL_PIN);

  pinMode(button1, INPUT_PULLUP);
  pinMode(button2, INPUT_PULLUP);

  // Initialize OLED
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("SSD1306 allocation failed");
    for (;;);
  }

  header();
  display.display();

  // Connect to Wi-Fi
  displayMessage("Connecting to WiFi");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(200);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected!");

  // Sync NTP time
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  displayMessage("Getting time...");
  delay(800);

  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    displayMessage("Time sync failed!");
    Serial.println("Failed to obtain time");
    return;
  }
  displayMessage("Time synced!");
  delay(1000);

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

  // Start button
  if (state1 == HIGH && !startPressed) {
    startPressed = true;
    endPressed = false;
    time(&startTime);
    displayStart();
  }

  // End button
  if (state2 == HIGH && startPressed && !endPressed) {
    endPressed = true;
    time(&endTime);
    displayEnd();
    startPressed = false;
  }
}

void displayStart() {
  header();
  display.setTextSize(1);
  display.setCursor(0, 45);
  display.println("Start:");
  display.setCursor(7, 55);
  display.println(getDateTimeString(startTime));
  display.display();

  Serial.print("Start Time: ");
  Serial.println(getDateTimeString(startTime));
}

void displayEnd() {

  Serial.print("End Time: ");
  Serial.println(getDateTimeString(endTime));

  // Compute duration
  double seconds = difftime(endTime, startTime);
  int hours = seconds / 3600;
  int minutes = ((int)seconds % 3600) / 60;
  int secs = (int)seconds % 60;

  Serial.printf("Usage Duration: %02d:%02d:%02d\n", hours, minutes, secs);

  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(10, 20);
  display.println("Total Usage:");
  display.setTextSize(2);
  display.setCursor(13, 35);
  char durationStr[20];
  sprintf(durationStr, "%02d:%02d:%02d", hours, minutes, secs);
  display.println(durationStr);
  display.display();
}
