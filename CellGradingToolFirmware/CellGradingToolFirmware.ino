/*
 * Project Name: Cell Grading Tool
 * Project Brief: Firmware for Cell Grading Tool built around STM32G071KBUx and INA236
 * Author: Jobit Joseph @ https://github.com/jobitjoseph
 * IDE: Arduino IDE 2.x.x
 * Arduino Core: STM32 Arduino Core V 2.7.1
 * Dependencies : PRDC_RS485HD_STM32 V 1.0.1 @ https://github.com/PR-DC/PRDC_RS485HD_STM32/
 *                INA236 Library V 0.1.0 @ https://github.com/RobTillaart/INA236
 * Copyright © Jobit Joseph
 * Copyright © Semicon Media Pvt Ltd
 * Copyright © Circuitdigest.com
 * 
 * This code is licensed under the following conditions:
 *
 * 1. Non-Commercial Use:
 * This program is free software: you can redistribute it and/or modify it
 * for personal or educational purposes under the condition that credit is given 
 * to the original author. Attribution is required, and the original author 
 * must be credited in any derivative works or distributions.
 *
 * 2. Commercial Use:
 * For any commercial use of this software, you must obtain a separate license
 * from the original author. Contact the author for permissions or licensing
 * options before using this software for commercial purposes.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
 * FITNESS FOR A PARTICULAR PURPOSE, AND NONINFRINGEMENT. IN NO EVENT SHALL 
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES, OR OTHER 
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT, OR OTHERWISE, ARISING 
 * FROM, OUT OF, OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER 
 * DEALINGS IN THE SOFTWARE.
 *
 * Author: Jobit Joseph
 * Date: 3 October 2024
 *
 * For commercial use or licensing requests, please contact [Your Contact Info].
 */


#include "stm32g0xx.h"
#include <Wire.h>
#include <PRDC_RS485HD_STM32.h>
#include "INA236.h"
#define EEPROM_ADDRESS 0x50
// Define variables for RS485
// --------------------
#define DE_PIN PB3  // RS485 transceiver DE pin
#define RE_PIN PB4  // RS485 transceiver RE pin
#define PIN_SERIAL1_TX PB6
#define PIN_SERIAL1_RX PB7
#define SERIAL_BAUDRATE 1000000
const int bufferSize = 200;    // Define buffer size
char inputBuffer[bufferSize];  // Buffer to store incoming data

int numDigits = 2;
int digitPins[2] = { PA8, PB0 };
int segPins[8] = { PB2, PB1, PA3, PA2, PA1, PA7, PA5, PA6 };  // A-G and DP
bool isComCathode = false;
// Define timing variables
unsigned long lastBlinkTime = 0;
bool showValue = true;
bool showDP = false;
unsigned long lastToggleTime = 0;
int displayState = 0;  // 0 = show value, 1 = blank, 2 = show --, 3 = blank again

float value = 0.0;
int ERC = 0;
bool ErrC = false;

uint8_t CID = 0x00;
uint8_t CMD_STS = 0x51;
uint8_t CMD_HST = 0x11;
uint8_t CMD_DCCONFIG = 0xD5;
uint8_t Mode = 0x09;
uint16_t CUTVoltage;
uint16_t CUTCurrent;
uint16_t CUTCapacity;
uint16_t DCVoltage;
uint8_t DCVoltage2;
uint16_t DCCurrent;
uint16_t LastData = 0;
uint16_t Mode_address = 600;
uint16_t DCVoltage_address = 610;
uint16_t DCCurrent_address = 620;
uint16_t CUTCapacity_address = 630;
uint16_t LastData_address = 640;
uint8_t Discharge_Voltage_Array[500];
unsigned long LastUpdate = 0;
unsigned long LastCapUpdate = 0;
unsigned long LastDC = 0;
bool StartDC = false;
unsigned long LastDCStart = 0;
uint8_t T8emp = 0;
uint16_t T16emp = 0;
unsigned long Lastflip = 0;
float Capacity;
float Current;
const unsigned long TIMEOUT = 1000;
//Charactor array for the display
bool DIGITS[17][7] = {
  // A     B      C     D     E     F     G
  { true, true, true, true, true, true, false },       // 0
  { false, true, true, false, false, false, false },   // 1
  { true, true, false, true, true, false, true },      // 2
  { true, true, true, true, false, false, true },      // 3
  { false, true, true, false, false, true, true },     // 4
  { true, false, true, true, false, true, true },      // 5
  { true, false, true, true, true, true, true },       // 6
  { true, true, true, false, false, false, false },    // 7
  { true, true, true, true, true, true, true },        // 8
  { true, true, true, false, false, true, true },      // 9
  { true, true, true, false, true, true, true },       // A
  { false, false, true, true, true, true, true },      // b
  { true, false, false, true, true, true, false },     // C
  { false, true, true, true, true, false, true },      // d
  { true, false, false, true, true, true, true },      // E
  { true, false, false, false, true, true, true },     // F
  { false, false, false, false, false, false, false }  // blank
};
//Init instances
HardwareTimer *MyTim = nullptr;
INA236 INA(0x48);
PRDC_RS485HD_STM32 VFD_HS(PIN_SERIAL1_RX, PIN_SERIAL1_TX);


/* Display related Functions Start*/
/*................................*/
void InitDisplay() {

  //Init digit pins
  for (int i = 0; i < numDigits; i++) {
    pinMode(digitPins[i], OUTPUT);
  }

  //Init segment pins
  for (int i = 0; i <= 7; i++) {
    pinMode(segPins[i], OUTPUT);
  }

  //determines common type and preps pin states
  if (!isComCathode) {
    for (int i = 0; i < numDigits; i++) {
      digitalWrite(digitPins[i], HIGH);
    }

    for (int i = 0; i <= 7; i++) {
      digitalWrite(segPins[i], HIGH);
    }
  }
}
// Function to display hex value
void DisplayHex(int hexValue) {
  int firstDigit = (hexValue >> 4) & 0xF;  // Get the higher nibble
  int secondDigit = hexValue & 0xF;        // Get the lower nibble

  int VArray[2] = { secondDigit, firstDigit };  // Display first high, then low

  for (int dig = 0; dig < numDigits; dig++) {
    digitalWrite(digitPins[dig], isComCathode ? LOW : HIGH);

    for (int seg = 0; seg <= 6; seg++) {
      digitalWrite(segPins[seg], isComCathode ? DIGITS[VArray[dig]][seg] : !DIGITS[VArray[dig]][seg]);
    }
    // Ensure DP (Decimal Point) is off when displaying hex
    digitalWrite(segPins[7], isComCathode ? LOW : HIGH);  // Turn off DP for hex
    delay(5);                                             // Small delay to avoid flickering
    digitalWrite(digitPins[dig], isComCathode ? HIGH : LOW);
  }
}

// Function to display float or int value
void DisplayFloat(float n) {
  int VArray[numDigits];
  int pointPos = numDigits + 1;
  if (n > 0.99) {
    while (n > 1.0) {
      n /= 10.0;
      pointPos--;
    }
    n *= (float)pow(10.0, (float)numDigits);

    for (int i = 0; i < numDigits; i++) {
      VArray[i] = (int)n % 10;
      n /= 10;
    }
  } else {
    pointPos = 2;
    VArray[1] = 0;
    VArray[0] = n * 10;
  }

  for (int dig = 0; dig < numDigits; dig++) {
    digitalWrite(digitPins[dig], isComCathode ? LOW : HIGH);

    for (int seg = 0; seg <= 6; seg++) {
      digitalWrite(segPins[seg], isComCathode ? DIGITS[VArray[dig]][seg] : !DIGITS[VArray[dig]][seg]);
    }

    if (pointPos == dig + 1) {
      digitalWrite(segPins[7], isComCathode ? HIGH : LOW);
    } else {
      digitalWrite(segPins[7], isComCathode ? LOW : HIGH);
    }

    delay(5);
    digitalWrite(digitPins[dig], isComCathode ? HIGH : LOW);
  }
}


// Function to manage the blinking DP for Mode 0x01
void Mode01_BlinkDP() {
  unsigned long currentMillis = millis();

  // Toggle DP every 500ms
  if (currentMillis - lastBlinkTime >= 500) {
    showDP = !showDP;
    lastBlinkTime = currentMillis;
  }

  // Show the float value with the blinking DP
  DisplayFloatWithBlinkingDP(value, showDP);
}

// Function to manage display updates for Mode 0x03
void Mode03_DisplayCycle() {
  unsigned long currentMillis = millis();

  switch (displayState) {
    case 0:  // Show the value
      if (currentMillis - lastToggleTime >= 1000) {
        displayState = 1;
        lastToggleTime = currentMillis;
      }
      DisplayFloat(value);
      break;

    case 1:  // Show blank for 500ms
      if (currentMillis - lastToggleTime >= 500) {
        displayState = 2;
        lastToggleTime = currentMillis;
      }
      DisplayBlank();
      break;

    case 2:  // Show "--" for 1000ms
      if (currentMillis - lastToggleTime >= 1000) {
        displayState = 3;
        lastToggleTime = currentMillis;
      }
      DisplayDashes();
      break;

    case 3:  // Show blank for 500ms
      if (currentMillis - lastToggleTime >= 500) {
        displayState = 0;
        lastToggleTime = currentMillis;
      }
      DisplayBlank();
      break;
  }
}

// Function to display blank (turn off all segments)
void DisplayBlank() {
  for (int dig = 0; dig < numDigits; dig++) {
    digitalWrite(digitPins[dig], isComCathode ? HIGH : LOW);  // Turn off digits
  }
}

// Function to display "--" on both digits
void DisplayDashes() {
  for (int dig = 0; dig < numDigits; dig++) {
    digitalWrite(digitPins[dig], isComCathode ? LOW : HIGH);

    for (int seg = 0; seg <= 7; seg++) {
      // Only turn on segment G to display "-"
      digitalWrite(segPins[seg], (seg == 6) ? (isComCathode ? HIGH : LOW) : (isComCathode ? LOW : HIGH));
    }

    delay(5);
    digitalWrite(digitPins[dig], isComCathode ? HIGH : LOW);  // Turn off digit after displaying
  }
}

// Update the display based on the mode
void Update_IT_callback(void) {
  if (ErrC) {
    DisplayHex(ERC);  // Display hex value if error
  } else {
    if (Mode == 0x03) {
      Mode03_DisplayCycle();  // Handle Mode 0x03 display logic
    } else if (Mode == 0x01) {
      Mode01_BlinkDP();  // Handle Mode 0x01 DP blinking logic
    } else {
      DisplayFloat(value);  // Default display
    }
  }
}

// Helper function to display float with optional DP blinking
void DisplayFloatWithBlinkingDP(float n, bool showDP) {
  int VArray[numDigits];
  int pointPos = numDigits + 1;
  if (n > 0.99) {
    while (n > 1.0) {
      n /= 10.0;
      pointPos--;
    }
    n *= (float)pow(10.0, (float)numDigits);

    for (int i = 0; i < numDigits; i++) {
      VArray[i] = (int)n % 10;
      n /= 10;
    }
  } else {
    pointPos = 2;
    VArray[1] = 0;
    VArray[0] = n * 10;
  }

  for (int dig = 0; dig < numDigits; dig++) {
    digitalWrite(digitPins[dig], isComCathode ? LOW : HIGH);

    for (int seg = 0; seg <= 6; seg++) {
      digitalWrite(segPins[seg], isComCathode ? DIGITS[VArray[dig]][seg] : !DIGITS[VArray[dig]][seg]);
    }

    if (dig == 0 && showDP) {
      digitalWrite(segPins[7], isComCathode ? HIGH : LOW);  // Show or hide the DP
    } else {
      digitalWrite(segPins[7], isComCathode ? LOW : HIGH);  // Keep DP off for other digits
    }
    if (dig == 1) {
      digitalWrite(segPins[7], isComCathode ? HIGH : LOW);  // Show or hide the DP
    }

    delay(5);
    digitalWrite(digitPins[dig], isComCathode ? HIGH : LOW);
  }
}
/* Display related Functions End*/
/*................................*/


/* EEPROM related Functions Start*/
/*...............................*/
void write8BitToEEPROM(uint16_t address, uint8_t value) {
  Wire.beginTransmission(EEPROM_ADDRESS);
  Wire.write((uint8_t)(address >> 8));    // MSB of address
  Wire.write((uint8_t)(address & 0xFF));  // LSB of address
  Wire.write(value);                      // Data byte
  Wire.endTransmission();
  delay(5);  // EEPROM write delay
}

// Function to read an 8-bit value from the EEPROM
uint8_t read8BitFromEEPROM(uint16_t address) {
  Wire.beginTransmission(EEPROM_ADDRESS);
  Wire.write((uint8_t)(address >> 8));    // MSB of address
  Wire.write((uint8_t)(address & 0xFF));  // LSB of address
  Wire.endTransmission();

  Wire.requestFrom(EEPROM_ADDRESS, 1);  // Request one byte
  return Wire.read();
}

// Function to write a 16-bit value to the EEPROM
void write16BitToEEPROM(uint16_t address, uint16_t value) {
  uint8_t highByte = (uint8_t)(value >> 8);   // Extract high byte
  uint8_t lowByte = (uint8_t)(value & 0xFF);  // Extract low byte

  // Write the low byte to the first address
  write8BitToEEPROM(address, lowByte);

  // Write the high byte to the address + 500
  write8BitToEEPROM(address + 250, highByte);
}

// Function to read a 16-bit value from the EEPROM
uint16_t read16BitFromEEPROM(uint16_t address) {
  // Read the low byte from the first address
  uint8_t lowByte = read8BitFromEEPROM(address);

  // Read the high byte from the address + 500
  uint8_t highByte = read8BitFromEEPROM(address + 250);

  // Combine high and low bytes into a 16-bit value
  return (uint16_t)((highByte << 8) | lowByte);
}

void clear_data() {

  write8BitToEEPROM(Mode_address, 0x09);
  write8BitToEEPROM(DCVoltage_address, 0x00);
  write16BitToEEPROM(DCCurrent_address, 0x0000);
  write16BitToEEPROM(CUTCapacity_address, 0x0000);
  write16BitToEEPROM(LastData_address, 0x0000);
  for (int i = 1; i < 500; i++) {
    Discharge_Voltage_Array[i] = 0;
    write8BitToEEPROM(i, 0x00);
  }

  Mode = 0x09;
  DCVoltage = 0;
  DCCurrent = 0;
  LastData = 0;
  CUTCapacity = 0;
}
void init_EEPROM() {
  //status = read8BitFromEEPROM(status_address);
  Mode = read8BitFromEEPROM(Mode_address);
  DCVoltage2 = read8BitFromEEPROM(DCVoltage_address);
  if (DCVoltage2 > 0) {
    DCVoltage = (DCVoltage2 * 10) + 2500;
  }
  DCCurrent = read16BitFromEEPROM(DCCurrent_address);
  Capacity = read16BitFromEEPROM(CUTCapacity_address);
  LastData = read16BitFromEEPROM(LastData_address);

  for (int i = 0; i < 500; i++) {
    Discharge_Voltage_Array[i] = read8BitFromEEPROM(i);
  }
}
/* EEPROM related Functions End*/
/*.............................*/


/* COM related Functions Start*/
/*............................*/
void handleSerialRequest() {
  unsigned long startMillis = millis();  // Record the start time

  // Wait for at least 2 bytes (deviceID and command) with timeout
  while (VFD_HS.available() < 2) {
    if (millis() - startMillis > TIMEOUT) {
      VFD_HS.flush();  // Timeout occurred, clear the buffer
      return;
    }
  }

  uint8_t receivedID = VFD_HS.read();  // First byte: Device ID
  if (receivedID != CID) {
    // If device ID doesn't match, clear the buffer
    VFD_HS.flush();
    return;
  }

  uint8_t command = VFD_HS.read();  // Second byte: Command

  if (command == 0x51) {
    // Handle command 0x51
    sendResponseForCommand51();
  } else if (command == 0x11) {
    // Handle command 0x11
    sendResponseForCommand11();
  } else if (command == 0xA3) {
    // Handle command 0xA3
    VFD_HS.write(CID);
    VFD_HS.write(0XA3);
    VFD_HS.write((DCCurrent >> 8) & 0xFF);  // Send MSB of CurrentC
    VFD_HS.write(DCCurrent & 0xFF);         // Send LSB of CurrentC
    VFD_HS.write((DCVoltage >> 8) & 0xFF);  // Send MSB of CurrentC
    VFD_HS.write(DCVoltage & 0xFF);         // Send LSB of CurrentC
  } else if (command == 0xA4) {
    // Handle command 0xA4
    VFD_HS.write(CID);
    VFD_HS.write(0XA4);
    VFD_HS.write((LastData >> 8) & 0xFF);  // Send MSB of CurrentC
    VFD_HS.write(LastData & 0xFF);         // Send LSB of CurrentC
  } else if (command == 0xD3) {
    // Handle command 0xD3
    VFD_HS.write(CID);
    VFD_HS.write(0XD3);
    if (Mode == 0x02) {
      digitalWrite(PA12, LOW);
      Mode = 0X01;
      write8BitToEEPROM(Mode_address, Mode);
      SetCurrent(DCCurrent);
    }
  } else if (command == 0xD7) {
    // Handle command 0xD7
    digitalWrite(PA12, LOW);
    digitalWrite(PA4, LOW);
    pinMode(PA4, INPUT);
    write8BitToEEPROM(Mode_address, 0x09);
    write8BitToEEPROM(DCVoltage_address, 0x00);
    write16BitToEEPROM(DCCurrent_address, 0x0000);
    write16BitToEEPROM(CUTCapacity_address, 0x0000);
    write16BitToEEPROM(LastData_address, 0x0000);
    for (int i = 1; i < 500; i++) {
      Discharge_Voltage_Array[i] = 0;
      write8BitToEEPROM(i, 0x00);
    }
    Mode = 0x09;
    DCVoltage = 0;
    DCVoltage2 = 0;
    DCCurrent = 0;
    LastData = 0;
    CUTCapacity = 0;
  } else if (command == 0xD9) {
    // Handle command 0x11
    Mode = 0x09;
    write8BitToEEPROM(Mode_address, Mode);
    digitalWrite(PA12, LOW);
    digitalWrite(PA4, LOW);
    pinMode(PA4, INPUT);
  } else if (command == 0xD5 && ErrC == false) {
    // Handle command 0xD5 (5-byte request)
    startMillis = millis();  // Reset the start time
    while (VFD_HS.available() < 3) {
      if (millis() - startMillis > TIMEOUT) {
        VFD_HS.flush();  // Timeout occurred, clear the buffer
        return;
      }
    }
    uint8_t setC_MSB = VFD_HS.read();
    uint8_t setC_LSB = VFD_HS.read();
    DCCurrent = (setC_MSB << 8) | setC_LSB;  // Combine two bytes into setC
    if (DCCurrent < 0 || DCCurrent > 5000) {
      DCCurrent = 0;
      VFD_HS.flush();  // Timeout occurred, clear the buffer
      return;
    }

    startMillis = millis();  // Reset the start time for the fifth byte
    while (VFD_HS.available() < 1) {
      if (millis() - startMillis > TIMEOUT) {
        VFD_HS.flush();  // Timeout occurred, clear the buffer
        return;
      }
    }
    DCVoltage2 = VFD_HS.read();
    DCVoltage = abs((DCVoltage2 * 10) + 2500);  // Fifth byte into setV
    if (DCVoltage > 4200 || DCVoltage < 0) {
      DCVoltage = 0;
      DCVoltage2 = 0;
      return;
    }

    write16BitToEEPROM(DCVoltage_address, DCVoltage2);
    write16BitToEEPROM(DCCurrent_address, DCCurrent);
    write16BitToEEPROM(CUTCapacity_address, 0x0000);
    write16BitToEEPROM(LastData_address, 0x0000);
    for (int i = 1; i < 500; i++) {
      Discharge_Voltage_Array[i] = 0;
      write8BitToEEPROM(i, 0x00);
    }
    Mode = 0x02;
    write8BitToEEPROM(Mode_address, Mode);
    digitalWrite(PA4, LOW);
    pinMode(PA4, INPUT);
    digitalWrite(PA12, HIGH);
    LastData = 0;
    CUTCapacity = 0;
    LastDC = millis();
  } else {
    // If command doesn't match, clear the buffer
    VFD_HS.flush();
    //VFD_HS.print("Error");
    return;
  }
}

void sendResponseForCommand51() {


  VFD_HS.write(CID);
  VFD_HS.write(Mode);
  int j = (CUTVoltage - 2500) / 10;
  uint8_t tmpv = (uint8_t)j;
  VFD_HS.write(tmpv);
  VFD_HS.write((CUTCurrent >> 8) & 0xFF);   // Send MSB of CurrentC
  VFD_HS.write(CUTCurrent & 0xFF);          // Send LSB of CurrentC
  VFD_HS.write((CUTCapacity >> 8) & 0xFF);  // Send MSB of Currentcap
  VFD_HS.write(CUTCapacity & 0xFF);         // Send LSB of Currentcap
}

void sendResponseForCommand11() {
  VFD_HS.write(CID);

  // Send Vhistory values up to lastV
  for (uint8_t i = 0; i < LastData; i++) {
    VFD_HS.write(Discharge_Voltage_Array[i]);
  }

  VFD_HS.write(0xFF);  // End with 0xFF
  VFD_HS.write(0xFF);  // End with 0xFF
}
/* COM related Functions End*/
/*..........................*/


/* INA236 Sensor related Functions Start*/
/*......................................*/
void ReadINA() {
  CUTVoltage = (uint16_t)(INA.getBusVoltage() * 1000.0);
  value = (float)CUTVoltage / 1000.0;  // Perform floating-point division
  if (value <= 0) {
    ErrC = true;
    ERC = 0xE0;
    digitalWrite(PA12, LOW);
    digitalWrite(PA4, LOW);
    pinMode(PA4, INPUT);
    Mode = 0x09;
    if (read8BitFromEEPROM(Mode_address) != 0x09) {
      write8BitToEEPROM(Mode_address, Mode);
      CUTCapacity = 0;
      write16BitToEEPROM(CUTCapacity_address, 0x0000);
    }
  } else {
    ErrC = false;
  }
  Current = fabs(INA.getCurrent_mA());  // Get the absolute value of the current
  if (Current < 10 || Mode == 0x09) {
    Current = 0;
  }
  CUTCurrent = (uint16_t)round(Current);
}
/* INA236 Sensor related Functions End*/
/*....................................*/

/* DAC/Load related Functions Start*/
/*.................................*/
void SetCurrent(uint16_t current_mA) {
  // Calculate the DAC output voltage in volts (10 mV per A or 1 mV per 100 mA for 0.01 Ohms Shunt)
  float voltage_DAC = current_mA * 0.0001;  // Convert mA to volts
  // Calculate the DAC value for 3.3V Vref
  uint16_t dacValue = (uint16_t)((voltage_DAC / 3.342) * 65535.0);
  dacValue *= 2;
  // Calculate the DAC value for internal 1.2V Vref
  //uint16_t dacValue = (voltage_DAC / 1.2) * 65535;
  // Output the calculated DAC value to PA4

  //VFD_HS.println("SetC ");

  pinMode(PA4, OUTPUT);
  analogWrite(PA4, dacValue);
}


void SetCurrentFix(uint16_t desiredCurrent_mA) {
  if (Mode == 0x01) {
    //VFD_HS.println("SetFix ");

    // Read the actual current from INA219 (in mA)
    float actualCurrent_mA = INA.getCurrent_mA();

    // Calculate the error (difference between desired and actual current)
    float error = desiredCurrent_mA - actualCurrent_mA;

    // Calculate the baseline DAC value for 3.3V Vref
    float voltage_DAC = desiredCurrent_mA * 0.0001;  // Convert desired mA to volts
    uint16_t baselineDacValue = (uint16_t)((voltage_DAC / 3.3) * 65535.0);

    // Double the DAC value as per your requirement
    baselineDacValue *= 2;

    // Adjust DAC value using the ratio of desired current to actual current
    if (actualCurrent_mA > 0) {
      float adjustmentFactor = (float)desiredCurrent_mA / actualCurrent_mA;
      baselineDacValue = (uint16_t)(baselineDacValue * adjustmentFactor);
    }

    // Ensure the DAC value stays within the valid range (0 - 65535)
    if (baselineDacValue > 65535) {
      baselineDacValue = 65535;
    } else if (baselineDacValue < 0) {
      baselineDacValue = 0;
    }

    // Output the adjusted DAC value to PA4
    analogWrite(PA4, baselineDacValue);
  }
}

/* DAC/Load related Functions End */
/*................................*/

/* Voltage Converstion Function Start */
/*....................................*/
uint8_t VoltageConversion(uint16_t voltage) {
  if (voltage >= 2500) {
    uint16_t result = (voltage - 2500) / 10;
    if (result > 255) {
      result = 255;  // Cap the result to the maximum value for an 8-bit unsigned integer
    }
    return (uint8_t)result;
  } else {
    return 0;  // If voltage is less than 2500, return 0
  }
}
/* Voltage Converstion Function End */
/*..................................*/

void setup() {
  VFD_HS.setPins(DE_PIN, RE_PIN);
  VFD_HS.begin(1000000, SERIAL_8N1);
  pinMode(PA4, INPUT);
  pinMode(PA12, OUTPUT);
  digitalWrite(PA12, LOW);
  //enableInternalVref();
  analogWriteResolution(16);
  pinMode(PA11, INPUT_PULLUP);
  pinMode(PA15, INPUT_PULLUP);

  TIM_TypeDef *Instance = TIM2;  // Using TIM2 for this example
  MyTim = new HardwareTimer(Instance);
  MyTim->setOverflow(60, HERTZ_FORMAT);  // Set interrupt frequency to 60 Hz
  MyTim->attachInterrupt(Update_IT_callback);
  MyTim->resume();

  InitDisplay();
  Wire.setClock(1000000);
  Wire.begin();
  if (!INA.begin()) {
    //VFD_HS.println("could not connect. Fix and Reboot");
  }
  INA.setAverage(6);
  INA.setMaxCurrentShunt(8, 0.01, true);
  ReadINA();
  //INA.setMaxCurrentShunt(8, 0.01, false);
  if (read8BitFromEEPROM(510) == 22) {
    clear_data();
    write8BitToEEPROM(510, 0x22);
  }
  init_EEPROM();
  if (Mode == 0x01) {
    SetCurrent(DCCurrent);
  } else if (Mode == 0x02) {
    digitalWrite(PA12, HIGH);
    LastDC = millis();
  }
  LastUpdate = millis();
}

void loop() {

  ReadINA();
  if (VFD_HS.available() > 0) {
    handleSerialRequest();
  }
  if (Mode == 0X02 && millis() - LastDC > 2000) {
    if (!digitalRead(PA11)) {
      digitalWrite(PA12, LOW);
      StartDC = true;
      LastDCStart = millis();
    }
  } else if (Mode == 0x01) {
    digitalWrite(PA12, LOW);
    SetCurrentFix(DCCurrent);
    if ((millis() - LastCapUpdate) >= 1000) {
      Capacity += (Current * (millis() - LastCapUpdate)) / 3600000.0;
      //Capacity += (Current * ((millis() - LastCapUpdate) / 1000.0)) / 3600.0;
      CUTCapacity = (uint16_t)Capacity;
      LastCapUpdate = millis();
    }
    if ((millis() - LastUpdate) > 30000) {
      //array+eeprom update
      write16BitToEEPROM(LastData_address, LastData);
      T8emp = VoltageConversion(CUTVoltage);
      Discharge_Voltage_Array[LastData] = T8emp;
      write8BitToEEPROM(LastData, T8emp);
      CUTCapacity = (uint16_t)Capacity;
      write16BitToEEPROM(CUTCapacity_address, CUTCapacity);
      LastData++;
      LastUpdate = millis();
    }
    if (CUTVoltage <= DCVoltage) {
      Capacity += (Current * (millis() - LastCapUpdate)) / 3600000.0;
      //Capacity += (Current * ((millis() - LastCapUpdate) / 1000.0)) / 3600.0;
      CUTCapacity = (uint16_t)Capacity;
      write16BitToEEPROM(CUTCapacity_address, CUTCapacity);
      Mode = 0x03;
      write8BitToEEPROM(Mode_address, 0x03);
      write8BitToEEPROM(DCVoltage_address, 0x00);
      write16BitToEEPROM(DCCurrent_address, 0x0000);
      DCVoltage = 0;
      DCVoltage2 = 0;
      DCCurrent = 0;
      digitalWrite(PA4, LOW);
      pinMode(PA4, INPUT);
    }
  }
  if (StartDC == true && (millis() - LastDCStart) > 2000) {
    StartDC = false;
    Mode = 0X01;
    write8BitToEEPROM(Mode_address, Mode);
    SetCurrent(DCCurrent);
  }
  if (Mode == 0x09 || Mode == 0x03) {
    digitalWrite(PA12, LOW);
    //digitalWrite(PA4, 0);
  }
}
