#include <WiFi.h>
#include <FirebaseESP32.h> // Common and stable version for Wokwi
#include <LiquidCrystal_I2C.h>

// 1. Credentials (Update your actual data here)
#define FIREBASE_HOST "vayumitra-project-default-rtdb.firebaseio.com" 
#define FIREBASE_AUTH "YOUR_KEY_HERE"
#define WIFI_SSID "Wokwi-GUEST"
#define WIFI_PASSWORD ""

// 2. Objects
FirebaseData firebaseData;
FirebaseConfig config;
FirebaseAuth auth;
LiquidCrystal_I2C lcd(0x27, 16, 2);

int sensorPin = 34;
int buzzerPin = 13;
int lastAqi = 0;

void setup() {
  Serial.begin(115200);
  pinMode(buzzerPin, OUTPUT);
  
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("VayuMitra Booting");

  // WiFi Connect
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  // Firebase Init
  config.host = FIREBASE_HOST;
  config.signer.tokens.legacy_token = FIREBASE_AUTH;
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  lcd.clear();
  lcd.print("System Online!");
  delay(2000);
}

void loop() {
  int sensorValue = analogRead(sensorPin);
  int aqi = map(sensorValue, 0, 4095, 0, 100);

  // AI Trend Logic
  int trend = aqi - lastAqi;
  String prediction = "Stable";
  
  if (trend > 5 && aqi < 50) {
    prediction = "Risk Rising";
    lcd.setCursor(0, 1);
    lcd.print("AI: Risk Rising!");
  }

  // Firebase Update
  Firebase.setInt(firebaseData, "/SPB_Colony/AQI", aqi);
  Firebase.setString(firebaseData, "/SPB_Colony/AI_Trend", prediction);

  // LCD and Alert Logic
  lcd.setCursor(0, 0);
  lcd.print("AQI Level: ");
  lcd.print(aqi);
  lcd.print("%  ");

  if (aqi > 50) {
    lcd.setCursor(0, 1);
    lcd.print("Trigger High Alert");
    Firebase.setString(firebaseData, "/SPB_Colony/Status", "DANGER");
    
    // Buzzer Beep
    tone(buzzerPin, 1000); 
    delay(200);
    noTone(buzzerPin);
  } else {
    if (trend <= 5) {
      lcd.setCursor(0, 1);
      lcd.print("Everything Alright");
    }
    Firebase.setString(firebaseData, "/SPB_Colony/Status", "SAFE");
    noTone(buzzerPin);
  }

  lastAqi = aqi; 
  delay(2000); 
}    
