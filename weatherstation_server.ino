#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

const char* ssid = "";
const char* password = "";

ESP8266WebServer server(80);

String temp = "--";
String hum = "--";
String press = "--";
String light = "--";

String serialBuffer = "";

void handleRoot() {
  String html = R"rawliteral(
    <!DOCTYPE html>
    <html lang="ru">
    <head>
      <meta charset="UTF-8">
      <title>Метеостанция</title>
      <meta name="viewport" content="width=device-width, initial-scale=1.0">
      <link href="https://fonts.googleapis.com/icon?family=Material+Icons" rel="stylesheet">
      <script src="https://cdn.jsdelivr.net/npm/chart.js"></script>
      <style>
        :root {
          --bg-color: #e9f0f4;
          --card-color: white;
          --text-color: #333;
          --icon-color: #007BFF;
        }
        body.dark {
          --bg-color: #121212;
          --card-color: #1e1e1e;
          --text-color: #f0f0f0;
          --icon-color: #00bcd4;
        }
        body {
          font-family: 'Segoe UI', sans-serif;
          background: var(--bg-color);
          color: var(--text-color);
          margin: 0;
          padding: 20px;
          display: flex;
          flex-direction: column;
          align-items: center;
          transition: background 0.3s, color 0.3s;
        }
        h1 {
          margin-bottom: 10px;
        }
        .theme-switch {
          position: absolute;
          top: 10px;
          right: 10px;
        }
        .switch {
          position: relative;
          display: inline-block;
          width: 60px;
          height: 34px;
        }
        .switch input {
          opacity: 0;
          width: 0;
          height: 0;
        }
        .slider {
          position: absolute;
          cursor: pointer;
          top: 0;
          left: 0;
          right: 0;
          bottom: 0;
          background-color: #ccc;
          transition: .4s;
          border-radius: 34px;
        }
        .slider:before {
          position: absolute;
          content: "☀️";
          height: 26px;
          width: 26px;
          left: 4px;
          bottom: 4px;
          background-color: white;
          display: flex;
          align-items: center;
          justify-content: center;
          font-size: 16px;
          transition: .4s;
          border-radius: 50%;
        }
        input:checked + .slider:before {
          transform: translateX(26px);
          content: "🌙";
        }
        input:checked + .slider {
          background-color: #2196F3;
        }
        .cards-container {
          display: flex;
          flex-wrap: wrap;
          justify-content: center;
          gap: 20px;
          margin: 20px 0;
        }
        .card {
          background: var(--card-color);
          border-radius: 15px;
          box-shadow: 0 4px 12px rgba(0,0,0,0.1);
          width: 200px;
          padding: 20px;
          text-align: center;
          transition: transform 0.2s, background 0.3s, color 0.3s;
        }
        .icon {
          font-size: 36px;
          color: var(--icon-color);
          margin-bottom: 10px;
        }
        .label {
          font-size: 1.1em;
        }
        .value {
          font-size: 2em;
          margin-top: 5px;
        }
        canvas {
          background: var(--card-color);
          border-radius: 12px;
          padding: 10px;
          box-shadow: 0 4px 12px rgba(0,0,0,0.2);
        }
      </style>
      <script>
        let chartTemp;
        let chartHum;
        const tempData = [];
        const humData = [];
        const labels = [];

        function updateData() {
          fetch("/data")
            .then(response => response.json())
            .then(data => {
              document.getElementById("temp").innerText = data.temp + " °C";
              document.getElementById("hum").innerText = data.hum + " %";
              document.getElementById("press").innerText = data.press + " hPa";
              document.getElementById("light").innerText = data.light + " %";

              const now = new Date().toLocaleTimeString();
              labels.push(now);
              if (labels.length > 10) labels.shift();

              tempData.push(parseFloat(data.temp));
              humData.push(parseFloat(data.hum));
              if (tempData.length > 10) tempData.shift();
              if (humData.length > 10) humData.shift();

              chartTemp.data.labels = labels;
              chartHum.data.labels = labels;
              chartTemp.data.datasets[0].data = tempData;
              chartHum.data.datasets[0].data = humData;
              chartTemp.update();
              chartHum.update();
            });
        }

        function toggleTheme(checkbox) {
          const isDark = checkbox.checked;
          document.body.classList.toggle('dark', isDark);
          localStorage.setItem('theme', isDark ? 'dark' : 'light');
        }

        window.onload = () => {
          const savedTheme = localStorage.getItem('theme');
          const isDark = savedTheme === 'dark';
          document.body.classList.toggle('dark', isDark);
          document.querySelector('.switch input').checked = isDark;

          const ctx1 = document.getElementById("tempChart").getContext("2d");
          chartTemp = new Chart(ctx1, {
            type: 'line',
            data: {
              labels: [],
              datasets: [{
                label: "Температура",
                data: [],
                borderColor: "#ff6f00",
                backgroundColor: "rgba(255, 111, 0, 0.2)",
                tension: 0.3,
                fill: true
              }]
            },
            options: {
              responsive: true,
              maintainAspectRatio: false,
              scales: {
                y: {
                  min: -20,
                  max: 40,
                  ticks: {
                    stepSize: 5,
                    color: getComputedStyle(document.body).getPropertyValue('--text-color')
                  }
                },
                x: {
                  ticks: {
                    color: getComputedStyle(document.body).getPropertyValue('--text-color')
                  }
                }
              }
            }
          });

          const ctx2 = document.getElementById("humChart").getContext("2d");
          chartHum = new Chart(ctx2, {
            type: 'line',
            data: {
              labels: [],
              datasets: [{
                label: "Влажность",
                data: [],
                borderColor: "#00bcd4",
                backgroundColor: "rgba(0, 188, 212, 0.2)",
                tension: 0.3,
                fill: true
              }]
            },
            options: {
              responsive: true,
              maintainAspectRatio: false,
              scales: {
                y: {
                  min: 0,
                  max: 100,
                  ticks: {
                    stepSize: 10,
                    color: getComputedStyle(document.body).getPropertyValue('--text-color')
                  }
                },
                x: {
                  ticks: {
                    color: getComputedStyle(document.body).getPropertyValue('--text-color')
                  }
                }
              }
            }
          });

          updateData();
          setInterval(updateData, 2000);
        }
      </script>
    </head>
    <body>
      <div class="theme-switch">
        <label class="switch">
          <input type="checkbox" onchange="toggleTheme(this)">
          <span class="slider"></span>
        </label>
      </div>
      <h1>Метеостанция</h1>
      <div class="cards-container">
        <div class="card">
          <span class="material-icons icon">thermostat</span>
          <div class="label">Температура</div>
          <div class="value" id="temp">--</div>
        </div>
        <div class="card">
          <span class="material-icons icon">water_drop</span>
          <div class="label">Влажность</div>
          <div class="value" id="hum">--</div>
        </div>
        <div class="card">
          <span class="material-icons icon">speed</span>
          <div class="label">Давление</div>
          <div class="value" id="press">--</div>
        </div>
        <div class="card">
          <span class="material-icons icon">wb_sunny</span>
          <div class="label">Освещенность</div>
          <div class="value" id="light">--</div>
        </div>
      </div>

      <h2>Графики</h2>
      <div style="width: 90%; max-width: 800px; height: 300px; margin-bottom: 40px;">
        <canvas id="tempChart"></canvas>
      </div>
      <div style="width: 90%; max-width: 800px; height: 300px;">
        <canvas id="humChart"></canvas>
      </div>
    </body>
    </html>
  )rawliteral";

  server.send(200, "text/html", html);
}

void handleData() {
  String json = "{";
  json += "\"temp\":\"" + temp + "\",";
  json += "\"hum\":\"" + hum + "\",";
  json += "\"press\":\"" + press + "\",";
  json += "\"light\":\"" + light + "\"";
  json += "}";
  server.send(200, "application/json", json);
}

void parseData(String& data) {
  data.trim();
  int p1 = data.indexOf(',');
  int p2 = data.indexOf(',', p1 + 1);
  int p3 = data.indexOf(',', p2 + 1);

  if (p1 > 0 && p2 > p1 && p3 > p2) {
    temp = data.substring(0, p1);
    hum = data.substring(p1 + 1, p2);
    press = data.substring(p2 + 1, p3);
    light = data.substring(p3 + 1);
  }
}

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);

  Serial.print("Подключение к WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi подключено!");
  Serial.print("IP адрес: ");
  Serial.println(WiFi.localIP());

  server.on("/", handleRoot);
  server.on("/data", handleData);
  server.begin();
  Serial.println("HTTP сервер запущен");
}

void loop() {
  server.handleClient();

  while (Serial.available()) {
    char c = Serial.read();
    if (c == '\n') {
      if (serialBuffer.length() > 0) {
        parseData(serialBuffer);
        serialBuffer = "";
      }
    } else if (c != '\r' && serialBuffer.length() < 100) {
      serialBuffer += c;
    }
  }
}
