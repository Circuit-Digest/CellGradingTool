// Flags to indicate which type of data is available
boolean statusAvailable = false;
boolean historyAvailable = false;

//Funtion to read serial data when it arrives
void serialEvent(Serial myPort) {
  while (myPort.available() > 0) {
    int incomingByte = myPort.read();  // Read one byte from the serial buffer
    //System.out.println(String.format("0x%02X", incomingByte));

    // Check for status message (7 bytes)
    if (!statusAvailable && statusReceived.length < 7) {
      // Accumulate bytes into statusReceived
      statusReceived = append(statusReceived, (byte)incomingByte);

      // If we have 7 bytes, we have received the status message
      if (statusReceived.length == 7) {
        statusAvailable = true;
        //historyReceived.clear(); //clear history array
        println("Status message received: " + byteArrayToHex(statusReceived));
      }
    }

    // Check for history message, end of message marked by 0xFF, 0xFF
    if (!historyAvailable) {
      historyReceived.add((byte)incomingByte);

      // Check if the last two bytes are 0xFF, 0xFF
      if (historyReceived.size() > 1 && 
          historyReceived.get(historyReceived.size() - 1) == (byte)0xFF && 
          historyReceived.get(historyReceived.size() - 2) == (byte)0xFF) {
           
        // History message received
        historyAvailable = true;
        println("History message received: " + byteArrayListToHex(historyReceived));
        // Clear history buffer for the next message
        //historyReceived.clear();
      }
    }
  }
}

// Function to process the received data
int[] getStatusReceived() {
  // Extract the values from the received data
  int cellID = statusReceived[0] & 0xFF;
  int cellStatus = statusReceived[1] & 0xFF;
  int voltage = ((statusReceived[2] & 0xFF)+250)*10;
  int current = ((statusReceived[3] & 0xFF) << 8) | (statusReceived[4] & 0xFF);
  int capacity = ((statusReceived[5] & 0xFF) << 8) | (statusReceived[6] & 0xFF);
  
  
  /*// Print the values in decimal and hex
  println("Cell ID: " + cellID + " (Hex: " + hex(cellID) + ")");
  println("Cell Status: " + cellStatus + " (Hex: " + hex(cellStatus) + ")");
  println("Voltage: " + voltage + " V (Hex: " + hex(voltage) + ")");
  println("Current: " + current + " mA (Hex: " + hex(current) + ")");
  println("Capacity: " + capacity + " mAh (Hex: " + hex(capacity) + ")");*/
  
  // Return an array of the values
  return new int[]{cellID, cellStatus, voltage, current, capacity};
  
  // Send the command again if needed
  //myPort.write(requestCommand);
}
