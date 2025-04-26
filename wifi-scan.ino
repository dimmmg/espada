// Подключение необходимых библиотек
#include <WiFi.h> // Для работы с Wifi через ESP32
#include <HTTPClient.h> // Для формирования HTTP-запросов
#include <DHTesp.h> // Для работы с датчиком температуры DHT-22
#include <LiquidCrystal_I2C.h> // Для работы с LCD-дисплеем

// Конфигурация LCD дисплея
#define I2C_ADDR     0x27     // Адрес I2C для LCD
#define LCD_COLUMNS  16       // Количество столбцов
#define LCD_LINES    2        // Количество строк

// Пины и настройки датчиков
#define MOTOR_PIN    32       // Пин для датчика мотора (аналоговый)
#define DHT_PIN      15       // Пин для датчика DHT22 (цифровой)
#define DHT_TYPE     DHTesp::DHT22  // Тип датчика

// Настройки сети и сервера
const char* SSID = "Wokwi-GUEST";  // SSID Wi-Fi
const char* SERVER = "api.thingspeak.com";  // Сервер ThingSpeak
const String API_KEY = "YPSMZDVZECSR0AUM";  // API-ключ ThingSpeak

// Интервал отправки данных (мс)
const unsigned short UPDATE_INTERVAL = 15000;  // 15 секунд

// Инициализация объектов
DHTesp dhtSensor;
LiquidCrystal_I2C lcd(I2C_ADDR, LCD_COLUMNS, LCD_LINES);

void setup() {
  Serial.begin(115200);  // Инициализация Serial-порта
  
  // Настройка пинов
  pinMode(MOTOR_PIN, INPUT);  // Датчик мотора (аналоговый, INPUT не обязателен для analogRead)
  dhtSensor.setup(DHT_PIN, DHT_TYPE);  // Инициализация DHT22

  // Настройка LCD
  lcd.init();          // Инициализация дисплея
  lcd.backlight();     // Включение подсветки

  // Подключение к Wi-Fi
  WiFi.begin(SSID);
  Serial.print("Подключение к Wi-Fi");
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("\nWi-Fi подключен!");
  Serial.print("IP-адрес: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  // Чтение данных с потенциометра (симулирует мотор)
  int motorValue = analogRead(MOTOR_PIN);
  int motorPercent = map(motorValue, 0, 4095, 0, 100);  // Преобразует значения с потенциометра в проценты

  // Чтение температуры и влажности
  TempAndHumidity data = dhtSensor.getTempAndHumidity();

  // Вывод данных на LCD
  lcd.clear();  // Очистка дисплея перед обновлением
  lcd.setCursor(0, 0);
  lcd.print("Temp: " + String(data.temperature, 1) + "\xDF" + "C");
  
  lcd.setCursor(0, 1);
  lcd.print("Motor: " + String(motorPercent) + "%");

  // Отправка данных на ThingSpeak, если Wi-Fi подключен
  if (WiFi.status() == WL_CONNECTED) {
    sendToThingSpeak(data.temperature, motorPercent);
  }
  else {
    Serial.println("Ошибка: Wi-Fi не подключен!");
  }

  delay(UPDATE_INTERVAL);  // Задержка между отправками
}

// Функция отправки данных на ThingSpeak
void sendToThingSpeak(float temperature, int motorPercent) {
  HTTPClient http;
  
  // Формирование URL для запроса
  String url = "http://" + String(SERVER) + "/update?api_key=" + API_KEY +
               "&field1=" + String(temperature, 1) +
               "&field2=" + String(motorPercent);
  
  http.begin(url);  // Начало HTTP-запроса
  
  int httpCode = http.GET();  // Отправка GET-запроса
  
  if (httpCode == HTTP_CODE_OK) {
    Serial.println("Данные отправлены на ThingSpeak!");
  } else {
    Serial.printf("Ошибка HTTP: %s\n", http.errorToString(httpCode).c_str());
  }
  
  http.end();  // Закрытие соединения
}