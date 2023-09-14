/*
 * ESPButton.h
 * Обработчик кнопок на основе Ticker с антидребезгом.
 * Интервал сканирования по умолчанию — 16 мс (60 FPS).
 * Все добавленные кнопки сканируются глобальным Ticker (на основе таймера ОС).
 * [!!!] Все события срабатывают в прерывании, поэтому в обратном вызове нельзя использовать delay()!
 *
 * Использование:
 * в setup():
 * pinMode(pin, INPUT_PULLUP / INPUT /... );
 * ESPButton.add(id, pin, pin_down_digital);
 * ESPButton.setCallback(...); // обрабатывайте ваш ButtonEvent по id
 * ESPButton.begin(); // вызовите begin для начала сканирования.
 * в loop():
 * ESPButton.loop(); // будет уведомлять о событии в loop (не в прерывании таймера)
 *
 *  Создано:     2020-03-08
 *  Обновлено:   2020-04-14
 *      Автор:   Wang Bin
 */

#ifndef ESPBUTTON_H_
#define ESPBUTTON_H_

#include <Arduino.h>
#include <functional>
#include <Ticker.h>

#define ESPBUTTON_DEBUG(message, ...)  //printf_P(PSTR("[%7d] ESPButton: " message "\n"), millis(), ##__VA_ARGS__)

typedef struct _ESPButtonEntry {
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
} ESPButtonEntry;

enum ESPButtonEvent {
	ESPBUTTONEVENT_NONE = 0,
	ESPBUTTONEVENT_SINGLECLICK,
	ESPBUTTONEVENT_DOUBLECLICK,
	ESPBUTTONEVENT_LONGCLICK
};
class ESPButtonClass;

static void _esp32_ticker_cb(ESPButtonClass *esp_button);

class ESPButtonClass {

public:

	typedef std::function<void(uint8_t id, ESPButtonEvent event)> espbutton_callback;

	Ticker ticker;
	ESPButtonEntry *entries = nullptr;
	espbutton_callback callback;
	ESPButtonEvent notify_event = ESPBUTTONEVENT_NONE;
	uint8_t notify_id = 0;

	ESPButtonClass() {
	}
	~ESPButtonClass() {
	}

	void begin() {
		ticker.detach();
#if defined(ESP8266)
		ticker.attach_ms(16, std::bind(&ESPButtonClass::tick, this));
#elif defined(ESP32)
		ticker.attach_ms(16, _esp32_ticker_cb, this);
#endif
	}

	ESPButtonEntry* add(uint8_t _id, uint8_t _pin, uint8_t _pin_down_digital,
			bool _doubleclick_enable = false, bool _longclick_enable = true) {
		ESPButtonEntry *entry = new ESPButtonEntry();
		entry->id = _id;
		entry->pin = _pin;
		entry->pin_down_digital = _pin_down_digital;
		entry->doubleclick_enable = _doubleclick_enable;
		entry->longclick_enable = _longclick_enable;

		// инициализация состояния entry? Пока не нужно, мы считаем, что кнопка по умолчанию не нажата
		//entry->laststate_is_down = digitalReadEntryIsDown(entry);
		// добавление в список
		entry->next = entries;
		entries = entry;
		return entry;
	}

	void setCallback(espbutton_callback _callback) {
		callback = _callback;
	}

	PGM_P getButtonEventDescription(ESPButtonEvent e) {
		switch (e) {
		case ESPBUTTONEVENT_SINGLECLICK:
			return PSTR("Одиночный клик");
		case ESPBUTTONEVENT_DOUBLECLICK:
			return PSTR("Двойной клик");
		case ESPBUTTONEVENT_LONGCLICK:
			return PSTR("Долгое нажатие");
		default:
			return PSTR("<неизвестное событие>");
		}
	}

	void tick() {
		ESPButtonEntry *entry = entries;
		while (entry) {
			tickEntry(entry);
			entry = entry->next;
		}
	}

	void loop() {
		if (callback && (notify_event != ESPBUTTONEVENT_NONE)) {
			callback(notify_id, notify_event);
			notify_id = 0;
			notify_event = ESPBUTTONEVENT_NONE;
		}
	}

private:

	bool digitalReadEntryIsDown(ESPButtonEntry *entry) {
		if (entry->ext_digitalRead) {
			return entry->ext_digitalRead(entry->pin) == entry->pin_down_digital;
		}
		return digitalRead(entry->pin) == entry->pin_down_digital;
	}

	void tickEntry(ESPButtonEntry *entry) {
		const uint32_t t = millis();
		const bool down = digitalReadEntryIsDown(entry);
		if (down != entry->raw_down) {
			entry->raw_down = down;
			entry->is_stable = false;
			entry->raw_changed_time = t;
			ESPBUTTON_DEBUG("изменение (%s)", down ? PSTR("нажата") : PSTR("отпущена"));
		} else { // down == raw_down
			// если в течение stable_threshold времени не было изменений, считается стабильным
			if (!entry->is_stable) {
				if (t - entry->raw_changed_time > entry->stable_threshold) {
					ESPBUTTON_DEBUG("t: %d, raw: %d", t, entry->raw_changed_time);ESPBUTTON_DEBUG("стабильно (%s)", down ? PSTR("нажата") : PSTR("отпущена"));
					entry->is_stable = true;
				}
			}
		}
		if (!entry->is_stable) {
			//ESPBUTTON_DEBUG("не стабильно");
			return;
		}
		// код выше может обнаружить стабильное состояние, которое длится более определенного времени, после этого происходит обработка

		if (entry->stable_down == down) {
			handleEntryUnchanged(entry);
			return;
		} else {
			entry->stable_down = down;
			handleEntryChanged(entry);
		}

	}

	void handleEntryChanged(ESPButtonEntry *entry) {
		const bool down = entry->stable_down;
		// логика такова:
		// одиночный клик: нажатие->отпускание->и в течение определенного времени не было второго нажатия
		// двойной клик: нажатие->отпускание->и в течение определенного времени произошло второе нажатие
		// долгое нажатие: нажатие и в течение определенного времени не было отпускания
		if (down) { // нажата
			if (entry->wait_doubleclick && entry->doubleclick_enable) {
				// второе нажатие в течение определенного времени, считается двойным кликом
				// на практике, обычно мой двойной клик up->второе нажатие занимает около 80~100 мс
				ESPBUTTON_DEBUG("двойной клик, ожидание %d", (millis() - entry->up_time));
				entry->down_handled = true;
				notifyEvent(entry, ESPBUTTONEVENT_DOUBLECLICK);
			} else {
				// первое нажатие
				entry->down_handled = false;
			}
			entry->down_time = millis();
			entry->longclicked = false;
			entry->wait_doubleclick = false;
		} else { // отпущена
			if (!entry->down_handled) {
				if (entry->doubleclick_enable) {
					// в loop ожидается второе нажатие
					entry->up_time = millis();
					entry->wait_doubleclick = true;
				} else {
					entry->down_handled = true;
					notifyEvent(entry, ESPBUTTONEVENT_SINGLECLICK);
				}
			}
		}

	}

	void handleEntryUnchanged(ESPButtonEntry *entry) {
		bool down = entry->stable_down;
		if (down) { // нажата
			if (entry->longclick_enable) {
				if (!entry->longclicked && !entry->down_handled) {
					if (millis() - entry->down_time > entry->longclick_threshold) {
						entry->longclicked = true;
						entry->down_handled = true;
						notifyEvent(entry, ESPBUTTONEVENT_LONGCLICK);
					}
				}
			}
		} else { // отпущена
			entry->longclicked = false;
			if (entry->wait_doubleclick && entry->doubleclick_enable) {
				if (millis() - entry->up_time > entry->doubleclick_threshold) {
					entry->wait_doubleclick = false;
					entry->down_handled = true;
					//key2DoClick();
					notifyEvent(entry, ESPBUTTONEVENT_SINGLECLICK);
				}
			}

		}
	}

	void notifyEvent(ESPButtonEntry *entry, ESPButtonEvent event) {
		ESPBUTTON_DEBUG("Кнопка(%d): %s", entry->id, getButtonEventDescription(event));
		// Сохраняем событие и уведомляем о нем в loop
		if (notify_event != ESPBUTTONEVENT_NONE) {
			ESPBUTTON_DEBUG("Внимание! Предыдущее событие кнопки не обработано в loop!");
		}
		notify_event = event;
		notify_id = entry->id;
//		if (callback) {
//			callback(entry->id, event);
//		}
	}

};

ESPButtonClass ESPButton;

static void _esp32_ticker_cb(ESPButtonClass *esp_button) {
	esp_button->tick();
}

#endif /* ESPBUTTON_H_ */
