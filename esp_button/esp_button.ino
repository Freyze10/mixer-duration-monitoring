#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

// Use your custom I2C pins
#define SDA_PIN 25
#define SCL_PIN 26

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

int button1 = 13;
int button2 = 12;



void setup() {
  Serial.begin(115200);
  Wire.begin(SDA_PIN, SCL_PIN);

  // Initialize OLED display
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }

  pinMode(button1, INPUT_PULLUP);
  pinMode(button2, INPUT_PULLUP);

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
void loop() {
  // put your main code here, to run repeatedly:
  int state1 = digitalRead(button1);
  int state2 = digitalRead(button2);

  if (state1 == LOW && state2 == LOW) {
    display.clearDisplay();
    header();
    display.display();
  }
  if (state1 == HIGH) {
    header();
    display.setCursor(16, 44);
    display.setTextSize(3);
    display.println("Start");
    display.display();
  }
  
  if (state2 == HIGH) {
    header();
    display.setCursor(36, 44);
    display.setTextSize(3);
    display.println("End");
    display.display();
  }
}
