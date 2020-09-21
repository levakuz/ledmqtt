#include "WiFi.h" //библиотека для использования Wi-Fi на esp
#include <AsyncMqttClient.h> //библиотека mqtt клиента
#include "FastLED.h"

extern "C" {
  #include "freertos/FreeRTOS.h"
  #include "freertos/timers.h"
}

#include "soc/timer_group_struct.h"
#include "soc/timer_group_reg.h"

#define WIFI_SSID "Keenetic-8903" //имя Wi-Fi сети
#define WIFI_PASSWORD "89219297487" //пароль Wi-Fi сети
#define MQTT_HOST IPAddress(192,168,1,1)//ip адрес mqtt брокера
#define MQTT_PORT 1883 //порт mqtt брокера


#define FirstSectionNumLeds 160 //количество диодов 1й секции
#define SecondSectionNumLeds 159//количество диодов 2й секции 
#define ThirdSectionNumLeds 161 //количество диодов 3й секции
#define FourthSectionNumLeds 251 //количество диодов 4й секции

#define FirstSectionPin 27 //пин 1й секции
#define SecondSectionPin 26 //пин 2й секции 
#define ThirdSectionPin 25 //пин 3й секции
#define FourthSectionPin 33 //пин 4й секции


CLEDController *controllers[4];
CRGB FirstSectionLeds[FirstSectionNumLeds];
CRGB SecondSectionLeds[SecondSectionNumLeds];
CRGB ThirdSectionLeds[ThirdSectionNumLeds];
CRGB FourthSectionLeds[FourthSectionNumLeds];

long n;
AsyncMqttClient mqttClient;


TaskHandle_t TaskZone1Mode1, TaskZone1Mode2, TaskZone1Mode3, TaskZone1Mode4, TaskZone1Mode5, TaskZone1Mode6 = NULL;  //Tasks of mods for Zone№1
TaskHandle_t TaskZone2Mode1, TaskZone2Mode2, TaskZone2Mode3, TaskZone2Mode4, TaskZone2Mode5, TaskZone2Mode6 = NULL;  //Tasks of mods for Zone№2
TaskHandle_t TaskZone3Mode1, TaskZone3Mode2, TaskZone3Mode3, TaskZone3Mode4, TaskZone3Mode5, TaskZone3Mode6 = NULL;  //Tasks of mods for Zone№3
TaskHandle_t TaskZone4Mode1, TaskZone4Mode2, TaskZone4Mode3, TaskZone4Mode4, TaskZone4Mode5, TaskZone4Mode6 = NULL;  //Tasks of mods for Zone№4


TimerHandle_t mqttReconnectTimer; //timer of reconect to mqtt broker
TimerHandle_t wifiReconnectTimer; //timer of reconect to WiFi


bool flagZone1Mode1,flagZone1Mode2,flagZone1Mode3, flagZone1Mode4, flagZone1Mode5, flagZone1Mode6;//flags of task's states for Zone1
bool flagZone2Mode1,flagZone2Mode2,flagZone2Mode3, flagZone2Mode4, flagZone2Mode5, flagZone2Mode6;//flags of task's states for Zone2
bool flagZone3Mode1,flagZone3Mode2,flagZone3Mode3, flagZone3Mode4, flagZone3Mode5, flagZone3Mode6;//flags of task's states for Zone3
bool flagZone4Mode1,flagZone4Mode2,flagZone4Mode3, flagZone4Mode4, flagZone4Mode5, flagZone4Mode6;//flags of task's states for Zone4

void setup() {
  Serial.begin(115200);

  LEDS.addLeds<WS2812B, FirstSectionPin, GRB>(FirstSectionLeds, FirstSectionNumLeds).setCorrection( TypicalLEDStrip );//1й участок ленты
  LEDS.addLeds<WS2812B, SecondSectionPin, GRB>(SecondSectionLeds, SecondSectionNumLeds).setCorrection( TypicalLEDStrip );//2й участок ленты
  LEDS.addLeds<WS2812B, ThirdSectionPin, GRB>(ThirdSectionLeds, ThirdSectionNumLeds).setCorrection( TypicalLEDStrip );//3й участок ленты
  LEDS.addLeds<WS2812B, FourthSectionPin, GRB>(FourthSectionLeds, FourthSectionNumLeds).setCorrection( TypicalLEDStrip );//4й участок ленты

  WiFi.onEvent(WiFiEvent); //задает то. что при подключении к wi-fi будет запущена функция обратного вызова WiFiEvent(), которая напечатает данные о WiFi подключении
  connectToWifi();
  mqttClient.setServer(MQTT_HOST, MQTT_PORT);
  //mqttClient.setCredentials("mqtt-test","mqtt-test");
  mqttClient.onConnect(onMqttConnect);
  mqttClient.onMessage(onMqttMessage);
  connectToMqtt();  
  mqttReconnectTimer = xTimerCreate("mqttTimer", pdMS_TO_TICKS(2000), pdFALSE, (void*)0, reinterpret_cast<TimerCallbackFunction_t>(connectToMqtt));
  wifiReconnectTimer = xTimerCreate("wifiTimer", pdMS_TO_TICKS(2000), pdFALSE, (void*)0, reinterpret_cast<TimerCallbackFunction_t>(connectToWifi));


//////////////////MODS ZONE1//////////////////////////////////////  
  xTaskCreatePinnedToCore(rainbowCycle, "rainbowCycleZone1", 1000, (void*)FirstSectionNumLeds, 2, &TaskZone1Mode1,1); // assigned to core 1
  vTaskSuspend( TaskZone1Mode1 );
  flagZone1Mode1 = 0;

  xTaskCreatePinnedToCore(CyclonZone1, "CyclonZone1", 1000, NULL, 2, &TaskZone1Mode2, 1); // assigned to core 1  
  vTaskSuspend( TaskZone1Mode2 );
  flagZone1Mode2 = 0;

  xTaskCreatePinnedToCore(RGBLoopZone1, "RGBLoopZone1", 1000, (void*)FirstSectionNumLeds, 2, &TaskZone1Mode3, 1); // assigned to core 1
  vTaskSuspend( TaskZone1Mode3 );
  flagZone1Mode3 = 0;
  
  xTaskCreatePinnedToCore(fireTickZone1, "fireTickZone1", 1000, (void*)FirstSectionNumLeds, 2, &TaskZone1Mode4, 1); // assigned to core 1
  vTaskSuspend( TaskZone1Mode4 );
  flagZone1Mode4 = 0;

  xTaskCreatePinnedToCore(pacifica_loop_zone1, "pacificZone1", 1000, (void*)FirstSectionNumLeds, 2, &TaskZone1Mode5, 1); // assigned to core 1
  vTaskSuspend( TaskZone1Mode5 );
  flagZone1Mode5 = 0;

  xTaskCreatePinnedToCore(sunrise, "pacificZone1", 1000, (void*)FirstSectionNumLeds, 2, &TaskZone1Mode6, 1); // assigned to core 1
  vTaskSuspend( TaskZone1Mode6 );
  flagZone1Mode6 = 0;


//////////////////MODS ZONE2//////////////////////////////////////
  xTaskCreatePinnedToCore(rainbowCycle2, "rainbowCycleZone2", 1000, (void*)SecondSectionNumLeds, 2, &TaskZone2Mode1,1); // assigned to core 1
  vTaskSuspend( TaskZone2Mode1 );
  flagZone2Mode1 = 0;
  
  xTaskCreatePinnedToCore(CyclonZone2, "CyclonZone2", 1000, NULL, 2, &TaskZone2Mode2, 1); // assigned to core 1  
  vTaskSuspend( TaskZone2Mode2 );
  flagZone2Mode2 = 0;
  
  xTaskCreatePinnedToCore(RGBLoopZone2, "RGBLoopZone2", 1000, (void*)SecondSectionNumLeds, 2, &TaskZone2Mode3, 1); // assigned to core 1
  vTaskSuspend( TaskZone2Mode3 );
  flagZone2Mode3 = 0;
  
  xTaskCreatePinnedToCore(fireTickZone2, "fireTickZone2", 1000, (void*)SecondSectionNumLeds, 2, &TaskZone2Mode4, 1); // assigned to core 1
  vTaskSuspend( TaskZone2Mode4 );
  flagZone2Mode4 = 0;

  xTaskCreatePinnedToCore(pacifica_loop_zone2, "pacificZone2", 1000, (void*)SecondSectionNumLeds, 2, &TaskZone2Mode5, 1); // assigned to core 1
  vTaskSuspend( TaskZone2Mode5 );
  flagZone2Mode5 = 0;

  xTaskCreatePinnedToCore(sunrise2, "pacificZone1", 1000, (void*)SecondSectionNumLeds, 2, &TaskZone2Mode6, 1); // assigned to core 1
  vTaskSuspend( TaskZone2Mode6 );
  flagZone2Mode6 = 0;


////////////////////MODS ZONE3//////////////////////////////////////
  xTaskCreatePinnedToCore(rainbowCycle3, "rainbowCycleZone3", 1000, (void*)ThirdSectionNumLeds, 2, &TaskZone3Mode1,1); // assigned to core 1
  vTaskSuspend( TaskZone3Mode1 );
  flagZone3Mode1 = 0;
  
  xTaskCreatePinnedToCore(CyclonZone3, "CyclonZone3", 1000, NULL, 2, &TaskZone3Mode2, 1); // assigned to core 1  
  vTaskSuspend( TaskZone3Mode2 );
  flagZone3Mode2 = 0;
  
  xTaskCreatePinnedToCore(RGBLoopZone3, "RGBLoopZone3", 1000, (void*)ThirdSectionNumLeds, 2, &TaskZone3Mode3, 1); // assigned to core 1
  vTaskSuspend( TaskZone3Mode3 );
  flagZone3Mode3 = 0;
  
  xTaskCreatePinnedToCore(fireTickZone3, "fireTickZone3", 1000, (void*)ThirdSectionNumLeds, 2, &TaskZone3Mode4, 1); // assigned to core 1
  vTaskSuspend( TaskZone3Mode4 );
  flagZone3Mode4 = 0;


  xTaskCreatePinnedToCore(pacifica_loop_zone3, "pacificZone3", 1000, (void*)ThirdSectionNumLeds, 2, &TaskZone3Mode5, 1); // assigned to core 1
  vTaskSuspend( TaskZone3Mode5 );
  flagZone3Mode5 = 0;

  xTaskCreatePinnedToCore(sunrise3, "pacificZone1", 1000, (void*)ThirdSectionNumLeds, 2, &TaskZone3Mode6, 1); // assigned to core 1
  vTaskSuspend( TaskZone3Mode6 );
  flagZone3Mode6 = 0;


////////////////////MODS ZONE4//////////////////////////////////////
  xTaskCreatePinnedToCore(rainbowCycle4, "rainbowCycleZone4", 1000, (void*)FourthSectionNumLeds, 2, &TaskZone4Mode1,1); // assigned to core 1
  vTaskSuspend( TaskZone4Mode1 );
  flagZone4Mode1 = 0;
  
  xTaskCreatePinnedToCore(CyclonZone4, "CyclonZone4", 1000, NULL, 2, &TaskZone4Mode2, 1); // assigned to core 1  
  vTaskSuspend( TaskZone4Mode2 );
  flagZone4Mode2 = 0;
  
  xTaskCreatePinnedToCore(RGBLoopZone4, "RGBLoopZone4", 1000, (void*)FourthSectionNumLeds, 2, &TaskZone4Mode3, 1); // assigned to core 1
  vTaskSuspend( TaskZone4Mode3 );
  flagZone4Mode3 = 0;
  
  xTaskCreatePinnedToCore(fireTickZone4, "fireTickZone4", 1000, (void*)FourthSectionNumLeds, 2, &TaskZone4Mode4, 1); // assigned to core 1
  vTaskSuspend( TaskZone4Mode4 );
  flagZone4Mode4 = 0;  


  xTaskCreatePinnedToCore(pacifica_loop_zone4, "pacificZone4", 1000, (void*)FourthSectionNumLeds, 2, &TaskZone4Mode5, 1); // assigned to core 1
  vTaskSuspend( TaskZone4Mode5 );
  flagZone3Mode5 = 0;


  xTaskCreatePinnedToCore(sunrise4, "pacificZone4", 1000, (void*)FourthSectionNumLeds, 2, &TaskZone4Mode6, 1); // assigned to core 1
  vTaskSuspend( TaskZone4Mode6 );
  flagZone4Mode6 = 0;
  
}


void connectToWifi() {
Serial.println("Connecting to Wi-Fi...");
           //  "Подключаемся к WiFi..."
Serial.println(WIFI_SSID);
WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
}


void connectToMqtt() {
  Serial.println("Connecting to MQTT...");
           //  "Подключаемся к MQTT..."
  mqttClient.connect();
  }


void onMqttConnect(bool sessionPresent) {
    Serial.println("Connected to MQTT.");                           //  "Подключились к MQTT."
    Serial.print("Session present: ");                              //  "Текущая сессия: "
    Serial.println(sessionPresent);
    uint16_t packetIdSub1 = mqttClient.subscribe("room1/allroom/color", 0);
    Serial.print("Subscribing at QoS 0, packetId: ");               //  "Подписка при QoS 0, ID пакета: "
    Serial.println(packetIdSub1);
    uint16_t packetIdSub2 = mqttClient.subscribe("room1/zone4/color", 0);
    Serial.print("Subscribing at QoS 0, packetId: ");               //  "Подписка при QoS 0, ID пакета: "
    Serial.println(packetIdSub2);
    uint16_t packetIdSub3 = mqttClient.subscribe("room1/zone3/color", 0);
    Serial.print("Subscribing at QoS 0, packetId: ");               //  "Подписка при QoS 0, ID пакета: "
    Serial.println(packetIdSub3);
    uint16_t packetIdSub4 = mqttClient.subscribe("room1/zone2/color", 0);
    Serial.print("Subscribing at QoS 0, packetId: ");               //  "Подписка при QoS 0, ID пакета: "
    Serial.println(packetIdSub4);
    uint16_t packetIdSub5 = mqttClient.subscribe("room1/zone1/color", 0);
    Serial.print("Subscribing at QoS 0, packetId: ");               //  "Подписка при QoS 0, ID пакета: "
    Serial.println(packetIdSub5);
    uint16_t packetIdSub6 = mqttClient.subscribe("room1/zone1", 0);
    Serial.print("Subscribing at QoS 0, packetId: ");               //  "Подписка при QoS 0, ID пакета: "
    Serial.println(packetIdSub6);
    uint16_t packetIdSub7 = mqttClient.subscribe("room1/zone2", 0);
    Serial.print("Subscribing at QoS 0, packetId: ");               //  "Подписка при QoS 0, ID пакета: "
    Serial.println(packetIdSub7);
    uint16_t packetIdSub8 = mqttClient.subscribe("room1/zone3", 0);
    Serial.print("Subscribing at QoS 0, packetId: ");               //  "Подписка при QoS 0, ID пакета: "
    Serial.println(packetIdSub8);
    uint16_t packetIdSub9 = mqttClient.subscribe("room1/zone4", 0);
    Serial.print("Subscribing at QoS 0, packetId: ");               //  "Подписка при QoS 0, ID пакета: "
    Serial.println(packetIdSub9);
    uint16_t packetIdSub10 = mqttClient.subscribe("room1/allroom", 0);
    Serial.print("Subscribing at QoS 0, packetId: ");               //  "Подписка при QoS 0, ID пакета: "
    Serial.println(packetIdSub10);

}


void onMqttSubscribe(uint16_t packetId, uint8_t qos) { //подписка на топик
  Serial.println("Subscribe acknowledged.");
             //  "Подписка подтверждена."
  Serial.print("  packetId: ");
  Serial.println(packetId);
  Serial.print("  qos: ");
  Serial.println(qos);
}


void WiFiEvent(WiFiEvent_t event) {
  Serial.printf("[WiFi-event] event: %d\n", event);
  switch(event) {
    case SYSTEM_EVENT_STA_GOT_IP:
      Serial.println("WiFi connected");  //  "Подключились к WiFi"
      Serial.println("IP address: ");  //  "IP-адрес: "
      Serial.println(WiFi.localIP());
      connectToMqtt();
      break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
      Serial.println("WiFi lost connection");
                 //  "WiFi-связь потеряна"
      // делаем так, чтобы ESP32
      // не переподключалась к MQTT
      // во время переподключения к WiFi:
      xTimerStop(mqttReconnectTimer, 0);
      xTimerStart(wifiReconnectTimer, 0);
      break;
  }
}


void onMqttDisconnect(AsyncMqttClientDisconnectReason reason) {
  Serial.println("Disconnected from MQTT.");
             //  "Отключились от MQTT."
  if (WiFi.isConnected()) {
    xTimerStart(mqttReconnectTimer, 0);
  }
}


void onMqttMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total) {
  char messageTemp[7], *end;
  for (int i = 0; i < len; i++) {
    messageTemp[i] = (char)payload[i];
  }

  Serial.println("Publish received.");
             //  "Опубликованные данные получены."
  Serial.print("  message: ");  //  "  сообщение: "
  Serial.println(messageTemp);
  Serial.print("  topic: ");  //  "  топик: "
  Serial.println(topic);
  Serial.print("  qos: ");  //  "  уровень обслуживания: "
  Serial.println(properties.qos);
  Serial.print("  dup: ");  //  "  дублирование сообщения: "
  Serial.println(properties.dup);
  Serial.print("  retain: ");  //  "сохраненные сообщения: "
  Serial.println(properties.retain);
  Serial.print("  len: ");  //  "  размер: "
  Serial.println(len);
  Serial.print("  index: ");  //  "  индекс: "
  Serial.println(index);
  Serial.print("  total: ");  //  "  суммарно: "
  Serial.println(total);

  
  if (strcmp(topic, "room1/allroom/color") == 0) {
     
     long n = strtoul(messageTemp, NULL, 16);
     
/////Stop all tasks for 1st zone
     if(flagZone1Mode1 != 0){
        pauseTasks(1, 1);}
     if(flagZone1Mode2 != 0){   
        pauseTasks(1, 2);}
     if(flagZone1Mode3 != 0){
        pauseTasks(1, 3);}
     if(flagZone1Mode4 != 0){
        pauseTasks(1, 4);}
     if(flagZone1Mode5 != 0){
        pauseTasks(1, 5);}
     if(flagZone1Mode6 != 0){
        pauseTasks(1, 6);}
        

/////Stop all tasks for 2nd zone
      if(flagZone2Mode1 != 0){
        pauseTasks(2, 1);}
      if(flagZone2Mode2 != 0){  
        pauseTasks(2, 2);}
      if(flagZone2Mode3 != 0){
        pauseTasks(2, 3);}
      if(flagZone2Mode4 != 0){
        pauseTasks(2, 4);}
      if(flagZone2Mode5 != 0){
        pauseTasks(2, 5);}
      if(flagZone2Mode6 != 0){
        pauseTasks(2, 6);}
        
/////Stop all tasks for 3rd zone
      if(flagZone3Mode1 != 0){
        pauseTasks(3, 1);}
      if(flagZone3Mode2 != 0){
        pauseTasks(3, 2);}
      if(flagZone3Mode3 != 0){
        pauseTasks(3, 3);}
      if(flagZone3Mode4 != 0){
        pauseTasks(3, 4);}
      if(flagZone3Mode5 != 0){
        pauseTasks(3, 5);}
      if(flagZone3Mode6 != 0){
        pauseTasks(3, 6);}  
//
/////Stop all tasks for 4th zone
      if(flagZone4Mode1 != 0){
        pauseTasks(4, 1);}
      if(flagZone4Mode2 != 0){
        pauseTasks(4, 2);}
      if(flagZone4Mode3 != 0){
        pauseTasks(4, 3);}
      if(flagZone4Mode4 != 0){
        pauseTasks(4, 4);}
      if(flagZone4Mode5 != 0){
        pauseTasks(4, 5);}
      if(flagZone4Mode6 != 0){
        pauseTasks(4, 6);}
     
     fill_solid(FirstSectionLeds, FirstSectionNumLeds, n);
     fill_solid(SecondSectionLeds, SecondSectionNumLeds, n);
     fill_solid(ThirdSectionLeds, ThirdSectionNumLeds, n);
     fill_solid(FourthSectionLeds, FourthSectionNumLeds, n);
     LEDS[0].showLeds(255);
     LEDS[1].showLeds(255);
     LEDS[2].showLeds(255);
     LEDS[3].showLeds(255);
    
    }

    
  if (strcmp(topic, "room1/zone4/color") == 0) {
      
    long n = strtoul(messageTemp, NULL, 16);
      
///Stop all tasks for 4th zone
    if(flagZone4Mode1 != 0){
      pauseTasks(4, 1);}
    if(flagZone4Mode2 != 0){
      pauseTasks(4, 2);}
    if(flagZone4Mode3 != 0){
      pauseTasks(4, 3);}
    if(flagZone4Mode4 != 0){
      pauseTasks(4, 4);}
    if(flagZone4Mode5 != 0){
      pauseTasks(4, 5);}
    if(flagZone4Mode6 != 0){
      pauseTasks(4, 6);}
         
    fill_solid(FourthSectionLeds, FourthSectionNumLeds, n);
    LEDS[3].showLeds();
    }

    
  if (strcmp(topic, "room1/zone3/color") == 0) {
    long n = strtoul(messageTemp, NULL, 16);

///Stop all tasks for 3rd zone
    if(flagZone3Mode1 != 0){
      pauseTasks(3, 1);}
    if(flagZone3Mode2 != 0){
      pauseTasks(3, 2);}
    if(flagZone3Mode3 != 0){
      pauseTasks(3, 3);}
    if(flagZone3Mode4 != 0){
      pauseTasks(3, 4);}
    if(flagZone3Mode5 != 0){
      pauseTasks(3, 5);}
    if(flagZone3Mode6 != 0){
      pauseTasks(3, 6);}
    
    fill_solid(ThirdSectionLeds, ThirdSectionNumLeds, n);
    LEDS[2].showLeds();
    }

    
  if (strcmp(topic, "room1/zone2/color") == 0) {
    long n = strtoul(messageTemp, NULL, 16);
    
///Stop all tasks for 2nd zone
    if(flagZone2Mode1 != 0){  
      pauseTasks(2, 1);}
    if(flagZone2Mode2 != 0){  
      pauseTasks(2, 2);}
    if(flagZone2Mode3 != 0){  
      pauseTasks(2, 3);}
    if(flagZone2Mode4 != 0){  
      pauseTasks(2, 4);}
    if(flagZone2Mode5 != 0){  
      pauseTasks(2, 5);}
    if(flagZone2Mode6 != 0){  
      pauseTasks(2, 6);}
        
    fill_solid(SecondSectionLeds, SecondSectionNumLeds, n);
    LEDS[1].showLeds(255);
    }

    
  if (strcmp(topic, "room1/zone1/color") == 0) {
    n = strtoul(messageTemp, NULL, 16);

///Stop all tasks for 1st zone

    if(flagZone1Mode1 != 0){  
      pauseTasks(1, 1);}
    if(flagZone1Mode2 != 0){  
      pauseTasks(1, 2);}
    if(flagZone1Mode3 != 0){  
      pauseTasks(1, 3);}
    if(flagZone1Mode4 != 0){  
      pauseTasks(1, 4);}
    if(flagZone1Mode5 != 0){  
      pauseTasks(1, 5);}
    if(flagZone1Mode6 != 0){  
      pauseTasks(1, 6);}
     
    fill_solid(FirstSectionLeds, FirstSectionNumLeds, n);
    LEDS[0].showLeds(255);
    }

    
  if (strcmp(topic, "room1/zone1") == 0) {
    long i = strtoul(messageTemp, &end, 10);
    if (i == 1){
      FastLED.clear();  // clear all pixel data
      FastLED.show();
      vTaskDelay(10);
///Stop all tasks except 1st task for 1st zone 
      Serial.println("zone1");    
      
      pauseTasks(1, 2);
      
      pauseTasks(1, 3);
 
      pauseTasks(1, 4);

      pauseTasks(1, 5);

      pauseTasks(1, 6);

      resumeTasks(1, 1);
    
      }
    else if(i == 2){

///Stop all tasks except 2nd task for 1st zone 
      FastLED.clear();  // clear all pixel data
      FastLED.show();
      vTaskDelay(10);
  
      pauseTasks(1, 1);

      pauseTasks(1, 3);

      pauseTasks(1, 4);

      pauseTasks(1, 5);

      pauseTasks(1, 6);

      resumeTasks(1, 2);
    }
    else if(i == 3){  
    
    ///Stop all tasks except 3rd task for 1st zone 
      FastLED.clear();  // clear all pixel data
      FastLED.show();
      vTaskDelay(10);
      pauseTasks(1, 1);

      pauseTasks(1, 2);

      pauseTasks(1, 4);

      pauseTasks(1, 5);

      pauseTasks(1, 6);

      resumeTasks(1, 3);  
    }
    else if(i == 4){
          
    ///Stop all tasks except 4th task for 1st zone 
      FastLED.clear();  // clear all pixel data
      FastLED.show();
      vTaskDelay(10);
      pauseTasks(1, 1);

      pauseTasks(1, 2);

      pauseTasks(1, 3);

      pauseTasks(1, 5);
 
      pauseTasks(1, 6);

      resumeTasks(1, 4);
    
      }
    else if(i == 5){

///Stop all tasks except 5th task for 1st zone 
      FastLED.clear();  // clear all pixel data
      FastLED.show();
      vTaskDelay(10);
      pauseTasks(1, 1);

      pauseTasks(1, 2);

      pauseTasks(1, 3);
 
      pauseTasks(1, 4);
  
      pauseTasks(1, 6);

      resumeTasks(1, 5); 
      }
    else if(i == 6){
      FastLED.clear();  // clear all pixel data
      FastLED.show();
      vTaskDelay(10);
///Stop all tasks except 6th task for 1st zone 

      pauseTasks(1, 1);

      pauseTasks(1, 2);

      pauseTasks(1, 3);

      pauseTasks(1, 4);

      pauseTasks(1, 5);
      
      resumeTasks(1, 6);
      }
    
    
  }
    
  if (strcmp(topic, "room1/zone2") == 0) {
    long i = strtoul(messageTemp, &end, 10);
    if (i == 1){
      FastLED.clear();  // clear all pixel data
      FastLED.show();
      vTaskDelay(10);
 ///Stop all tasks except 1st for 2nd zone
      Serial.println("zone2");    
      
      pauseTasks(2, 2);
      
      pauseTasks(2, 3);
      
      pauseTasks(2, 4);
      
      pauseTasks(2, 5);
       
      pauseTasks(2, 6);
      resumeTasks(2, 1);  
    }
      
    else if(i == 2){
      FastLED.clear();  // clear all pixel data
      FastLED.show();
      vTaskDelay(10);
///Stop all tasks except 2nd for 2nd zone
      
      pauseTasks(2, 1);
      
      pauseTasks(2, 3);
      
      pauseTasks(2, 4);
      
      pauseTasks(2, 5);
      
      pauseTasks(2, 6);
      
      resumeTasks(2, 2);      
    
      }
    else if(i == 3){  
      FastLED.clear();  // clear all pixel data
      FastLED.show();
      vTaskDelay(10);
///Stop all tasks except 3rd for 2nd zone
      
      pauseTasks(2, 1);
      
      pauseTasks(2, 2);
      
      pauseTasks(2, 4);
      
      pauseTasks(2, 5);
      
      pauseTasks(2, 6);
      resumeTasks(2, 3);
    }
    else if(i == 4){
      FastLED.clear();  // clear all pixel data
      FastLED.show();
      vTaskDelay(10);
      ///Stop all tasks except 4th for 2nd zone
      
      pauseTasks(2, 1);
      
      pauseTasks(2, 2);
      
      pauseTasks(2, 3);
      
      pauseTasks(2, 5);
       
      pauseTasks(2, 6);
      resumeTasks(2, 4);
        
      }
    else if(i == 5){
      FastLED.clear();  // clear all pixel data
      FastLED.show();
      vTaskDelay(10);
///Stop all tasks except 5th for 2nd zone
      
      pauseTasks(2, 1);
      
      pauseTasks(2, 2);
      
      pauseTasks(2, 3);
      
      pauseTasks(2, 4);
        
      pauseTasks(2, 6);
      
      resumeTasks(2, 5);
      }
    else if(i == 6){
      FastLED.clear();  // clear all pixel data
      FastLED.show();
      vTaskDelay(10);      
///Stop all tasks except 6th for 2nd zone
       
      pauseTasks(2, 1);
      
      pauseTasks(2, 2);
      
      pauseTasks(2, 3);
      
      pauseTasks(2, 4);
      
      pauseTasks(2, 5);
      
      resumeTasks(2, 6);
      }
    }

    
  if (strcmp(topic, "room1/zone3") == 0) {
    long i = strtoul(messageTemp, &end, 10);
    if (i == 1){
      FastLED.clear();  // clear all pixel data
      FastLED.show();
      vTaskDelay(10);     
///Stop all tasks except 1st for 3rd zone
      pauseTasks(3, 2);
      pauseTasks(3, 3);
      pauseTasks(3, 4);
      pauseTasks(3, 5);
      pauseTasks(3, 6);
      resumeTasks(3, 1); 
    }
    else if(i == 2){
      FastLED.clear();  // clear all pixel data
      FastLED.show();
      vTaskDelay(10);
      ///Stop all tasks except 2nd for 3rd zone
      pauseTasks(3, 1);
      pauseTasks(3, 3);
      pauseTasks(3, 4);
      pauseTasks(3, 5);
      pauseTasks(3, 6);
      resumeTasks(3, 2); 
      }
    else if(i == 3){  
      FastLED.clear();  // clear all pixel data
      FastLED.show();
      vTaskDelay(10);    
///Stop all tasks except 3rd for 3rd zone
      pauseTasks(3, 1);
      pauseTasks(3, 2);
      pauseTasks(3, 4);
      pauseTasks(3, 5);
      pauseTasks(3, 6);
      resumeTasks(3, 3); 
    }
    else if(i == 4){
      FastLED.clear();  // clear all pixel data
      FastLED.show();
      vTaskDelay(10);      
///Stop all tasks except 4th for 3rd zone
      pauseTasks(3, 1);
      pauseTasks(3, 2);
      pauseTasks(3, 3);
      pauseTasks(3, 5);
      pauseTasks(3, 6);
      resumeTasks(3, 4); 
    }
      
    else if(i == 5){
      FastLED.clear();  // clear all pixel data
      FastLED.show();
      vTaskDelay(10);    
///Stop all tasks except 5th for 3rd zone
      pauseTasks(3, 1);
      pauseTasks(3, 2);
      pauseTasks(3, 3);
      pauseTasks(3, 4);
      pauseTasks(3, 6);
      resumeTasks(3, 5); 
    
      }
    else if(i == 6){
      FastLED.clear();  // clear all pixel data
      FastLED.show();
      vTaskDelay(10);    
///Stop all tasks except 6th for 3rd zone

      pauseTasks(3, 1);
      pauseTasks(3, 2);
      pauseTasks(3, 3);
      pauseTasks(3, 4);
      pauseTasks(3, 5);
      resumeTasks(3, 6);  
      }
    }

    
  if (strcmp(topic, "room1/zone4") == 0) {
    long i = strtoul(messageTemp, &end, 10);
    if (i == 1){
      FastLED.clear();  // clear all pixel data
      FastLED.show();
      vTaskDelay(10); 
///Stop all tasks except 1st for 4th zone

      pauseTasks(4, 3);
      pauseTasks(4, 4);
      pauseTasks(4, 5);
      pauseTasks(4, 6);
      pauseTasks(4, 2);
      resumeTasks(4, 1);   
      }
    else if(i == 2){
      FastLED.clear();  // clear all pixel data
      FastLED.show();
      vTaskDelay(10); 
///Stop all tasks except 2nd for 4th zone

      pauseTasks(4, 3);
      pauseTasks(4, 4);
      pauseTasks(4, 5);
      pauseTasks(4, 6);
      pauseTasks(4, 1);
      resumeTasks(4, 2);  
      }
    else if(i == 3){  
      FastLED.clear();  // clear all pixel data
      FastLED.show();
      vTaskDelay(10); 
///Stop all tasks except 3rd for 4th zone
      
      pauseTasks(4, 2);
      pauseTasks(4, 4);
      pauseTasks(4, 5);
      pauseTasks(4, 6);
      pauseTasks(4, 1);
      resumeTasks(4, 3); 
    }
    else if(i == 4){
      FastLED.clear();  // clear all pixel data
      FastLED.show();
      vTaskDelay(10); 
///Stop all tasks except 4th for 4th zone

      pauseTasks(4, 2);
      pauseTasks(4, 3);
      pauseTasks(4, 5);
      pauseTasks(4, 6);
      pauseTasks(4, 1);
      resumeTasks(4, 4); 
      }
    else if(i == 5){
      FastLED.clear();  // clear all pixel data
      FastLED.show();
      vTaskDelay(10); 
      
///Stop all tasks except 5th for 4th zone

      pauseTasks(4, 2);
      pauseTasks(4, 3);
      pauseTasks(4, 4);
      pauseTasks(4, 6);
      pauseTasks(4, 1);
      resumeTasks(4, 5);   
      }
    else if(i == 6){
      FastLED.clear();  // clear all pixel data
      FastLED.show();
      vTaskDelay(10); 
            
///Stop all tasks except 6th for 4th zone

      pauseTasks(4, 2);
      pauseTasks(4, 3);
      pauseTasks(4, 4);
      pauseTasks(4, 5);
      pauseTasks(4, 1);
      resumeTasks(4, 6);   
    }
  }

    
  if (strcmp(topic, "room1/allroom") == 0) {
    long i = strtoul(messageTemp, &end, 10);
    if (i == 1){
      FastLED.clear();  // clear all pixel data
      FastLED.show();
      vTaskDelay(10); 
      
///Stop all tasks except 1st task for 1st zone 
      pauseTasks(1, 2);
      pauseTasks(1, 3);
      pauseTasks(1, 4);
      pauseTasks(1, 5);
      pauseTasks(1, 6);
      resumeTasks(1, 1);   

///Stop all tasks except 1st for 2nd zone
 
      pauseTasks(2, 2);
      pauseTasks(2, 3);
      pauseTasks(2, 4);
      pauseTasks(2, 5);
      pauseTasks(2, 6);
      resumeTasks(2, 1);   
        
///Stop all tasks except 1st for 3rd zone
      pauseTasks(3, 2);
      pauseTasks(3, 3);
      pauseTasks(3, 4);
      pauseTasks(3, 5);
      pauseTasks(3, 6);
      resumeTasks(3, 1);   
        
///Stop all tasks except 1st for 4th zone

      pauseTasks(4, 2);
      pauseTasks(4, 3);
      pauseTasks(4, 4);
      pauseTasks(4, 5);
      pauseTasks(4, 6);
      resumeTasks(4, 1);      
      
      }
    else if(i == 2){
      FastLED.clear();  // clear all pixel data
      FastLED.show();
      vTaskDelay(10); 

///Stop all tasks except 2nd task for 1st zone 

      pauseTasks(1, 1);
      pauseTasks(1, 3);
      pauseTasks(1, 4);
      pauseTasks(1, 5);
      pauseTasks(1, 6);
      resumeTasks(1, 2);

///Stop all tasks except 2nd for 2nd zone
 
      pauseTasks(2, 1);
      pauseTasks(2, 3);
      pauseTasks(2, 4);
      pauseTasks(2, 5);
      pauseTasks(2, 6);
      resumeTasks(2, 2);
      
///Stop all tasks except 2nd for 3rd zone
      pauseTasks(3, 1);
      pauseTasks(3, 3);
      pauseTasks(3, 4);
      pauseTasks(3, 5);
      pauseTasks(3, 6);
      resumeTasks(3, 2);
        
///Stop all tasks except 2nd for 4th zone

      pauseTasks(4, 1);
      pauseTasks(4, 3);
      pauseTasks(4, 4);
      pauseTasks(4, 5);
      pauseTasks(4, 6);
      resumeTasks(4, 2);
            
      }
    else if(i == 3){
      FastLED.clear();  // clear all pixel data
      FastLED.show();
      vTaskDelay(10); 

///Stop all tasks except 3rd task for 1st zone 

      pauseTasks(1, 1);
      pauseTasks(1, 2);
      pauseTasks(1, 4);
      pauseTasks(1, 5);
      pauseTasks(1, 6);
      resumeTasks(1, 3);

///Stop all tasks except 3rd for 2nd zone
 
      pauseTasks(2, 1);
      pauseTasks(2, 2);
      pauseTasks(2, 4);
      pauseTasks(2, 5);
      pauseTasks(2, 6);
      resumeTasks(2, 3);

///Stop all tasks except 3rd for 3rd zone
      pauseTasks(3, 1);
      pauseTasks(3, 2);
      pauseTasks(3, 4);
      pauseTasks(3, 5);
      pauseTasks(3, 6);
      resumeTasks(3, 3);   
    
///Stop all tasks except 3rd for 4th zone


      pauseTasks(4, 1);
      pauseTasks(4, 2);
      pauseTasks(4, 4);
      pauseTasks(4, 5);
      pauseTasks(4, 6);
      resumeTasks(4, 3);          
    }
    else if(i == 4){
      FastLED.clear();  // clear all pixel data
      FastLED.show();
      vTaskDelay(10); 
    ///Stop all tasks except 4th task for 1st zone 

      pauseTasks(1, 1);
      pauseTasks(1, 2);
      pauseTasks(1, 3);
      pauseTasks(1, 5);
      pauseTasks(1, 6);
      resumeTasks(1, 4); 
        
///Stop all tasks except 4th for 2nd zone
 
      pauseTasks(2, 1);
      pauseTasks(2, 2);
      pauseTasks(2, 3);
      pauseTasks(2, 5);
      pauseTasks(2, 6);
      resumeTasks(2, 4); 

///Stop all tasks except 4th for 3rd zone
      pauseTasks(3, 1);
      pauseTasks(3, 2);
      pauseTasks(3, 3);
      pauseTasks(3, 5);
      pauseTasks(3, 6);
      resumeTasks(3, 4); 
      
      ///Stop all tasks except 4th for 4th zone

      pauseTasks(4, 1);
      pauseTasks(4, 2);
      pauseTasks(4, 3);
      pauseTasks(4, 5);
      pauseTasks(4, 6);
      resumeTasks(4, 4); 
      }
    else if(i == 5){
      FastLED.clear();  // clear all pixel data
      FastLED.show();
      vTaskDelay(10); 
      
///Stop all tasks except 5th task for 1st zone 
      pauseTasks(1, 1);
      pauseTasks(1, 2);
      pauseTasks(1, 3);
      pauseTasks(1, 4);
      pauseTasks(1, 6);
      resumeTasks(1,5);
///Stop all tasks except 5th for 2nd zone
      
      pauseTasks(2, 1);
      pauseTasks(2, 2);
      pauseTasks(2, 3);
      pauseTasks(2, 4);
      pauseTasks(2, 6);
      resumeTasks(2,5);
      

///Stop all tasks except 5th for 3rd zone
      
      pauseTasks(3, 1);
      pauseTasks(3, 2);
      pauseTasks(3, 3);
      pauseTasks(3, 4);
      pauseTasks(3, 6);
      resumeTasks(3,5);

///Stop all tasks except 5th for 4th zone

      pauseTasks(4, 1);
      pauseTasks(4, 2);
      pauseTasks(4, 3);
      pauseTasks(4, 4);
      pauseTasks(4, 6);
      resumeTasks(4,5);
      }
    else if(i == 6){
      
      FastLED.clear();  // clear all pixel data
      FastLED.show();
      vTaskDelay(10); 
      
///Stop all tasks except 5th task for 1st zone 
      pauseTasks(1, 1);
      pauseTasks(1, 2);
      pauseTasks(1, 3);
      pauseTasks(1, 4);
      pauseTasks(1, 5);
      resumeTasks(1, 6);
///Stop all tasks except 5th for 2nd zone
      
      pauseTasks(2, 1);
      pauseTasks(2, 2);
      pauseTasks(2, 3);
      pauseTasks(2, 4);
      pauseTasks(2, 5);
      resumeTasks(2, 6);
      

///Stop all tasks except 5th for 3rd zone
      
      pauseTasks(3, 1);
      pauseTasks(3, 2);
      pauseTasks(3, 3);
      pauseTasks(3, 4);
      pauseTasks(3, 5);
      resumeTasks(3, 6);

///Stop all tasks except 5th for 4th zone

      pauseTasks(4, 1);
      pauseTasks(4, 2);
      pauseTasks(4, 3);
      pauseTasks(4, 4);
      pauseTasks(4, 5);
      resumeTasks(4, 6);
      
      }
    }
}



void pauseTasks(int zone, int Mode){
  if (zone == 1){
    if(Mode == 1){
      vTaskSuspend( TaskZone1Mode1 );
      flagZone1Mode1 = 0;}
    else if(Mode == 2){
      vTaskSuspend( TaskZone1Mode2 );
      flagZone1Mode2=0;}
    else if(Mode == 3){
      vTaskSuspend( TaskZone1Mode3 );
      flagZone1Mode3=0;}
    else if(Mode == 4){
      vTaskSuspend( TaskZone1Mode4 );
      flagZone1Mode4=0;}
    else if(Mode == 5){
      vTaskSuspend( TaskZone1Mode5 );
      flagZone1Mode5=0;}
    else if(Mode == 6){
      vTaskSuspend( TaskZone1Mode6 );
      flagZone1Mode6=0;}
  }    
  if (zone == 2){
    ///Stop all tasks for 2nd zone
    if(Mode == 1){
      vTaskSuspend( TaskZone2Mode1 );
      flagZone2Mode1 = 0;}
    else if(Mode == 2){
      vTaskSuspend( TaskZone2Mode2 );
      flagZone2Mode2=0;}
    else if(Mode == 3){
      vTaskSuspend( TaskZone2Mode3 );
      flagZone2Mode3=0;}
    else if(Mode == 4){
      vTaskSuspend( TaskZone2Mode4 );
      flagZone2Mode4=0;}
    else if(Mode == 5){
      vTaskSuspend( TaskZone2Mode5 );
      flagZone2Mode5=0;}
    else if(Mode == 6){
      vTaskSuspend( TaskZone2Mode6 );
      flagZone2Mode6=0;}
  }
  if (zone == 3){
    ///Stop all tasks for 3rd zone
    if(Mode == 1){
      vTaskSuspend( TaskZone3Mode1 );
      flagZone3Mode1 = 0;}
    if(Mode == 2){
      vTaskSuspend( TaskZone3Mode2 );
      flagZone3Mode2=0;}
    if(Mode == 3){
      vTaskSuspend( TaskZone3Mode3 );
      flagZone3Mode3=0;}
    if(Mode == 4){
      vTaskSuspend( TaskZone3Mode4 );
      flagZone3Mode4=0;}
    if(Mode == 5){
      vTaskSuspend( TaskZone3Mode5 );
      flagZone3Mode5=0;}
    if(Mode == 6){
      vTaskSuspend( TaskZone3Mode6 );
      flagZone3Mode6=0;}      
  }
  if (zone == 4){
    if(Mode == 1){
      vTaskSuspend( TaskZone4Mode1 );
      flagZone4Mode1 = 0;}
    if(Mode == 2){
      vTaskSuspend( TaskZone4Mode2 );
      flagZone4Mode2=0;}
    if(Mode == 3){
      vTaskSuspend( TaskZone4Mode3 );
      flagZone4Mode3=0;}
    if(Mode == 4){
      vTaskSuspend( TaskZone4Mode4 );
      flagZone4Mode4=0;}
    if(Mode == 5){
      vTaskSuspend( TaskZone4Mode5 );
      flagZone4Mode5=0;}
    if(Mode == 6){
      vTaskSuspend( TaskZone4Mode6 );
      flagZone4Mode6=0;} 
    }  
}


void resumeTasks(int zone, int Mode){
  if (zone == 1){
    if(Mode == 1){
      vTaskResume( TaskZone1Mode1 );
      flagZone1Mode1 = 1;}
    else if(Mode == 2){
      vTaskResume( TaskZone1Mode2 );
      flagZone1Mode2=1;}
    else if(Mode == 3){
      vTaskResume( TaskZone1Mode3 );
      flagZone1Mode3=1;}
    else if(Mode == 4){
      vTaskResume( TaskZone1Mode4 );
      flagZone1Mode4=1;}
    else if(Mode == 5){
      vTaskResume( TaskZone1Mode5 );
      flagZone1Mode5=1;}
    else if(Mode == 6){
      vTaskResume( TaskZone1Mode6 );
      flagZone1Mode6=1;}
  }    
  if (zone == 2){
    ///Stop all tasks for 2nd zone
    if(Mode == 1){
      vTaskResume( TaskZone2Mode1 );
      flagZone2Mode1 = 1;}
    else if(Mode == 2){
      vTaskResume( TaskZone2Mode2 );
      flagZone2Mode2=1;}
    else if(Mode == 3){
      vTaskResume( TaskZone2Mode3 );
      flagZone2Mode3=1;}
    else if(Mode == 4){
      vTaskResume( TaskZone2Mode4 );
      flagZone2Mode4=1;}
    else if(Mode == 5){
      vTaskResume( TaskZone2Mode5 );
      flagZone2Mode5=1;}
    else if(Mode == 6){
      vTaskResume( TaskZone2Mode6 );
      flagZone2Mode6=1;}
  }
  if (zone == 3){
    ///Stop all tasks for 3rd zone
    if(Mode == 1){
      vTaskResume( TaskZone3Mode1 );
      flagZone3Mode1 = 1;}
    if(Mode == 2){
      vTaskResume( TaskZone3Mode2 );
      flagZone3Mode2=1;}
    if(Mode == 3){
      vTaskResume( TaskZone3Mode3 );
      flagZone3Mode3=1;}
    if(Mode == 4){
      vTaskResume( TaskZone3Mode4 );
      flagZone3Mode4=1;}
    if(Mode == 5){
      vTaskResume( TaskZone3Mode5 );
      flagZone3Mode5=1;}
    if(Mode == 6){
      vTaskResume( TaskZone3Mode6 );
      flagZone3Mode6=1;}      
  }
  if (zone == 4){
    if(Mode == 1){
      vTaskResume( TaskZone4Mode1 );
      flagZone4Mode1 = 1;}
    if(Mode == 2){
      vTaskResume( TaskZone4Mode2 );
      flagZone4Mode2=1;}
    if(Mode == 3){
      vTaskResume( TaskZone4Mode3 );
      flagZone4Mode3=1;}
    if(Mode == 4){
      vTaskResume( TaskZone4Mode4 );
      flagZone4Mode4=1;}
    if(Mode == 5){
      vTaskResume( TaskZone4Mode5 );
      flagZone4Mode5=1;}
    if(Mode == 6){
      vTaskResume( TaskZone4Mode6 );
      flagZone4Mode6=1;} 
    }  
}

//////////////MODE 1 ZONE 1 rainbowCycle////////////////////////////////////////////////////////////////
void rainbowCycle(void * pvParameters ) {
  TIMERG0.wdt_wprotect=TIMG_WDT_WKEY_VALUE;
  TIMERG0.wdt_feed=1;
  TIMERG0.wdt_wprotect=0;
  int NUM_LEDS = (int) pvParameters;
  byte *c;
  uint16_t i, j;
  for(;;){
    vTaskDelay( 100 );
    for(j=0; j<256*5; j++) {
      vTaskDelay( 10 );// 5 cycles of all colors on wheel
      for(i=0; i< NUM_LEDS; i++) {
        c=Wheel(((i * 256 / NUM_LEDS) + j) & 255);
        setPixel(i, *c, *(c+1), *(c+2));
      }
      LEDS[0].showLeds(250);
      vTaskDelay( 10 );
    }
  }
}

// Set a LED color (not yet visible)
void setPixel(int Pixel, byte red, byte green, byte blue) {
 #ifdef ADAFRUIT_NEOPIXEL_H 
   // NeoPixel
   strip.setPixelColor(Pixel, strip.Color(red, green, blue));
 #endif
 #ifndef ADAFRUIT_NEOPIXEL_H 
   // FastLED
   FirstSectionLeds[Pixel].r = red;
   FirstSectionLeds[Pixel].g = green;
   FirstSectionLeds[Pixel].b = blue;
 #endif
};


//////////////END MODE 1 ZONE 1 rainbowCycle////////////////////////////////////////////////////////////////



//////////////MODE 1 ZONE 2 rainbowCycle////////////////////////////////////////////////////////////////
void rainbowCycle2(void * pvParameters ) {
  TIMERG0.wdt_wprotect=TIMG_WDT_WKEY_VALUE;
  TIMERG0.wdt_feed=1;
  TIMERG0.wdt_wprotect=0;
  int NUM_LEDS = (int) pvParameters;
  byte *c;
  uint16_t i, j;
  for(;;){
    vTaskDelay( 100 );
    for(j=0; j<256*5; j++) {
      vTaskDelay( 10 );// 5 cycles of all colors on wheel
      for(i=0; i< NUM_LEDS; i++) {
        c=Wheel(((i * 256 / NUM_LEDS) + j) & 255);
        setPixel2(i, *c, *(c+1), *(c+2));
      }
      LEDS[1].showLeds(250);
      vTaskDelay( 10 );
    }
  }
}

void setPixel2(int Pixel, byte red, byte green, byte blue) {
 #ifdef ADAFRUIT_NEOPIXEL_H 
   // NeoPixel
   strip.setPixelColor(Pixel, strip.Color(red, green, blue));
 #endif
 #ifndef ADAFRUIT_NEOPIXEL_H 
   // FastLED
   SecondSectionLeds[Pixel].r = red;
   SecondSectionLeds[Pixel].g = green;
   SecondSectionLeds[Pixel].b = blue;
 #endif
}


//////////////END MODE 1 ZONE 2 rainbowCycle////////////////////////////////////////////////////////////////


////////////// MODE 1 ZONE 3 rainbowCycle////////////////////////////////////////////////////////////////
void rainbowCycle3(void * pvParameters ) {
 TIMERG0.wdt_wprotect=TIMG_WDT_WKEY_VALUE;
TIMERG0.wdt_feed=1;
TIMERG0.wdt_wprotect=0;
  int NUM_LEDS = (int) pvParameters;
  byte *c;
  uint16_t i, j;
  for(;;){
    vTaskDelay( 100 );
    for(j=0; j<256*5; j++) {
      vTaskDelay( 10 );// 5 cycles of all colors on wheel
      for(i=0; i< NUM_LEDS; i++) {
        c=Wheel(((i * 256 / NUM_LEDS) + j) & 255);
        setPixel3(i, *c, *(c+1), *(c+2));
      }
      LEDS[2].showLeds(250);
      vTaskDelay( 10 );
    }
  }
}


//////////////END MODE 1 ZONE 3 rainbowCycle////////////////////////////////////////////////////////////////




//////////////END MODE 1 ZONE 4 rainbowCycle////////////////////////////////////////////////////////////////
void rainbowCycle4(void * pvParameters ) {
  TIMERG0.wdt_wprotect=TIMG_WDT_WKEY_VALUE;
  TIMERG0.wdt_feed=1;
  TIMERG0.wdt_wprotect=0;
  int NUM_LEDS = (int) pvParameters;
  byte *c;
  uint16_t i, j;
  for(;;){
    vTaskDelay( 100 );
    for(j=0; j<256*5; j++) {
      vTaskDelay( 10 );// 5 cycles of all colors on wheel
      for(i=0; i< NUM_LEDS; i++) {
        c=Wheel(((i * 256 / NUM_LEDS) + j) & 255);
        setPixel2(i, *c, *(c+1), *(c+2));
      }
      LEDS[3].showLeds(250);
      vTaskDelay( 10 );
    }
  }
}

void setPixel4(int Pixel, byte red, byte green, byte blue) {
 #ifdef ADAFRUIT_NEOPIXEL_H 
   // NeoPixel
   strip.setPixelColor(Pixel, strip.Color(red, green, blue));
 #endif
 #ifndef ADAFRUIT_NEOPIXEL_H 
   // FastLED
   FourthSectionLeds[Pixel].r = red;
   FourthSectionLeds[Pixel].g = green;
   FourthSectionLeds[Pixel].b = blue;
 #endif
}
//////////////END MODE 1 ZONE 4 rainbowCycle////////////////////////////////////////////////////////////////



byte * Wheel(byte WheelPos) {
  static byte c[3];
  
  if(WheelPos < 85) {
   c[0]=WheelPos * 3;
   c[1]=255 - WheelPos * 3;
   c[2]=0;
  } else if(WheelPos < 170) {
   WheelPos -= 85;
   c[0]=255 - WheelPos * 3;
   c[1]=0;
   c[2]=WheelPos * 3;
  } else {
   WheelPos -= 170;
   c[0]=0;
   c[1]=WheelPos * 3;
   c[2]=255 - WheelPos * 3;
  }

  return c;
}



void fadeall() { for(int i = 0; i < FirstSectionNumLeds; i++) { FirstSectionLeds[FirstSectionNumLeds].nscale8(250); } }

void fadeal2() { for(int i = 0; i < SecondSectionNumLeds; i++) { SecondSectionLeds[SecondSectionNumLeds].nscale8(250); } }

void fadeal3() { for(int i = 0; i < ThirdSectionNumLeds; i++) { ThirdSectionLeds[ThirdSectionNumLeds].nscale8(250); } }

void fadeal4() { for(int i = 0; i < FourthSectionNumLeds; i++) { FourthSectionLeds[FourthSectionNumLeds].nscale8(250); } }
  


/////////////MODE 2 ZONE 1 Cyclon////////////////////////////////////////////////////////////////


void CyclonZone1(void * pvParameters){
  TIMERG0.wdt_wprotect=TIMG_WDT_WKEY_VALUE;
  TIMERG0.wdt_feed=1;
  TIMERG0.wdt_wprotect=0;
  vTaskDelay( 100 );

  static uint8_t hue = 0;
  for(;;){
    vTaskDelay( 100 );
    // First slide the led in one direction
    for(int i = 0; i < FirstSectionNumLeds; i++) {
      // Set the i'th led to red 
      FirstSectionLeds[i] = CHSV(hue++, 255, 255);
      // Show the leds
      LEDS[0].showLeds(); 
      // now that we've shown the leds, reset the i'th led to black
      // leds[i] = CRGB::Black;
      fadeall();
      // Wait a little bit before we loop around and do it again
      vTaskDelay( 10 );
      //delay(2);
    }


  // Now go in the other direction.  
    for(int i = (FirstSectionNumLeds)-1; i >= 0; i--) {
      // Set the i'th led to red 
      FirstSectionLeds[i] = CHSV(hue++, 255, 255);
      // Show the leds
      LEDS[0].showLeds();
      // now that we've shown the leds, reset the i'th led to black
      // leds[i] = CRGB::Black;
      fadeall();
      // Wait a little bit before we loop around and do it again
      vTaskDelay( 10 );
      //delay(2);
    }

 }
}
//////////////END MODE 2 ZONE 1 Cyclon////////////////////////////////////////////////////////////////




//////////////MODE 2 ZONE 2 Cyclon////////////////////////////////////////////////////////////////
void CyclonZone2(void * pvParameters){
  TIMERG0.wdt_wprotect=TIMG_WDT_WKEY_VALUE;
  TIMERG0.wdt_feed=1;
  TIMERG0.wdt_wprotect=0;


  static uint8_t hue = 0;
  for(;;){
    vTaskDelay( 100 );
    // First slide the led in one direction
    for(int i = 0; i < FirstSectionNumLeds; i++) {
      // Set the i'th led to red 
      SecondSectionLeds[i] = CHSV(hue++, 255, 255);
      // Show the leds
      LEDS[1].showLeds();
      // now that we've shown the leds, reset the i'th led to black
      // leds[i] = CRGB::Black;
      fadeal2();
      // Wait a little bit before we loop around and do it again
      vTaskDelay( 10 );
      //delay(2);
    }


    // Now go in the other direction.  
    for(int i = (FirstSectionNumLeds)-1; i >= 0; i--) {
      // Set the i'th led to red 
      SecondSectionLeds[i] = CHSV(hue++, 255, 255);
      // Show the leds
      LEDS[1].showLeds();
      // now that we've shown the leds, reset the i'th led to black
      // leds[i] = CRGB::Black;
      fadeal2();
      // Wait a little bit before we loop around and do it again
      vTaskDelay( 10 );
      //delay(2);
  }

 }
}
//////////////END MODE 2 ZONE 2 Cyclon////////////////////////////////////////////////////////////////


//////////////MODE 2 ZONE 3 Cyclon////////////////////////////////////////////////////////////////
void CyclonZone3(void * pvParameters){

  TIMERG0.wdt_wprotect=TIMG_WDT_WKEY_VALUE;
  TIMERG0.wdt_feed=1;
  TIMERG0.wdt_wprotect=0;
  static uint8_t hue = 0;
  for(;;){
    vTaskDelay( 100 );
    // First slide the led in one direction
    for(int i = 0; i < FirstSectionNumLeds; i++) {
      // Set the i'th led to red 
      ThirdSectionLeds[i] = CHSV(hue++, 255, 255);
      // Show the leds
      LEDS[2].showLeds(); 
      // now that we've shown the leds, reset the i'th led to black
      // leds[i] = CRGB::Black;
      fadeal3();
      // Wait a little bit before we loop around and do it again
      vTaskDelay( 10 );
      //delay(2);
    }


  // Now go in the other direction.  
    for(int i = (FirstSectionNumLeds)-1; i >= 0; i--) {
      // Set the i'th led to red 
      ThirdSectionLeds[i] = CHSV(hue++, 255, 255);
      // Show the leds
      LEDS[2].showLeds();
      // now that we've shown the leds, reset the i'th led to black
      // leds[i] = CRGB::Black;
      fadeal3();
      // Wait a little bit before we loop around and do it again
      vTaskDelay( 10 );
      //delay(2);
    }

  }
}

//////////////END MODE 2 ZONE 3 Cyclon////////////////////////////////////////////////////////////////


//////////////MODE 2 ZONE 4 Cyclon////////////////////////////////////////////////////////////////

void CyclonZone4(void * pvParameters){

  TIMERG0.wdt_wprotect=TIMG_WDT_WKEY_VALUE;
  TIMERG0.wdt_feed=1;
  TIMERG0.wdt_wprotect=0;

  static uint8_t hue = 0;
  for(;;){
    vTaskDelay( 100 );
    // First slide the led in one direction
    for(int i = 0; i < FirstSectionNumLeds; i++) {
      // Set the i'th led to red 
      FourthSectionLeds[i] = CHSV(hue++, 255, 255);
      // Show the leds
      LEDS[3].showLeds();
      // now that we've shown the leds, reset the i'th led to black
      // leds[i] = CRGB::Black;
      fadeal4();
      // Wait a little bit before we loop around and do it again
      vTaskDelay( 10 );
      //delay(2);
    }


    // Now go in the other direction.  
    for(int i = (FirstSectionNumLeds)-1; i >= 0; i--) {
      // Set the i'th led to red 
      FourthSectionLeds[i] = CHSV(hue++, 255, 255);
      // Show the leds
      LEDS[3].showLeds();
      // now that we've shown the leds, reset the i'th led to black
      // leds[i] = CRGB::Black;
      fadeal4();
      // Wait a little bit before we loop around and do it again
      vTaskDelay( 10 );
      //delay(2);
    }

  }
}
//////////////END MODE 2 ZONE 4 Cyclon////////////////////////////////////////////////////////////////



////////////// MODE 3 ZONE 1 RGBLOOP////////////////////////////////////////////////////////////////
void RGBLoopZone1(void * pvParameters){
  TIMERG0.wdt_wprotect=TIMG_WDT_WKEY_VALUE;
  TIMERG0.wdt_feed=1;
  TIMERG0.wdt_wprotect=0;


  int i;
  int NUM_LEDS = (int) pvParameters;
  for(;;){
    vTaskDelay( 100 );
    for(int j = 0; j < 3; j++ ) { 
      // Fade IN
      for(int k = 0; k < 256; k++) { 
        vTaskDelay( 10 );
        switch(j) { 
          case 0: setAll(k,0,0, NUM_LEDS); break;
          case 1: setAll(0,k,0, NUM_LEDS); break;
          case 2: setAll(0,0,k, NUM_LEDS); break;
        }
        //delay(2);
        vTaskDelay(2);
        showStrip();
      }
    // Fade OUT
      for(int k = 255; k >= 0; k--) { 
        vTaskDelay( 10 );
        switch(j) { 
          case 0: setAll(k,0,0, NUM_LEDS); break;
          case 1: setAll(0,k,0, NUM_LEDS); break;
          case 2: setAll(0,0,k, NUM_LEDS); break;
        }
        //delay(2);
        vTaskDelay(2);
        showStrip(); 
      }
    
    }
  }
}



void showStrip() {
  #ifdef ADAFRUIT_NEOPIXEL_H 
   // NeoPixel
      strip.show();
  #endif
  #ifndef ADAFRUIT_NEOPIXEL_H
   // FastLED
    LEDS[0].showLeds(250);
  #endif
}

// Set all LEDs to a given color and apply it (visible)
void setAll(byte red, byte green, byte blue, int NUM_LEDS) {
  for(int i = 0; i < NUM_LEDS; i++ ) {
    setPixel(i, red, green, blue); 
  }
  showStrip();
}





////////////// END MODE 3 ZONE 1 RGBLOOP////////////////////////////////////////////////////////////////


////////////// MODE 3 ZONE 2 RGBLOOP////////////////////////////////////////////////////////////////
void RGBLoopZone2(void * pvParameters){
 
  int i;
  int NUM_LEDS = (int) pvParameters;
  for(;;){
    vTaskDelay( 100 );
    for(int j = 0; j < 3; j++ ) { 
      // Fade IN
      for(int k = 0; k < 256; k++) { 
        vTaskDelay( 10 );
        switch(j) { 
          case 0: setAll2(k,0,0, NUM_LEDS); break;
          case 1: setAll2(0,k,0, NUM_LEDS); break;
          case 2: setAll2(0,0,k, NUM_LEDS); break;
        }
        //delay(2);
        vTaskDelay(2);
        showStrip2();
      }
      // Fade OUT
      for(int k = 255; k >= 0; k--) { 
        vTaskDelay( 10 );
        switch(j) { 
          case 0: setAll2(k,0,0, NUM_LEDS); break;
          case 1: setAll2(0,k,0, NUM_LEDS); break;
          case 2: setAll2(0,0,k, NUM_LEDS); break;
        }
        //delay(2);
        vTaskDelay(2);
        showStrip2(); 
      }
    
    }
  }
}

void showStrip2() {
  #ifdef ADAFRUIT_NEOPIXEL_H 
   // NeoPixel
    strip.show();
  #endif
  #ifndef ADAFRUIT_NEOPIXEL_H
     // FastLED
    LEDS[1].showLeds(250);
  #endif
}

void setAll2(byte red, byte green, byte blue, int NUM_LEDS) {
  for(int i = 0; i < NUM_LEDS; i++ ) {
    setPixel2(i, red, green, blue); 
  }
  showStrip2();
}

////////////// END MODE 3 ZONE 2 RGBLOOP////////////////////////////////////////////////////////////////


////////////// MODE 3 ZONE 3 RGBLOOP////////////////////////////////////////////////////////////////
void RGBLoopZone3(void * pvParameters){

  int i;
  int NUM_LEDS = (int) pvParameters;
  for(;;){
    vTaskDelay( 100 );
    for(int j = 0; j < 3; j++ ) { 
      // Fade IN
      for(int k = 0; k < 256; k++) { 
        vTaskDelay( 10 );
        switch(j) { 
          case 0: setAll3(k,0,0, NUM_LEDS); break;
          case 1: setAll3(0,k,0, NUM_LEDS); break;
          case 2: setAll3(0,0,k, NUM_LEDS); break;
        }
        //delay(2);
        vTaskDelay(2);
        showStrip3();
      }
      // Fade OUT
      for(int k = 255; k >= 0; k--) { 
        vTaskDelay( 10 );
        switch(j) { 
          case 0: setAll3(k,0,0, NUM_LEDS); break;
          case 1: setAll3(0,k,0, NUM_LEDS); break;
          case 2: setAll3(0,0,k, NUM_LEDS); break;
        }
        //delay(2);
        vTaskDelay(2);
        showStrip3(); 
      }
    
    }
  }
}




void showStrip3() {
  #ifdef ADAFRUIT_NEOPIXEL_H 
     // NeoPixel
    strip.show();
  #endif
  #ifndef ADAFRUIT_NEOPIXEL_H
     // FastLED
    LEDS[2].showLeds(250);
  #endif
}


void setPixel3(int Pixel, byte red, byte green, byte blue) {
   #ifdef ADAFRUIT_NEOPIXEL_H 
     // NeoPixel
    strip.setPixelColor(Pixel, strip.Color(red, green, blue));
  #endif
  #ifndef ADAFRUIT_NEOPIXEL_H 
     // FastLED
    ThirdSectionLeds[Pixel].r = red;
    ThirdSectionLeds[Pixel].g = green;
    ThirdSectionLeds[Pixel].b = blue;
  #endif
}



void setAll3(byte red, byte green, byte blue, int NUM_LEDS) {
  for(int i = 0; i < NUM_LEDS; i++ ) {
    setPixel3(i, red, green, blue); 
  }
  showStrip3();
}


////////////// END MODE 3 ZONE 3 RGBLOOP////////////////////////////////////////////////////////////////


////////////// MODE 3 ZONE 4 RGBLOOP////////////////////////////////////////////////////////////////
void RGBLoopZone4(void * pvParameters){
  TIMERG0.wdt_wprotect=TIMG_WDT_WKEY_VALUE;
  TIMERG0.wdt_feed=1;
  TIMERG0.wdt_wprotect=0; 

  int i;
  int NUM_LEDS = (int) pvParameters;
  for(;;){
    vTaskDelay( 100 );
    for(int j = 0; j < 3; j++ ) { 
      // Fade IN
      for(int k = 0; k < 256; k++) { 
        vTaskDelay( 10 );
        switch(j) { 
          case 0: setAll4(k,0,0, NUM_LEDS); break;
          case 1: setAll4(0,k,0, NUM_LEDS); break;
          case 2: setAll4(0,0,k, NUM_LEDS); break;
        }
        //delay(2);
        vTaskDelay(2);
        showStrip4();
      }
      // Fade OUT
      for(int k = 255; k >= 0; k--) { 
        vTaskDelay( 10 );
        switch(j) { 
          case 0: setAll4(k,0,0, NUM_LEDS); break;
          case 1: setAll4(0,k,0, NUM_LEDS); break;
          case 2: setAll4(0,0,k, NUM_LEDS); break;
        }
        //delay(2);
        vTaskDelay(2);
        showStrip4(); 
      }
    
    }
  }
}

void showStrip4() {
  #ifdef ADAFRUIT_NEOPIXEL_H 
     // NeoPixel
    strip.show();
  #endif
  #ifndef ADAFRUIT_NEOPIXEL_H
    // FastLED
    LEDS[3].showLeds();
  #endif
}

void setAll4(byte red, byte green, byte blue, int NUM_LEDS) {
  for(int i = 0; i < NUM_LEDS; i++ ) {
    setPixel4(i, red, green, blue); 
  }
  showStrip4();
}


////////////// END MODE 3 ZONE 4 RGBLOOP////////////////////////////////////////////////////////////////



/////////////////////////PALLETES///////////////////////////////////////////////
DEFINE_GRADIENT_PALETTE( firepal ) {
0,    255, 77, 77,
100,  255, 210, 77,
125,  148, 255, 77,
150,  77, 255, 169,
200,  77, 92, 255,
255,  202, 77, 255,
 }; 
CRGBPalette256 myPal = firepal;

DEFINE_GRADIENT_PALETTE( sunpal ) {
0,    5, 1, 18,
100,  156, 73, 5,
125,  209, 96, 4,
150,  209, 165, 4,
200,  4, 135, 217,
255,  2, 148, 245,
 }; 
CRGBPalette256 sunrisepal = sunpal;
 DEFINE_GRADIENT_PALETTE(midsummerSky) { // Only this one is used for this 10 minute demo
  0,  33, 55, 153,
  25,  80, 119, 197,
  89, 153, 215, 250,
  95, 199, 233, 252,
  102, 255, 255, 255,
  120, 197, 219, 240,
  147, 150, 187, 223,
  200, 159, 171, 172,
  255, 169, 154, 128
};

CRGBPalette256 midsummerSkys = midsummerSky;
DEFINE_GRADIENT_PALETTE(sunsetSky) {
  0,  10, 62, 123,
  36,  56, 130, 103,
  87, 153, 225, 85,
  100, 199, 217, 68,
  107, 255, 207, 54,
  115, 247, 152, 57,
  120, 239, 107, 61,
  128, 247, 152, 57,
  180, 255, 207, 54,
  223, 255, 227, 48,
  255, 255, 248, 42
};
CRGBPalette256 sunsetSky1 = sunsetSky;
/////////////////////////END PALLETES///////////////////////////////////////////////



//////////////  MODE 4 ZONE 1 fireTick////////////////////////////////////////////////////////////////
void fireTickZone1(void * pvParameters) {
  int NUM_LEDS = (int) pvParameters;
  CRGB leds[NUM_LEDS];
  static uint32_t prevTime;
  // настройки пламени
  int FIRE_STEP = 15;    // шаг огня
  int counter = 0;
  // двигаем пламя
  for(;;){
      vTaskDelay(50);
      for(int i = 0; i < NUM_LEDS; i++) {
        FirstSectionLeds[i] = ColorFromPalette(myPal,inoise8(i * FIRE_STEP, counter));
      }
      FastLED.show();
      counter += 20;
      
    }
}
//////////////END  MODE 4 ZONE 1 fireTick////////////////////////////////////////////////////////////////


//////////////MODE 4 ZONE 2 fireTick////////////////////////////////////////////////////////////////

void fireTickZone2(void * pvParameters) {
  int NUM_LEDS = (int) pvParameters;
  CRGB leds[NUM_LEDS];
  static uint32_t prevTime;
  // настройки пламени
  int FIRE_STEP = 15;    // шаг огня
  int counter = 0;
  // двигаем пламя
  for(;;){
      vTaskDelay(50);
      int thisPos = 0;
      int lastPos = 0;
      for(int i = 0; i < NUM_LEDS; i++) {
        SecondSectionLeds[i] = ColorFromPalette(myPal,inoise8(i * FIRE_STEP, counter));
        
      }
      FastLED.show();
      counter += 20;
      
  }
}
//////////////END  MODE 4 ZONE 2 fireTick////////////////////////////////////////////////////////////////

//////////////MODE 4 ZONE 3 fireTick////////////////////////////////////////////////////////////////
void fireTickZone3(void * pvParameters) {
  int NUM_LEDS = (int) pvParameters;
  CRGB leds[NUM_LEDS];
  static uint32_t prevTime;
  // настройки пламени
  int FIRE_STEP = 15;    // шаг огня
  int counter = 0;
  // двигаем пламя
  for(;;){
      vTaskDelay(50);
      int thisPos = 0;
      int lastPos = 0;
      for(int i = 0; i < NUM_LEDS; i++) {
        ThirdSectionLeds[i] = ColorFromPalette(myPal,inoise8(i * FIRE_STEP, counter));
        
      }
      FastLED.show();
      counter += 20;
      
    }
}

//////////////END  MODE 4 ZONE 3 fireTick////////////////////////////////////////////////////////////////


//////////////MODE 4 ZONE 4 fireTick////////////////////////////////////////////////////////////////
void fireTickZone4(void * pvParameters) {
  int NUM_LEDS = (int) pvParameters;
  CRGB leds[NUM_LEDS];
  static uint32_t prevTime;
  // настройки пламени
  int FIRE_STEP = 15;    // шаг огня
  int counter = 0;
  // двигаем пламя
  for(;;){
      vTaskDelay(50);
      int thisPos = 0;
      int lastPos = 0;
      for(int i = 0; i < NUM_LEDS; i++) {
        FourthSectionLeds[i] = ColorFromPalette(myPal,inoise8(i * FIRE_STEP, counter));
        
      }
      FastLED.show();
      counter += 20;
      
    }
}

//////////////END  MODE 4 ZONE 4 fireTick////////////////////////////////////////////////////////////////


CRGBPalette16 pacifica_palette_1 = 
    { 0x000507, 0x000409, 0x00030B, 0x00030D, 0x000210, 0x000212, 0x000114, 0x000117, 
      0x000019, 0x00001C, 0x000026, 0x000031, 0x00003B, 0x000046, 0x14554B, 0x28AA50 };
CRGBPalette16 pacifica_palette_2 = 
    { 0x000507, 0x000409, 0x00030B, 0x00030D, 0x000210, 0x000212, 0x000114, 0x000117, 
      0x000019, 0x00001C, 0x000026, 0x000031, 0x00003B, 0x000046, 0x0C5F52, 0x19BE5F };
CRGBPalette16 pacifica_palette_3 = 
    { 0x000208, 0x00030E, 0x000514, 0x00061A, 0x000820, 0x000927, 0x000B2D, 0x000C33, 
      0x000E39, 0x001040, 0x001450, 0x001860, 0x001C70, 0x002080, 0x1040BF, 0x2060FF };
      
////////////////MODE 5 ZONE 1 PACIFIC////////////////////////////////////////////////////////////////


void pacifica_loop_zone1(void * pvParameters)
{
  int NUM_LEDS = (int) pvParameters;
  // Increment the four "color index start" counters, one for each wave layer.
  // Each is incremented at a different speed, and the speeds vary over time.
  static uint16_t sCIStart1, sCIStart2, sCIStart3, sCIStart4;
  static uint32_t sLastms = 0;
  uint32_t ms = GET_MILLIS();
  uint32_t deltams = ms - sLastms;
  sLastms = ms;
  uint16_t speedfactor1 = beatsin16(3, 179, 269);
  uint16_t speedfactor2 = beatsin16(4, 179, 269);
  uint32_t deltams1 = (deltams * speedfactor1) / 256;
  uint32_t deltams2 = (deltams * speedfactor2) / 256;
  uint32_t deltams21 = (deltams1 + deltams2) / 2;
  sCIStart1 += (deltams1 * beatsin88(1011,10,13));
  sCIStart2 -= (deltams21 * beatsin88(777,8,11));
  sCIStart3 -= (deltams1 * beatsin88(501,5,7));
  sCIStart4 -= (deltams2 * beatsin88(257,4,6));
  for(;;){
    vTaskDelay(100);
  // Clear out the LED array to a dim background blue-green
  fill_solid( FirstSectionLeds, NUM_LEDS, CRGB( 2, 6, 10));

  // Render each of four layers, with different scales and speeds, that vary over time
  pacifica_one_layer_zone1( pacifica_palette_1, sCIStart1, beatsin16( 3, 11 * 256, 14 * 256), beatsin8( 10, 70, 130), 0-beat16( 301),NUM_LEDS);
  pacifica_one_layer_zone1( pacifica_palette_2, sCIStart2, beatsin16( 4,  6 * 256,  9 * 256), beatsin8( 17, 40,  80), beat16( 401), NUM_LEDS );
  pacifica_one_layer_zone1( pacifica_palette_3, sCIStart3, 6 * 256, beatsin8( 9, 10,38), 0-beat16(503), NUM_LEDS);
  pacifica_one_layer_zone1( pacifica_palette_3, sCIStart4, 5 * 256, beatsin8( 8, 10,28), beat16(601), NUM_LEDS);

  // Add brighter 'whitecaps' where the waves lines up more
  pacifica_add_whitecaps_zone1(NUM_LEDS);

  // Deepen the blues and greens a bit
  pacifica_deepen_colors_zone1(NUM_LEDS);

  }
}


// Add one layer of waves into the led array
void pacifica_one_layer_zone1( CRGBPalette16& p, uint16_t cistart, uint16_t wavescale, uint8_t bri, uint16_t ioff, int NUM_LEDS)
{ 
  uint16_t ci = cistart;
  uint16_t waveangle = ioff;
  uint16_t wavescale_half = (wavescale / 2) + 20;
  for( uint16_t i = 0; i < NUM_LEDS; i++) {
    waveangle += 250;
    uint16_t s16 = sin16( waveangle ) + 32768;
    uint16_t cs = scale16( s16 , wavescale_half ) + wavescale_half;
    ci += cs;
    uint16_t sindex16 = sin16( ci) + 32768;
    uint8_t sindex8 = scale16( sindex16, 240);
    CRGB c = ColorFromPalette( p, sindex8, bri, LINEARBLEND);
    FirstSectionLeds[i] += c;
  }
}

// Add extra 'white' to areas where the four layers of light have lined up brightly
void pacifica_add_whitecaps_zone1(int NUM_LEDS)
{
  uint8_t basethreshold = beatsin8( 9, 55, 65);
  uint8_t wave = beat8( 7 );
  
  for( uint16_t i = 0; i < NUM_LEDS; i++) {
    uint8_t threshold = scale8( sin8( wave), 20) + basethreshold;
    wave += 7;
    uint8_t l = FirstSectionLeds[i].getAverageLight();
    if( l > threshold) {
      uint8_t overage = l - threshold;
      uint8_t overage2 = qadd8( overage, overage);
      FirstSectionLeds[i] += CRGB( overage, overage2, qadd8( overage2, overage2));
    }
  }
}

// Deepen the blues and greens
void pacifica_deepen_colors_zone1(int NUM_LEDS)
{
  for( uint16_t i = 0; i < NUM_LEDS; i++) {
    FirstSectionLeds[i].blue = scale8( FirstSectionLeds[i].blue,  145); 
    FirstSectionLeds[i].green= scale8( FirstSectionLeds[i].green, 200); 
    FirstSectionLeds[i] |= CRGB( 2, 5, 7);
  }
}
////////////// END MODE 5 ZONE 1 PACIFIC////////////////////////////////////////////////////////////////


//////////////MODE 5 ZONE 2 PACIFIC////////////////////////////////////////////////////////////////


void pacifica_loop_zone2(void * pvParameters)
{
  int NUM_LEDS = (int) pvParameters;
  // Increment the four "color index start" counters, one for each wave layer.
  // Each is incremented at a different speed, and the speeds vary over time.
  static uint16_t sCIStart1, sCIStart2, sCIStart3, sCIStart4;
  static uint32_t sLastms = 0;
  uint32_t ms = GET_MILLIS();
  uint32_t deltams = ms - sLastms;
  sLastms = ms;
  uint16_t speedfactor1 = beatsin16(3, 179, 269);
  uint16_t speedfactor2 = beatsin16(4, 179, 269);
  uint32_t deltams1 = (deltams * speedfactor1) / 256;
  uint32_t deltams2 = (deltams * speedfactor2) / 256;
  uint32_t deltams21 = (deltams1 + deltams2) / 2;
  sCIStart1 += (deltams1 * beatsin88(1011,10,13));
  sCIStart2 -= (deltams21 * beatsin88(777,8,11));
  sCIStart3 -= (deltams1 * beatsin88(501,5,7));
  sCIStart4 -= (deltams2 * beatsin88(257,4,6));
  for(;;){
    vTaskDelay(100);
  // Clear out the LED array to a dim background blue-green
  fill_solid( SecondSectionLeds, NUM_LEDS, CRGB( 2, 6, 10));

  // Render each of four layers, with different scales and speeds, that vary over time
  pacifica_one_layer_zone2( pacifica_palette_1, sCIStart1, beatsin16( 3, 11 * 256, 14 * 256), beatsin8( 10, 70, 130), 0-beat16( 301),NUM_LEDS);
  pacifica_one_layer_zone2( pacifica_palette_2, sCIStart2, beatsin16( 4,  6 * 256,  9 * 256), beatsin8( 17, 40,  80), beat16( 401), NUM_LEDS );
  pacifica_one_layer_zone2( pacifica_palette_3, sCIStart3, 6 * 256, beatsin8( 9, 10,38), 0-beat16(503), NUM_LEDS);
  pacifica_one_layer_zone2( pacifica_palette_3, sCIStart4, 5 * 256, beatsin8( 8, 10,28), beat16(601), NUM_LEDS);

  // Add brighter 'whitecaps' where the waves lines up more
  pacifica_add_whitecaps_zone2(NUM_LEDS);

  // Deepen the blues and greens a bit
  pacifica_deepen_colors_zone2(NUM_LEDS);
  }
}

// Add one layer of waves into the led array
void pacifica_one_layer_zone2( CRGBPalette16& p, uint16_t cistart, uint16_t wavescale, uint8_t bri, uint16_t ioff, int NUM_LEDS)
{ 
  uint16_t ci = cistart;
  uint16_t waveangle = ioff;
  uint16_t wavescale_half = (wavescale / 2) + 20;
  for( uint16_t i = 0; i < NUM_LEDS; i++) {
    waveangle += 250;
    uint16_t s16 = sin16( waveangle ) + 32768;
    uint16_t cs = scale16( s16 , wavescale_half ) + wavescale_half;
    ci += cs;
    uint16_t sindex16 = sin16( ci) + 32768;
    uint8_t sindex8 = scale16( sindex16, 240);
    CRGB c = ColorFromPalette( p, sindex8, bri, LINEARBLEND);
    SecondSectionLeds[i] += c;
  }
}

// Add extra 'white' to areas where the four layers of light have lined up brightly
void pacifica_add_whitecaps_zone2(int NUM_LEDS)
{
  uint8_t basethreshold = beatsin8( 9, 55, 65);
  uint8_t wave = beat8( 7 );
  
  for( uint16_t i = 0; i < NUM_LEDS; i++) {
    uint8_t threshold = scale8( sin8( wave), 20) + basethreshold;
    wave += 7;
    uint8_t l = FirstSectionLeds[i].getAverageLight();
    if( l > threshold) {
      uint8_t overage = l - threshold;
      uint8_t overage2 = qadd8( overage, overage);
      SecondSectionLeds[i] += CRGB( overage, overage2, qadd8( overage2, overage2));
    }
  }
}

// Deepen the blues and greens
void pacifica_deepen_colors_zone2(int NUM_LEDS)
{
  for( uint16_t i = 0; i < NUM_LEDS; i++) {
    SecondSectionLeds[i].blue = scale8( SecondSectionLeds[i].blue,  145); 
    SecondSectionLeds[i].green= scale8( SecondSectionLeds[i].green, 200); 
    SecondSectionLeds[i] |= CRGB( 2, 5, 7);
  }
}
////////////// END MODE 5 ZONE 2 PACIFIC////////////////////////////////////////////////////////////////


//////////////MODE 5 ZONE 3 PACIFIC////////////////////////////////////////////////////////////////


void pacifica_loop_zone3(void * pvParameters)
{
  int NUM_LEDS = (int) pvParameters;
  // Increment the four "color index start" counters, one for each wave layer.
  // Each is incremented at a different speed, and the speeds vary over time.
  static uint16_t sCIStart1, sCIStart2, sCIStart3, sCIStart4;
  static uint32_t sLastms = 0;
  uint32_t ms = GET_MILLIS();
  uint32_t deltams = ms - sLastms;
  sLastms = ms;
  uint16_t speedfactor1 = beatsin16(3, 179, 269);
  uint16_t speedfactor2 = beatsin16(4, 179, 269);
  uint32_t deltams1 = (deltams * speedfactor1) / 256;
  uint32_t deltams2 = (deltams * speedfactor2) / 256;
  uint32_t deltams21 = (deltams1 + deltams2) / 2;
  sCIStart1 += (deltams1 * beatsin88(1011,10,13));
  sCIStart2 -= (deltams21 * beatsin88(777,8,11));
  sCIStart3 -= (deltams1 * beatsin88(501,5,7));
  sCIStart4 -= (deltams2 * beatsin88(257,4,6));
  for(;;){
    vTaskDelay(100);
    // Clear out the LED array to a dim background blue-green
    fill_solid( ThirdSectionLeds, NUM_LEDS, CRGB( 2, 6, 10));

    // Render each of four layers, with different scales and speeds, that vary over time
    pacifica_one_layer_zone3( pacifica_palette_1, sCIStart1, beatsin16( 3, 11 * 256, 14 * 256), beatsin8( 10, 70, 130), 0-beat16( 301),NUM_LEDS);
    pacifica_one_layer_zone3( pacifica_palette_2, sCIStart2, beatsin16( 4,  6 * 256,  9 * 256), beatsin8( 17, 40,  80), beat16( 401), NUM_LEDS );
    pacifica_one_layer_zone3( pacifica_palette_3, sCIStart3, 6 * 256, beatsin8( 9, 10,38), 0-beat16(503), NUM_LEDS);
    pacifica_one_layer_zone3( pacifica_palette_3, sCIStart4, 5 * 256, beatsin8( 8, 10,28), beat16(601), NUM_LEDS);

    // Add brighter 'whitecaps' where the waves lines up more
    pacifica_add_whitecaps_zone3(NUM_LEDS);

    // Deepen the blues and greens a bit
    pacifica_deepen_colors_zone3(NUM_LEDS);
  }
}

// Add one layer of waves into the led array
void pacifica_one_layer_zone3( CRGBPalette16& p, uint16_t cistart, uint16_t wavescale, uint8_t bri, uint16_t ioff, int NUM_LEDS)
{ 
  uint16_t ci = cistart;
  uint16_t waveangle = ioff;
  uint16_t wavescale_half = (wavescale / 2) + 20;
  for( uint16_t i = 0; i < NUM_LEDS; i++) {
    waveangle += 250;
    uint16_t s16 = sin16( waveangle ) + 32768;
    uint16_t cs = scale16( s16 , wavescale_half ) + wavescale_half;
    ci += cs;
    uint16_t sindex16 = sin16( ci) + 32768;
    uint8_t sindex8 = scale16( sindex16, 240);
    CRGB c = ColorFromPalette( p, sindex8, bri, LINEARBLEND);
    ThirdSectionLeds[i] += c;
  }
}

// Add extra 'white' to areas where the four layers of light have lined up brightly
void pacifica_add_whitecaps_zone3(int NUM_LEDS)
{
  uint8_t basethreshold = beatsin8( 9, 55, 65);
  uint8_t wave = beat8( 7 );
  
  for( uint16_t i = 0; i < NUM_LEDS; i++) {
    uint8_t threshold = scale8( sin8( wave), 20) + basethreshold;
    wave += 7;
    uint8_t l = ThirdSectionLeds[i].getAverageLight();
    if( l > threshold) {
      uint8_t overage = l - threshold;
      uint8_t overage2 = qadd8( overage, overage);
      ThirdSectionLeds[i] += CRGB( overage, overage2, qadd8( overage2, overage2));
    }
  }
}

// Deepen the blues and greens
void pacifica_deepen_colors_zone3(int NUM_LEDS)
{
  for( uint16_t i = 0; i < NUM_LEDS; i++) {
    ThirdSectionLeds[i].blue = scale8( ThirdSectionLeds[i].blue,  145); 
    ThirdSectionLeds[i].green= scale8( ThirdSectionLeds[i].green, 200); 
    ThirdSectionLeds[i] |= CRGB( 2, 5, 7);
  }
}
////////////// END MODE 5 ZONE 3 PACIFIC////////////////////////////////////////////////////////////////


//////////////MODE 5 ZONE 4 PACIFIC////////////////////////////////////////////////////////////////


void pacifica_loop_zone4(void * pvParameters)
{
  int NUM_LEDS = (int) pvParameters;
  // Increment the four "color index start" counters, one for each wave layer.
  // Each is incremented at a different speed, and the speeds vary over time.
  static uint16_t sCIStart1, sCIStart2, sCIStart3, sCIStart4;
  static uint32_t sLastms = 0;
  uint32_t ms = GET_MILLIS();
  uint32_t deltams = ms - sLastms;
  sLastms = ms;
  uint16_t speedfactor1 = beatsin16(3, 179, 269);
  uint16_t speedfactor2 = beatsin16(4, 179, 269);
  uint32_t deltams1 = (deltams * speedfactor1) / 256;
  uint32_t deltams2 = (deltams * speedfactor2) / 256;
  uint32_t deltams21 = (deltams1 + deltams2) / 2;
  sCIStart1 += (deltams1 * beatsin88(1011,10,13));
  sCIStart2 -= (deltams21 * beatsin88(777,8,11));
  sCIStart3 -= (deltams1 * beatsin88(501,5,7));
  sCIStart4 -= (deltams2 * beatsin88(257,4,6));
  for(;;){
    vTaskDelay(100);
    // Clear out the LED array to a dim background blue-green
    fill_solid( FourthSectionLeds, NUM_LEDS, CRGB( 2, 6, 10));

    // Render each of four layers, with different scales and speeds, that vary over time
    pacifica_one_layer_zone4( pacifica_palette_1, sCIStart1, beatsin16( 3, 11 * 256, 14 * 256), beatsin8( 10, 70, 130), 0-beat16( 301),NUM_LEDS);
    pacifica_one_layer_zone4( pacifica_palette_2, sCIStart2, beatsin16( 4,  6 * 256,  9 * 256), beatsin8( 17, 40,  80), beat16( 401), NUM_LEDS );
    pacifica_one_layer_zone4( pacifica_palette_3, sCIStart3, 6 * 256, beatsin8( 9, 10,38), 0-beat16(503), NUM_LEDS);
    pacifica_one_layer_zone4( pacifica_palette_3, sCIStart4, 5 * 256, beatsin8( 8, 10,28), beat16(601), NUM_LEDS);

    // Add brighter 'whitecaps' where the waves lines up more
    pacifica_add_whitecaps_zone4(NUM_LEDS);
  
    // Deepen the blues and greens a bit
    pacifica_deepen_colors_zone4(NUM_LEDS);
  }
}

// Add one layer of waves into the led array
void pacifica_one_layer_zone4( CRGBPalette16& p, uint16_t cistart, uint16_t wavescale, uint8_t bri, uint16_t ioff, int NUM_LEDS)
{ 
  uint16_t ci = cistart;
  uint16_t waveangle = ioff;
  uint16_t wavescale_half = (wavescale / 2) + 20;
  for( uint16_t i = 0; i < NUM_LEDS; i++) {
    waveangle += 250;
    uint16_t s16 = sin16( waveangle ) + 32768;
    uint16_t cs = scale16( s16 , wavescale_half ) + wavescale_half;
    ci += cs;
    uint16_t sindex16 = sin16( ci) + 32768;
    uint8_t sindex8 = scale16( sindex16, 240);
    CRGB c = ColorFromPalette( p, sindex8, bri, LINEARBLEND);
    FourthSectionLeds[i] += c;
  }
}

// Add extra 'white' to areas where the four layers of light have lined up brightly
void pacifica_add_whitecaps_zone4(int NUM_LEDS)
{
  uint8_t basethreshold = beatsin8( 9, 55, 65);
  uint8_t wave = beat8( 7 );
  
  for( uint16_t i = 0; i < NUM_LEDS; i++) {
    uint8_t threshold = scale8( sin8( wave), 20) + basethreshold;
    wave += 7;
    uint8_t l = FourthSectionLeds[i].getAverageLight();
    if( l > threshold) {
      uint8_t overage = l - threshold;
      uint8_t overage2 = qadd8( overage, overage);
      FourthSectionLeds[i] += CRGB( overage, overage2, qadd8( overage2, overage2));
    }
  }
}

// Deepen the blues and greens
void pacifica_deepen_colors_zone4(int NUM_LEDS)
{
  for( uint16_t i = 0; i < NUM_LEDS; i++) {
    FourthSectionLeds[i].blue = scale8( FourthSectionLeds[i].blue,  145); 
    FourthSectionLeds[i].green= scale8( FourthSectionLeds[i].green, 200); 
    FourthSectionLeds[i] |= CRGB( 2, 5, 7);
  }
}
//////////////// END MODE 5 ZONE 4 PACIFIC////////////////////////////////////////////////////////////////


/////////////////////////MODE 6 ZONE 1 Sunrise////////////////////////////////////////////////////////////////
void sunrise(void * pvParameters) {
  int NUM_LEDS = (int) pvParameters;
  // total sunrise length, in minutes
  static const uint8_t sunriseLength = 30;

  // how often (in seconds) should the heat color increase?
  // for the default of 30 minutes, this should be about every 7 seconds
  // 7 seconds x 256 gradient steps = 1,792 seconds = ~30 minutes
  static const uint8_t interval = (sunriseLength * 60) / 256;

  // current gradient palette color index
  static uint8_t heatIndex = 0; // start out at 0
  static uint8_t heatIndex1 = 0; // start out at 0
  
  int mins = 5; //delay in minutes
  long n = mins * 6000;
  for(;;){
  // HeatColors_p is a gradient palette built in to FastLED
  // that fades from black to red, orange, yellow, white
  // feel free to use another palette or define your own custom one
  

  // slowly increase the heat
  
    LEDS[0].showLeds();
    Serial.println(heatIndex);
      // stop incrementing at 255, we don't want to overflow back to 0
      if(heatIndex < 255) {
        CRGB color = ColorFromPalette(sunsetSky1, heatIndex);
        // fill the entire strip with the current color
        fill_solid(FirstSectionLeds, NUM_LEDS, color);
        heatIndex++;
        heatIndex1=0;
      }
      else if (heatIndex == 255){
        CRGB color1 = ColorFromPalette(sunrisepal, heatIndex1);

        // fill the entire strip with the current color
        fill_solid(FirstSectionLeds, NUM_LEDS, color1);
        if(heatIndex1 < 255) {
          heatIndex1++;
        }
        else if (heatIndex1 == 255){heatIndex = 0;}
      }
      vTaskDelay(20000); 
  }  
}

/////////////////////////END MODE 6 ZONE 1 Sunrise////////////////////////////////////////////////////////////////


/////////////////////////MODE 6 ZONE 2 Sunrise////////////////////////////////////////////////////////////////

void sunrise2(void * pvParameters) {
  int NUM_LEDS = (int) pvParameters;
  // total sunrise length, in minutes
  static const uint8_t sunriseLength = 30;

  // how often (in seconds) should the heat color increase?
  // for the default of 30 minutes, this should be about every 7 seconds
  // 7 seconds x 256 gradient steps = 1,792 seconds = ~30 minutes
  static const uint8_t interval = (sunriseLength * 60) / 256;

  // current gradient palette color index
  static uint8_t heatIndex = 0; // start out at 0
  static uint8_t heatIndex1 = 0; // start out at 0
  for(;;){
  // HeatColors_p is a gradient palette built in to FastLED
  // that fades from black to red, orange, yellow, white
  // feel free to use another palette or define your own custom one
  

  // slowly increase the heat
    vTaskDelay(100); 
    LEDS[1].showLeds();
    Serial.println(heatIndex);
    // stop incrementing at 255, we don't want to overflow back to 0
    if(heatIndex < 255) {
      CRGB color = ColorFromPalette(sunsetSky1, heatIndex);
      // fill the entire strip with the current color
      fill_solid(SecondSectionLeds, NUM_LEDS, color);
      heatIndex++;
      heatIndex1=0;
    }
    else if (heatIndex == 255){
      CRGB color1 = ColorFromPalette(sunrisepal, heatIndex1);

      // fill the entire strip with the current color
      fill_solid(SecondSectionLeds, NUM_LEDS, color1);
      if(heatIndex1 < 255) {
        heatIndex1++;
      }
      else if (heatIndex1 == 255){heatIndex = 0;}
      }
  }  
}


/////////////////////////END MODE 6 ZONE 2 Sunrise////////////////////////////////////////////////////////////////


/////////////////////////MODE 6 ZONE 3 Sunrise////////////////////////////////////////////////////////////////

void sunrise3(void * pvParameters) {
  int NUM_LEDS = (int) pvParameters;
  // total sunrise length, in minutes
  static const uint8_t sunriseLength = 30;

  // how often (in seconds) should the heat color increase?
  // for the default of 30 minutes, this should be about every 7 seconds
  // 7 seconds x 256 gradient steps = 1,792 seconds = ~30 minutes
  static const uint8_t interval = (sunriseLength * 60) / 256;

  // current gradient palette color index
  static uint8_t heatIndex = 0; // start out at 0
  static uint8_t heatIndex1 = 0; // start out at 0
  for(;;){
  // HeatColors_p is a gradient palette built in to FastLED
  // that fades from black to red, orange, yellow, white
  // feel free to use another palette or define your own custom one
  

  // slowly increase the heat
    vTaskDelay(100); 
    LEDS[2].showLeds();
    Serial.println(heatIndex);
    // stop incrementing at 255, we don't want to overflow back to 0
    if(heatIndex < 255) {
      CRGB color = ColorFromPalette(sunsetSky1, heatIndex);
      // fill the entire strip with the current color
      fill_solid(ThirdSectionLeds, NUM_LEDS, color);
      heatIndex++;
      heatIndex1=0;
    }
    else if (heatIndex == 255){
      CRGB color1 = ColorFromPalette(sunrisepal, heatIndex1);

      // fill the entire strip with the current color
      fill_solid(ThirdSectionLeds, NUM_LEDS, color1);
      if(heatIndex1 < 255) {
        heatIndex1++;
      }
      else if (heatIndex1 == 255){heatIndex = 0;}
      }
  }  
}


/////////////////////////END MODE 6 ZONE 3 Sunrise////////////////////////////////////////////////////////////////


/////////////////////////MODE 6 ZONE 4 Sunrise////////////////////////////////////////////////////////////////

void sunrise4(void * pvParameters) {
  int NUM_LEDS = (int) pvParameters;
  // total sunrise length, in minutes
  static const uint8_t sunriseLength = 30;

  // how often (in seconds) should the heat color increase?
  // for the default of 30 minutes, this should be about every 7 seconds
  // 7 seconds x 256 gradient steps = 1,792 seconds = ~30 minutes
  static const uint8_t interval = (sunriseLength * 60) / 256;

  // current gradient palette color index
  static uint8_t heatIndex = 0; // start out at 0
  static uint8_t heatIndex1 = 0; // start out at 0
  for(;;){
  // HeatColors_p is a gradient palette built in to FastLED
  // that fades from black to red, orange, yellow, white
  // feel free to use another palette or define your own custom one
  

  // slowly increase the heat
  vTaskDelay(100); 
    LEDS[3].showLeds();
    Serial.println(heatIndex);
    // stop incrementing at 255, we don't want to overflow back to 0
    if(heatIndex < 255) {
      CRGB color = ColorFromPalette(sunsetSky1, heatIndex);
      // fill the entire strip with the current color
      fill_solid(ThirdSectionLeds, NUM_LEDS, color);
      heatIndex++;
      heatIndex1=0;
    }
    else if (heatIndex == 255){
      CRGB color1 = ColorFromPalette(sunrisepal, heatIndex1);

      // fill the entire strip with the current color
      fill_solid(ThirdSectionLeds, NUM_LEDS, color1);
      if(heatIndex1 < 255) {
        heatIndex1++;
      }
      else if (heatIndex1 == 255){heatIndex = 0;}
      }
  }  
}


/////////////////////////END MODE 6 ZONE 4 Sunrise////////////////////////////////////////////////////////////////
void loop() {
  // put your main code here, to run repeatedly:

}
