#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2); // I2C LCD
String input = "";

void setup() {
  Serial.begin(115200);
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Waiting...");
}

void loop() {
  while (Serial.available()) {
    char c = Serial.read();
    if (c == '\n') {
      input.trim(); // Remove trailing \r or spaces

      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print(input);

      // Optional: reset buffer for next message
      input = "";
    } else {
      input += c;
    }
  }
}
