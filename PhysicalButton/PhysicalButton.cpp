#include "PhysicalButton.h"

// Конструктор
PhysicalButton::PhysicalButton() {
  // Инициализация переменных и ресурсов здесь
}

// Деструктор
PhysicalButton::~PhysicalButton() {
  // Освобождение ресурсов здесь
}

// Инициализация
void PhysicalButton::begin() {
  // Инициализация таймера и других ресурсов
  ticker.attach_ms(10, [this] { this->tick(); });
}

// Добавление новой кнопки
ButtonEntry* PhysicalButton::add(uint8_t _id, uint8_t _pin, uint8_t _pin_down_digital, bool _doubleclick_enable, bool _longclick_enable) {
  // Создание и инициализация новой записи о кнопке
  ButtonEntry* newEntry = new ButtonEntry;
  newEntry->id = _id;
  newEntry->pin = _pin;
  newEntry->pin_down_digital = _pin_down_digital;
  newEntry->doubleclick_enable = _doubleclick_enable;
  newEntry->longclick_enable = _longclick_enable;
  newEntry->next = entries;
  entries = newEntry;
  return newEntry;
}

// Установка коллбэка
void PhysicalButton::setCallback(physicalbutton_callback _callback) {
  callback = _callback;
}

// Обработка тиков
void PhysicalButton::tick() {
  // Обход всех записей и вызов tickEntry для каждой
  ButtonEntry* entry = entries;
  while (entry != nullptr) {
    tickEntry(entry);
    entry = entry->next;
  }
}

// Обработка в основном цикле
void PhysicalButton::loop() {
  // Если есть событие для уведомления, вызываем коллбэк
  if (notify_event != PHYSICALBUTTONEVENT_NONE) {
    callback(notify_id, notify_event);
    notify_event = PHYSICALBUTTONEVENT_NONE;
  }
}

// Чтение состояния кнопки
bool PhysicalButton::digitalReadEntryIsDown(ButtonEntry *entry) {
  // Чтение состояния кнопки и возврат true, если она нажата
  return digitalRead(entry->pin) == entry->pin_down_digital;
}

// Обработка одной кнопки
void PhysicalButton::tickEntry(ButtonEntry *entry) {
  // Логика обработки состояния кнопки
  bool isDown = digitalReadEntryIsDown(entry);
  if (isDown) {
    // Обработка нажатия кнопки
    handleEntryChanged(entry);
  } else {
    // Обработка отпускания кнопки
    handleEntryUnchanged(entry);
  }
}

// Обработка изменения состояния кнопки
void PhysicalButton::handleEntryChanged(ButtonEntry *entry) {
  // Логика обработки изменения состояния кнопки
  if (entry->longclick_enable && !entry->longclicked) {
    if (millis() - entry->down_time >= entry->longclick_threshold) {
      entry->longclicked = true;
      notifyEvent(entry, PHYSICALBUTTONEVENT_LONGCLICK);
    }
  }
}

// Обработка неизменного состояния кнопки
void PhysicalButton::handleEntryUnchanged(ButtonEntry *entry) {
  // Логика обработки неизменного состояния кнопки
  if (entry->doubleclick_enable && entry->wait_doubleclick) {
    if (millis() - entry->up_time >= entry->doubleclick_threshold) {
      entry->wait_doubleclick = false;
      notifyEvent(entry, PHYSICALBUTTONEVENT_SINGLECLICK);
    }
  }
}

// Уведомление о событии
void PhysicalButton::notifyEvent(ButtonEntry *entry, PhysicalButtonEvent event) {
  // Установка события и ID для уведомления в основном цикле
  notify_event = event;
  notify_id = entry->id;
}
