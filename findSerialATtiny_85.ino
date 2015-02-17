#include <SoftwareSerial.h>

#define LED_PIN 0

SoftwareSerial espSerial(2, 3);

bool isOK = false;
int resultLen = 0;
char* result;

//Strings to search
char ipdSt[6] = "+IPD,";
char getSt[4] = "GET";

void setup() {
  pinMode(LED_PIN, OUTPUT);
  espSerial.begin(9600);
  espSerial.println("AT+RST");
  delay(2000);
  espSerial.println("AT+CIPMUX=1");
  delay(500);
  espSerial.println("AT+CIPSERVER=1,8080");
  delay(500);
}

void loop() {
  while(espSerial.available()) {
    if (search(ipdSt)) {
      char id = findConnID();
      
      while(espSerial.available()) {

        if (search(getSt)) {
          skip(7);
          char* out = serialRead(4); //Read 3 chars from the serial buffer into out (RAM Buffer)
          if (strstr(out, "on") != NULL) { //Return a pointer to the first occurrence of 2 in 1
            digitalWrite(LED_PIN, HIGH);
            isOK = true; //Ok flag set
          } else if (strstr(out, "off") != NULL) {
            digitalWrite(LED_PIN, LOW);
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










