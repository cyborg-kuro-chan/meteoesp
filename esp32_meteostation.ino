#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>

// Настройки Wi-Fi
const char* ssid = "YOUR_SSID";
const char* password = "YOUR_PASSWORD";

// Создаем объект сервера
AsyncWebServer server(80);

// Переменные для симуляции данных датчиков
float temperature = 23.5;
float humidity = 65.2;
float pressure = 1013.25;
int lightLevel = 450;
float uvIndex = 4.2;
float noiseLevel = 38.7;

// HTML-страница из файла index.html
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="ru">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Метеостанция ESP32</title>
    <style>
        * {
            margin: 0;
            padding: 0;
            box-sizing: border-box;
        }

        body {
            font-family: Arial, sans-serif;
            background-color: #f0f0f0;
            color: #333;
            padding: 10px;
        }

        .header {
            text-align: center;
            padding: 15px;
            background-color: #2c3e50;
            color: white;
            border-radius: 10px;
            margin-bottom: 20px;
        }

        /* Аккордеон для мобильного режима */
        .accordion {
            display: none;
            width: 100%;
            margin-bottom: 20px;
        }

        .accordion.active {
            display: block;
        }

        .accordion-button {
            display: block;
            width: 100%;
            padding: 15px;
            background-color: #34495e;
            color: white;
            border: none;
            text-align: left;
            font-size: 16px;
            cursor: pointer;
            border-radius: 5px;
            margin-bottom: 5px;
        }

        .accordion-button:hover {
            background-color: #2c3e50;
        }

        .accordion-content {
            padding: 15px;
            background-color: white;
            display: none;
            border-radius: 5px;
            margin-top: 5px;
            box-shadow: 0 2px 5px rgba(0,0,0,0.1);
        }

        .accordion-content.show {
            display: block;
        }

        /* Сетка для планшетного режима */
        .sensor-grid {
            display: none;
            grid-template-columns: repeat(auto-fit, minmax(200px, 1fr));
            gap: 15px;
            margin-bottom: 20px;
        }

        .sensor-grid.active {
            display: grid;
        }

        .sensor-zone {
            padding: 20px;
            border-radius: 10px;
            color: white;
            text-align: center;
            box-shadow: 0 4px 8px rgba(0,0,0,0.2);
            transition: transform 0.3s ease;
        }

        .sensor-zone:hover {
            transform: translateY(-5px);
        }

        .zone-title {
            font-size: 18px;
            font-weight: bold;
            margin-bottom: 10px;
        }

        .sensor-value {
            font-size: 24px;
            font-weight: bold;
        }

        .sensor-unit {
            font-size: 14px;
            opacity: 0.8;
        }

        .zone-1 { background: linear-gradient(135deg, #3498db, #2980b9); }
        .zone-2 { background: linear-gradient(135deg, #e74c3c, #c0392b); }
        .zone-3 { background: linear-gradient(135deg, #2ecc71, #27ae60); }
        .zone-4 { background: linear-gradient(135deg, #f39c12, #d35400); }
        .zone-5 { background: linear-gradient(135deg, #9b59b6, #8e44ad); }
        .zone-6 { background: linear-gradient(135deg, #1abc9c, #16a085); }

        .mode-toggle {
            display: flex;
            justify-content: center;
            margin-bottom: 20px;
        }

        .toggle-btn {
            padding: 10px 20px;
            background-color: #3498db;
            color: white;
            border: none;
            border-radius: 5px;
            cursor: pointer;
            font-size: 16px;
        }

        .toggle-btn:hover {
            background-color: #2980b9;
        }

        .footer {
            text-align: center;
            padding: 10px;
            font-size: 14px;
            color: #7f8c8d;
        }

        /* Мобильная версия */
        @media screen and (max-width: 768px) {
            .sensor-grid {
                display: none;
            }
            
            .accordion {
                display: block;
            }
        }

        /* Планшетная/десктопная версия */
        @media screen and (min-width: 769px) {
            .accordion {
                display: none;
            }
            
            .sensor-grid {
                display: grid;
            }
        }
    </style>
</head>
<body>
    <div class="header">
        <h1>Метеостанция ESP32</h1>
        <p>Отслеживание погодных условий в реальном времени</p>
    </div>

    <div class="mode-toggle">
        <button class="toggle-btn" id="modeToggle">Переключить режим отображения</button>
    </div>

    <!-- Аккордеон для мобильного режима -->
    <div class="accordion active" id="accordionView">
        <button class="accordion-button" onclick="toggleAccordion(0)">Температура</button>
        <div class="accordion-content" id="content0">
            <div class="sensor-value"><span id="tempValue">--</span><span class="sensor-unit"> °C</span></div>
        </div>

        <button class="accordion-button" onclick="toggleAccordion(1)">Влажность</button>
        <div class="accordion-content" id="content1">
            <div class="sensor-value"><span id="humValue">--</span><span class="sensor-unit"> %</span></div>
        </div>

        <button class="accordion-button" onclick="toggleAccordion(2)">Давление</button>
        <div class="accordion-content" id="content2">
            <div class="sensor-value"><span id="presValue">--</span><span class="sensor-unit"> hPa</span></div>
        </div>

        <button class="accordion-button" onclick="toggleAccordion(3)">Уровень света</button>
        <div class="accordion-content" id="content3">
            <div class="sensor-value"><span id="lightValue">--</span><span class="sensor-unit"> lux</span></div>
        </div>
    </div>

    <!-- Сетка для планшетного режима -->
    <div class="sensor-grid" id="gridView">
        <div class="sensor-zone zone-1">
            <div class="zone-title">Температура</div>
            <div class="sensor-value"><span id="gTempValue">--</span><span class="sensor-unit"> °C</span></div>
        </div>
        
        <div class="sensor-zone zone-2">
            <div class="zone-title">Влажность</div>
            <div class="sensor-value"><span id="gHumValue">--</span><span class="sensor-unit"> %</span></div>
        </div>
        
        <div class="sensor-zone zone-3">
            <div class="zone-title">Давление</div>
            <div class="sensor-value"><span id="gPresValue">--</span><span class="sensor-unit"> hPa</span></div>
        </div>
        
        <div class="sensor-zone zone-4">
            <div class="zone-title">Уровень света</div>
            <div class="sensor-value"><span id="gLightValue">--</span><span class="sensor-unit"> lux</span></div>
        </div>
        
        <div class="sensor-zone zone-5">
            <div class="zone-title">Ультрафиолет</div>
            <div class="sensor-value"><span id="gUVValue">--</span><span class="sensor-unit"></span></div>
        </div>
        
        <div class="sensor-zone zone-6">
            <div class="zone-title">Шум</div>
            <div class="sensor-value"><span id="gNoiseValue">--</span><span class="sensor-unit"> dB</span></div>
        </div>
    </div>

    <div class="footer">
        Метеостанция ESP32 | Обновлено: <span id="updateTime">--:--:--</span>
    </div>

    <script>
        // Функция для получения данных с датчиков
        function fetchSensorData() {
            fetch('/api/sensors')
                .then(response => response.json())
                .then(data => {
                    // Обновление значений в мобильном режиме
                    document.getElementById('tempValue').textContent = data.temperature.toFixed(1);
                    document.getElementById('humValue').textContent = data.humidity.toFixed(1);
                    document.getElementById('presValue').textContent = data.pressure.toFixed(2);
                    document.getElementById('lightValue').textContent = data.light_level;
                    
                    // Обновление значений в планшетном режиме
                    document.getElementById('gTempValue').textContent = data.temperature.toFixed(1);
                    document.getElementById('gHumValue').textContent = data.humidity.toFixed(1);
                    document.getElementById('gPresValue').textContent = data.pressure.toFixed(2);
                    document.getElementById('gLightValue').textContent = data.light_level;
                    document.getElementById('gUVValue').textContent = data.uv_index.toFixed(1);
                    document.getElementById('gNoiseValue').textContent = data.noise_level.toFixed(1);
                    
                    // Обновление времени
                    const now = new Date();
                    document.getElementById('updateTime').textContent = now.toLocaleTimeString();
                })
                .catch(error => console.error('Ошибка при получении данных:', error));
        }

        // Переключение между режимами отображения
        document.getElementById('modeToggle').addEventListener('click', function() {
            const accordion = document.getElementById('accordionView');
            const gridView = document.getElementById('gridView');
            
            if(accordion.classList.contains('active')) {
                accordion.classList.remove('active');
                gridView.classList.add('active');
                this.textContent = 'Переключить в мобильный режим';
            } else {
                accordion.classList.add('active');
                gridView.classList.remove('active');
                this.textContent = 'Переключить в планшетный режим';
            }
        });

        // Функция для работы аккордеона
        function toggleAccordion(index) {
            const content = document.getElementById(`content${index}`);
            content.classList.toggle('show');
        }

        // Обновляем данные каждые 5 секунд
        setInterval(fetchSensorData, 5000);
        
        // Инициализируем данные при загрузке
        fetchSensorData();
        
        // Обработка изменения размера окна для автоматического переключения режимов
        window.addEventListener('resize', function() {
            const accordion = document.getElementById('accordionView');
            const gridView = document.getElementById('gridView');
            const toggleBtn = document.getElementById('modeToggle');
            
            if(window.innerWidth < 769) {
                accordion.classList.add('active');
                gridView.classList.remove('active');
                toggleBtn.textContent = 'Переключить в планшетный режим';
            } else {
                accordion.classList.remove('active');
                gridView.classList.add('active');
                toggleBtn.textContent = 'Переключить в мобильный режим';
            }
        });
    </script>
</body>
</html>
)rawliteral";

// Функция генерации JSON с данными датчиков
String getSensorData() {
  StaticJsonDocument<512> doc;
  
  doc["temperature"] = temperature;
  doc["humidity"] = humidity;
  doc["pressure"] = pressure;
  doc["light_level"] = lightLevel;
  doc["uv_index"] = uvIndex;
  doc["noise_level"] = noiseLevel;
  
  String jsonString;
  serializeJson(doc, jsonString);
  return jsonString;
}

void setup() {
  Serial.begin(115200);

  // Подключаемся к Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Подключение к Wi-Fi...");
  }

  Serial.println("Wi-Fi подключен");
  Serial.print("IP-адрес: ");
  Serial.println(WiFi.localIP());

  // Определение маршрутов
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html);
  });

  server.on("/api/sensors", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "application/json", getSensorData());
  });

  // Запуск сервера
  server.begin();
}

void loop() {
  // В реальной реализации здесь будет чтение с датчиков
  // Для демонстрации просто обновляем значения с небольшими изменениями
  
  // Симуляция изменения данных датчиков
  static unsigned long lastUpdate = 0;
  if(millis() - lastUpdate > 10000) { // Обновляем каждые 10 секунд
    temperature += (random(10) - 5) / 10.0; // Небольшие изменения температуры
    humidity += (random(10) - 5) / 10.0;
    pressure += (random(10) - 5) / 10.0;
    lightLevel += random(100) - 50;
    uvIndex += (random(10) - 5) / 10.0;
    noiseLevel += (random(10) - 5) / 10.0;
    
    // Ограничиваем диапазоны значений
    temperature = constrain(temperature, -10, 50);
    humidity = constrain(humidity, 0, 100);
    pressure = constrain(pressure, 950, 1050);
    lightLevel = constrain(lightLevel, 0, 10000);
    uvIndex = constrain(uvIndex, 0, 15);
    noiseLevel = constrain(noiseLevel, 20, 100);
    
    lastUpdate = millis();
    
    Serial.println("Данные датчиков обновлены");
    Serial.printf("Температура: %.2f°C, Влажность: %.2f%%, Давление: %.2fhPa\n", 
                  temperature, humidity, pressure);
  }
}