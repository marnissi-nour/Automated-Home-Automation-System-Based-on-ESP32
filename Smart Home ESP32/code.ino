#include <WiFi.h>
#include <WebServer.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <DHT.h>
#include <ESP32Servo.h>   // ✅ Remplace ServoESP32 par ESP32Servo
#include <SD.h>

// ================== CONFIG ==================
#define DHTTYPE DHT11
#define DHTInsidePin 32
#define DHTOutsidePin 33
#define LDRPin 34
#define CurrentSensorPin 35
#define DoorSensorPin 27
#define ServoPin 25
#define BuzzerPin 26
#define LEDPin 15
#define SD_CS 23

// WiFi credentials
const char* ssid = "Your_SSID";
const char* password = "Your_PASSWORD";

// GSM (UART2 sur ESP32)
HardwareSerial gsm(2); // utilise GPIO16 (RX2) et GPIO17 (TX2)

// ================== OBJECTS ==================
DHT dhtInside(DHTInsidePin, DHTTYPE);
DHT dhtOutside(DHTOutsidePin, DHTTYPE);
LiquidCrystal_I2C lcd(0x27, 16, 2);
Servo windowServo;
WebServer server(80);

// ✅ Prototypes pour éviter les erreurs
void handleRoot();
void handleData();
float readCurrentSensor();
void sendGSMNotification(String message);
void logDataToSD(float insideTemp, float outsideTemp, String lightCondition, float current);

// ================== SETUP ==================
void setup() {
  Serial.begin(115200);
  gsm.begin(9600, SERIAL_8N1, 16, 17); // RX=16, TX=17

  // Init LCD
  lcd.init();
  lcd.backlight();

  // Init sensors
  dhtInside.begin();
  dhtOutside.begin();
  windowServo.attach(ServoPin);
  pinMode(DoorSensorPin, INPUT_PULLUP);
  pinMode(LDRPin, INPUT);
  pinMode(BuzzerPin, OUTPUT);
  pinMode(LEDPin, OUTPUT);

  // Init SD
  if (!SD.begin(SD_CS)) {
    Serial.println("⚠️ SD card init failed!");
  } else {
    Serial.println("✅ SD card ready");
  }

  // WiFi connection
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n✅ WiFi connected. IP: " + WiFi.localIP().toString());

  // Web server routes
  server.on("/", handleRoot);
  server.on("/data", handleData);
  server.begin();
  Serial.println("✅ Web server started");
}

// ================== LOOP ==================
void loop() {
  // Read sensors
  float insideTemp = dhtInside.readTemperature();
  float outsideTemp = dhtOutside.readTemperature();
  float tempDiff = abs(outsideTemp - insideTemp);
  int ldrValue = analogRead(LDRPin);
  float current = readCurrentSensor();

  String lightCondition = (ldrValue < 1000) ? "Dark" : (ldrValue < 2000) ? "Medium" : "Intense";

  // LCD display
  lcd.setCursor(0, 0);
  lcd.print("In:");
  lcd.print(insideTemp);
  lcd.print("C ");
  lcd.setCursor(8, 0);
  lcd.print("Out:");
  lcd.print(outsideTemp);
  lcd.print("C ");
  lcd.setCursor(0, 1);
  lcd.print("Light:");
  lcd.print(lightCondition);
  lcd.setCursor(10, 1);
  lcd.print("I:");
  lcd.print(current);
  lcd.print("A ");

  // Window & light control
  if (tempDiff > 5) {
    if (digitalRead(DoorSensorPin) == LOW) {
      tone(BuzzerPin, 1000, 500); // Beep alert
    }
    windowServo.write(0); // Close blinds
    digitalWrite(LEDPin, LOW);
  } else {
    if (lightCondition == "Dark" || lightCondition == "Medium") {
      windowServo.write(180); // Open blinds
      digitalWrite(LEDPin, HIGH);
    } else if (lightCondition == "Intense") {
      windowServo.write(0); // Close blinds
      digitalWrite(LEDPin, LOW);
    }
  }

  // Log data
  logDataToSD(insideTemp, outsideTemp, lightCondition, current);

  // Web server
  server.handleClient();

  delay(2000);
}

// ================== FUNCTIONS ==================
float readCurrentSensor() {
  int sensorValue = analogRead(CurrentSensorPin);
  float voltage = sensorValue * (3.3 / 4095.0); // ESP32 ADC
  float current = (voltage - 2.5) / 0.185;      // ACS712
  return abs(current);
}

void sendGSMNotification(String message) {
  gsm.println("AT+CMGF=1");
  delay(100);
  gsm.println("AT+CMGS=\"+216XXXXXXXX\""); // num tel
  delay(100);
  gsm.print(message);
  gsm.write(26); // CTRL+Z
  delay(1000);
}

void logDataToSD(float insideTemp, float outsideTemp, String lightCondition, float current) {
  File dataFile = SD.open("/energy.txt", FILE_APPEND);
  if (dataFile) {
    String logData = String(millis() / 60000) + "," + insideTemp + "," + outsideTemp + "," +
                     lightCondition + "," + current;
    dataFile.println(logData);
    dataFile.close();
  }
}

// ================== WEB PAGES ==================
void handleRoot() {
  String html = R"rawliteral(
   <!DOCTYPE html>
<html lang="fr">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Dashboard Capteurs</title>

    <!-- Bootstrap -->
    <link href="https://cdn.jsdelivr.net/npm/bootstrap@5.3.0/dist/css/bootstrap.min.css" rel="stylesheet">

    <!-- Chart.js -->
    <script src="https://cdn.jsdelivr.net/npm/chart.js"></script>
</head>
<body class="bg-light">

    <div class="container py-4">

        <h2 class="text-center mb-4">📊 Dashboard</h2>

        <!-- TABLEAU DES MESURES -->
        <div class="card mb-4 shadow">
            <div class="card-header bg-primary text-white">
                <h5 class="mb-0">Tableau des dernières mesures</h5>
            </div>
            <div class="card-body">
                <table class="table table-striped text-center">
                    <thead class="table-dark">
                        <tr>
                            <th>Date</th>
                            <th>Temp. Intérieure (°C)</th>
                            <th>Temp. Extérieure (°C)</th>
                            <th>Courant (A)</th>
                        </tr>
                    </thead>
                    <tbody id="tableBody">
                        <!-- Lignes générées dynamiquement -->
                    </tbody>
                </table>
            </div>
        </div>

        <!-- GRAPHIQUES -->
        <div class="row">
            <div class="col-md-6 mb-4">
                <div class="card shadow">
                    <div class="card-header bg-secondary text-white">Température Intérieure</div>
                    <div class="card-body">
                        <canvas id="tempInChart"></canvas>
                    </div>
                </div>
            </div>

            <div class="col-md-6 mb-4">
                <div class="card shadow">
                    <div class="card-header bg-secondary text-white">Température Extérieure</div>
                    <div class="card-body">
                        <canvas id="tempOutChart"></canvas>
                    </div>
                </div>
            </div>

            <div class="col-md-6 mb-4">
                <div class="card shadow">
                    <div class="card-header bg-secondary text-white">Courant (A)</div>
                    <div class="card-body">
                        <canvas id="currentChart"></canvas>
                    </div>
                </div>
            </div>
        </div>
    </div>

    <script>
        fetch('/data')
            .then(response => response.json())
            .then(data => {
                const labels = data.timestamps;
                const tempInside = data.insideTemps;
                const tempOutside = data.outsideTemps;
                const current = data.currents;

                // ----- AJOUT AU TABLEAU -----
                const tableBody = document.getElementById("tableBody");
                tableBody.innerHTML = "";
                for (let i = 0; i < labels.length; i++) {
                    tableBody.innerHTML += `
                        <tr>
                            <td>${labels[i]}</td>
                            <td>${tempInside[i]} °C</td>
                            <td>${tempOutside[i]} °C</td>
                            <td>${current[i]} A</td>
                        </tr>
                    `;
                }

                // ----- GRAPHIQUES -----
                new Chart(document.getElementById("tempInChart"), {
                    type: "line",
                    data: {
                        labels: labels,
                        datasets: [{
                            label: "Température Intérieure",
                            data: tempInside,
                            borderColor: "red",
                            borderWidth: 2,
                            tension: 0.3
                        }]
                    }
                });

                new Chart(document.getElementById("tempOutChart"), {
                    type: "line",
                    data: {
                        labels: labels,
                        datasets: [{
                            label: "Température Extérieure",
                            data: tempOutside,
                            borderColor: "orange",
                            borderWidth: 2,
                            tension: 0.3
                        }]
                    }
                });

                new Chart(document.getElementById("currentChart"), {
                    type: "line",
                    data: {
                        labels: labels,
                        datasets: [{
                            label: "Courant (A)",
                            data: current,
                            borderColor: "green",
                            borderWidth: 2,
                            tension: 0.3
                        }]
                    }
                });
            })
            .catch(err => console.error("Erreur lors de la récupération des données :", err));
    </script>

</body>
</html>

  )rawliteral";
  server.send(200, "text/html", html);
}

void handleData() {
  File dataFile = SD.open("/energy.txt");
  if (!dataFile) {
    server.send(500, "application/json", "{\"error\":\"Log file not found\"}");
    return;
  }

  String timestamps = "[";
  String insideTemps = "[";
  String outsideTemps = "[";
  String currents = "[";

  while (dataFile.available()) {
    String line = dataFile.readStringUntil('\n');
    if (line.length() < 5) continue;

    int idx1 = line.indexOf(',');
    int idx2 = line.indexOf(',', idx1 + 1);
    int idx3 = line.indexOf(',', idx2 + 1);
    int idx4 = line.indexOf(',', idx3 + 1);

    if (idx1 < 0 || idx2 < 0 || idx3 < 0 || idx4 < 0) continue;

    String ts = line.substring(0, idx1);
    String inT = line.substring(idx1 + 1, idx2);
    String outT = line.substring(idx2 + 1, idx3);
    String current = line.substring(idx3 + 1, idx4);

    timestamps += "\"" + ts + "\",";
    insideTemps += inT + ",";
    outsideTemps += outT + ",";
    currents += current + ",";
  }
  dataFile.close();

  if (timestamps.endsWith(",")) timestamps.remove(timestamps.length() - 1);
  if (insideTemps.endsWith(",")) insideTemps.remove(insideTemps.length() - 1);
  if (outsideTemps.endsWith(",")) outsideTemps.remove(outsideTemps.length() - 1);
  if (currents.endsWith(",")) currents.remove(currents.length() - 1);

  timestamps += "]";
  insideTemps += "]";
  outsideTemps += "]";
  currents += "]";

  String json = "{\"timestamps\":" + timestamps +
                ",\"insideTemps\":" + insideTemps +
                ",\"outsideTemps\":" + outsideTemps +
                ",\"currents\":" + currents + "}";

  server.send(200, "application/json", json);
}
