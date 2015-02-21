#include <SoftwareSerial.h>

#define LED_PIN 4

SoftwareSerial espSerial(2, 3);

bool isOK = false;
int resultLen = 0;
char* result;

//Strings to search
char ipdSt[6] = "+IPD,";
char getSt[4] = "GET";

void setup() {
  pinMode(LED_PIN, OUTPUT);
  Serial.begin(9600);
  espSerial.begin(9600);
  espSerial.println("AT+RST");
  delay(2000);
  espSerial.println("AT+CIPMUX=1");
  delay(500);
  espSerial.println("AT+CIPSERVER=1,8080");
  delay(500);
  Serial.println("Ready");
}

void loop() {
  while(espSerial.available()) {
    if (search(ipdSt)) {
      char id = findConnID();
      Serial.print("New connection with ID: ");
      Serial.println(id);

      while(espSerial.available()) {

        if (search(getSt)) {
          Serial.println("New GET Request");
          skip(7);
          char* out = serialRead(4); //Read 3 chars from the serial buffer into out (RAM Buffer)
          if (strstr(out, "on") != NULL) { //Return a pointer to the first occurrence of 2 in 1
            Serial.println("ON");
            digitalWrite(LED_PIN, HIGH);
            isOK = true; //Ok flag set
          } else if (strstr(out, "off") != NULL) {
            Serial.println("OFF");
            digitalWrite(LED_PIN, LOW);
          } else {
          Serial.println("Unk?");
          }
          
          espSerial.flush(); //Clean the serial buffer
        }
      }
      
      if (isOK == true) {
        resultLen = 16;
        result = (char*) malloc(resultLen*sizeof(char));
        strcpy(result, "{\"result\": \"ok\"}");
      } else {
        resultLen = 19;
        result = (char*) malloc(resultLen*sizeof(char));
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
      Serial.println("Connection closed.");
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
  static int count = 0;
  int len = strlen(text); //Get text length

  //Reset prev count (it's static!)
  if (count >= len) {
    count = 0;
  }

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

  if (count==len) {
    return true;
  } 
  else {
    return false;
  }
}










