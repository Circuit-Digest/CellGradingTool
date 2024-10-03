/*
 * Project Name: Cell Grading Tool
 * Project Brief: Companion App for Cell Grading Tool
 * Author: Aswinth Raj @ https://github.com/Aswinth-raj
 * IDE: Processing V 4.3
 * Copyright © Aswinth Raj 
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
 * Author: Aswinth Raj
 * Date: 3 October 2024
 *
 * For commercial use or licensing requests, please contact [Your Contact Info].
 */

PImage img;
import processing.serial.*;

Serial myPort;  // The serial port
byte[] dischargeCommand = { (byte) 0x00, (byte) 0xD5, (byte) 0x09, (byte) 0xC4, (byte) 0x46 };

byte[] statusCommand = { (byte) 0x00, (byte) 0x51 };
byte[] statusReceived = new byte[7];  // Buffer to store the 7 bytes received
int[] statusData = new int[5];  // Size of 5 based on your data array: cellID, cellStatus, voltage, current, capacity
int[] voltageHistory;  // Declare globally for plotting graph

ArrayList<Byte> historyReceived = new ArrayList<Byte>();
boolean dataAvailable = false;

BatteryCell[] batteries = new BatteryCell[8];  // Array to hold 16 batteries
int selectedCell = -1;  // To track which cell is clicked for displaying graph
int readCell = -1;

String cellCapacity = "";
String dischargeCurrent = "";
String cutoffVoltage = "";
boolean submitted = false;
boolean plot = false;

void setup() {
  
  
  println(dischargeCommand);
  
  statusReceived = new byte[0]; 
  // List all the available serial ports
  println(Serial.list());
  
  // Initialize serial port at baud rate of 115200. Change port index accordingly.
  myPort = new Serial(this, "/dev/tty.usbmodem56580042701", 1000000);
  myPort.clear();
  
  size(800, 600);
  
  img = loadImage("cdlogo.png");
  
  // Initialize battery cells (4 on top and 4 on bottom)
  int count = 0;
  for (int i = 0; i < 4; i++) {
    batteries[count++] = new BatteryCell(100 * i + 50, 120);  // Top row
    batteries[count++] = new BatteryCell(100 * i + 50, 330);  // Bottom row
  }
}

void draw() {
  background(240);
  
  fill(0);
  textSize(32);             // Set font size
  text("Cell Grading Machine", width / 2, 40);  // Centered horizontally, near the top
  image(img, width-220, height-80); //display logo
  
  // Draw each battery
  for (int i = 0; i < batteries.length; i++) {
    batteries[i].display();
  }
  
  // Display data for selected cell
  if (selectedCell != -1) {
    batteries[selectedCell].drawTable();
    batteries[selectedCell].displayGraph();
    batteries[selectedCell].selectionIndicator();

    if(plot)
    batteries[selectedCell].updateGraph();
  }
  
  //Check if we have got history data and plot graph
  if (historyAvailable == true) {
    int validDataSize = historyReceived.size() - 2;
    voltageHistory = new int[validDataSize];
    //display recived voltage
    for (int i = 0; i < validDataSize; i++)
    {
    int rawByte = historyReceived.get(i) & 0xFF;  // Convert to unsigned
    int voltage = ((rawByte + 250) * 10);  // Apply your specific mapping
    voltageHistory[i] = voltage;
    println(voltage);
    }
    
    historyAvailable = false;
    print("End of History Updating");
    statusReceived = new byte[0];  // Clear status buffer for the next message
    statusAvailable = false;
    plot = true;
    
    //while(true);
  }


  
  
  // If status received print that value
  if (statusAvailable == true && historyAvailable == false) {
    statusData = getStatusReceived();
    
    //Update the status in background
    batteries[statusData[0]].updateCharge(statusData[1], statusData[2], statusData[3], statusData[4], (cutoffVoltageInt + 250)*10); //data[0] is cell ID, followed by cellstatus, V, I and capacity
  
    statusAvailable = false;
    statusReceived = new byte[0];  // Clear status buffer for the next message
  }
  
  
  readCell = readCell +1 ;
  //readCell = 0;
  byte[] statusCommand = { (byte) readCell, (byte) 0x51 };
  myPort.write(statusCommand); //write the status command to hardware
  //println("sent status comamnd");
  
  // Print statusCommand as hex values
    StringBuilder hexString = new StringBuilder();
    for (byte b : statusCommand) {
      hexString.append(String.format("%02X ", b));  // Convert byte to 2-digit hex and append to string
    }
  //println("Sent Status Command (Hex): " + hexString.toString().trim());
  
  if (readCell >= 7)
  readCell = -1;
  
}

// Handle key presses to capture input
void keyPressed() {
  if (key == ENTER) {
    submitted = true; // Submit when Enter is pressed
  } else if (key == BACKSPACE) {
    if (focusField == 1) {
      cellCapacity = cellCapacity.length() > 0 ? cellCapacity.substring(0, cellCapacity.length() - 1) : "";
    } else if (focusField == 2) {
      dischargeCurrent = dischargeCurrent.length() > 0 ? dischargeCurrent.substring(0, dischargeCurrent.length() - 1) : "";
    } else if (focusField == 3) {
      cutoffVoltage = cutoffVoltage.length() > 0 ? cutoffVoltage.substring(0, cutoffVoltage.length() - 1) : "";
    }
  } else {
    if (focusField == 1) {
      cellCapacity += key;
    } else if (focusField == 2) {
      dischargeCurrent += key;
    } else if (focusField == 3) {
      cutoffVoltage += key;
    }
  }
}

// Track which input field is focused
int focusField = 0;

void mousePressed() {
  // Check if any battery cell is clicked
  for (int i = 0; i < batteries.length; i++) {
    if (batteries[i].isMouseOver()) {
      selectedCell = i;  // Set the selected cell
      plot = false;
      break;
    }
  }
  
  if (mouseX > 650 && mouseX < 770) {
    if (mouseY > 95 && mouseY < 115) {
      focusField = 1; // Cell Capacity
    } else if (mouseY > 125 && mouseY < 145) {
      focusField = 2; // Discharge Current
    } else if (mouseY > 155 && mouseY < 175) {
      focusField = 3; // Cut-off Voltage
    } else if (mouseY > 200 && mouseY < 240) {
      submitted = true; // Submit Button
    }
  }
  
  
  // Check if mouse is inside the "Start Test" button
  if (mouseX >= 520 && mouseX <= 520 + 120 && mouseY >= 450 && mouseY <= 450 + 40) {
    println("Start Test button clicked!");
    handleStartTestClick(); //get the values from canvas and preapre a 5byte dischargeCommand and write it
    println("#Command Sent#");
    
    //batteries[selectedCell].discharging = false;
  }
  
  // Check if mouse is inside the "Plot Graph" button
  if (mouseX >= 650 && mouseX <= 650 + 120 && mouseY >= 450 && mouseY <= 450 + 40) {
    println("Plot button clicked!");
    
    delay(2000);
    historyReceived.clear();
    myPort.clear(); // Clear the serial buffer if there is any data in it
    statusReceived = new byte[0];  // Clear status buffer for the next message
    statusAvailable = false;
    
    delay(500);
    handlePlotGraphClick(); //send histroy command to device
    
    int historySize = historyReceived.size();  // Get the size of the received history
    int dataSize = historySize - 2;  // Ignore the last two 0xFF, 0xFF bytes
    
      // Iterate over the valid history bytes and map them to the graph
  for (int i = 0; i < dataSize; i++) {
    // Each byte represents a charge level, scale as needed
    int rawValue = Byte.toUnsignedInt(historyReceived.get(i));  // Convert to unsigned int
    println(rawValue);
  }
    //println (cellCapacity, dischargeCurrent, cutoffVoltage );
      //byte[] cellCapacity_byte = stringToBytes(cellCapacity);
    //println("cellCapacity" + byteArrayToHex(cellCapacity_byte));
    //batteries[selectedCell].charging = false;
  }
}
