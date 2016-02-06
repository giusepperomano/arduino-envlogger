/*

*/
#include <EEPROM.h>
#include "DHT.h"

DHT dht(2, DHT22);

#define CONFIG_OFFSET 32
#define CONFIG_SIGNATURE "env"


String buffer;
struct eepromdata {
  char version[4];
  char wifi_ssid[64];
  char wifi_password[64];
  char wifi_ip_address[16];
  char wifi_default_route[16];
  char wifi_netmask[16];
  char dweet_device_name[32];
} envlogger_config;

boolean loadConfig()
{
  if(EEPROM.read(CONFIG_OFFSET)==CONFIG_SIGNATURE[0]&&EEPROM.read(CONFIG_OFFSET+1)==CONFIG_SIGNATURE[1]&&EEPROM.read(CONFIG_OFFSET+2)==CONFIG_SIGNATURE[2]){
    for(unsigned int i=0; i<sizeof(envlogger_config); i++){
      *((char*)&envlogger_config+i) = EEPROM.read(CONFIG_OFFSET+i);
    }
    return 1;
  }
  return 0;
}

boolean mydebug()
{
  while(Serial3.available()){
    char c = Serial3.read();
    Serial.write(c);
    if(c=='\r'){
      Serial.print('\n');
    }
  }
}

void getStatus(String prefix)
{
  buffer = "";

  while(Serial3.available()){
    char c = Serial3.read();
    buffer.concat(c);
  }
  /* Uncomment for debugging: prints ESP8266 output to serial monitor
  Serial.println(">>> START "+prefix+" <<<");
  Serial.print(buffer);
  Serial.println(">>> END "+prefix+" <<<");
  */
}

boolean checkStatus(String str)
{
  if(buffer.length()>=str.length()){
    for(int n=0; n<(buffer.length()-str.length()); n++){
      if(buffer.substring(n, n+str.length())==str){
        return 1;
      }
    }
  }
  return 0;
}

boolean connectWiFi()
{
  /* Restart esp8266 module */
  Serial3.println("AT+RST");
  delay(1000);
  getStatus("AT+RST");
  if(checkStatus("OK")){
    Serial.println("connectWiFi: restarted esp8266");
  }
  else {
    Serial.println("connectWiFi: error restarting esp8266");
    return 0;
  }

  Serial3.println("ATE0");
  delay(1000);
  getStatus("ATE0");
  
  /* Switch to station mode */
  Serial3.println("AT+CWMODE=1");
  delay(1000);
  getStatus("AT+CWMODE=1");
  if(checkStatus("OK")){
    Serial.println("connectWiFi: set station mode");
  }
  else {
    Serial.println("connectWiFi: error setting station mode");
    return 0;
  }
  
  /* Set the esp8266 station's IP address, netmask and default gateway */
  Serial3.print("AT+CIPSTA=\"");
  Serial3.print(envlogger_config.wifi_ip_address);
  Serial3.print("\",\"");
  Serial3.print(envlogger_config.wifi_default_route);
  Serial3.print("\",\"");
  Serial3.print(envlogger_config.wifi_netmask);
  Serial3.println("\"");
  delay(1000);
  getStatus("AT+CIPSTA");
  if(checkStatus("OK")){
    Serial.println("connectWiFi: set IP address");
  }
  else {
    Serial.println("connectWiFi: error setting IP address");
    return 0;
  }
  
  /* Connect to AP */
  Serial3.print("AT+CWJAP=\"");
  Serial3.print(envlogger_config.wifi_ssid);
  Serial3.print("\",\"");
  Serial3.print(envlogger_config.wifi_password);
  Serial3.println("\"");
  delay(5000);
  getStatus("AT+CWJAP");
  if(checkStatus("WIFI CONNECTED")){
    Serial.println("connectWiFi: connected to Wi-Fi network");
  }
  else {
    Serial.println("connectWiFi: error connecting to Wi-Fi network");
    return 0;
  }
  /* Enable multiple connections */
  Serial3.println("AT+CIPMUX=1");
  delay(1000);
  getStatus("AT+CIPMUX");
  if(checkStatus("OK")){
    Serial.println("connectWiFi: enabled multiple connections");
  }
  else {
    Serial.println("connectWiFi: error enabling multiple connections");
    return 0;
  }
  return 1;
}

boolean checkWiFi()
{
  Serial3.println("AT+CWJAP?");
  mydebug();
  if(Serial3.find(envlogger_config.wifi_ssid)){
    return 1;
  }
  return 0;
}

boolean httpGet(String host, String request)
{

  Serial3.print("AT+CIPSTART=4,\"TCP\",\"");
  Serial3.print(host);
  Serial3.println("\",80");
  if(Serial3.find("Error")){
    return 0;
  }
  Serial3.print("AT+CIPSEND=4,");
  Serial3.println(request.length());
  if(Serial3.find(">")){
    Serial.print(">");
    Serial3.print(request);
    if(Serial3.find("OK")){
      Serial.println("SEND OK");
    }
    else {
      Serial.println("SEND KO");
    }
  }
  Serial3.println("AT+CIPCLOSE=4");

  delay(1000);
  if(Serial3.find("HTTP/1.1 200 OK")){
    Serial.print("OK");
  }
  else {
    Serial.print("KO");
  }
  mydebug();
  return 1;
}

void setup()
{
  Serial.begin(115200);
  Serial3.begin(115200);
  Serial3.setTimeout(1000);

  if(loadConfig()==0){
    Serial.println("Error loading configuration data from EEPROM");
  }

  dht.begin();
  do {
    Serial.println("Setting up Wi-Fi...");
  } while(connectWiFi()==0);
}

void loop()
{
  char buffer[50];
  char hStr[10];
  char tStr[10];

  float h = dht.readHumidity();
  float t = dht.readTemperature();
  if(isnan(t)||isnan(h)){
    Serial.println("Cannot read sensor");
  }
  
  dtostrf(t, 5, 2, tStr);
  dtostrf(h, 5, 2, hStr);
  sprintf(buffer, "GET /dweet/for/%s?temp=%s&humid=%s\r\n\r\n", envlogger_config.dweet_device_name, tStr, hStr);
  Serial.println(buffer);

  httpGet("dweet.io", buffer);

  delay(10000);
}
