

void initButton() {
  pinMode(BUTTON_1_GREEN_PASS_SAVE, INPUT);
  pinMode(BUTTON_2_RED_FAIL_RETAKE, INPUT);
  pinMode(BUTTON_3_YELLOW_START_NEXT, INPUT);
}

// Function to read the state of the Green button
bool readGreenButton() {
  return digitalRead(BUTTON_1_GREEN_PASS_SAVE) == LOW;  // Button pressed (LOW when pressed)
}

// Function to read the state of the Red button
bool readRedButton() {
  return digitalRead(BUTTON_2_RED_FAIL_RETAKE) == LOW;  // Button pressed (LOW when pressed)
}

// Function to read the state of the Yellow button
bool readYellowButton() {
  return digitalRead(BUTTON_3_YELLOW_START_NEXT) == LOW;  // Button pressed (LOW when pressed)
}

