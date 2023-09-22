#include "Arduino.h"  // Подключение стандартной библиотеки Arduino
#include "arduino_homekit_server.h"  // Подключение библиотеки для работы с HomeKit
#include "ESPButton.h"  // Подключение библиотеки для работы с кнопками на ESP
#include "WiFiConfig.h"  // Подключение заголовочного файла класса WiFiConfig

extern "C" homekit_server_config_t config;  // Объявление конфигурации HomeKit
extern "C" homekit_characteristic_t name;  // Объявление характеристики "имя" в HomeKit
extern "C" char ACCESSORY_NAME[32];  // Объявление массива для хранения имени аксессуара
extern "C" void accessory_init();  // Объявление функции для инициализации аксессуара

#define PIN_BTN 0  // Определение пина для кнопки
#define PIN_LED 2  // Определение пина для LED

// Создание объекта класса WiFiConfig
WiFiConfig wifiConfig("Orange_Swiatlowod_AFEA_2.4", "R7EP375RFJHF");

void setup() {
  Serial.begin(115200);  // Инициализация последовательного порта

  wifiConfig.connectToWiFi();  // Подключение к Wi-Fi с использованием метода класса

  pinMode(PIN_LED, OUTPUT);  // Установка режима работы пина для LED

  // Вывод информации о системе
  printf("\n");
  printf("SketchSize: %d B\n", ESP.getSketchSize());
  printf("FreeSketchSpace: %d B\n", ESP.getFreeSketchSpace());
  printf("FlashChipSize: %d B\n", ESP.getFlashChipSize());
  printf("FlashChipSpeed: %d\n", ESP.getFlashChipSpeed());
  printf("SdkVersion: %s\n", ESP.getSdkVersion());
  printf("FreeHeap: %d B\n", ESP.getFreeHeap());

  // Инициализация кнопки
  pinMode(PIN_BTN, INPUT_PULLUP);
  ESPButton.add(0, PIN_BTN, LOW, false, true);
  ESPButton.setCallback([](uint8_t id, ESPButtonEvent event) {
    printf("ButtonEvent: %s\n", ESPButton.getButtonEventDescription(event));
    if (event == ESPBUTTONEVENT_SINGLECLICK) {
      led_toggle();
    } else if (event == ESPBUTTONEVENT_DOUBLECLICK) {
      // Действие для двойного клика
    } else if (event == ESPBUTTONEVENT_LONGCLICK) {
      homekit_storage_reset();
      esp_restart();
    }
  });
  ESPButton.begin();
  homekit_setup();
}

extern "C" void led_toggle() {
  static bool ledState = false;
  ledState = !ledState;
  digitalWrite(PIN_LED, ledState ? HIGH : LOW);
}

void loop() {
  ESPButton.loop();
  uint32_t time = millis();
  static uint32_t next_heap_millis = 0;
  if (time > next_heap_millis) {
    next_heap_millis = time + 5000;
  }
  delay(5);
}

void homekit_setup() {
  accessory_init();
  arduino_homekit_setup(&config);
}
