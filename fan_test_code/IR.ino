
//volatile int pulseCount = 0; // Count the number of pulses
//unsigned long lastTime = 0;  // Time tracking

//void rpmCounter() {
    //pulseCount++;  // Increment the pulse count whenever the IR sensor detects a pulse
//}



// void loop() {
//     if (millis() - lastTime >= 500) {  // Update every 1 second
//         detachInterrupt(digitalPinToInterrupt(IR_SENSOR_PIN));  // Disable interrupt temporarily
//         int rpm = (pulseCount * 60)/7;  // Convert pulses per second to RPM
//         Serial.print("Fan RPM: ");
//         Serial.println(rpm);
//         pulseCount = 0;  // Reset count for next measurement
//         lastTime = millis();
//         attachInterrupt(digitalPinToInterrupt(IR_SENSOR_PIN), rpmCounter, RISING);  // Re-enable interrupt
//     }
// }