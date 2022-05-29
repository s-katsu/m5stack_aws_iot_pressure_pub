#include <M5Stack.h>
#include <WiFiClient.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "time.h"
#include "src/config.h"
#include "src/cert.h"
#include "Omron2SMPB02E.h"

const uint32_t ALARM_WAIT_SEC = 1000 * 60 * 1;

WiFiClientSecure https_client;
PubSubClient mqtt_client(https_client);
Omron2SMPB02E prs;

StaticJsonDocument<2000> json_document;

void setupPressureSensor()
{
  prs.begin();
  prs.set_mode(MODE_NORMAL);
}

//return pressure[hPa]
uint16_t readPressure() {
  float pressure = prs.read_pressure(); //[Pa]
  pressure /= 100;
  M5.Lcd.printf("%04.2f [hPa]\n", pressure);
  M5.Lcd.setTextColor(WHITE, BLACK);

  return (uint16_t)pressure;
}

void setup()
{
  Serial.begin(115200);

  M5.begin();
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setCursor(0, 0, 2);
  M5.Lcd.setTextSize(3);

  setupPressureSensor();
  connectWifi();
  const uint32_t JST = 3600 * 9;
  configTime(JST, 0, "ntp.nict.jp", "ntp.jst.mfeed.ad.jp");
  initMQTT();
  connectAWSIOT();
}

// connect to wifi – returns true if successful or false if not
boolean connectWifi(){
  WiFi.disconnect(true);
  delay(1000);

  WiFi.begin(SSID, PASS);

  Serial.println("Connecting to WiFi");

  bool is_connected = true;
  Serial.print("Connecting...");
  uint32_t i = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    if (i > 40) {
      is_connected = false;
      break;
    }
    i++;
  }
  Serial.println("");
  if(is_connected){
    Serial.print("Connected to ");
    Serial.println(SSID);
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
  }else{
    Serial.println("Connection failed.");
  }
  return is_connected;
}

void reconnectWifi(){
  if(WiFi.status() != WL_CONNECTED){
    connectWifi();
  }
}

void connectAWSIOT() {
  reconnectWifi();

  while(!mqtt_client.connected()) {
    Serial.println("Try to MQTT connect..");
    if (mqtt_client.connect(THING_NAME)){
      Serial.println("Connected");
    }else{
      Serial.printf("Failed rc=%d ", mqtt_client.state());
      Serial.println("try again in 5 sec");
      delay(5000);
    }
  }
}

void initMQTT() {
  https_client.setCACert(root_ca);
  https_client.setCertificate(certificate);
  https_client.setPrivateKey(private_key);
  mqtt_client.setServer(AWS_IOT_ENDPOINT, AWS_IOT_PORT);
}

uint32_t getTime() {
  time_t now;
  struct tm time_info;

  if (!getLocalTime(&time_info)){
    return 0;
  }
  time(&now);
  return now;
}

void printMenu()
{
  M5.Lcd.setCursor(0, 0, 2);
  M5.Lcd.println("Pressure Sensor");
}

void loop()
{
  M5.update();  // ボタン状態更新
  printMenu();

  char json_string[1000];
  uint16_t pressure = readPressure();

  static uint32_t last_pub_time = 0;
  uint32_t current_time = getTime();
  if((current_time - last_pub_time) >= 60 * 10){ //10min
    connectAWSIOT();
    json_document["device_name"] = "prsstack01";
    json_document["type"] = "Prssensor";
    json_document["pressure"] = pressure;
    json_document["timestamp"] = current_time;

    serializeJson(json_document, json_string);
    Serial.println(json_string);
    bool ret = mqtt_client.publish(TOPIC, json_string);
    Serial.printf("pub ret %d\n", ret);
    last_pub_time = current_time;
  }
  delay(ALARM_WAIT_SEC);
}
