#include "SoftwareSerial.h"
#include <avr/wdt.h>

#define RED_PIN 8
#define GREEN_PIN 9
#define RELAY_PIN 7
#define BUTTON_PIN 6

SoftwareSerial espSerial(2, 3);

bool isOK = false;
int resultLen = 0;
char* result;
int count = 0;

//Strings to search
char ipdSt[6] = "+IPD,";
char getSt[4] = "GET";
char okSt[3] = "OK";

void setup() {
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(RED_PIN, OUTPUT);
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT);

  //Green OFF, Red ON
  digitalWrite(GREEN_PIN, HIGH);
  digitalWrite(RED_PIN, LOW);

  //Init serial connection with ESP8266
  espSerial.begin(9600);
  espSerial.println("AT+RST");
  delay(2000);
  espSerial.flush(); //Flush weird stuff
  delay(100);
  espSerial.println("AT+CIPMUX=1");
  if (!waitOK()) {
    wdt_enable(WDTO_15MS);
  }
  delay(100);
  espSerial.println("AT+CIPSERVER=1,8080");
  if (!waitOK()) {
    wdt_enable(WDTO_15MS);
  }
  delay(100);
}

void loop() {

  if (digitalRead(BUTTON_PIN) == HIGH) {
    openDoorSequence();
    delay(1000);
  }

  checkConnection();

  while (espSerial.available()) {
    if (search(ipdSt)) {
      count = 0;
      char id = findConnID();

      while (espSerial.available()) {

        if (search(getSt)) {
          count = 0;
          skip(7);
          char* out = serialRead(3); //Read 3 chars from the serial buffer into out (RAM Buffer)
          if (strstr(out, "on") != NULL) { //Return a pointer to the first occurrence of 2 in 1
            openDoorSequence();
            isOK = true; //Ok flag set
          }

          espSerial.flush(); //Clean the serial buffer
        }
      }

      if (isOK == true) {
        resultLen = 16;
        result = (char*) malloc(resultLen * sizeof(char));
        strcpy(result, "{\"result\": \"ok\"}");
      } else {
        resultLen = 19;
        result = (char*) malloc(resultLen * sizeof(char));
        strcpy(result, "{\"result\": \"error\"}");
      }
      isOK = false; //Reset to false

      delay(100);
      espSerial.print("AT+CIPSEND=");
      espSerial.print(id);
      espSerial.print(",");
      espSerial.println(resultLen);
      delay(100);

      espSerial.println(result);
      delay(100);

      espSerial.print("AT+CIPCLOSE=");
      espSerial.println(id);
      delay(100);
    }
  }
}

void skip(int count) {
  int i;
  for (i = 0; i < count; i++) {
    espSerial.read();
  }
}

char findConnID() {
  return espSerial.read();
}

char* serialRead(int len) {
  char out[len]; //Init buffer
  int i;

  for (int i = 0; (i < len && espSerial.available()); i++) {
    out[i] = espSerial.read();
    delay(100);
  }

  return out;
}

bool search(char* text) {
  char c;
  bool orRes = false;
  int i;
  int len = strlen(text); //Get text length

  //Read just one char from serial buffer
  c = espSerial.read();

  /* For each char in text */
  for (i = 0; i < len; i++) {
    if (c == text[i]) {
      orRes = true;
    }
  }

  if (orRes == true) {
    count++;
  }
  else {
    if (count > 0) {
      count--;
    }
  }

  if (count == len) {
    return true;
  }
  else {
    return false;
  }
}

void notBlockingBlink() {
  long blinkInterval = 250;
  static long previousMillis = 0;
  static bool prevState = HIGH; //This works in the opposite way to what it usually does...
  unsigned long currentMillis = millis();

  if ((currentMillis - previousMillis) >= blinkInterval) {
    previousMillis = currentMillis;
    prevState = !prevState;
    digitalWrite(RED_PIN, prevState);
  }
}

bool waitOK() {
  int timeout = 0;
  count = 0;
  while (!espSerial.available()) { //Wait until we get some data in...
    notBlockingBlink();
  }
  while (true) { //We got sth
    while (espSerial.available()) {
      if (search(okSt)) {
        return true;
      } else {
        notBlockingBlink();
      }
    }
    notBlockingBlink();
    if (timeout > 300) {
      return false;
    } else {
      delay(10);
      timeout++;
    }
  }
}

bool isOnline() {
  int timeout = 0;
  int maxTime = 100;
  count = 0;
  while (!espSerial.available() && timeout <= maxTime) { //Wait until we get some data in...
    delay(10);
    timeout++;
  }
  while (timeout <= maxTime) { //We got sth
    while (espSerial.available() && timeout <= maxTime) {
      if (search(okSt)) {
        return true;
      } else {
        delay(10);
        timeout++;
      }
    }
    delay(10);
    timeout++;
  }
  return false;
}

void checkConnection() {
  static bool isConnected = true;
  long checkEvery = 5000;
  static long previousMillis = 0;
  static long lastStateTime = 0;
  static bool prevWiFiState = false;
  unsigned long currentMillis = millis();

  if ((currentMillis - previousMillis) >= checkEvery) {
    previousMillis = currentMillis;
    espSerial.println("AT+CWJAP?");
    isConnected = isOnline();
  }

  if (isConnected != prevWiFiState) {
    lastStateTime = currentMillis;
    prevWiFiState = isConnected;
  }

  if ((currentMillis - lastStateTime) < checkEvery) {
    if (isConnected) {
      //Green ON, Red OFF
      digitalWrite(GREEN_PIN, LOW);
      digitalWrite(RED_PIN, HIGH);
    } else {
      //Green OFF
      digitalWrite(GREEN_PIN, HIGH);
      notBlockingBlink(); //Blink RED
    }
  } else {
    digitalWrite(GREEN_PIN, HIGH);
    digitalWrite(RED_PIN, HIGH);
  }
}

void openDoorSequence() {
  digitalWrite(RELAY_PIN, HIGH);
  delay(500);
  digitalWrite(RELAY_PIN, LOW);
}
