#include <ESP8266WiFi.h>     
//#include <WiFi.h>              

#define SENSOR_FRONT RISING 

#ifdef ESP32
  #define SENSOR_PIN 0  //IO0
  //#define SENSOR_PIN 4  //IO4
  #define RECONNECT_INTERVAL 500
  #define TCP_TIMEOUT 100
  #define LED_ON HIGH
  #define LED_OFF LOW

#else //8266
  #define SENSOR_PIN 4  //D2
  #define RECONNECT_INTERVAL 20000  
  #define TCP_TIMEOUT 1000
  #define LED_ON LOW
  #define LED_OFF HIGH
#endif  

#define SSID_NAME    "Finisher"
#define SSID_PASSWORD  "322-223Karlson"
#define APP_PORT 3223

IPAddress hostIp;
boolean   gotIP = false;


WiFiClient wifiClient;

unsigned long previousMillis = 0;
char cstr[80];


volatile int interruptCount;

void IRAM_ATTR onInterrupt() {
    interruptCount++;
}

void setup() {
  pinMode(LED_BUILTIN,OUTPUT);
  pinMode(SENSOR_PIN,INPUT_PULLUP);
  digitalWrite(LED_BUILTIN,HIGH);
  Serial.begin(115200);

  interruptCount=0;
  attachInterrupt(SENSOR_PIN, onInterrupt, SENSOR_FRONT);
  previousMillis = millis();  

  WiFi.mode(WIFI_STA);
  wifiClient.setTimeout(TCP_TIMEOUT);
  
}

void loop() {
  digitalWrite(LED_BUILTIN,LED_OFF);
  unsigned long currentMillis = millis();
  // if WiFi is down, try reconnecting every CHECK_WIFI_TIME seconds
  if (WiFi.status() != WL_CONNECTED) {
    digitalWrite(LED_BUILTIN,LED_OFF);
    gotIP=false;    
    if(currentMillis - previousMillis >=RECONNECT_INTERVAL) {      
      Serial.println("Reconnecting to WiFi...");
      WiFi.disconnect();
      WiFi.begin(SSID_NAME, SSID_PASSWORD);  
      previousMillis = currentMillis;            
    }
    return;
  }
  if(gotIP == false) {      
    hostIp = WiFi.gatewayIP();    
    gotIP=true;
    Serial.print("Connected to:");  
    Serial.print(hostIp);
  }
  
  
  if(!wifiClient.connected()) {
    digitalWrite(LED_BUILTIN,HIGH);
    wifiClient.connect(hostIp,APP_PORT);
    if(!wifiClient.connected()) {
      Serial.println("TCP Conn NOT opened");
      digitalWrite(LED_BUILTIN,LED_OFF);    
      return;
    }
    
    Serial.println("TCP Conn opened");
    digitalWrite(LED_BUILTIN,LED_OFF);    
  }

  int count; 
  noInterrupts();
  count = interruptCount;
  interruptCount =0;
  interrupts();

    
  sprintf_P(cstr, (PGM_P)F("%d\n"), count);
  wifiClient.write(cstr,strlen(cstr));
  if(count>0) {
    Serial.print("InterruptsCnt:");
    Serial.print(cstr);
  }
  
}
