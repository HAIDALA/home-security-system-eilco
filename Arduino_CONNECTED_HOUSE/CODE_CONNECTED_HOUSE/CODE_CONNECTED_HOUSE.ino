#include <SPI.h>
#include <MFRC522.h>
#include "DHT.h"
 ////////////////////////////////////////////////////////////////
 #include <Arduino.h>
  #if defined(ESP32)
    #include <WiFi.h>
  #elif defined(ESP8266)
    #include <ESP8266WiFi.h>
  #endif
  #include <Firebase_ESP_Client.h>
  
  //Provide the token generation process info.
  #include "addons/TokenHelper.h"
  //Provide the RTDB payload printing info and other helper functions.
  #include "addons/RTDBHelper.h"
  
  // Insert your network credentials
  #define WIFI_SSID "Rca1949"
  #define WIFI_PASSWORD "Raja@Casa@"

  
  // Insert Firebase project API Key
  #define API_KEY "AIzaSyCB8iYcUrrl9VvAL7E8hsWZfOTL553A2tY"
  
  // Insert RTDB URLefine the RTDB URL */
  #define DATABASE_URL "https://home-security-system-eilco-default-rtdb.europe-west1.firebasedatabase.app" 
  
  //Define Firebase Data object
  FirebaseData fbdo;
  
  FirebaseAuth auth;
  FirebaseConfig config;
  unsigned long sendDataPrevMillis = 0;
  int count = 0;
  bool signupOK = false;
 ///////////////////////////////////////////////////////////////
// Definit la broche de l'Arduino sur laquelle la 
// broche DATA du capteur est reliee 
#define DHTPIN 13
 
// Definit le type de capteur utilise
#define DHTTYPE DHT11
 
// Declare un objet de type DHT
// Il faut passer en parametre du constructeur 
// de l'objet la broche et le type de capteur
DHT dht(DHTPIN, DHTTYPE);


// Constants
#define SS_PIN 5
#define RST_PIN 0
#define LED_GREEN 26
#define LED_RED 25
#define LED_Temp 12
const int buttonPin = 27; // broche du capteur PIR
const int ledPin = 14; // la LED du Arduino
int buttonState = 0;

// Variables
byte nuidPICC[4] = {0, 0, 0, 0};
MFRC522::MIFARE_Key key;
MFRC522 rfid = MFRC522(SS_PIN, RST_PIN);

// Access status
bool accessGranted = false;

void setup() {
  // Init Serial USB
  Serial.begin(115200);

  //////////////////////////////////////////////////////////////////
   WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.print("Connecting to Wi-Fi");
    while (WiFi.status() != WL_CONNECTED){
      Serial.print(".");
      delay(300);
    } 
    Serial.println();
    Serial.print("Connected with IP: ");
    Serial.println(WiFi.localIP());
    Serial.println();
 /////////////////////////////////////////////////// 
    /* Assign the api key (required) */
    config.api_key = API_KEY;
  
    /* Assign the RTDB URL (required) */
    config.database_url = DATABASE_URL;
  
    /* Sign up */
    if (Firebase.signUp(&config, &auth, "", "")){
      Serial.println("ok");
      signupOK = true;
    }
    else{
      Serial.printf("%s\n", config.signer.signupError.message.c_str());
    }
  
    /* Assign the callback function for the long running token generation task */
    config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h
    
    Firebase.begin(&config, &auth);
    Firebase.reconnectWiFi(true);
  ////////////////////////////////////////////////////////////////////
  Serial.println(F("Initialize System"));
 pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_RED, OUTPUT);
    pinMode(LED_Temp, OUTPUT);
  pinMode(ledPin, OUTPUT); //la broche de la LED est mise en sortie
  pinMode(buttonPin, INPUT); //la broche du capteur est mise en entree

  // Init RFID
  SPI.begin();
  rfid.PCD_Init();
  Serial.print(F("Reader :"));
  rfid.PCD_DumpVersionToSerial();

   
   
  // Initialise la capteur DHT11
  dht.begin();
}

void loop() {
  readRFID();
  checkPresence();
  checktemp();
}

void readRFID() {
  // Read RFID card
  for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF;
  }

  // Look for new cards
  if ( ! rfid.PICC_IsNewCardPresent()) {
    return;
  }

  // Verify if the NUID has been read
  if ( ! rfid.PICC_ReadCardSerial()) {
    return;
  }

  // Store NUID into nuidPICC array
  for (byte i = 0; i < 4; i++) {
    nuidPICC[i] = rfid.uid.uidByte[i];
  }

  // Check if card is authorized
  checkAuthorization();

  // Print access status

  
  // Halt PICC
  rfid.PICC_HaltA();

  // Stop encryption on PCD
  rfid.PCD_StopCrypto1();
}

// Function to check if card is authorized
/*void checkAuthorization() {
  // Here you would add your own logic to check if the card is authorized
  // For example, you could compare the nuidPICC variable to a list of authorized NUIDs
  // In this example, we'll just grant access if the first byte of the NUID is 0x01
  if (nuidPICC[0] == 203 && nuidPICC[1] == 243 && nuidPICC[2] == 145 && nuidPICC[3] == 105) {
    accessGranted = true;
    Serial.println("Access granted");
    // Turn on green LED
    digitalWrite(LED_GREEN, HIGH);
    // Turn off red LED
    digitalWrite(LED_RED, LOW);
     delay(3000);
    digitalWrite(LED_GREEN, LOW);
  } else {
    accessGranted = false;
    Serial.println("Access denied");
    // Turn off green LED
    digitalWrite(LED_GREEN, LOW);
    // Turn on red LED
    digitalWrite(LED_RED, HIGH);
    delay(3000);
    digitalWrite(LED_RED, LOW);
  }
  }
*/
void checkAuthorization() {
// Here you would add your own logic to check if the card is authorized
// For example, you could compare the nuidPICC variable to a list of authorized NUIDs
// In this example, we'll just grant access if the first byte of the NUID is 0x01
if (nuidPICC[0] == 203 && nuidPICC[1] == 243 && nuidPICC[2] == 145 && nuidPICC[3] == 105) {
accessGranted = true;
Serial.println("Access granted");
// Turn on green LED
digitalWrite(LED_GREEN, HIGH);
// Turn off red LED
digitalWrite(LED_RED, LOW);
// Create a string to store the card ID
    String cardID = "";
    // Concatenate the bytes of the NUID into the string
    for (int i = 0; i < 4; i++) {
      cardID += nuidPICC[i];
      if (i < 3) {
        cardID += ",";
      }
    }

if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 1000 || sendDataPrevMillis == 0)){
sendDataPrevMillis = millis();
// Write the card scan information and authorization result on the database path
if (Firebase.RTDB.setInt(&fbdo, "ESP32_DATA/authorizationResult", accessGranted)
          && Firebase.RTDB.setString(&fbdo, "ESP32_DATA/cardID", cardID.c_str())){
Serial.println("PASSED");
Serial.println("PATH: " + fbdo.dataPath());
Serial.println("TYPE: " + fbdo.dataType());
}
else {
Serial.println("FAILED");
Serial.println("REASON: " + fbdo.errorReason());
}
}
delay(3000);
digitalWrite(LED_GREEN, LOW);
} else {
accessGranted = false;
Serial.println("Access denied");
// Turn off green LED
digitalWrite(LED_GREEN, LOW);
// Turn on red LED
digitalWrite(LED_RED, HIGH);
// Create a string to store the card ID
    String cardID = "";
    // Concatenate the bytes of the NUID into the string
    for (int i = 0; i < 4; i++) {
      cardID += nuidPICC[i];
      if (i < 3) {
        cardID += ",";
      }
    }

if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 1000 || sendDataPrevMillis == 0)){
sendDataPrevMillis = millis();
// Write the card scan information and authorization result on the database path
if (Firebase.RTDB.setInt(&fbdo, "ESP32_DATA/authorizationResult", accessGranted)
          && Firebase.RTDB.setString(&fbdo, "ESP32_DATA/cardID", cardID.c_str())){
Serial.println("PASSED");
Serial.println("PATH: " + fbdo.dataPath());
Serial.println("TYPE: " + fbdo.dataType());
}
else {
Serial.println("FAILED");
Serial.println("REASON: " + fbdo.errorReason());
}
}
delay(1000);
digitalWrite(LED_RED, LOW);
}
}





/**
Helper routine to dump a byte array as hex values to Serial.
*/
void printHex(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], HEX);
  }
}



void checktemp() {
// Récupère la température et l'humidité du capteur
float temperature = dht.readTemperature();
float humidity = dht.readHumidity();

// Affiche les données sur le moniteur série
Serial.println("Temperature = " + String(temperature) + " °C");
Serial.println("Humidite = " + String(humidity) + " %");
if(temperature>=38 || humidity>50){
  digitalWrite(LED_Temp, HIGH);
  }
  else{
    digitalWrite(LED_Temp, LOW);
  }
// Envoie les données à Firebase
if (Firebase.RTDB.setFloat(&fbdo, "ESP32_DATA/temperature", temperature) && Firebase.RTDB.setFloat(&fbdo, "ESP32_DATA/humidity", humidity)) {
Serial.println("PASSED");
Serial.println("PATH: " + fbdo.dataPath());
Serial.println("TYPE: " + fbdo.dataType());
}
else {
Serial.println("FAILED");
Serial.println("REASON: " + fbdo.errorReason());
}
delay(1000);
}

void checkPresence() {
  // Read the button state
  buttonState = digitalRead(buttonPin);

  // Check if something was detected
  if (buttonState == HIGH) {
    // Turn on the LED
    digitalWrite(ledPin, HIGH);
    // Print a message to the serial monitor
    Serial.println("Alert Someone is in the house");
    // Wait for 1 second
    delay(1000);
    // Turn off the LED
    digitalWrite(ledPin, LOW);
    // Send the presence data to Firebase
    if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 1000 || sendDataPrevMillis == 0)) {
      sendDataPrevMillis = millis();
      // Write the presence data on the database path "presence"
      if (Firebase.RTDB.setBool(&fbdo, "ESP32_DATA/presence", true)) {
        Serial.println("PASSED");
        Serial.println("PATH: " + fbdo.dataPath());
        Serial.println("TYPE: " + fbdo.dataType());
      } else {
        Serial.println("FAILED");
        Serial.println("REASON: " + fbdo.errorReason());
      }
    }
  } else {
    // Turn off the LED
    digitalWrite(ledPin, LOW);
    // Send the absence data to Firebase
    if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 1000 || sendDataPrevMillis == 0)) {
      sendDataPrevMillis = millis();
      // Write the absence data on the database path "presence"
      if (Firebase.RTDB.setBool(&fbdo, "ESP32_DATA/presence", false)) {
        Serial.println("PASSED");
        Serial.println("PATH: " + fbdo.dataPath());
        Serial.println("TYPE: " + fbdo.dataType());
      } else {
        Serial.println("FAILED");
        Serial.println("REASON: " + fbdo.errorReason());
      }
    }
  }
}

/*void checkPresence()
{
  buttonState = digitalRead(buttonPin);//lecture du capteur
  if (buttonState == HIGH) //si quelquechose est detecte
  {
    digitalWrite(ledPin, HIGH); //on allume la LED
    Serial.println("Alert Someone is the house");
    delay(1000);
    digitalWrite(ledPin, LOW);
  }
  else //sinon
  {
    digitalWrite(ledPin, LOW); //on eteint la LED
  }
}*/
