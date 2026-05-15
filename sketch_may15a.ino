#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include "BluetoothSerial.h"

// Bluetooth Serial object initialization
BluetoothSerial SerialBT;

// LCD initialization (Address: 0x27) - SDA=GPIO 21, SCL=GPIO 22
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Hardware Pin Definitions
const int motorPin = 34;   // Analog input pin for Generator voltage sensing
const int mosfetPin = 4;   // Control pin for the BJT/MOSFET switching matrix

void setup() {
  // Initialize hardware serial for debugging
  Serial.begin(115200);
  
  // Initialize Bluetooth with the project SSID
  SerialBT.begin("VibTronics"); 
  
  // Initialize I2C communication and LCD display
  Wire.begin(21, 22); 
  lcd.init();         
  lcd.backlight();    
  
  // Configure switching pin as Output
  pinMode(mosfetPin, OUTPUT);
  digitalWrite(mosfetPin, LOW); // Default state: LOAD OFF
  
  // System startup splash screen
  lcd.setCursor(0, 0);
  lcd.print("System Loading..");
  delay(2000);
  lcd.clear();
}

void loop() {
  // Read raw ADC value (Range: 0 - 4095)
  int raw = analogRead(motorPin);
  
  // Convert raw value to voltage based on ESP32 3.3V reference
  float vOut = raw * (3.3 / 4095.0);
  
  // Calculate Actual Generator Voltage based on the Voltage Divider (10k & 4.7k)
  float vGen = vOut / (4.7 / (10.0 + 4.7)); 

  // --- 1. Local LCD Monitoring ---
  lcd.setCursor(0, 0);
  lcd.print("Volt: ");
  lcd.print(vGen, 2);
  lcd.print(" V   ");

  // --- 2. Wireless Bluetooth Telemetry ---
  SerialBT.print("Voltage: ");
  SerialBT.print(vGen);
  SerialBT.println(" V");

  lcd.setCursor(0, 1);
  
  // --- 3. Switching Control Logic ---
  // Trigger threshold set to 2.0V to filter out low-level noise
  if (vGen >= 2.0) { 
    // Load Activation Logic
    digitalWrite(mosfetPin, LOW); // Specific hardware configuration for active state
    lcd.print("STATUS: ON     ");
    SerialBT.println(">> LOAD IS ON");
  } else {
    // Load Deactivation Logic
    digitalWrite(mosfetPin, HIGH); 
    lcd.print("STATUS: OFF    ");
    SerialBT.println(">> LOAD IS OFF");
  }
  
  // Update frequency: 3.3Hz
  delay(300); 
}