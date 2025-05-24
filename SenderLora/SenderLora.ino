#include <Wire.h>
#include <SPI.h>
#include <LoRa.h>

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

String inputBuffer = "";

void setup() {
  Serial.begin(115200);
  Wire.begin();
  
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
  LoRa.enableCrc();
  
  Serial.println("LoRa configuration complete!");
}

void loop() {
  Wire.requestFrom(CARDKB_ADDR, 1); // Request 1 byte from the CardKB.
  while (Wire.available()) { // If received data is detected,
    char c = Wire.read(); // store the received data.
    if (c != 0) {
      if (c == 0x0D || c == 0x0A) { // Check for Enter key
        if (inputBuffer.length() > 0) {
          Serial.println("You typed: " + inputBuffer); // print the line serial monitor
          
          // Send via LoRa
          Serial.print("Sending LoRa message: ");
          Serial.println(inputBuffer);
          
          LoRa.beginPacket();
          LoRa.print(inputBuffer);
          LoRa.endPacket();
          
          // Wait for transmission to complete
          delay(100);
          
          Serial.println("Message sent!");
          inputBuffer = ""; // Clear the buffer
        }
      } else if (c == 0x08 || c == 0x7F) { // Check for Backspace key
        if (inputBuffer.length() > 0) {
          inputBuffer.remove(inputBuffer.length() - 1); // Remove last character
          Serial.print("\b \b"); // Visual backspace on serial monitor
        }
      } else {
        inputBuffer += c; // Add character to buffer
        Serial.print(c); // Show character being typed (for visual feedback)
      }
    }
  }
}