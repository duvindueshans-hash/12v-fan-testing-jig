//String qrCode = ""; // String variable to store the received QR code data

void initQr() {
  // Initialize UART2 with custom pins
  Serial2.begin(115200, SERIAL_8N1, QR_RX_PIN, QR_TX_PIN); // UART2, QR_RX_pin=3, qr_TX_PIN=1, baud rate 9600
  Serial.println("UART2 Initialized on Pins 3 and 1");
}


String readQr() {
  

  // Check if there is incoming data from Serial2 (QR code scanner)
  if (Serial2.available() > 0) {
    // Read the string data received from Serial2
    qrCode = Serial2.readString();
    // Remove any unnecessary spaces or newlines
    qrCode.trim();
    // Print the received QR code to the Serial Monitor
    Serial.println(qrCode);
  }
  
  // Always return a string, even if no QR code is found
  return qrCode;
}