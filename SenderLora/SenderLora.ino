#include <Wire.h>
#include <SPI.h>
#include <LoRa.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);
#define CARDKB_ADDR 0x5F // Define the I2C address of CardKB.
#define SS 10
#define RST 8
#define DI0 3
#define BAND 865E6

int txPower = 14; // from 0 to 20, default is 14
int spreadingFactor = 12; // from 7 to 12, default is 12
long signalBandwidth = 125E3; // 7.8E3, 10.4E3, 15.6E3, 20.8E3, 31.25E3,41.7E3,62.5E3,125E3,250E3,500e3, default is 125E3
int codingRateDenominator = 5; // Numerator is 4, and denominator from 5 to 8, default is 5
int preambleLength = 8; // from 2 to 20, default is 8
int cursor = 0;

String inputBuffer = "";

void delayreadmessage() {
  int n = 0;
  while (n != 1) {
    Wire.requestFrom(CARDKB_ADDR, 1);
    while (Wire.available()) { // If received data is detected,
      char d = Wire.read();
      if (d != 0) {
        if (d == 0x0D || d == 0x0A) {
          n = 1;
        }
      }
    }
    delay(10); // Small delay to prevent excessive I2C requests
  }
}


void setup() {
  Serial.begin(115200);
  Wire.begin();

  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  
  // Initialize LoRa with explicit pin configuration
  LoRa.setPins(SS, RST, DI0);
  
  if (!LoRa.begin(BAND)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }
  
  Serial.println("LoRa Initialized Successfully!");
  Serial.println("Type and press Enter to send");
  
  // Configure LoRa parameters
  LoRa.setTxPower(txPower, 1);
  LoRa.setSpreadingFactor(spreadingFactor);
  LoRa.setSignalBandwidth(signalBandwidth);
  LoRa.setCodingRate4(codingRateDenominator);
  LoRa.setPreambleLength(preambleLength);
  
  // Enable CRC for error checking
  
  Serial.println("LoRa configuration complete!");
}

void loop() {
int packetSize = LoRa.parsePacket();
  if (packetSize) {
    Serial.print("Received packet '");
    lcd.clear();
    lcd.print("RX: ");
    
    // Read packet character by character
    while (LoRa.available()) {
      char c = (char)LoRa.read();
      Serial.print(c);
      lcd.print(c);
    }
    
    // Print RSSI of packet
    Serial.print("' with RSSI ");
    Serial.println(LoRa.packetRssi());
    
    delayreadmessage();
    lcd.clear();
  }
  Wire.requestFrom(CARDKB_ADDR, 1); // Request 1 byte from the CardKB.
  while (Wire.available()) { // If received data is detected,
    char c = Wire.read(); // store the received data.
    if (c != 0) {
      if (c == 0x0D || c == 0x0A) { // Check for Enter
        if (inputBuffer.length() > 0) {
          Serial.println("You typed: " + inputBuffer);
          
          // Send via LoRa
          Serial.print("Sending LoRa message: ");
          Serial.println(inputBuffer);
          
          LoRa.beginPacket();
          LoRa.print(inputBuffer);
          LoRa.endPacket();
          lcd.clear();  // Efface tout l'écran et repositionne le curseur à (0,0)
          // Wait for transmission to complete
          delay(100);
          
          Serial.println("Message sent!");
          inputBuffer = ""; // Clear the buffer
        }
      } else if (c == 0x08 || c == 0x7F) { // Check for Backspace key
        if (inputBuffer.length() > 0) {
          inputBuffer.remove(inputBuffer.length() - 1); // Remove last character
          Serial.print("\b \b");
        }
      } else {
        inputBuffer += c; // Add character to buffer
        Serial.print(c);
        //Serial.print(inputBuffer);
        lcd.print(c); // Show character being typed (for visual feedback)
      }
    }
  }
}