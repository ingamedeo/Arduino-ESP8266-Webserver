#include <SoftwareSerial.h>
#define LED 4
#define NETWORK_NAME "Ingamedeo's Network"
#define HTML_PAGE "<html> <header> <title>ESP8266</title> </header> <body style='background: #000000;'> <h1 style='color: #E6E6E6; font-size: 60px; position: absolute; top: 50%; margin: auto; margin-top: -50px; text-align: center; width: 99%'>ESP8266</h1> </body> </html>"
int errCount = 0;
int timeOut = 0;
SoftwareSerial espSerial(2, 3);
void setup() {
  pinMode(LED, OUTPUT);
  espSerial.begin(9600);
  Serial.begin(9600);
  Serial.println("Arduino ready, resetting ESP8266...");
  espSerial.println("AT+RST"); //Reset and wait
  delay(2000);
  espSerial.println("AT+CIPMUX=1");
  delay(500);
  espSerial.println("AT+CIPSERVER=1,81");
  delay(500);
  espSerial.println("AT+CWJAP?");
  while (!espSerial.find(NETWORK_NAME)) {
    Serial.println("Waiting for the module to connect to your Wi-Fi Network...");
    delay(500);
    if (timeOut >= 10) {
      Serial.println("Um..Is your Wi-Fi network up and running? I can't connect... Retry in 5 seconds");
      delay(5000);
      timeOut = 0;
    } 
    else {
      timeOut++;
    }
    espSerial.println("AT+CWJAP?");
  }
  if (errCount==0) {
    Serial.println("Everything looks great! ;) (ESP8266 is now accepting connections on port 80)");
  } 
  else {
    Serial.println("Something went wrong ;(");
  }
}
void loop() {
  delay(500);
  espSerial.println("AT+CWJAP?");
  if (espSerial.find(NETWORK_NAME)) {
    digitalWrite(LED, HIGH);
  } 
  else {
    digitalWrite(LED, LOW);
  }
  if(espSerial.find("IPD,")) {
    delay(100); //Wait 100ms for the module to prepare to next serial command
    char id = espSerial.read(); //Read next char
    Serial.print("New connection! (ID: ");
    Serial.print(id);
    Serial.println(")");
    espSerial.print("AT+CIPSEND=");
    espSerial.print(id);
    espSerial.print(",");
    int len = strlen(HTML_PAGE); //Find out the HTML length
    espSerial.println(len);
    if (espSerial.find(">")) {
      espSerial.println(HTML_PAGE);
      delay(100);
      espSerial.print("AT+CIPCLOSE=");
      espSerial.print(id);
      espSerial.println("");
    }
  }
}
