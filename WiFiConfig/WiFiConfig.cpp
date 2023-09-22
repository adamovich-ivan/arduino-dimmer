#include "WiFiConfig.h"  // Подключение заголовочного файла класса

// Конструктор класса, инициализирует переменные ssid и password
WiFiConfig::WiFiConfig(const char* initialSSID, const char* initialPassword) : ssid(initialSSID), password(initialPassword) {}

// Метод для подключения к WiFi
void WiFiConfig::connectToWiFi() {
  WiFi.mode(WIFI_STA);  // Установка режима работы WiFi в режим станции
  WiFi.persistent(false);  // Отключение постоянного сохранения настроек WiFi
  WiFi.disconnect(false);  // Отключение от предыдущей WiFi сети, если было подключение
  WiFi.setAutoReconnect(true);  // Включение автоматического переподключения

  WiFi.begin(ssid, password);  // Начало подключения к WiFi сети с заданными SSID и паролем

  // Цикл, который ждет подключения к WiFi
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);  // Задержка в 1 секунду
    Serial.println("Подключение к Wi-Fi...");  // Вывод сообщения о статусе подключения
  }
  Serial.println("Подключено к Wi-Fi");  // Вывод сообщения о успешном подключении
}

// Метод для изменения пароля
void WiFiConfig::changePassword(const char* newPassword) {
  // Обновление переменной 'password'
  password = newPassword;

  // Переподключение к WiFi с новым паролем
  connectToWiFi();
}
