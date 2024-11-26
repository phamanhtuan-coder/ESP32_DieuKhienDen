#include <WiFi.h>
#include <FirebaseESP32.h>

// Firebase Config
#define FIREBASE_HOST "ltnhungnhomanhtuan-default-rtdb.asia-southeast1.firebasedatabase.app"
#define FIREBASE_API_KEY "AIzaSyB9BX1OwHhOcKPRHJfPX_dxeVNKA9PV2cI"

// WiFi Credentials
#define WIFI_SSID "Anh Tuan"
#define WIFI_PASSWORD "21032001"

// Firebase objects
FirebaseData firebaseData;
FirebaseAuth auth;
FirebaseConfig config;

// LED and Button Pins
const int LedPin = 2;
const int ButtonPin = 4;

bool ledState = LOW;
bool lastButtonState = LOW;
bool currentButtonState = LOW;

// Callback function for token status (using the library's typedef)
void tokenStatusCallback(TokenInfo tokenInfo) {
  switch (tokenInfo.status) {
    case token_status_error:
      Serial.printf("Token Status Error: %s\n", tokenInfo.error.message.c_str());
      break;
    case token_status_ready:
      Serial.println("Token Status: Ready");
      break;
    default:
      Serial.println("Token Status: Unknown");
      break;
  }
}

void setup() {
  Serial.begin(115200);

  // GPIO Setup
  pinMode(LedPin, OUTPUT);
  pinMode(ButtonPin, INPUT_PULLUP);

  // WiFi Connection
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(300);
  }
  Serial.println("\nConnected to Wi-Fi");

  // Firebase Configuration
  config.api_key = FIREBASE_API_KEY;
  config.database_url = FIREBASE_HOST;

  // Anonymous Authentication
  config.signer.test_mode = true;

  // Set token status callback (note the lowercase and function reference)
  config.token_status_callback = tokenStatusCallback;
  
  Firebase.reconnectWiFi(true);

  // Initialize Firebase
  Firebase.begin(&config, &auth);

  // Optional: Wait for Firebase to be ready
  Serial.println("Connecting to Firebase...");
  unsigned long startAttemptTime = millis();
  while (!Firebase.ready()) {
    Serial.print(".");
    if (millis() - startAttemptTime > 10000) {
      Serial.println("\nFailed to connect to Firebase. Restart required.");
      break;
    }
    delay(300);
  }
  Serial.println("\nFirebase Connected!");
}

void loop() {
  // Ensure Firebase connection
  if (!Firebase.ready()) {
    Serial.println("Firebase connection lost. Reconnecting...");
    return;
  }

  // Read LED status from Firebase
  if (Firebase.getString(firebaseData, "/light_status")) {
    String ledStatus = firebaseData.stringData();
    Serial.println("Firebase value: " + ledStatus);
    
    if (ledStatus == "ON") {
      digitalWrite(LedPin, HIGH);
      ledState = HIGH;
    } else {
      digitalWrite(LedPin, LOW);
      ledState = LOW;
    }
  } else {
    Serial.println("Failed to get value: " + firebaseData.errorReason());
  }

  // Button toggle logic
  lastButtonState = currentButtonState;
  currentButtonState = digitalRead(ButtonPin);
  
  if (lastButtonState == HIGH && currentButtonState == LOW) {
    ledState = !ledState;
    digitalWrite(LedPin, ledState);
    
    // Update Firebase with new state
    if (ledState) {
      Firebase.setString(firebaseData, "/light_status", "ON");
    } else {
      Firebase.setString(firebaseData, "/light_status", "OFF");
    }
  }

  delay(50); // Small delay to debounce
}