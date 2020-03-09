/************************************* -> INCLUDE <- *******************************************/
#include <FastLED.h>
#include <string>

#include <ezTime.h>               // time library

#include <WiFi.h>
#include <AsyncTCP.h>

#include <ESPAsyncWebServer.h>
#include <ESPAsyncWiFiManager.h>
#include <ESPDash.h>

#include <AsyncMqttClient.h>

#include <AsyncElegantOTA.h>

#include <Preferences.h>

#include <SSD1306Wire.h>
#include <OLEDDisplayUi.h>

// include menu item class
#include "Menus.h" 

#include "menu_strings.h"

// Include custom images
#include "images.h"

// include encoder lib
#include <AiEsp32RotaryEncoder.h>

// include dht library
#include <DHTesp.h>

// include ticker library for buzzer
#include <Ticker.h>

// include BLE libraries
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLEAddress.h>

#include <TaskScheduler.h>
#include <TaskSchedulerDeclarations.h>

/************************************* -> DEFINE <- ****************************************88*********************/

// DHT11
#define DHT_PIN 0
#define DHT_TYPE DHTesp::DHT11

// Encoder
#define ENCODER_A_PIN 2
#define ENCODER_B_PIN 14
#define ENCODER_BUTTON_PIN 12
#define ENCODER_VCC_PIN -1        // put -1 of Rotary encoder Vcc is connected directly to 5V*
#define ENCODER_STEPS 2           // put 2 for Quadrature and 4 for half quadrature

// Buzzer
#define BUZZER_PIN 13             // номер виводу на платі ESP32
#define BUZZER_FREQ 1000          // частота шим сигналу для базера
#define BUZZER_CH 0               // канал для сигналу
#define BUZZER_RES 8              // бітність сигналу
#define BUZZER_DC 128             // duty cycle

// налаштування для LED плати:
#define LED_PIN 15                // номер виводу на платі ESP32
#define NUM_LEDS 8                // кількість світлодіодів на платі
#define BRIGHTNESS 64             // рівень яскравості
#define LED_TYPE WS2812           // модель світлодіодів
#define COLOR_ORDER GRB           // порядок чергування кольорів
#define UPDATES_PER_SECOND 100

#define PHOTORESISTOR_RIGHT_PIN 15
#define PHOTORESISTOR_LEFT_PIN 14

#define MQ135_PIN 39
#define SOILMOISTURE_PIN 36

#define PUMP_PIN 25
#define FAN_PIN 26

// #define BUTTON_PIN1 25
// #define BUTTON_PIN2 4

// Bluetooth
#define SERVICE_UUID "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define LEDCHARACTERISTIC_UUID "c0c9a98a-f419-44d6-924f-a3f7c3b40f46"
#define DHTCHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

#define MAXDISPLAYEDITEMS 4

#define BLUETOOTH_NAME    "ESP32"   // what other bluetooth devices will see

#define WATERING_FREQ     1         // how often to water your plant in days
#define WATERING_TIME     0         // what time to start watering in minutes from 00:00 to 23:59 (0 equals 00:00, 720 = 12:00)
#define WATERING_DUR      5         // how long pump needs to work, in seconds, that is, how much water does your plant needs
#define VENTILATION_FREQ  1         // how often to ventilate air in greenhouse in days
#define VENTILATION_TIME  0         // same as watering time
#define VENTILATION_DUR   5         // same as watering duration
#define OPTIMAL_TEMP      15        // optimal temperature for your plant in celsius
#define OPTIMAL_HUM       50        // optimal humidity percentage for your plant
#define OPTIMAL_SM        50        // optimal soil moisture percentage for your plant

#define SYNC_TIME         0         // set time at which thingspeak updating occurs (same as watering time)

#define ENOUGH_LIGHT      0
#define NOT_ENOUGH_LIGHT  1
#define TOO_DARK          2

/************************************* -> VARIABLES <- *******************************************************/

// buzer stuff
int buzzerCounter = 0;
int buzzerVolume = 30;
bool buzzerState = false;

// LED stuff
int currentRed = 0;
int currentGreen = 0;
int currentBlue = 0;
int ledsOnCount = 0;
bool ledSwitch = false;

// Bluetooth stuff
int deviceConnected = 0;

// encoder stuff
int encoderMinValue = 0;
int encoderMaxValue = 10;
bool encoderCycle = false;
int16_t encoderValue = 0;

// display menu stuff
bool isEditing = false;
int animationCount = 0;
int currMenu = 2;
int pointerType = 0;   // '>' or white rectangle

int buttonValue = 0;

//dht stuff
float temperature;
float humidity;
float heatIndex;
float dewPoint;

//pump & fan stuff
int pumpSpeed = 0;
int fanSpeed = 0;
uint32_t pumpIsOn = 0;
uint32_t fanIsOn = 0;

// main ui stuff
const int frameCount = 2;
const int overlaysCount = 1;
FrameCallback frames[frameCount];
OverlayCallback overlays[overlaysCount];
uint16_t      localTicksSinceLastStateSwitch; // need to count ticks to properly update ui

// automation stuff
int wateringFreq  = 1;
int wateringTime  = 0;
int wateringDur   = 5;
int ventilationFreq = 1;
int ventilationTime = 0;
int ventilationDur  = 0;
int optimalTemp = 0;
int optimalHum  = 0;
int optimalSM   = 0;
int thingspeakSyncTime = 0;
bool isDayTime;
bool isWatering;
bool isVentilating;

// thingspeak stuff
const char *host = "api.thingspeak.com";
const char *thingspeakServer = "mqtt.thingspeak.com"; 
const char *apiRequests = "update";
const char *apiKey = "C7U8XM2GLOZ3ILF3";
const char *mqttUserName = "GreenhouseESP32MQTT76";  // Use any name.
const char *mqttPass = "CJD8Z2XQG21PWIHK";
long channelID = 735921; 
const int tempField = 1;
const int humField = 2;
const int co2Field = 3;
const int smField = 4;
bool mqttConnected = false;

const char *ssid = "Bot";
const char *password = "robotics";

CRGB leds[NUM_LEDS]; // масив кольорів для кожного світлодіода

// dht task stuff
DHTesp dht11;
Scheduler ts;
void dhtCallback();
Task tWrapper(5000, TASK_FOREVER, &dhtCallback, &ts, true);

// timer stuff for buzzer and dht
Ticker buzzerTicker;
Ticker sensorTicker;

SSD1306Wire display(0x3c, 5, 4);
OLEDDisplayUi ui(&display);

AsyncWebServer server(80);
DNSServer dns;
AsyncWiFiManager wifiManager(&server,&dns);

AsyncMqttClient mqttClient;

Preferences preferences;

Timezone Ukraine;

AiEsp32RotaryEncoder encoder = AiEsp32RotaryEncoder(ENCODER_A_PIN, ENCODER_B_PIN, ENCODER_BUTTON_PIN, ENCODER_VCC_PIN);//, ENCODER_STEPS);

// web ui cards
Card sliderRed, sliderGreen, sliderBlue, sliderLED, chartTemp, ledButton;

Menu mainMenu, settingsMenu, buzzerMenu, ledMenu, sensorMenu, automationMenu, bleMenu;

MenuNames mNames[] = 
{
  {menus::main, titleMenuItems[menus::main].c_str(), mainMenuItems},
  {menus::settings, titleMenuItems[menus::settings].c_str(), settingsMenuItems},
  {menus::buzzer, titleMenuItems[menus::buzzer].c_str(), buzzerMenuItems},
  {menus::led, titleMenuItems[menus::led].c_str(), ledMenuItems},
  {menus::sensors, titleMenuItems[menus::sensors].c_str(), sensorsMenuItems},
  {menus::automation, titleMenuItems[menus::automation].c_str(), automationMenuItems},
  {menus::ble, titleMenuItems[menus::ble].c_str(), bleMenuItems}
};

SaveItemNames saveItems[] = 
{
  {0, "wFreq", WATERING_FREQ}, {1, "wTime", WATERING_DUR}, {2, "wDur", WATERING_TIME},
  {3, "vFreq", VENTILATION_FREQ}, {4, "vTime", VENTILATION_DUR}, {5, "vDur", VENTILATION_TIME},
  {6, "optTemp", OPTIMAL_TEMP}, {7, "optHum", OPTIMAL_HUM}, {8, "optSM", OPTIMAL_SM}, {9, "syncTime", SYNC_TIME}
};

/***************************************** -> FUNCTIONS <- ************************************************************/

/**** HELPER FUNCTIONS ****/

String processLightSensors(int value) // return custom string from light sensors values
{
  String str;
  switch(value)
  {
    case ENOUGH_LIGHT:
      str = "Enough Light";
      break;
    case NOT_ENOUGH_LIGHT:
      str = "Not Enough Light";
      break;
    case TOO_DARK:
      str = "Too Dark";
      break;
    default:
      break;
  }
  return str;
}

String twoDigits(int digits) // utility function for clock display: adds leading 0
{
  if(digits < 10) {
    String i = '0'+String(digits);
    return i;
  }
  else {
    return String(digits);
  }
}

String processMinutes(int minutes) // returns time from minutes in format HH:MM
{
  return String(minutes/60) + ":" + twoDigits(minutes % 60);
}

int sort3(int arr[])  // function to sort three numbers in array
{
  // Insert arr[1] 
  if (arr[1] < arr[0]) 
      std::swap(arr[0], arr[1]); 

  // Insert arr[2] 
  if (arr[2] < arr[1]) 
  { 
      std::swap(arr[1], arr[2]); 
      if (arr[1] < arr[0]) 
        std::swap(arr[1], arr[0]); 
  }
}
/**** END HELPER FUNCTIONS ****/

//callback class for receiving device connection events
class ServerCallbacks: public BLEServerCallbacks
{
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
    }
 
    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
    }
};

class LedCallbacks : public BLECharacteristicCallbacks
{
  void onRead(BLECharacteristic *pCharacteristic)
  {
    String tempStr = "Num-" + String(Menus.getItem(ledMenu, 3)->numValue) + "\n" +
                      "R-" + String(Menus.getItem(ledMenu, 0)->numValue) +
                      " G-" + String(Menus.getItem(ledMenu, 1)->numValue) +
                      " B-" + String(Menus.getItem(ledMenu, 2)->numValue);

    pCharacteristic->setValue(tempStr.c_str());
  }

  void onWrite(BLECharacteristic *pCharacteristic)
  {
    std::string value = pCharacteristic->getValue();

    std::string temps1;
    std::string temps2;
    int tempInt1;
    int tempInt2;

    if (value.length() > 0 && value.length() < 12)
    {
      std::size_t foundL = value.find("l");
      std::size_t foundR = value.find("r");
      std::size_t foundG = value.find("g");
      std::size_t foundB = value.find("b");

      if(foundL != std::string::npos && value.length() < 3 && isDigit(value[foundL + 1]))
      {
        if(value[foundL + 1] - '0' < 9) Menus.updateItem(ledMenu, 3, (value[foundL + 1] - '0'));
        ESPDash.UpdateCard(sliderLED, Menus.getItem(ledMenu, 3)->numValue);
      }
      else if(foundL == std::string::npos && (foundR != std::string::npos || foundG != std::string::npos || foundB != std::string::npos))
      {
        if(foundR != std::string::npos && foundG == std::string::npos && foundB == std::string::npos) // r = (0-255);
        {
          if(isDigit(value[foundR + 1]) && std::all_of(value.begin() + (foundR + 1), value.end(), ::isdigit))
          {
            temps1 = value.substr(foundR + 1);
            char cstr[temps1.size() + 1];
            strcpy(cstr, temps1.c_str());
            tempInt1 = String(cstr).toInt();
            if(tempInt1 < 256)
              Menus.updateItem(ledMenu, 0, tempInt1);
          }
        }
        else if(foundR == std::string::npos && foundG != std::string::npos && foundB == std::string::npos) // g = (0-255);
        {
          if(isDigit(value[foundG + 1]) && std::all_of(value.begin() + (foundG + 1), value.end(), ::isdigit))
          {
            temps1 = value.substr(foundG + 1);
            char cstr[temps1.size() + 1];
            strcpy(cstr, temps1.c_str());
            tempInt1 = String(cstr).toInt();
            if(tempInt1 < 256)
              Menus.updateItem(ledMenu, 1, tempInt1);
          }
        }
        else if(foundR == std::string::npos && foundG == std::string::npos && foundB != std::string::npos) // b = (0-255);
        {
          if(isDigit(value[foundB + 1]) && std::all_of(value.begin() + (foundB + 1), value.end(), ::isdigit))
          {
            temps1 = value.substr(foundB + 1);
            char cstr[temps1.size() + 1];
            strcpy(cstr, temps1.c_str());
            tempInt1 = String(cstr).toInt();
            if(tempInt1 < 256)
              Menus.updateItem(ledMenu, 2, tempInt1);;
          }
        }
        else if(foundR != std::string::npos && foundG != std::string::npos && foundB == std::string::npos) // r = (0-255); g = (0-255);
        {
          if(foundR > foundG)
          {
            if(isDigit(value[foundR + 1]) && isDigit(value[foundG + 1]) && 
              std::all_of(value.begin() + (foundG + 1), value.begin() + foundR, ::isdigit) && 
              std::all_of(value.begin() + (foundR + 1), value.end(), ::isdigit))
            {
              temps1 = value.substr(foundG + 1, foundR);
              temps2 = value.substr(foundR + 1);
              char cstr1[temps1.size() + 1];
              char cstr2[temps2.size() + 1];
              strcpy(cstr1, temps1.c_str());
              strcpy(cstr2, temps2.c_str());
              tempInt1 = String(cstr1).toInt();
              tempInt2 = String(cstr2).toInt();
              if(tempInt1 < 256 && tempInt2 < 256)
              {
                Menus.updateItem(ledMenu, 0, tempInt2);
                Menus.updateItem(ledMenu, 1, tempInt1);
              }
            }
          }
          else
          {
            if(isDigit(value[foundR + 1]) && isDigit(value[foundG + 1]) && 
              std::all_of(value.begin() + (foundR + 1), value.begin() + foundG, ::isdigit) && 
              std::all_of(value.begin() + (foundG + 1), value.end(), ::isdigit))
            {
              temps1 = value.substr(foundR + 1, foundG);
              temps2 = value.substr(foundG + 1);
              char cstr1[temps1.size() + 1];
              char cstr2[temps2.size() + 1];
              strcpy(cstr1, temps1.c_str());
              strcpy(cstr2, temps2.c_str());
              tempInt1 = String(cstr1).toInt();
              tempInt2 = String(cstr2).toInt();
              if(tempInt1 < 256 && tempInt2 < 256)
              {
                Menus.updateItem(ledMenu, 0, tempInt1);
                Menus.updateItem(ledMenu, 1, tempInt2);
              }
            }
          }
        }
        else if(foundR != std::string::npos && foundG == std::string::npos && foundB != std::string::npos) // r = (0-255); b = (0-255);
        {
          if(foundR > foundB)
          {
            if(isDigit(value[foundR + 1]) && isDigit(value[foundB + 1]) && 
              std::all_of(value.begin() + (foundB + 1), value.begin() + foundR, ::isdigit) && 
              std::all_of(value.begin() + (foundR + 1), value.end(), ::isdigit))
            {
              temps1 = value.substr(foundB + 1, foundR);
              temps2 = value.substr(foundR + 1);
              char cstr1[temps1.size() + 1];
              char cstr2[temps2.size() + 1];
              strcpy(cstr1, temps1.c_str());
              strcpy(cstr2, temps2.c_str());
              tempInt1 = String(cstr1).toInt();
              tempInt2 = String(cstr2).toInt();
              if(tempInt1 < 256 && tempInt2 < 256)
              {
                Menus.updateItem(ledMenu, 0, tempInt2);
                Menus.updateItem(ledMenu, 2, tempInt1);
              }
            }
          }
          else
          {
            if(isDigit(value[foundR + 1]) && isDigit(value[foundB + 1]) && 
              std::all_of(value.begin() + (foundR + 1), value.begin() + foundB, ::isdigit) && 
              std::all_of(value.begin() + (foundB + 1), value.end(), ::isdigit))
            {
              temps1 = value.substr(foundR + 1, foundB);
              temps2 = value.substr(foundB + 1);
              char cstr1[temps1.size() + 1];
              char cstr2[temps2.size() + 1];
              strcpy(cstr1, temps1.c_str());
              strcpy(cstr2, temps2.c_str());
              tempInt1 = String(cstr1).toInt();
              tempInt2 = String(cstr2).toInt();
              if(tempInt1 < 256 && tempInt2 < 256)
              {
                Menus.updateItem(ledMenu, 0, tempInt1);
                Menus.updateItem(ledMenu, 2, tempInt2);
              }
            }
          }
        }
        else if(foundR == std::string::npos && foundG != std::string::npos && foundB != std::string::npos) // g = (0-255); b = (0-255);
        {
          if(foundG > foundB)
          {
            if(isDigit(value[foundB + 1]) && isDigit(value[foundG + 1]) && 
              std::all_of(value.begin() + (foundB + 1), value.begin() + foundG, ::isdigit) && 
              std::all_of(value.begin() + (foundG + 1), value.end(), ::isdigit))
            {
              temps1 = value.substr(foundB + 1, foundG);
              temps2 = value.substr(foundG + 1);
              char cstr1[temps1.size() + 1];
              char cstr2[temps2.size() + 1];
              strcpy(cstr1, temps1.c_str());
              strcpy(cstr2, temps2.c_str());
              tempInt1 = String(cstr1).toInt();
              tempInt2 = String(cstr2).toInt();
              if(tempInt1 < 256 && tempInt2 < 256)
              {
                Menus.updateItem(ledMenu, 1, tempInt2);
                Menus.updateItem(ledMenu, 2, tempInt1);
              }
            }
          }
          else
          {
            if(isDigit(value[foundB + 1]) && isDigit(value[foundG + 1]) && 
              std::all_of(value.begin() + (foundG + 1), value.begin() + foundB, ::isdigit) && 
              std::all_of(value.begin() + (foundB + 1), value.end(), ::isdigit))
            {
              temps1 = value.substr(foundG + 1, foundB);
              temps2 = value.substr(foundB + 1);
              char cstr1[temps1.size() + 1];
              char cstr2[temps2.size() + 1];
              strcpy(cstr1, temps1.c_str());
              strcpy(cstr2, temps2.c_str());
              tempInt1 = String(cstr1).toInt();
              tempInt2 = String(cstr2).toInt();
              if(tempInt1 < 256 && tempInt2 < 256)
              {
                Menus.updateItem(ledMenu, 1, tempInt1);
                Menus.updateItem(ledMenu, 2, tempInt2);
              }
            }
          }
        }
        else if(foundR != std::string::npos && foundG != std::string::npos && foundB != std::string::npos) // r = (0-255); g = (0-255); b = (0-255);
        {
          int tempArray[] = {foundR, foundG, foundB};
          sort3(tempArray);
          if(isDigit(value[tempArray[0] + 1]) && isDigit(value[tempArray[1] + 1]) && isDigit(value[tempArray[2] + 1]) &&
              std::all_of(value.begin() + (tempArray[0] + 1), value.begin() + tempArray[1], ::isdigit) && 
              std::all_of(value.begin() + (tempArray[1] + 1), value.begin() + tempArray[2], ::isdigit) &&
              std::all_of(value.begin() + (tempArray[2] + 1), value.end(), ::isdigit))
              {
                for(int i = 0; i < 3; i++)
                {
                  if(tempArray[i] == foundR) 
                  {
                    if(foundR == tempArray[2]) temps1 = value.substr(tempArray[i] + 1);
                    else temps1 = value.substr(tempArray[i] + 1, tempArray[i + 1]);
                    char cstr[temps1.size() + 1];
                    strcpy(cstr, temps1.c_str());
                    tempInt1 = String(cstr).toInt();
                    if(tempInt1 < 256)
                      Menus.updateItem(ledMenu, 0, tempInt1);
                  }
                  else if(tempArray[i] == foundG) 
                  {
                    if(foundG == tempArray[2]) temps1 = value.substr(tempArray[i] + 1);
                    else temps1 = value.substr(tempArray[i] + 1, tempArray[i + 1]);
                    char cstr[temps1.size() + 1];
                    strcpy(cstr, temps1.c_str());
                    tempInt1 = String(cstr).toInt();
                    if(tempInt1 < 256)
                      Menus.updateItem(ledMenu, 1, tempInt1);
                  }
                  else if(tempArray[i] == foundB) 
                  {
                    if(foundB == tempArray[2]) temps1 = value.substr(tempArray[i] + 1);
                    else temps1 = value.substr(tempArray[i] + 1, tempArray[i + 1]);
                    char cstr[temps1.size() + 1];
                    strcpy(cstr, temps1.c_str());
                    tempInt1 = String(cstr).toInt();
                    if(tempInt1 < 256)
                      Menus.updateItem(ledMenu, 2, tempInt1);
                  }
                }
              }
        }
        ESPDash.UpdateCard(sliderRed, Menus.getItem(ledMenu, 0)->numValue);
        ESPDash.UpdateCard(sliderGreen, Menus.getItem(ledMenu, 1)->numValue);
        ESPDash.UpdateCard(sliderBlue, Menus.getItem(ledMenu, 2)->numValue);
      }
    }
  }
};

class DHTCallbacks : public BLECharacteristicCallbacks
{
  void onRead(BLECharacteristic* pCharacteristic)
  {
    String tempStr = "Temp-" + String(Menus.getItem(sensorMenu, 0)->fltValue) + "°C" + "\n" +
                      "Hum-" + String(Menus.getItem(sensorMenu, 1)->fltValue) + "%" + "\n" +
                      "HI-" + String(Menus.getItem(sensorMenu, 2)->fltValue) + "°C" + "\n" +
                      "Dew-" + String(Menus.getItem(sensorMenu, 3)->fltValue) + "°C";

    pCharacteristic->setValue(tempStr.c_str());
  }
};

// bluetooth low energy stuff
BLECharacteristic *ledCharacteristic;
BLECharacteristic *dhtCharacteristic;

void drawFrame1(OLEDDisplay *display, OLEDDisplayUiState *state, int16_t x, int16_t y)
{
  display->drawXbm(x + 2, y + 20, image1Width, image1Height, BoteonBits);
}

void drawFrame2(OLEDDisplay *display, OLEDDisplayUiState *state, int16_t x, int16_t y)
{
  display->setTextAlignment(TEXT_ALIGN_CENTER_BOTH);
  display->setFont(ArialMT_Plain_10);
  display->drawString(x + (display->getWidth() / 2), y + (display->getHeight() / 2), Menus.getItem(mainMenu, 1)->name);

  if(Menus.getItem(mainMenu, 1)->numValue)
  {
    if(animationCount < (display->getHeight() * 2))
    {
      display->drawCircle(x + (display->getWidth()/2), y + (display->getHeight()/2), animationCount);
      animationCount += 2;
    } 
    else
    {
      animationCount = 0;
      Menus.updateItem(mainMenu, 1, false);
    }
  }

  display->setTextAlignment(TEXT_ALIGN_LEFT);
}

void bluetoothOverlay(OLEDDisplay *display, OLEDDisplayUiState* state)
{

  display->drawString(95, 1, Ukraine.dateTime("H:i"));

  if(deviceConnected)
    display->drawXbm(80, 1, bluetoothConnectedIconWidth, bluetoothConnectedIconHeight, bluetoothConnectedIconBits);
  else
    display->drawXbm(80, 1, bluetoothDisconnectedIconWidth, bluetoothDisconnectedIconHeight, bluetoothDisconnectedIconBits);
}

void drawItemProgressBar(Item *itemData)
{
  encoder.setBoundaries((itemData->minNumValue / itemData->stepValue), (itemData->maxNumValue / itemData->stepValue), itemData->cycleValues);
  encoder.reset(itemData->numValue / itemData->stepValue);

  display.clear();

  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.drawString(1, 0, itemData->name);

  display.drawProgressBar(4, 22, 120, 15, map(itemData->numValue, itemData->minNumValue, itemData->maxNumValue, 0, 100));
  display.setTextAlignment(TEXT_ALIGN_CENTER_BOTH);
  display.drawString(display.getWidth() / 2, 50, String(itemData->numValue) + itemData->symbol);
  display.setTextAlignment(TEXT_ALIGN_LEFT);

  display.display();
}

void drawItemSwitch(Item *itemData)
{
  encoder.setBoundaries(0, 1, itemData->cycleValues);
  encoder.reset(itemData->numValue);

  display.clear();

  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.drawString(1, 0, itemData->name);

  display.fillRect(14, 20, 100, 28);
  display.setFont(ArialMT_Plain_16);
  if (itemData->numValue)
  {
    display.setColor(BLACK);
    display.fillRect(65, 21, 48, 26);
    display.drawString(22, 26, "ON");
    display.setColor(WHITE);
    display.drawString(72, 26, "OFF");
  }
  else
  {
    display.setColor(BLACK);
    display.fillRect(15, 21, 48, 26);
    display.drawString(72, 26, "OFF");
    display.setColor(WHITE);
    display.drawString(22, 26, "ON");
  }
  display.setFont(ArialMT_Plain_10);
  display.display();
}

void drawItemText(Item *itemData)
{
  display.clear();

  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.drawString(1, 0, itemData->name);

  display.setTextAlignment(TEXT_ALIGN_CENTER_BOTH);
  if(itemData->valueType == Item::FLOAT)
  {
    display.drawString(display.getWidth() / 2, display.getHeight() / 2, String(itemData->fltValue) + itemData->symbol);
  }
  else
  {
    if(itemData->symbol != NULL)
    {
      display.drawString(display.getWidth() / 2, display.getHeight() / 2, processLightSensors(itemData->numValue));
    }
    else
      display.drawString(display.getWidth() / 2, display.getHeight() / 2, itemData->strValue);
  }
    
  display.setTextAlignment(TEXT_ALIGN_LEFT);

  display.display();
}

void drawItemTime(Item *itemData)
{
  encoder.setBoundaries((itemData->minNumValue / itemData->stepValue), (itemData->maxNumValue / itemData->stepValue), itemData->cycleValues);
  encoder.reset(itemData->numValue / itemData->stepValue);

  display.clear();

  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.drawString(1, 0, itemData->name);

  display.setFont(ArialMT_Plain_16);
  display.setTextAlignment(TEXT_ALIGN_CENTER_BOTH);
  display.drawString(display.getWidth() / 2, display.getHeight() / 2, processMinutes(itemData->numValue));
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.setFont(ArialMT_Plain_10);

  display.display();
}

void drawItemScrollWheel(Item *itemData)
{
  int minValue;
  int maxValue;
  int posX = 0;
  int posY = 0;

  encoder.setBoundaries((itemData->minNumValue / itemData->stepValue), (itemData->maxNumValue / itemData->stepValue), itemData->cycleValues);
  encoder.reset(itemData->numValue / itemData->stepValue);

  display.clear();

  display.setFont(ArialMT_Plain_10);
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.drawString(1, 0, itemData->name);

  display.setTextAlignment(TEXT_ALIGN_CENTER);
  
  if((itemData->numValue - 1) >= itemData->minNumValue)
    display.drawString(display.getWidth()/2, (display.getHeight()/2) - 15, String(itemData->numValue - 1));

  display.fillRect(0, (display.getHeight()/2) + 1, display.getWidth(), 16);
  display.setColor(BLACK);
  display.setFont(ArialMT_Plain_16);
  display.drawString(display.getWidth()/2, (display.getHeight()/2), String(itemData->numValue));
  display.setFont(ArialMT_Plain_10);
  display.setColor(WHITE);

  if((itemData->numValue + 1) <= itemData->maxNumValue)
    display.drawString(display.getWidth()/2, (display.getHeight()/2) + 20, String(itemData->numValue + 1));


  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.display();
}

void drawMenuItems(MenuItems *menuData)
{
  int minValue;
  int maxValue;
  int posX = 0;
  int posY = 0;

  String itemStr;
  int index;

  encoder.setBoundaries(0, menuData->itemsAdded - 1, menuData->cycleItems);
  encoder.reset(menuData->prevSelectedItem);

  display.clear();
  display.setColor(WHITE);
  display.drawRect(0, 0, 128, 13);
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.drawString(2, 0, menuData->menuTitle);
  display.drawString(98, 0, Ukraine.dateTime("H:i"));

  if(deviceConnected)
    display.drawXbm(85, 1, bluetoothConnectedIconWidth, bluetoothConnectedIconHeight, bluetoothConnectedIconBits);
  else
    display.drawXbm(85, 1, bluetoothDisconnectedIconWidth, bluetoothDisconnectedIconHeight, bluetoothDisconnectedIconBits);               

  minValue = Menus.getFirstItemToDisplay();
  if(MAXDISPLAYEDITEMS > menuData->itemsAdded) maxValue = menuData->itemsAdded;
  else maxValue = minValue + MAXDISPLAYEDITEMS;

  for (int i = minValue; i < maxValue; i++)
  {
    posX = 6;
    posY = ((display.getHeight() / 5) * (i - menuData->idFirstDisplayedItem + 1)) + 2; // 2 pixels down for a better readability of menu items
    
    itemStr = Menus.getItemString(i);
    index = itemStr.indexOf(':');

    if(Menus.checkItemSelected(i))
    {
      display.fillRect(posX - 6, posY + 1, display.getWidth(), 11);
      display.setColor(BLACK);
    }

    if(index == -1)
    {
      display.drawString(posX, posY, itemStr);
    }
    else
    {
      display.drawString(posX, posY, itemStr.substring(0, index + 1));
      display.drawString(89, posY, itemStr.substring(index + 1));
    }

    display.setColor(WHITE);
  }

  display.setColor(WHITE);
  display.display();
}

void drawCustomMainUI(MenuItems *menuData)
{
  encoder.setBoundaries(0, 1, false);
  encoder.reset(menuData->prevSelectedItem);

  if (ui.getUiState()->frameState != IN_TRANSITION) 
    ui.transitionToFrame(menuData->prevSelectedItem);

  ui.update();
  
  if(ui.getUiState()->frameState == IN_TRANSITION || ui.getUiState()->ticksSinceLastStateSwitch == localTicksSinceLastStateSwitch) 
    menuData->changed = true;
  else
    localTicksSinceLastStateSwitch = ui.getUiState()->ticksSinceLastStateSwitch;
}

void drawMainMenuAnimation(Item *itemData)
{
  if(!itemData->numValue) itemData->numValue = true;

  if(itemData->numValue) itemData->changed = true;
  
  ui.update();
}

void changedRed(CardData *cData)
{
  Menus.updateItem(ledMenu, 0, cData->value_i);
}

void changedGreen(CardData *cData)
{
  Menus.updateItem(ledMenu, 1, cData->value_i);
}

void changedBlue(CardData *cData)
{
  Menus.updateItem(ledMenu, 2, cData->value_i); 
}

void changedLED(CardData *cData)
{
  Menus.updateItem(ledMenu, 3, cData->value_i);
}

void changedButton(CardData *cData)
{
  ledSwitch = !ledSwitch;
}

void onMqttConnect(bool sessionPresent)
{
  Serial.println("Connected to MQTT.");
}

void onMqttDisconnect(AsyncMqttClientDisconnectReason reason)
{
  Serial.println("Disconnected from MQTT.");
  if (reason == AsyncMqttClientDisconnectReason::TLS_BAD_FINGERPRINT) {
    Serial.println("Bad server fingerprint.");
  }
  else if(reason == AsyncMqttClientDisconnectReason::MQTT_UNACCEPTABLE_PROTOCOL_VERSION)
    Serial.println("MQTT_UNACCEPTABLE_PROTOCOL_VERSION");
  else if(reason == AsyncMqttClientDisconnectReason::MQTT_IDENTIFIER_REJECTED)
    Serial.println("MQTT_IDENTIFIER_REJECTED");
  else if(reason == AsyncMqttClientDisconnectReason::MQTT_SERVER_UNAVAILABLE)
    Serial.println("MQTT_SERVER_UNAVAILABLE");
  else if(reason == AsyncMqttClientDisconnectReason::MQTT_MALFORMED_CREDENTIALS)
    Serial.println("MQTT_MALFORMED_CREDENTIALS");
  else if(reason == AsyncMqttClientDisconnectReason::MQTT_NOT_AUTHORIZED)
    Serial.println("MQTT_NOT_AUTHORIZED");
  else if(reason == AsyncMqttClientDisconnectReason::ESP8266_NOT_ENOUGH_SPACE)
    Serial.println("ESP8266_NOT_ENOUGH_SPACE");
  else if(reason == AsyncMqttClientDisconnectReason::TCP_DISCONNECTED)
    Serial.println("TCP_DISCONNECTED");
}

void saveItemValueToNVS(Item *item)
{
  preferences.begin("greenhouse", false);

  if(item->numValue != preferences.getInt(saveItems[item->id].saveName))
    preferences.putInt(saveItems[item->id].saveName, item->numValue);

  preferences.end();

}

void setBuzzerTimer()
{
  if (buzzerCounter < 25 && Menus.getItem(buzzerMenu, 1)->numValue)
  {
    buzzerCounter++;
    ledcWriteTone(BUZZER_CH, 1000);
    ledcWrite(BUZZER_CH, map(Menus.getItem(buzzerMenu, 0)->numValue, 0, 20, 0, 255));
  }
  else
  {
    buzzerTicker.detach();
    ledcWrite(BUZZER_CH, 0);
    buzzerCounter = 0;
  }
}

void webUIUpdateEvent()
{
  //ESPDash.UpdateCard(chartTemp, temperature, timeString);
  ESPDash.UpdateCard(sliderRed, Menus.getItem(ledMenu, 0)->numValue);
  ESPDash.UpdateCard(sliderGreen, Menus.getItem(ledMenu, 1)->numValue);
  ESPDash.UpdateCard(sliderBlue, Menus.getItem(ledMenu, 2)->numValue);
  ESPDash.UpdateCard(sliderLED, Menus.getItem(ledMenu, 3)->numValue);

  ESPDash.SendUpdates();

  setEvent(webUIUpdateEvent, (UTC.now() + 5));
}

void wateringEvent()
{
  /* if(needToStartPump)
  {

  } */
  
  // Serial.println("watering");

  //setEvent(wateringEvent, (UTC.now() + 30));
}

void minuteRoutine()
{
  if(isDayTime)
  {
    // check light (photoresistors)
    if(Menus.getItem(sensorMenu, 6)->numValue == NOT_ENOUGH_LIGHT)
      //if(ledSwitchON) turnLightOn();
      // Serial.println("Turn on light");

    // check soil moisture
    if(Menus.getItem(sensorMenu, 5)->numValue < Menus.getItem(automationMenu, 8)->numValue)
    {
      if(!isWatering)
      {
        // Serial.println("start watering");
        isWatering = true;
        // start pump
      }
    }

    /* // check co2 level
    if(Menus.getItem(sensorMenu, 4)->numValue < Menus.getItem(automationMenu, 8)->numValue)
    {
      // send warning?
    } */
   

    // check temp and humidity
    if(round(Menus.getItem(sensorMenu, 0)->fltValue) > Menus.getItem(automationMenu, 6)->numValue && 
       round(Menus.getItem(sensorMenu, 1)->fltValue) > Menus.getItem(automationMenu, 7)->numValue)
    {
      if(!isVentilating)
      {
        // Serial.println("start ventilating");
        fanIsOn = UTC.now();
        isVentilating = true;
        // start fans
      }
    }
  }
  else
  {
    
  }
  
  setEvent(minuteRoutine, (UTC.now() + 10));
}

void thingspeakSyncEvent()
{
  String data = String("field1=") + String(Menus.getItem(sensorMenu, 0)->fltValue) + "&field2=" + String(Menus.getItem(sensorMenu, 1)->fltValue) + "&field4=" + String(Menus.getItem(sensorMenu, 5)->fltValue);
  const char *msgBuffer;
  msgBuffer=data.c_str();

  String topicString = "channels/" + String( channelID ) + "/publish/"+String(apiKey);
  const char *topicBuffer;
  topicBuffer = topicString.c_str();

  mqttClient.publish(topicBuffer, 0, false, msgBuffer);

  setEvent(thingspeakSyncEvent, (UTC.now() + 60));
}

void mqttConnectEvent()
{
  mqttClient.connect();
}

void getSensorData()
{
  int readData1;
  int readData2;
  String strData;

  readData1 = digitalRead(PHOTORESISTOR_LEFT_PIN);
  readData2 = digitalRead(PHOTORESISTOR_RIGHT_PIN);

  if(readData1 == HIGH && readData2 == HIGH)
  {
    Menus.updateItem(sensorMenu, 6, 2);
  }
  else if(readData1 == HIGH || readData2 == HIGH)
  {
    Menus.updateItem(sensorMenu, 6, 1);
  }
  else
  {
    Menus.updateItem(sensorMenu, 6, 0);
  }

  readData1 = analogRead(SOILMOISTURE_PIN);
  float soil = map(readData1, 4095, 0, 0, 100);
  Menus.updateItem(sensorMenu, 5, soil);

  readData1 = analogRead(MQ135_PIN);
  float mq135 = map(readData1, 4095, 0, 0, 100);
  Menus.updateItem(sensorMenu, 4, mq135);
}

void storageSetup()
{
  preferences.begin("greenhouse", false);

  wateringFreq = preferences.getInt("wFreq", WATERING_FREQ);
  wateringTime = preferences.getInt("wTime", WATERING_TIME);
  wateringDur  = preferences.getInt("wDur",  WATERING_DUR);
  ventilationFreq = preferences.getInt("vFreq", VENTILATION_FREQ);
  ventilationTime = preferences.getInt("vTime", VENTILATION_TIME);
  ventilationDur  = preferences.getInt("vDur",  VENTILATION_DUR);
  optimalTemp = preferences.getInt("optTemp", OPTIMAL_TEMP);
  optimalHum  = preferences.getInt("optHum",  OPTIMAL_HUM);
  optimalSM   = preferences.getInt("optSM",   OPTIMAL_SM);
  thingspeakSyncTime = preferences.getInt("syncTime", SYNC_TIME);

  preferences.end();
}

void sensorSetup()
{
  ledcSetup(0, 30000, 8);
  ledcAttachPin(PUMP_PIN, 0);

  ledcSetup(1, 10000, 8);
  ledcAttachPin(FAN_PIN, 1);

  pinMode(PHOTORESISTOR_LEFT_PIN, INPUT); 
  pinMode(PHOTORESISTOR_RIGHT_PIN, INPUT);

  //pinMode(BUTTON_PIN1, INPUT);
  //pinMode(BUTTON_PIN2, INPUT);

  pinMode(SOILMOISTURE_PIN, INPUT);

  sensorTicker.attach(5, getSensorData);

  setEvent(thingspeakSyncEvent, (UTC.now() + 60));
  setEvent(minuteRoutine, (UTC.now() + 30));
}

void dhtSetup()
{
  // Initialize temperature sensor
	dht11.setup(DHT_PIN, DHT_TYPE);
}

void dhtCallback()
{
	TempAndHumidity newValues = dht11.getTempAndHumidity();

	// Check if any reads failed and exit early (to try again).
	if (dht11.getStatus() != 0) {
		// Serial.println("DHT11 error status: " + String(dht11.getStatusString()));
    return;
	}

  temperature = newValues.temperature;
  humidity = newValues.humidity;
	heatIndex = dht11.computeHeatIndex(newValues.temperature, newValues.humidity);
  dewPoint = dht11.computeDewPoint(newValues.temperature, newValues.humidity);
  
  Menus.updateItem(sensorMenu, 0, temperature);
  Menus.updateItem(sensorMenu, 1, humidity);
  Menus.updateItem(sensorMenu, 2, dewPoint);
  Menus.updateItem(sensorMenu, 3, heatIndex);
}

void ledSetup()
{
  // передавання налаштувань LED плати в бібліотеку FastLED
  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(BRIGHTNESS);
}

void wifiSetup()
{
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  wifiManager.autoConnect("AutoConnectAP");

  AsyncElegantOTA.begin(&server);

  mqttClient.onConnect(onMqttConnect);
  mqttClient.onDisconnect(onMqttDisconnect);
  mqttClient.setCredentials(mqttUserName, mqttPass);
  mqttClient.setServer(thingspeakServer, 1883);

  // ezTime library
  setDebug(INFO);
  waitForSync();

  if (!Ukraine.setCache(0))
  {
    Ukraine.setPosix("EET-2");
  }

  if(Ukraine.hour() >= 6 && Ukraine.hour() <= 18)
    isDayTime = true;
  else
    isDayTime = false;

  
}

void webuiSetup()
{
  // Initiate ESPDash and attach your Async webserver instance
  ESPDash.init(&server);

  //add and configure cards
  sliderRed = ESPDash.AddCard(7, "Red", 0, 255);
  sliderGreen = ESPDash.AddCard(7, "Green", 0, 255);
  sliderBlue = ESPDash.AddCard(7, "Blue", 0, 255);
  sliderLED = ESPDash.AddCard(7, "Led", 0, 8, 2);
  ledButton = ESPDash.AddCard(1, "Switch");
  //chartTemp = ESPDash.AddCard(5, "Temperature", "°C");

  ESPDash.UpdateCard(sliderRed, *changedRed);  
  ESPDash.UpdateCard(sliderGreen, *changedGreen);
  ESPDash.UpdateCard(sliderBlue, *changedBlue);
  ESPDash.UpdateCard(sliderLED, *changedLED);
  ESPDash.UpdateCard(ledButton, *changedButton);

  /* if (!time(&now))
  {
    return;
  }

  timeString = (String) now + String("000"); */

  //ESPDash.setTime(timeString);

  mqttClient.connect();

  setEvent(webUIUpdateEvent, (UTC.now() + 10));
}

void uiSetup()
{
  frames[0] = drawFrame1;
  frames[1] = drawFrame2;

  overlays[0] = bluetoothOverlay;

  ui.setTargetFPS(60);
  ui.setFrameAnimation(SLIDE_LEFT);
  ui.disableAutoTransition();
  ui.setFrames(frames, frameCount);
  ui.setOverlays(overlays, overlaysCount);
  ui.init();
}

void encoderSetup()
{
  //we must initialize rotary encoder
  encoder.begin();
  encoder.setup([] { encoder.readEncoder_ISR(); });
  //optionally we can set boundaries and if values should cycle or not
  //minValue, maxValue, cycle values (when max go to min and vice versa)
  encoder.setBoundaries(encoderMinValue, encoderMaxValue, encoderCycle);
}

void buzzerSetup()
{
  ledcSetup(BUZZER_CH, BUZZER_FREQ, BUZZER_RES);
  ledcAttachPin(BUZZER_PIN, BUZZER_CH);
}

void bleSetup()
{
  BLEDevice::init(BLUETOOTH_NAME);
  BLEServer *pServer = BLEDevice::createServer();

  pServer->setCallbacks(new ServerCallbacks());

  BLEService *pService = pServer->createService(SERVICE_UUID);

  ledCharacteristic = pService->createCharacteristic(
      LEDCHARACTERISTIC_UUID,
      BLECharacteristic::PROPERTY_READ |
          BLECharacteristic::PROPERTY_WRITE);

  ledCharacteristic->setCallbacks(new LedCallbacks());

  dhtCharacteristic = pService->createCharacteristic(
      DHTCHARACTERISTIC_UUID,
          BLECharacteristic::PROPERTY_READ);

  dhtCharacteristic->setCallbacks(new DHTCallbacks());

  pService->start();

  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06); // functions that help with iPhone connections issue
  //pAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();

  std::string macStr = BLEDevice::getAddress().toString();
  char cstr[macStr.size()];
  strcpy(cstr, macStr.c_str());
  String bluetoothMAC = String(cstr);
  bluetoothMAC.toUpperCase();
  Menus.updateItem(bleMenu, 1, bluetoothMAC.c_str());
}

void initMenus()
{
  Menus.init(MAXDISPLAYEDITEMS, drawMenuItems, saveItemValueToNVS);

       mainMenu = Menus.createMenu(mNames[menus::main].name, mNames[menus::main].itemStrings, sizeof(mainMenuItems) / sizeof(mainMenuItems[0]), false, drawCustomMainUI);
   settingsMenu = Menus.createMenu(mNames[menus::settings].name, mNames[menus::settings].itemStrings, sizeof(settingsMenuItems) / sizeof(settingsMenuItems[0]));
     buzzerMenu = Menus.createMenu(mNames[menus::buzzer].name, mNames[menus::buzzer].itemStrings, sizeof(buzzerMenuItems) / sizeof(buzzerMenuItems[0]));
        ledMenu = Menus.createMenu(mNames[menus::led].name, mNames[menus::led].itemStrings, sizeof(ledMenuItems) / sizeof(ledMenuItems[0]));
     sensorMenu = Menus.createMenu(mNames[menus::sensors].name, mNames[menus::sensors].itemStrings, sizeof(sensorsMenuItems) / sizeof(sensorsMenuItems[0]));
 automationMenu = Menus.createMenu(mNames[menus::automation].name, mNames[menus::automation].itemStrings, sizeof(automationMenuItems) / sizeof(automationMenuItems[0]));
        bleMenu = Menus.createMenu(mNames[menus::ble].name, mNames[menus::ble].itemStrings, sizeof(bleMenuItems) / sizeof(bleMenuItems[0]));

  Menus.createItem(mainMenu, menus::settings);
  Menus.createItem(mainMenu, false, drawMainMenuAnimation);

  Menus.createItem(settingsMenu, menus::buzzer);
  Menus.createItem(settingsMenu, menus::led);
  Menus.createItem(settingsMenu, menus::sensors);
  Menus.createItem(settingsMenu, menus::automation);
  Menus.createItem(settingsMenu, menus::ble);
  Menus.createItem(settingsMenu, menus::main);

  Menus.createItem(buzzerMenu, false, " %", drawItemProgressBar, 100, 0, 5);
  Menus.createItem(buzzerMenu, false, drawItemSwitch);
  Menus.createItem(buzzerMenu, menus::settings);

  Menus.createItem(ledMenu, true, "", drawItemProgressBar, 255, 0, 10);
  Menus.createItem(ledMenu, true, "", drawItemProgressBar, 255, 0, 10);
  Menus.createItem(ledMenu, true, "", drawItemProgressBar, 255, 0, 10);
  Menus.createItem(ledMenu, false, "", drawItemProgressBar, 8);
  Menus.createItem(ledMenu, menus::settings);

  Menus.createItem(sensorMenu, 0.0, " °C", drawItemText);
  Menus.createItem(sensorMenu, 0.0, " %", drawItemText);
  Menus.createItem(sensorMenu, 0.0, " °C", drawItemText);
  Menus.createItem(sensorMenu, 0.0, " °C", drawItemText);
  Menus.createItem(sensorMenu, 0.0, " ppm", drawItemText);
  Menus.createItem(sensorMenu, 0.0, " %", drawItemText);
  Menus.createItem(sensorMenu, "", drawItemText, false, 1, processLightSensors);
  Menus.createItem(sensorMenu, menus::settings);

  Menus.createItem(automationMenu, false, "", drawItemScrollWheel, 7, 1, 1, true);               // need to process days
  Menus.createItem(automationMenu, true, "", drawItemTime, 1440, 1, 5, true, processMinutes);    // process as minutes, step 5 minutes
  Menus.createItem(automationMenu, false, "", drawItemScrollWheel, 10, 1, 1, true);              // need to process duration time
  Menus.createItem(automationMenu, false, "", drawItemScrollWheel, 7, 1, 1, true);               // need to process days
  Menus.createItem(automationMenu, true, "", drawItemTime, 1440, 1, 5, true, processMinutes);    // process as minutes, step 5 minutes
  Menus.createItem(automationMenu, false, "", drawItemScrollWheel, 10, 1, 1, true);              // need to process duration time
  Menus.createItem(automationMenu, false, " °C", drawItemScrollWheel, 35, 0, 1, true);           // optimal temp
  Menus.createItem(automationMenu, false, " %", drawItemScrollWheel, 95, 5, 5, true);            // optimal humidity
  Menus.createItem(automationMenu, false, " %", drawItemScrollWheel, 95, 5, 5, true);            // optimal soil moisture
  Menus.createItem(automationMenu, true, "", drawItemTime, 1440, 1, 5, true, processMinutes);       // process as minutes
  Menus.createItem(automationMenu, true, "", drawItemTime, 1440, 1, 5, true, processMinutes);       // process as minutes
  Menus.createItem(automationMenu, true, "", drawItemTime, 1440, 1, 5, true, processMinutes);       // process as minutes
  Menus.createItem(automationMenu, menus::settings);

  Menus.createItem(bleMenu, BLUETOOTH_NAME, drawItemText);
  Menus.createItem(bleMenu, "", drawItemText);
  Menus.createItem(bleMenu, menus::settings);

  Menus.updateItem(automationMenu, 0, wateringFreq);
  Menus.updateItem(automationMenu, 1, wateringTime);
  Menus.updateItem(automationMenu, 2, wateringDur);
  Menus.updateItem(automationMenu, 3, ventilationFreq);
  Menus.updateItem(automationMenu, 4, ventilationTime);
  Menus.updateItem(automationMenu, 5, ventilationDur);
  Menus.updateItem(automationMenu, 6, optimalTemp);
  Menus.updateItem(automationMenu, 7, optimalHum);
  Menus.updateItem(automationMenu, 8, optimalSM);
  Menus.updateItem(automationMenu, 9, thingspeakSyncTime);

  Menus.setCurrentMenu(mainMenu);
}

void menuLoop()
{
  if (encoder.currentButtonState() == BUT_RELEASED)
  {
    buzzerTicker.attach_ms(5, setBuzzerTimer);
    Menus.prepareMenu();
  }

  int16_t encoderDelta = encoder.encoderChanged();

  if (encoderDelta != 0)
  {
    encoderValue = encoder.readEncoder();
    Menus.setSelected(encoderValue);
  }

  Menus.update();
}

void ledLoop()
{
  if(Menus.getItem(ledMenu, 3)->numValue > 0)   // "switch LEDS" - menu item
  {
    if(Menus.getItem(ledMenu, 3)->numValue != ledsOnCount ||
        Menus.getItem(ledMenu, 0)->numValue != currentRed ||
        Menus.getItem(ledMenu, 1)->numValue != currentGreen ||
        Menus.getItem(ledMenu, 2)->numValue != currentBlue ||
        !leds[0])
        {
          currentRed = Menus.getItem(ledMenu, 0)->numValue;
          currentGreen = Menus.getItem(ledMenu, 1)->numValue;
          currentBlue = Menus.getItem(ledMenu, 2)->numValue;
          ledsOnCount = Menus.getItem(ledMenu, 3)->numValue;

          for(int i = 0; i < NUM_LEDS; i++)
          {
            if(i < ledsOnCount) 
              leds[i] = CRGB(currentRed, currentGreen, currentBlue);
            else leds[i] = CRGB::Black;
          }
        }
  }
  else 
  {
    for(int i = 0; i < NUM_LEDS; i++)
    {
      leds[i] = CRGB::Black;
    }
  }
  
  FastLED.show();
}

void fanLoop()
{
  if(isVentilating)
  {
    if(fanSpeed < 255) fanSpeed++;
    if(UTC.now() >= (fanIsOn + Menus.getItem(automationMenu, 5)->numValue))
    { 
      fanSpeed = 0;
      isVentilating = false;
    }
  }
}

void pumpLoop()
{
  if(isWatering)
  {
    if(pumpSpeed < 255) pumpSpeed++;
    if(UTC.now() >= (pumpIsOn + Menus.getItem(automationMenu, 2)->numValue)) 
    {
      pumpSpeed = 0;
      isWatering = false;
    }
  }
}

/****************************************** -> SETUP AND MAIN LOOP <- *********************************************************/

void setup()
{
  Serial.begin(115200);

  storageSetup();

  initMenus();

  wifiSetup();

  buzzerSetup();

  ledSetup();

  uiSetup();

  encoderSetup();

  bleSetup();

  dhtSetup();

  webuiSetup();

  sensorSetup();

  Menus.update();
}

void loop()
{
  menuLoop();

  ledLoop();

  fanLoop();

  pumpLoop();

  ledcWrite(0, pumpSpeed);

  ledcWrite(0, fanSpeed);

  //wifiManager.loop();
  
  AsyncElegantOTA.loop();

  events();

  ts.execute(); // task scheduler loop function
}