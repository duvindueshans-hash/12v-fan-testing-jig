void initNextion() {
  nextion.begin(115200, SERIAL_8N1, NEXTION_RX, NEXTION_TX);  // Nextion baud rate
}

// Nextion TXT send function
void sendStringToNextion(String text) {
  nextion.flush();
  String command = "t0.txt=\"" + text + "\"";
  nextion.print(command);  // Send command to Nextion
  nextion.write(0xFF);     // End of command
  nextion.write(0xFF);
  nextion.write(0xFF);
  delay(100);  // Add a small delay to allow Nextion to process the command

}

void sendStringToNextion(String text, int textBoxNumber) {
  nextion.flush();  // Clear any remaining data in the serial buffer

  // Construct the correct command based on the text box number
  String command = "";
  
  // Select the correct text box (t0, t1, or t2) based on the input parameter
  switch(textBoxNumber) {
    case 0:
      command = "t0.txt=\"" + text + "\"";  // Update text box 0 (t0)
      break;
    case 1:
      command = "t1.txt=\"" + text + "\"";  // Update text box 1 (t1)
      break;
    case 2:
      command = "t2.txt=\"" + text + "\"";  // Update text box 2 (t2)
      break;
          case 3:
      command = "t3.txt=\"" + text + "\"";  // Update text box 2 (t2)
      break;
          case 4:
      command = "t5.txt=\"" + text + "\"";  // Update text box 2 (t2)
      break;
    default:
      Serial.println("Invalid text box number! Use 1, 2, or 3.");
      return;  // Exit the function if the box number is invalid
  }

  nextion.print(command);  // Send the constructed command to Nextion
  nextion.write(0xFF);     // End of command
  nextion.write(0xFF);
  nextion.write(0xFF);
  delay(100);  // Add a small delay to allow Nextion to process the command

}

void GoToPage (int pageNumber) {
  nextion.flush();
  String command = "page " + String(pageNumber); // Create the command
  nextion.print(command);                         // Send command to Nextion
  nextion.write(0xFF);                            // End of command
  nextion.write(0xFF);
  nextion.write(0xFF);
  delay(150);  // Small delay after the page change command
}







