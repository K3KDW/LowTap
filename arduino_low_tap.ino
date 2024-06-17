// Arduino code to encode and decode LowTap signals
// Roe Gammon K3KDW
// 6/17/24

// Search for "EDIT" to adjust speed and tone (if using a speaker) parameters

// Pins for the onboard LED, optional piezo buzzer, optional speaker/headphones, optional CW transmitter trigger, and microphone
const int ledPin = 13;
const int piezoPin = 12;
const int speakerPin = 11;
const int cwPin = 10;  // CW transmitter pin
const int micPin = A0;  // Microphone input pin

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
// EDIT tapDuration below to adjust speed.
const int tapDuration = 200;  // Duration of a tap in milliseconds.
// Multipliers below conform to LowTap best practice
const int pauseBetweenTaps = tapDuration;  // Pause between taps in the same coordinate
const int pauseBetweenCoordinates = tapDuration * 3;  // Pause between row and column taps
const int letterSpacing = tapDuration * 5;  // Space between letters
const int wordSpacing = tapDuration * 7;  // Space between words

// PWM frequency and duration for the speaker and piezo
// EDIT the number below to adjust the tone if using a speaker
const int pwmFrequency = 700;  // Frequency of the PWM signal
const int pwmPeriod = 1000000 / pwmFrequency;  // Period in microseconds

// Timing constants for decoding
const int micThreshold = 500;  // Threshold for detecting a tap (adjust as necessary)
const int tapGap = 500;        // Minimum gap between taps to count as separate taps (in milliseconds)
const int silenceDuration = 2000; // Duration to consider end of character (in milliseconds)

// LowTap grid definition for decoding
const char lowTapGrid[6][6] = {
  {'A', 'B', 'C', 'D', 'E', 'F'},
  {'G', 'H', 'I', 'J', 'K', 'L'},
  {'M', 'N', 'O', 'P', 'Q', 'R'},
  {'S', 'T', 'U', 'V', 'W', 'X'},
  {'Y', 'Z', '0', '1', '2', '3'},
  {'4', '5', '6', '7', '8', '9'}
};

void setup() {
  pinMode(ledPin, OUTPUT);
  pinMode(piezoPin, OUTPUT);
  pinMode(speakerPin, OUTPUT);
  pinMode(cwPin, OUTPUT);
  pinMode(micPin, INPUT);
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

  // Continuously decode audio signals
  decodeLowTapAudio();
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
  // Blink the LED
  digitalWrite(ledPin, HIGH);
  // Key the CW transmitter
  digitalWrite(cwPin, HIGH);

  // Generate PWM signal for the piezo buzzer
  unsigned long startTime = millis();
  while (millis() - startTime < tapDuration) {
    digitalWrite(piezoPin, HIGH);
    delayMicroseconds(pwmPeriod / 2);
    digitalWrite(piezoPin, LOW);
    delayMicroseconds(pwmPeriod / 2);
  }
  
  // Generate a tone for the speaker using the tone function
  tone(speakerPin, pwmFrequency, tapDuration);  // Beep for the duration of a tap

  // Stop LED and speaker tone
  digitalWrite(ledPin, LOW);
  digitalWrite(cwPin, LOW);  // Stop keying the CW transmitter
  noTone(speakerPin);  // Stop the tone
  
  delay(pauseBetweenTaps);  // Space after tap
}

// Function to decode LowTap audio signals
void decodeLowTapAudio() {
  static int tapCount = 0;
  static unsigned long lastTapTime = 0;
  static unsigned long lastSilenceTime = 0;

  int micValue = analogRead(micPin);
  
  if (micValue > micThreshold) {
    unsigned long currentTime = millis();
    if (currentTime - lastTapTime > tapGap) {
      tapCount++;
      lastTapTime = currentTime;
    }
  } else {
    unsigned long currentTime = millis();
    if (currentTime - lastTapTime > silenceDuration && tapCount > 0) {
      decodeTap(tapCount);
      tapCount = 0;
    }
  }
}

// Function to decode a tap count into LowTap coordinates
void decodeTap(int tapCount) {
  static int rowTaps = 0;
  static int columnTaps = 0;
  static bool waitingForColumn = false;

  if (!waitingForColumn) {
    rowTaps = tapCount;
    waitingForColumn = true;
  } else {
    columnTaps = tapCount;
    waitingForColumn = false;
    if (rowTaps > 0 && rowTaps <= 6 && columnTaps > 0 && columnTaps <= 6) {
      char decodedChar = lowTapGrid[rowTaps - 1][columnTaps - 1];
      Serial.print(decodedChar);
    }
  }
}
