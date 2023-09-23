#ifndef WIFI_CONFIG_H  // Проверка, определен ли заголовок
#define WIFI_CONFIG_H  // Определение заголовка

#include "Arduino.h"  // Подключение стандартной библиотеки Arduino
#include "WiFi.h"  // Подключение библиотеки для работы с Wi-Fi

// Объявление класса WiFiConfig
class WiFiConfig {
public:
  WiFiConfig(const char* initialSSID, const char* initialPassword);  // Конструктор с параметрами SSID и пароля
  void connectToWiFi();  // Метод для подключения к WiFi
  void changePassword(const char* newPassword);  // Метод для изменения пароля
private:
  const char* ssid;  // Приватная переменная для хранения SSID
  const char* password;  // Приватная переменная для хранения пароля
};

#endif // WIFI_CONFIG_H  // Закрытие условной директивы
