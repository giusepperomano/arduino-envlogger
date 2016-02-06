/*

*/
#include <EEPROM.h>

#define WIFI_SSID "wifi-network-ssid"
#define WIFI_PASSWORD "wifi-network-password"
#define WIFI_IP_ADDRESS "1.2.3.4"
#define WIFI_DEFAULT_ROUTE "1.0.0.1"
#define WIFI_NETMASK "255.0.0.0"
#define DWEET_DEVICE_NAME "unique-identifier-for-device-on-dweet-io"

#define CONFIG_OFFSET 32
#define CONFIG_SIGNATURE "env"


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

void saveConfig()
{
  sprintf(envlogger_config.version, "%s", CONFIG_SIGNATURE);
  sprintf(envlogger_config.wifi_ssid, "%s", WIFI_SSID);
  sprintf(envlogger_config.wifi_password, "%s", WIFI_PASSWORD);
  sprintf(envlogger_config.wifi_ip_address, "%s", WIFI_IP_ADDRESS);
  sprintf(envlogger_config.wifi_default_route, "%s", WIFI_DEFAULT_ROUTE);
  sprintf(envlogger_config.wifi_netmask, "%s", WIFI_NETMASK);
  sprintf(envlogger_config.dweet_device_name, "%s", DWEET_DEVICE_NAME);
  
  for(unsigned int i=0; i<sizeof(envlogger_config); i++){
    EEPROM.write(CONFIG_OFFSET+i, *((char *)&envlogger_config+i));
  }
}

void setup()
{
  Serial.begin(115200);

  /* Store configuration data to EEPROM */
  saveConfig();

  /* Read back configuration data from EEPROM */
  if(loadConfig()==0){
    Serial.println("Error loading configuration data from EEPROM");
  }
}

void loop()
{
  Serial.println("Do nothing");
  delay(10000);
}
