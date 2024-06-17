//Arduino code to blink input from the keyboard using the Arduino IDE serial monitor.
//Roe Gammon K3KDW
//6/17/24

//TO DO
//Add buzzer to simulate use with a simple cw radio

// Pin for onboard LED
const int ledPin = 13;

// LowTap definitions using dashes for row and column taps
const char* lowTapCode[] = {
  "- -",        // A (1,1)
  "- --",       // B (1,2)
  "- ---",      // C (1,3)
  "- ----",     // D (1,4)
  "- -----",    // E (1,5)
  "- ------",   // F (1,6)
  "-- -",       // G (2,1)
  "-- --",      // H (2,2)
  "-- ---",     // I (2,3)
  "-- ----",    // J (2,4)
  "-- -----",   // K (2,5)
  "-- ------",  // L (2,6)
  "--- -",      // M (3,1)
  "--- --",     // N (3,2)
  "--- ---",    // O (3,3)
  "--- ----",   // P (3,4)
  "--- -----",  // Q (3,5)
  "--- ------", // R (3,6)
  "---- -",     // S (4,1)
  "---- --",    // T (4,2)
  "---- ---",   // U (4,3)
  "---- ----",  // V (4,4)
  "---- -----", // W (4,5)
  "---- ------",// X (4,6)
  "----- -",    // Y (5,1)
  "----- --",   // Z (5,2)
  "----- ---",  // 0 (5,3)
  "----- ----", // 1 (5,4)
  "----- -----",// 2 (5,5)
  "----- ------",// 3 (5,6)
  "------ -",   // 4 (6,1)
  "------ --",  // 5 (6,2)
  "------ ---", // 6 (6,3)
  "------ ----",// 7 (6,4)
  "------ -----",// 8 (6,5)
  "------ ------"// 9 (6,6)
};

// Timing constants
const int tapDuration = 200;  // Duration of a tap in milliseconds.
//EDIT tapDuration to adjust speed. Multipliers below conform to LowTap best practice.
const int pauseBetweenTaps = tapDuration;  // Pause between taps in the same coordinate
const int pauseBetweenCoordinates = tapDuration * 3;  // Pause between row and column taps
const int letterSpacing = tapDuration * 5;  // Space between letters
const int wordSpacing = tapDuration * 10;  // Space between words

void setup() {
  pinMode(ledPin, OUTPUT);
  Serial.begin(9600);
  Serial.println("Enter a message to send using LowTap:");
}

void loop() {
  if (Serial.available() > 0) {
    // Read the input message
    String message = Serial.readStringUntil('\n');
    message.toUpperCase(); // Convert message to uppercase
    
    // Send the message using LowTap
    sendLowTap(message.c_str());
  }
}

// Function to send a message using LowTap
void sendLowTap(const char* message) {
  for (int i = 0; message[i] != '\0'; i++) {
    char c = message[i];
    if (c >= 'A' && c <= 'Z') {
      // Send the LowTap code for the current letter
      sendLowTapCode(lowTapCode[c - 'A']);
    } else if (c >= '0' && c <= '9') {
      // Send the LowTap code for the current number
      sendLowTapCode(lowTapCode[c - '0' + 26]);
    } else if (c == ' ') {
      // Space between words
      delay(wordSpacing);
    }
    // Space between letters
    delay(letterSpacing);
  }
}

// Function to send a LowTap code
void sendLowTapCode(const char* code) {
  for (int i = 0; code[i] != '\0'; i++) {
    if (code[i] == '-') {
      sendTap();
    }
    // Space between taps in the same coordinate
    delay(pauseBetweenTaps);
  }
}

// Function to send a single tap
void sendTap() {
  digitalWrite(ledPin, HIGH);
  delay(tapDuration);
  digitalWrite(ledPin, LOW);
  delay(pauseBetweenTaps);  // Space after tap
}
