#include <WiFi.h>
#include <WebServer.h>
#include "DHT.h"
#include <LiquidCrystal_I2C.h>
#include <ESP32Servo.h>
#include <Wire.h>
#include <BH1750.h>

BH1750 lightMeter;

const char *ssid = "Nocta - better sleep";
const char *password = "noctaontop";  // At least 8 characters

WebServer server(80);

// PINS
#define DHTPIN 33    // Temp and hum pin
#define lightSensor 35  // Light pin
#define soundSensor 34  // Sound sensor
#define buttonPin 17 // Button for changing the stages of sleep
#define motorPIN 14 // Morisca for AC
#define ledRed 19 // Heater LED
#define ledYellow 23 // Humidifier LED
#define ledBlue 18 // Dehumidifier LED
#define rgbRed 25 // RGB Red
#define rgbBlue 27 // RGB Blue
#define rgbGreen 26 // RGB Green

#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// Servo motor
Servo myServo;

// LCD
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Define values to orient parameters
// Temperature
const float tempMax = 22;
const float tempMin = 17;
// Humidity
const int humidMax = 60;
const int humidMin = 40;

// Define actuators status
String servoMotorStatus = "OFF";
String lampStatus = "OFF";
String acStatus = "OFF";
String ventilatorStatus = "OFF";
String heaterStatus = "OFF";
String humidifierStatus = "OFF";
String dehumidifierStatus = "OFF";
String soundStatus = "OK";

const int led = 13;

// Variables for user input
String userName = "";
int userAge = 0;
String userSex = "";
String hoursRecommended = "";

// Sleep stage calculation (simple for now)
String sleepStage = "Not Calculated";

// Sleep changes according to button
int sleepStageIndex = 0;
String sleepStages[7] = {
  "Awake",
  "Before Sleep",
  "Stage 1",
  "Stage 2",
  "Stage 3",
  "REM",
  "After sleep"
};

bool isUserSubmitted = false;

float temperature = 0.0;
float humidity = 0.0;
int soundValue = 0;
int lightValue = 0;

// Handle root page (user input and sensor data)
void handleRoot() {

  // HTML content (same as before)
  String html = "<html><body>";
  
  // User Input Form
  html += "<h1>Enter Your Details</h1>";
  html += "<form method='POST' action='/submit'>";
  html += "Name: <input type='text' name='name'><br>";
  html += "Age: <input type='number' name='age'><br>";
  html += "Sex: <input type='text' name='sex'><br>";
  html += "<input type='submit' value='Submit'><br>";
  html += "</form>";

  // Display user details after form submission
  if (isUserSubmitted) {
    if (userAge < 2) {
    hoursRecommended = "11 - 14";
  }

  if (userAge >= 2 && userAge < 12) {
    hoursRecommended = "10 - 13";
  }

  if (userAge >= 12 && userAge <= 17) {
    hoursRecommended = "8 - 10";
  }

  if (userAge >= 18 && userAge <= 30 && userSex == "male") {
    hoursRecommended = "7 - 9";
  }

  if (userAge >= 18 && userAge <= 30 && userSex == "female") {
    hoursRecommended = "8 - 9";
  }

  if (userAge >= 31 && userAge <= 59 && userSex == "male") {
    hoursRecommended = "7 - 9";
  }

  if (userAge >= 31 && userAge <= 59 && userSex == "female") {
    hoursRecommended = "8 - 9";
  }

  if (userAge >= 60 && userSex == "male") {
    hoursRecommended = "7 - 9";
  }

  if (userAge >= 60 && userSex == "female") {
    hoursRecommended = "8 - 9";
  }

    html += "<h2>User Details:</h2>";
    html += "<p><b>Name:</b> " + userName + "</p>";
    html += "<p><b>Age:</b> " + String(userAge) + "</p>";
    html += "<p><b>Sex:</b> " + userSex + "</p>";
    html += "<p><b>Hours of sleep we recommend: </b> " + hoursRecommended + "</p>"; 

    // Alarm Setter
    html += "<h2>Set Alarm Time</h2>";
    html += "<form method='POST' action='/setalarm'>";
    html += "Wake Up Time: <input type='time' name='alarmTime'><br>";
    html += "<input type='submit' value='Set Alarm'><br>"; 
    if (server.hasArg("alarmSet")) {
      html += "<div style='color:green; margin-top:10px;'>Alarm has been set!</div>";
    }
    html += "</form>";
  }

  // CSS
    html += "<style>";
    html += "table { width: 100%; border-collapse: collapse; margin-bottom: 20px; }";
    html += "th, td { border: 1px solid #ddd; padding: 8px; text-align: center; }";
    html += "th { background-color: #f2f2f2; }";
    html += "h2 { color: #333; }";
    html += "</style>";

  // Sensor Table
    html += "<h2>Sensor Data</h2>";
    html += "<table id='sensorTable'>";
    html += "<tr><th>Sensor</th><th>Value</th></tr>";
    html += "<tr><td>Temperature</td><td id='temp'>--</td></tr>";
    html += "<tr><td>Humidity</td><td id='humid'>--</td></tr>";
    html += "<tr><td>Light</td><td id='light'>--</td></tr>";
    html += "<tr><td>Sound</td><td id='sound'>--</td></tr>";
    html += "</table>";

  // Actuator Table
    html += "<h2>Actuator Control</h2>";
    html += "<table>";
    html += "<tr><th>Actuator</th><th>Status</th></tr>";
    html += "<tr><td>Servo Motor</td><td id='servo'>--</td></tr>";
    html += "<tr><td>Lamp</td><td id='lamp'>--</td></tr>";
    html += "<tr><td>AC</td><td id='ac'>--</td></tr>";
    html += "<tr><td>Heater</td><td id='heater'>--</td></tr>";
    html += "<tr><td>Humidifier</td><td id='humidifier'>--</td></tr>";
    html += "<tr><td>Dehumidifier</td><td id='dehumidifier'>--</td></tr>";
    html += "<tr><td>Sound</td><td id='soundStatus'>--</td></tr>";
    html += "</table>";

    html += "<script>";
  // JavaScript to fetch updated data every second and update the table
    html += "function fetchData() {";
    html += "  fetch('/data')";  // Fetch data from the `/data` endpoint
    html += "    .then(response => response.json())";  // Parse the JSON response
    html += "    .then(data => {";
    html += "      document.getElementById('temp').innerText = data.temperature + ' &#8451;';";  // Update Temperature cell
    html += "      document.getElementById('humid').innerText = data.humidity + ' %';";  // Update Humidity cell
    html += "      document.getElementById('light').innerText = data.light;";  // Update Light cell
    html += "      document.getElementById('sound').innerText = data.sound;";  // Update Sound cell
    html += "      document.getElementById('ac').innerText = data.acStatus;";  // Update AC status
    html += "      document.getElementById('heater').innerText = data.heaterStatus;";  // Update Heater status
    html += "      document.getElementById('lamp').innerText = data.lampStatus;";  // Update Lamp status
    html += "      document.getElementById('servo').innerText = data.servoMotorStatus;";  // Update Servo status
    html += "      document.getElementById('humidifier').innerText = data.humidifierStatus;";  // Update Humidifier status
    html += "      document.getElementById('dehumidifier').innerText = data.dehumidifierStatus;";  // Update Dehumidifier status
    html += "      document.getElementById('soundStatus').innerText = data.soundStatus;";  // Update Sound status
    html += "    })";
    html += "    .catch(error => {";
    html += "      console.error('Error fetching data:', error);";
    html += "      // Handle errors gracefully (e.g., update tables with '--')";
    html += "      document.getElementById('temp').innerText = '--';";
    html += "      document.getElementById('humid').innerText = '--';";
    html += "      document.getElementById('light').innerText = '--';";
    html += "      document.getElementById('sound').innerText = '--';";
    html += "      document.getElementById('ac').innerText = '--';";
    html += "      document.getElementById('heater').innerText = '--';";
    html += "      document.getElementById('lamp').innerText = '--';";
    html += "      document.getElementById('servo').innerText = '--';";
    html += "      document.getElementById('humidifier').innerText = '--';";
    html += "      document.getElementById('dehumidifier').innerText = '--';";
    html += "      document.getElementById('soundStatus').innerText = '--';";
    html += "    });";
    html += "}";

    // Set interval to update data every second
    html += "setInterval(fetchData, 1000);";  // Fetch data every 1000ms (1 second)
    html += "</script>";

    html += "</body></html>";

    server.send(200, "text/html", html);  // Send the HTML response
}


void updateLCD() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(sleepStages[sleepStageIndex]);  
  lcd.setCursor(0, 1);
  lcd.print("T:" + String(temperature, 1) + "C");
  delay(2000);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(sleepStages[sleepStageIndex]);  
  lcd.setCursor(0, 1);
  lcd.print("H:" + String(humidity, 1) + "%");
  delay(2000);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(sleepStages[sleepStageIndex]);  
  lcd.setCursor(0, 1);
  lcd.print("S:" + soundStatus);
  delay(2000);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(sleepStages[sleepStageIndex]);  
  lcd.setCursor(0, 1);
  lcd.print("SM: " + servoMotorStatus);
  delay(2000);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(sleepStages[sleepStageIndex]);  
  lcd.setCursor(0, 1);
  lcd.print("L: " + lampStatus);
  delay(2000);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(sleepStages[sleepStageIndex]);  
  lcd.setCursor(0, 1);
  lcd.print("AC:" + acStatus);
  delay(2000);
  
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(sleepStages[sleepStageIndex]);  
  lcd.setCursor(0, 1);
  lcd.print("Htr:" + heaterStatus);
  delay(2000);
  
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(sleepStages[sleepStageIndex]);  
  lcd.setCursor(0, 1);
  lcd.print("Hu:" + humidifierStatus);
  delay(2000);
  
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(sleepStages[sleepStageIndex]);  
  lcd.setCursor(0, 1);
  lcd.print("Deh:" + dehumidifierStatus);
  delay(2000);
}

// Handle form submission for user data
void handleSubmit() {
  if (server.hasArg("name")) {
    userName = server.arg("name");
  }
  if (server.hasArg("age")) {
    userAge = server.arg("age").toInt();
  }
  if (server.hasArg("sex")) {
    userSex = server.arg("sex");
  }

  isUserSubmitted = true; // <-- Set the flag here

  server.sendHeader("Location", "/");  // Redirect to the root after form submission
  server.send(303);  // Send the response and redirect
}

String alarmTime = "";  // Store the alarm time

// Handle alarm time setting
void handleSetAlarm() {
  if (server.hasArg("alarmTime")) {
    alarmTime = server.arg("alarmTime");  // Get the alarm time
    server.sendHeader("Location", "/?alarmSet=true");  // Redirect with parameter
  } else {
      server.sendHeader("Location", "/");  // Redirect to the root page after setting alarm
  }
  server.send(303);  // Send the response and redirect
}

// Sleep stages logic
void sleepStagesLogic() {
    if (sleepStageIndex == 0) {
    digitalWrite(motorPIN, LOW);
    digitalWrite(ledRed, LOW);
    digitalWrite(ledYellow, LOW);
    digitalWrite(ledBlue, LOW);
    lampStatus = "OFF";
    servoMotorStatus = "OFF";
    analogWrite(rgbRed, 0);
    analogWrite(rgbGreen, 0);
    analogWrite(rgbBlue, 0);
    myServo.write(0);
    
    lcd.setCursor(1, 0);
    lcd.print("Rise and shine :) ");
    updateLCD();
  }

  if (sleepStageIndex == 1) {
      servoMotorStatus = "ON"; 
        for (int pos = 0; pos <= 180; pos += 1) {
          myServo.write(pos);
          Serial.print("🔁 Moving to: ");
          Serial.print(pos);
          Serial.println("°");
          delay(10);
        }

      lcd.setCursor(1, 0);
      lcd.print("Good nighty ;) ");
      updateLCD();
  }
  if (sleepStageIndex >= 2 && sleepStageIndex < 6) {
      // Actuators changes
      // Heater + AC
      if (temperature >= tempMax) {
        acStatus = "ON";
        heaterStatus = "OFF";
        digitalWrite(motorPIN, HIGH);  // Turn motor on
        digitalWrite(ledRed, LOW);  // Turn the red LED on when the heater is on

      } else if (temperature <= tempMin) {
          acStatus = "OFF";
          heaterStatus = "ON";
          digitalWrite(motorPIN, LOW);  // Turn motor off
          digitalWrite(ledRed, HIGH);  // Turn the red LED on when the heater is on

      } else {
            acStatus = "OFF";
            heaterStatus = "OFF";
            digitalWrite(motorPIN, LOW);  // Turn motor off
            digitalWrite(ledRed, LOW);  // Turn the red LED on when the heater is on
      }

      if (soundValue > 500) {
        soundStatus = "Too loud";
      }
      else soundStatus = "OK";

      // Humidifier + Dehumidifier
      if (humidity <= humidMin) {
        humidifierStatus = "ON";
        dehumidifierStatus = "OFF";
        digitalWrite(ledYellow, HIGH);
        digitalWrite(ledBlue, LOW);

      } else if (humidity >= humidMax) {
          humidifierStatus = "OFF";
          dehumidifierStatus = "ON";
          digitalWrite(ledYellow, LOW);
          digitalWrite(ledBlue, HIGH);

      } else {
            humidifierStatus = "OFF";
            dehumidifierStatus = "OFF";
            digitalWrite(ledYellow, LOW);
            digitalWrite(ledBlue, LOW);
      }

      // Check if failed
      if (isnan(temperature) || isnan(humidity)) {
        temperature = 0.0;
        humidity = 0.0;
      }
      updateLCD();
  }

    if (sleepStageIndex >= 6) {
      sleepStageIndex = 0;  // Reset to "No Sleep" after REM
      lightValue = analogRead(lightSensor);

      // If light level is high, trigger actions
      if (lightValue < 500) { 
        // LAMP
        for (int i = 0; i <= 255; i++) {
          analogWrite(rgbRed, i);
          analogWrite(rgbGreen, 255 - i);
          analogWrite(rgbBlue, i / 2);
          delay(20);
        }
        lampStatus = "ON";
        servoMotorStatus = "OFF";

        updateLCD();

      } else {
          // Servo motor
          for (int pos = 180; pos >= 0; pos -= 1) {
            myServo.write(pos);
            Serial.print("🔁 Moving to: ");
            Serial.print(pos);
            Serial.println("°");
            delay(10);
          }            
          servoMotorStatus = "ON";
          lampStatus = "OFF";
          updateLCD();
        }
  }
  sleepStage = sleepStages[sleepStageIndex];
}

void handleData() {
  temperature = dht.readTemperature();
  humidity = dht.readHumidity();
  lightValue = analogRead(lightSensor);
  soundValue = analogRead(soundSensor);

  String json = "{";
  json += "\"temperature\":" + String(temperature) + ",";
  json += "\"humidity\":" + String(humidity) + ",";
  json += "\"light\":" + String(lightValue) + ",";
  json += "\"sound\":" + String(soundValue) + ",";
  json += "\"acStatus\":\"" + acStatus + "\",";
  json += "\"heaterStatus\":\"" + heaterStatus + "\",";
  json += "\"lampStatus\":\"" + lampStatus + "\",";
  json += "\"servoMotorStatus\":\"" + servoMotorStatus + "\",";
  json += "\"humidifierStatus\":\"" + humidifierStatus + "\",";
  json += "\"dehumidifierStatus\":\"" + dehumidifierStatus + "\",";
  json += "\"soundStatus\":\"" + soundStatus + "\"";
  json += "}";
  Serial.println(json);

  server.send(200, "application/json", json);
}

void setup() {
  // Start serial communication
  Serial.begin(115200);
  Serial.println();

  // Servo motor
  myServo.attach(13);  // GPIO13 for signal

  // Motor for AC
  pinMode(motorPIN, OUTPUT);
  // Red LED for heater
  pinMode(ledRed, OUTPUT);
  // Yellow LED for humidifier
  pinMode(ledYellow, OUTPUT);
  // Blue LED for dehumidifier
  pinMode(ledBlue, OUTPUT);

  // Route for alarm
  server.on("/setalarm", HTTP_POST, handleSetAlarm);  // Handle alarm time submission

  // LCD Initialize
  Wire.begin(21, 22);  // SDA = 21, SCL = 22
  lcd.init();          // Initialize the LCD
  lcd.backlight();     // Turn on the backlight
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Sleep Monitor");

  // RGB
  analogWrite(rgbRed, 0);
  analogWrite(rgbGreen, 0);
  analogWrite(rgbBlue, 0);

  // Button setup
  pinMode(buttonPin, INPUT_PULLUP);  // Set button pin as input with pull-up

  // Set up sensors
  pinMode(led, OUTPUT);
  digitalWrite(led, 0);

  // RGB
  pinMode(rgbRed, OUTPUT);
  pinMode(rgbGreen, OUTPUT);
  pinMode(rgbBlue, OUTPUT);

  // Start DHT sensor
  dht.begin();
  delay(2000);  

  // Set up Access Point
  WiFi.softAP(ssid, password);
  Serial.print("Access Point started. IP: ");
  Serial.println(WiFi.softAPIP());

  // Set up web server
  server.on("/", handleRoot);  // Main page
  server.on("/data", HTTP_GET, handleData);
  server.on("/submit", HTTP_POST, handleSubmit);  // Form submission
  
  // Start web server
  server.begin();
  Serial.println("Web server started");
}

void loop() {
  if (digitalRead(buttonPin) == LOW) {  // Button pressed (assuming active low)
    delay(200);  // Simple debounce delay (adjust as necessary)
    
    // Change to the next sleep stage
    sleepStageIndex++;
    Serial.println("sleepstage index:");
    Serial.println(sleepStageIndex);
    sleepStagesLogic();
    // Update the LCD with the current sleep stage
    updateLCD();
  }

  server.handleClient();
}
