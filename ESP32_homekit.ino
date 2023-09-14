#include "Arduino.h"
#include "WiFi.h"
#include "arduino_homekit_server.h"
#include "ESPButton.h"

extern "C" homekit_server_config_t config;
extern "C" homekit_characteristic_t name;
extern "C" char ACCESSORY_NAME[32];
// extern "C" void led_toggle();
extern "C" void accessory_init();

#define PIN_BTN 0
#define PIN_LED 2
const char *ssid = "Orange_Swiatlowod_AFEA_2.4";
const char *password = "R7EP375RFJHF";

void setup() {
  Serial.begin(115200);

  WiFi.mode(WIFI_STA);
  WiFi.persistent(false);
  WiFi.disconnect(false);
  WiFi.setAutoReconnect(true);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Подключение к Wi-Fi...");
  }
  Serial.println("Подключено к Wi-Fi");
  pinMode(PIN_LED, OUTPUT);
	printf("\n");
	printf("SketchSize: %d B\n", ESP.getSketchSize());
	printf("FreeSketchSpace: %d B\n", ESP.getFreeSketchSpace());
	printf("FlashChipSize: %d B\n", ESP.getFlashChipSize());
	//printf("FlashChipRealSize: %d B\n", ESP.getFlashChipRealSize());
	printf("FlashChipSpeed: %d\n", ESP.getFlashChipSpeed());
	printf("SdkVersion: %s\n", ESP.getSdkVersion());
	//printf("FullVersion: %s\n", ESP.getFullVersion().c_str());
	//printf("CpuFreq: %dMHz\n", ESP.getCpuFreqMHz());
	printf("FreeHeap: %d B\n", ESP.getFreeHeap());
	//printf("ResetInfo: %s\n", ESP.getResetInfo().c_str());
	//printf("ResetReason: %s\n", ESP.getResetReason().c_str());
	DEBUG_HEAP();

	//Init Button
	pinMode(PIN_BTN, INPUT_PULLUP);
	ESPButton.add(0, PIN_BTN, LOW, false, true);
	ESPButton.setCallback([](uint8_t id, ESPButtonEvent event) {
		printf("ButtonEvent: %s\n", ESPButton.getButtonEventDescription(event));
		if (event == ESPBUTTONEVENT_SINGLECLICK) {
			led_toggle();
		} else if (event == ESPBUTTONEVENT_DOUBLECLICK) {

		} else if (event == ESPBUTTONEVENT_LONGCLICK) {
			homekit_storage_reset();
			esp_restart();
		}
	});
	ESPButton.begin();
	homekit_setup();
}

extern "C" void led_toggle() {
  static bool ledState = false;  // Состояние лампочки (вкл/выкл)
  ledState = !ledState;  // Переключение состояния
  digitalWrite(PIN_LED, ledState ? HIGH : LOW);  // Установка напряжения на пине D4
}

void loop() {
	ESPButton.loop();
	uint32_t time = millis();
	static uint32_t next_heap_millis = 0;
	if (time > next_heap_millis) {
		INFO("heap: %u, sockets: %d", ESP.getFreeHeap(), arduino_homekit_connected_clients_count());
		next_heap_millis = time + 5000;
	}
	delay(5);
}

void homekit_setup() {
	accessory_init();
	// We create one FreeRTOS-task for HomeKit
	// No need to call arduino_homekit_loop
	arduino_homekit_setup(&config);
}
