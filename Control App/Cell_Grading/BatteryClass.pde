// BatteryCell Class
class BatteryCell {
  int cellstatus;
  int voltage;
  int current;
  int capacity;
  int cutoff;
  float x, y, width, height;
  float chargeLevel ;  // Between 0 and 1 representing battery percentage
  boolean charging = false;
  boolean discharging = false;
  
  BatteryCell(float x, float y) {
    this.x = x;
    this.y = y;
    this.width = 72;
    this.height = 135;
    this.chargeLevel = 0.0;  // Start at 0% charge
    this.cellstatus = 0;
    this.voltage = 0;
    this.current = 0;
    this.capacity = 0;
    this.cutoff = 0;
  }
  
  // Display the battery
  void display() {
    stroke(0);
    strokeWeight(1);
    fill(255);
    rect(x, y, width, height, 10);  // Battery body

    // Draw positive terminal
    fill(0);
    rect(x + width * 0.25, y - 10, width * 0.5, 10);
    
    // Display voltage below the battery
    fill(0);
    textSize(12);
    textAlign(CENTER);
    fill(0, 123, 255); text(nf(voltage, 1, 0) + " mV", (x + width / 2)-22, y + height + 12);
    fill(40, 167, 69); text(nf(current, 1, 0) + " mA", (x + width / 2)+22, y + height + 12);
    fill(255, 165, 0); text(nf(capacity, 1, 0) + " mAh", (x + width / 2), y + height + 25);
    
    // Draw the current charge level
    float fillHeight = chargeLevel * height;
    int redValue = int(lerp(255, 0, chargeLevel));  // From 255 (red) to 0 (no red)
    int greenValue = int(lerp(0, 200, chargeLevel));  // From 0 (no green) to 200 (full green)
    stroke(redValue, greenValue, 0, 150);
    fill(redValue, greenValue, 0, 150);
    rect(x, y + height - fillHeight, width, fillHeight, 10);
  }
  
  // Update the charging level and record history
  void updateCharge(int cellstatus, int voltage, int current, int capacity, int cutoff) {
    
    this.cellstatus = cellstatus;
    this.voltage = voltage;
    this.current = current;
    this.capacity = capacity;
    this.cutoff = cutoff;
    
    if (cellstatus == 9) //if cell is idle
    this.voltage = 0;
    
    chargeLevel = map(voltage, cutoff, 4500, 0, 1); 
    if (chargeLevel>1 || chargeLevel<0)
    chargeLevel = 0;
    
    if (cellstatus == 9) //if cell is idle
    {
    this.voltage = 0;
    }
    
    //chargeHistory.add(voltagechargeLevel);
  }
  
  // Check if mouse is over the battery cell
  boolean isMouseOver() {
    return (mouseX > x && mouseX < x + width && mouseY > y && mouseY < y + height);
  }
  
    //Selection Indicator 
  void selectionIndicator() {
    // change positive terminal solor
    fill(0, 100, 255);
    rect(x + width * 0.25, y - 10, width * 0.5, 10);
    noFill();
    stroke(0, 100, 255);
    rect(x, y, width, height, 10);  // Battery body
    
    // Draw a line at the base of the rectangle
    strokeWeight(2); // Thickness of the line
    //stroke(0, 0, 255); // Color of the line (blue)
    line(x-2, y + height, x+2 + width, y + height); // Draw the line across the base
  }
  
  
// Display the charge and discharge graph in the bottom-right corner
void displayGraph() {
  // Define the new position for the graph in the bottom-right corner
  int graphWidth = 250;  // Width of the graph
  int graphHeight = 150; // Height of the graph
  float graphX = 800 - graphWidth - 30;  // X-position, 10 pixels padding from right
  float graphY = 600 - (graphHeight + 220); // Y-position, adjusted for space for buttons
  
  // Draw the graph background
  stroke(0);
  fill(255);
  rect(graphX, graphY, graphWidth, graphHeight);  // Graph background
  
  // Label the graph
  fill(0);
  textAlign(CENTER);
  text("Charge/Discharge Graph", graphX + graphWidth / 2, graphY - 10);  // Title above the graph
  text("Time (min)", graphX + graphWidth / 2, graphY + graphHeight + 30);  // X-axis label below the graph
  textAlign(RIGHT);
  text("Volt(V)", graphX - 30, graphY + graphHeight / 2);  // Y-axis label to the left of the graph
  
  // Draw the X and Y axis ticks and labels
  /*for (int i = 0; i <= 6; i++) {  // X-axis (Time in 20-minute increments)
    float xPos = map(i * 20, 0, 200, graphX, graphX + graphWidth);
    line(xPos, graphY + graphHeight, xPos, graphY + graphHeight + 5);  // Tick mark
    textAlign(CENTER);
    text(i * 20, xPos, graphY + graphHeight + 15);  // Label for time in minutes
  }*/
  
  for (int i = 0; i <= 6; i++) {  // Y-axis (Voltage from 3V to 4.2V)
    float yPos = map(i * 0.2 + 3.0, 3, 4.2, graphY + graphHeight, graphY);
    line(graphX - 5, yPos, graphX, yPos);  // Tick mark
    textAlign(RIGHT);
    text(nf(3.0 + i * 0.2, 1, 1), graphX - 10, yPos + 5);  // Label for voltage in Volts
  }
  
}

void updateGraph(){
    // Define the new position for the graph in the bottom-right corner
    int graphWidth = 250;  // Width of the graph
    int graphHeight = 150; // Height of the graph
    float graphX = 800 - graphWidth - 30;  // X-position, 10 pixels padding from right
    float graphY = 600 - (graphHeight + 220); // Y-position, adjusted for space for buttons
  
    // Draw the graph (charge level over time)
    stroke(0, 0, 255);
    noFill();
    beginShape();
    float closestDistance = Float.MAX_VALUE;
    float closestX = 0;
    float closestY = 0;
    String hoverInfo = "";
  
    for (int i = 0; i < voltageHistory.length; i++) {
        int value = (voltageHistory[i]);  // Voltage value
        
        // Map the X and Y positions to fit inside the graph area
        float timeValue = map(i, 0, voltageHistory.length - 1, 0, graphWidth);  // Time axis (0 to graphWidth)
        
        // Map value to Y-axis (ensuring it stays within graphHeight)
        float yPos = map(value, 3000, 4200, graphY + graphHeight, graphY);  // Y-axis (charge level, flipped)

        // Ensure xPos is correctly mapped
        float xPos = graphX + timeValue;  // X-position

        // Draw the vertex for the graph line
        if (timeValue>0)
        vertex(xPos, yPos);
        
        // Check if the mouse is close to this point
        float distance = dist(mouseX, mouseY, xPos, yPos);
        if (distance < closestDistance) {
            closestDistance = distance;
            closestX = xPos;
            closestY = yPos;
            hoverInfo = "Time: " + i + " min\nVoltage: " + value + " mV";
        }
    }
    endShape();
  
    // If the mouse is hovering near the line, display the voltage and time
    if (closestDistance < 10) {
        fill(0);
        textAlign(LEFT);
        text(hoverInfo, closestX - 100, closestY - 10);  // Show info near the hovered point
    }
    

}

// Function to draw buttons
void drawButton(String label, float x, float y, color btnColor) {
  fill(btnColor);
  stroke(0);
  rect(x, y, 120, 30);  // Button dimensions (120x30)

  // Button label
  fill(0);
  textAlign(CENTER, CENTER);
  text(label, x + 60, y + 15);  // Center text within the button
}


void drawTable(){
 
  textAlign(LEFT);
  textSize(14);
  
  // Labels
  fill(0, 100, 255);text("SELECTED CELL:", 590, 80); text(selectedCell, 700, 80);
  fill(0);
  text("Cell Capacity (mAh):", 520, 110);
  text("Dis. Current (mA):", 520, 140);
  text("Cut-off Voltage (mV):",520, 170);
  
  // Input fields
  fill(255);
  rect(650, 95, 120, 20);
  rect(650, 125, 120, 20);
  rect(650, 155, 120, 20);
  
  // Display user inputs
  fill(0);
  text(cellCapacity, 655, 110);
  text(dischargeCurrent, 655, 140);
  text(cutoffVoltage, 655, 170);
  
  // Draw the buttons
  drawButton("Start Test", 520, 450, color(0, 100, 255));  // Green button for charging
  drawButton("Plot Graph", 650, 450, color(155));  // Red button for discharging
  
}


  
}
