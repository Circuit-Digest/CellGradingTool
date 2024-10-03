
int cutoffVoltageInt = 0;

//Funtion to handle plot graph click 
void handlePlotGraphClick() {
  byte cellIDByte = (byte) selectedCell;  // Convert to byte
  byte commandByte = (byte) 0x11; // Second byte is always 0xD5
  
  // Create the dischargeCommand byte array with hex values
    byte[] historyCommand = new byte[2];
    historyCommand[0] = cellIDByte;  // First byte: selectedCell (cell ID)
    historyCommand[1] = commandByte;  // Second byte: 0x11
 
    myPort.write(historyCommand);
    
  // Print dischargeCommand as hex values
    StringBuilder hexString = new StringBuilder();
    for (byte b : historyCommand) {
      hexString.append(String.format("%02X ", b));  // Convert byte to 2-digit hex and append to string
    }

    println("History Command (Hex): " + hexString.toString().trim());
}

// Function to handle "Start Test" button click
void handleStartTestClick() {

    // Convert selectedCell to an 8-bit value
    //int cellID = Integer.parseInt(selectedCell, 16);  // Assuming selectedCell is a hex string
    byte cellIDByte = (byte) selectedCell;  // Convert to byte

    // Second byte is always 0xD5
    byte commandByte = (byte) 0xD5;

    // Convert dischargeCurrent to a 16-bit value in hex
    int dischargeCurrentInt = Integer.parseInt(dischargeCurrent);  // Convert hex string to int
    byte dischargeCurrentMSB = (byte) (dischargeCurrentInt >> 8);  // Most significant byte
    byte dischargeCurrentLSB = (byte) (dischargeCurrentInt & 0xFF);  // Least significant byte

    // Convert cutoffVoltage to an 8-bit value in hex
    cutoffVoltageInt = Integer.parseInt(cutoffVoltage);  // Convert hex string to int
    cutoffVoltageInt = (cutoffVoltageInt / 10) - 250;
    byte cutoffVoltageByte = (byte) cutoffVoltageInt;  // Convert to byte

    // Create the dischargeCommand byte array with hex values
    byte[] dischargeCommand = new byte[5];
    dischargeCommand[0] = cellIDByte;  // First byte: selectedCell (cell ID)
    dischargeCommand[1] = commandByte;  // Second byte: 0xD5
    dischargeCommand[2] = dischargeCurrentMSB;  // Third byte: Most significant byte of dischargeCurrent
    dischargeCommand[3] = dischargeCurrentLSB;  // Fourth byte: Least significant byte of dischargeCurrent
    dischargeCommand[4] = cutoffVoltageByte;  // Fifth byte: cutoffVoltage
    
    myPort.write(dischargeCommand);

    // Print dischargeCommand as hex values
    StringBuilder hexString = new StringBuilder();
    for (byte b : dischargeCommand) {
      hexString.append(String.format("%02X ", b));  // Convert byte to 2-digit hex and append to string
    }

    println("Discharge Command (Hex): " + hexString.toString().trim());
  }





// Helper function to convert byte array to a hex string for printing
String byteArrayToHex(byte[] byteArray) {
  StringBuilder hexString = new StringBuilder();
  for (int i = 0; i < byteArray.length; i++) {
    hexString.append(String.format("%02X ", byteArray[i]));
  }
  return hexString.toString().trim();
}


// Helper function to convert ArrayList<Byte> to hexadecimal string
String byteArrayListToHex(ArrayList<Byte> byteList) {
  StringBuilder hex = new StringBuilder();
  for (byte b : byteList) {
    hex.append(String.format("%02X ", b));
  }
  return hex.toString();
}


// Function to convert String (representing an int value) to byte array (1 or 2 bytes depending on the value)
byte[] stringToBytes(String valueStr) {
  int value;
  try {
    // Convert the string to an integer
    value = Integer.parseInt(valueStr);
  } catch (NumberFormatException e) {
    // Handle invalid input by returning a single byte with value 0
    println("Invalid input, returning 0");
    return new byte[]{0};
  }
  
  byte[] result;
  
  if (value <= 255) {
    // If the value fits into one byte (0-255)
    result = new byte[1];
    result[0] = (byte) (value & 0xFF); // Extract the least significant byte
  } else {
    // If the value requires two bytes
    result = new byte[2];
    result[0] = (byte) ((value >> 8) & 0xFF); // Extract the most significant byte
    result[1] = (byte) (value & 0xFF);        // Extract the least significant byte
  }
  
  return result;
}
