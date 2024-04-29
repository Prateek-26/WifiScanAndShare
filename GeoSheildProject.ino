#include <Arduino.h>
#include <WiFi.h>
#include <Firebase_ESP_Client.h>

//token generation process info.
#include "addons/TokenHelper.h"
//RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"

const int MAX_NETWORKS = 2;  // Maximum number of networks to store

String knownSSIDs[MAX_NETWORKS];
float rssiFeatures[MAX_NETWORKS];

// network credentials
#define WIFI_SSID "Redmi 8"
#define WIFI_PASSWORD "1234abcd"

// Firebase project API Key
#define API_KEY "AIzaSyCsUdxi4Hm8zMUO9O1FZYbkld_EgJVUJhI"

// RTDB URL */
#define DATABASE_URL "https://geosheildproject-default-rtdb.asia-southeast1.firebasedatabase.app/:"

//Define Firebase Data object
FirebaseData fbdo;

FirebaseAuth auth;
FirebaseConfig config;

unsigned long sendDataPrevMillis = 0;
int count = 0;
int pathNumber = 0;
int noOfValues = 0;
bool signupOK = false;

void setup() {
  Serial.print(".");
  Serial.begin(115200);
  WiFi.disconnect(); 
  knownSSIDs[0] = "Redmi 8";
  // knownSSIDs[1]="Galaxy M012162";
  // knownSSIDs[1] = "Redmi 9A";
  // knownSSIDs[2]="DIRECT-N3-BRAVIA";
  knownSSIDs[1] = "Redmi 9A";
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  /* Assign the api key */
  config.api_key = API_KEY;

  /* Assign the RTDB URL*/
  config.database_url = DATABASE_URL;

  /* Sign up */
  if (Firebase.signUp(&config, &auth, "", "")) {
    Serial.println("ok");
    signupOK = true;
  } else {
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }

  /*callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback;  //see addons/TokenHelper.h

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
  Serial.println("all ok");
}

void loop() {
  // scanAndAssignNetworks();

  if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 15000 || sendDataPrevMillis == 0)) {

    sendDataPrevMillis = millis();

    convertToFeaturesVector();
    printFeaturesVector();
    // Write an Int number on the database path test/int
    // if (Firebase.RTDB.setInt(&fbdo, "test/int", count)){
    //   Serial.println("PASSED");
    //   Serial.println("PATH: " + fbdo.dataPath());
    //   Serial.println("TYPE: " + fbdo.dataType());
    // }
    // else {
    //   Serial.println("FAILED");
    //   Serial.println("REASON: " + fbdo.errorReason());
    // }
    // count++;
    // String pathname = "wififi/";
    // char buffer[10];  // Adjust the size as needed
    // itoa(pathNumber++, buffer, 10);
    // pathname.concat(buffer);
    // Serial.println(pathname);
    // if (Firebase.RTDB.setInt(&fbdo, pathname, count)) {
    //   Serial.println("PASSED");
    //   Serial.println("PATH: " + fbdo.dataPath());
    //   Serial.println("TYPE: " + fbdo.dataType());
    // } else {
    //   Serial.println("FAILED");
    //   Serial.println("REASON: " + fbdo.errorReason());
    // }
    // count++;


    // Write an Float number on the database path test/float
    //   if (Firebase.RTDB.setFloat(&fbdo, "test/float", 0.01 + random(0,100))){
    //     Serial.println("PASSED");
    //     Serial.println("PATH: " + fbdo.dataPath());
    //     Serial.println("TYPE: " + fbdo.dataType());
    //   }
    //   else {
    //     Serial.println("FAILED");
    //     Serial.println("REASON: " + fbdo.errorReason());
    //   }
  }
}

void convertToFeaturesVector() {
  int numNetworks = WiFi.scanNetworks();

  // Clear previous RSSI feature values
  for (int i = 0; i < MAX_NETWORKS; i++) {
    rssiFeatures[i] = 0;
  }

  // Convert scan results to features vector
  // for (int i = 0; i < min(numNetworks, MAX_NETWORKS); i++) {
  //   int index = indexOf(knownSSIDs, MAX_NETWORKS, WiFi.SSID(i));
  //   if (index != -1) {
  //     rssiFeatures[index] = WiFi.RSSI(i);
  //   }
  // }
  for (int i = 0; i < min(numNetworks, MAX_NETWORKS); i++) {
    int index = indexOf(knownSSIDs, MAX_NETWORKS, WiFi.SSID(i));
    if (index != -1) {
      if (WiFi.RSSI(i) == 0) {
        rssiFeatures[index] = 0;
      } else
        rssiFeatures[index] = 100 - abs(WiFi.RSSI(i));
    }
  }
}

void printFeaturesVector() {
  Serial.println("Features Vector:");

  for (int i = 0; i < MAX_NETWORKS && !knownSSIDs[i].isEmpty(); i++) {
    Serial.print("SSID: ");
    Serial.print(knownSSIDs[i]);
    Serial.print(", RSSI: ");
    Serial.println(rssiFeatures[i]);

    String pathname = knownSSIDs[i] + "/";
    char buffer[10];  // Adjust the size as needed
    itoa(noOfValues, buffer, 10);
    pathname.concat(buffer);
    Serial.println(pathname);
    if (Firebase.RTDB.setInt(&fbdo, pathname, rssiFeatures[i])) {
      Serial.println("PASSED");
      Serial.println("PATH: " + fbdo.dataPath());
      Serial.println("TYPE: " + fbdo.dataType());
    } else {
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
    }
    // count++;
  }
  noOfValues++;
  // Serial.println("##########################################");

  Serial.println();
}

int indexOf(String arr[], int size, String target) {
  for (int i = 0; i < size; i++) {
    if (arr[i] == target) {
      return i;  // Return the index if the target is found
    }
  }
  return -1;  // Return -1 if the target is not found in the array
}
