#ifndef PHYSICAL_BUTTON_H // Проверка, определен ли заголовок
#define PHYSICAL_BUTTON_H // Определение заголовка

#include <Arduino.h> // Подключение Arduino библиотеки
#include <Ticker.h> // Подключение библиотеки Ticker
#include <functional> // Подключение библиотеки functional для std::function

// Определение перечисления для событий кнопки
enum PhysicalButtonEvent {
  PHYSICALBUTTONEVENT_NONE = 0,
  PHYSICALBUTTONEVENT_SINGLECLICK,
  PHYSICALBUTTONEVENT_DOUBLECLICK,
  PHYSICALBUTTONEVENT_LONGCLICK
};


// Определение структуры для хранения информации о кнопке
typedef struct {
  // ... (все поля из вашей структуры ESPButtonEntry)
  	uint8_t id = -1;
	uint8_t pin = -1;
	uint8_t pin_down_digital = LOW; // цифровое значение при нажатии

	bool stable_down = false; // стабильное состояние кнопки (true: нажата, false: отпущена)
	uint32_t stable_threshold = 40; // если состояние кнопки не меняется в течение этого времени, считается стабильным
	bool is_stable = false;	// текущее состояние стабильно или нет
	bool raw_down = false; // исходное состояние кнопки без антидребезга
	uint32_t raw_changed_time = 0; // время последнего изменения исходного состояния

	//void (*ext_digitalRead)(uint8_t pin);
	// если этот пин не является пином ESP, а считывается с другого расширяющего чипа, нужно передать ext_digitalRead
	std::function<uint8_t(uint8_t pin)> ext_digitalRead = nullptr;
	//======
	bool longclicked = false; // чтобы гарантировать, что долгое нажатие срабатывает только один раз
	bool down_handled = false; // обработано ли текущее нажатие (например, уже сработало долгое нажатие)
	// нажатие->отпускание, если в течение определенного времени снова нажать, то это двойной клик, иначе одиночный клик
	bool wait_doubleclick = false; // ожидается ли двойной клик
	uint32_t down_time = 0; // время в мс
	uint32_t up_time = 0;

	uint32_t longclick_threshold = 5000;
	uint32_t doubleclick_threshold = 150; // если после отпускания кнопки в течение этого времени снова нажать, считается двойным кликом

	bool longclick_enable = true;
	bool doubleclick_enable = true;
	//======
	struct _ESPButtonEntry *next;
} ButtonEntry;



class PhysicalButton {
public:
  PhysicalButton(int pin);
  // Определение типа для коллбэка
  typedef std::function<void(uint8_t id, PhysicalButtonEvent event)> physicalbutton_callback;

  // Конструктор и деструктор
  PhysicalButton();
  ~PhysicalButton();

  // Инициализация
  void begin();

  // Добавление новой кнопки
  ButtonEntry* add(uint8_t _id, uint8_t _pin, uint8_t _pin_down_digital, bool _doubleclick_enable = false, bool _longclick_enable = true);

  // Установка коллбэка
  void setCallback(physicalbutton_callback _callback);

  // Обработка тиков
  void tick();

  // Обработка в основном цикле
  void loop();

private:
  // Объект для таймера
  Ticker ticker;

  // Список всех кнопок
  ButtonEntry *entries = nullptr;

  // Коллбэк для событий
  physicalbutton_callback callback;

  // Текущее событие и ID кнопки
  PhysicalButtonEvent notify_event = PHYSICALBUTTONEVENT_NONE;
  uint8_t notify_id = 0;

  // Чтение состояния кнопки
  bool digitalReadEntryIsDown(ButtonEntry *entry);

  // Обработка одной кнопки
  void tickEntry(ButtonEntry *entry);

  // Обработка изменения состояния кнопки
  void handleEntryChanged(ButtonEntry *entry);

  // Обработка неизменного состояния кнопки
  void handleEntryUnchanged(ButtonEntry *entry);

  // Уведомление о событии
  void notifyEvent(ButtonEntry *entry, PhysicalButtonEvent event);
};

#endif // PHYSICAL_BUTTON_H