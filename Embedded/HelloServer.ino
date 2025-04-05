#include <WiFi.h>
#include <WebServer.h>
#include <DHT.h>
#include <LiquidCrystal_I2C.h>
#include <ESP32Servo.h>

const char *ssid = "Nocta - better sleep";
const char *password = "noctaontop";  // At least 8 characters

WebServer server(80);

// PINS
#define DHTPIN 4    // Temp pin
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

#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

// PWM Channels
const int redChan = 0;
const int greenChan = 1;
const int blueChan = 2;

// Servo motor
Servo myServo; // Create a servo object

// LCD
LiquidCrystal_I2C lcd(0x27, 16, 2); // 0x27 is the typical I2C address; adjust if needed

// Define values to orient parameters
// Temperature
const float tempMax = 22;
const float tempMin = 17;
// Humidity
const int humidMax = 60;
const int humidMin = 40;

// Define actuators status
String acStatus = "OFF";
String ventilatorStatus = "OFF";
String heaterStatus = "OFF";
String humidifierStatus = "OFF";
String dehumidifierStatus = "OFF";

const int led = 13;

// Variables for user input
String userName = "";
int userAge = 0;
String userSex = "";
bool sleepStageCalculated = false;
String hoursRecommended = "";

// Sleep stage calculation (simple for now)
String sleepStage = "Not Calculated";

// Sleep changes according to button
int sleepStageIndex = 0;  // 0 - No Sleep, 1 - Stage 1, 2 - Stage 2, 3 - Stage 3, 4 - REM
String sleepStages[5] = {
  "No Sleep",
  "Stage 1",
  "Stage 2",
  "Stage 3",
  "REM"
};

// Smooth color transition function
// void smoothColorFade(int r1, int g1, int b1, int r2, int g2, int b2, int duration) {
//   int steps = 100;
//   int delayTime = duration / steps;

//   for (int i = 0; i <= steps; i++) {
//     float progress = i / float(steps);
//     int r = r1 + (r2 - r1) * progress;
//     int g = g1 + (g2 - g1) * progress;
//     int b = b1 + (b2 - b1) * progress;

//     ledcWrite(redChan, r);
//     ledcWrite(greenChan, g);
//     ledcWrite(blueChan, b);

//     delay(delayTime);
//   }
// }

// Handle root page (user input and sensor data)
void handleRoot() {

  // Read sensor values
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();
  int lightValue = analogRead(lightSensor);
  int soundValue = analogRead(soundSensor);

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

  // HTML content to send to the client
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
  if (userName != "") {
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
    html += "</form>";
  }

  // Sleep stage button
  html += "<form method='POST' action='/sleepstage'>";
  html += "<input type='submit' value='Change Sleep Stage'>";
  html += "</form>";

  // Display sensor data
  html += "<h2>Sensor Data</h2>";
  html += "<table border='1'>";
  html += "<tr><th>Sensor</th><th>Value</th></tr>";
  html += "<tr><td>Temperature</td><td>" + String(temperature) + " &#8451;</td></tr>";
  html += "<tr><td>Humidity</td><td>" + String(humidity) + " %</td></tr>";
  html += "<tr><td>Light</td><td>" + String(lightValue) + "</td></tr>";
  html += "<tr><td>Sound</td><td>" + String(soundValue) + "</td></tr>";
  html += "</table>";

  // Display actuator control table
  html += "<h2>Actuator Control</h2>";
  html += "<table border='1'>";
  html += "<tr><th>Actuator</th><th>Action</th></tr>";
  
  // LIGHT
  html += "<tr><td>Light Control</td><td> Servo Motor </td></tr>";
  // TEMP
  html += "<tr><td>AC Control</td><td> Ventilator " + acStatus + "</td></tr>";
  html += "<tr><td>Heat Control</td><td> Heater " + heaterStatus + "</td></tr>";
  // HUMID
  html += "<tr><td>Humidity Control I</td><td> Humidifier " + humidifierStatus + "</td></tr>";
  html += "<tr><td>Humidity Control II</td><td> Dehumidifier " + dehumidifierStatus + "</td></tr>";
  // SOUND
  html += "<tr><td>Noise Control</td><td>Message: Too Loud</td></tr>";

  html += "</table>";

  html += "</body></html>";

  // Send HTML response to the browser
  server.send(200, "text/html", html);
}

void updateLCD() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Sleep Stage: ");
  lcd.setCursor(0, 1);
  lcd.print(sleepStages[sleepStageIndex]);
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
  server.sendHeader("Location", "/");  // Redirect to the root after form submission
  server.send(303);  // Send the response and redirect
}

String alarmTime = "";  // Store the alarm time

// Handle alarm time setting
void handleSetAlarm() {
  if (server.hasArg("alarmTime")) {
    alarmTime = server.arg("alarmTime");  // Get the alarm time
  }
  
  server.sendHeader("Location", "/");  // Redirect to the root page after setting alarm
  server.send(303);  // Send the response and redirect
}


// Sleep function
void handleSleepStage() {

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

  server.sendHeader("Location", "/");  // Redirect to root after calculation
  server.send(303);  // Redirect to the main page to show results
}

void setup() {
  // Start serial communication
  Serial.begin(115200);
  Serial.println();

  // Servo motor
  Serial.begin(115200);
  myServo.attach(13);  // GPIO13 for signal
  Serial.println("🔧 Servo Test Starting...");

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

  // Button setup
  pinMode(buttonPin, INPUT_PULLUP);  // Set button pin as input with pull-up

  // Set up sensors
  pinMode(led, OUTPUT);
  digitalWrite(led, 0);

  // // Attach PWM channels to pins
  // ledcSetup(redChan, 5000, 8);  // 5kHz, 8-bit resolution
  // ledcSetup(greenChan, 5000, 8);
  // ledcSetup(blueChan, 5000, 8);

  // ledcAttach(rgbRed, redChan);
  // ledcAttach(rgbGreen, greenChan);
  // ledcAttach(rgbBlue, blueChan);
  // RGB
  pinMode(rgbRed, OUTPUT);
  pinMode(rgbGreen, OUTPUT);
  pinMode(rgbBlue, OUTPUT);


  // Start DHT sensor
  dht.begin();

  // Set up Access Point
  WiFi.softAP(ssid, password);
  Serial.print("Access Point started. IP: ");
  Serial.println(WiFi.softAPIP());

  // Set up web server
  server.on("/", handleRoot);  // Main page
  server.on("/submit", HTTP_POST, handleSubmit);  // Form submission
  server.on("/sleepstage", HTTP_POST, handleSleepStage);  // Sleep stage button
  
  // Start web server
  server.begin();
  Serial.println("Web server started");
}

void loop() {
  // Handle the button press
  if (digitalRead(buttonPin) == LOW) {  // Button pressed (assuming active low)
    delay(200);  // Simple debounce delay (adjust as necessary)
    
    // Change to the next sleep stage
    sleepStageIndex++;
    if (sleepStageIndex == 1)
    {
       lcd.setCursor(1, 0);
       lcd.print("Good nighty ;) ");
    }
    if (sleepStageIndex >= 5) {
      sleepStageIndex = 0;  // Reset to "No Sleep" after REM
    }

    if (sleepStageIndex == 5) {
      int lightValue = analogRead(lightSensor);

      // If light level is high, trigger actions
      if (lightValue > 500) {  // Adjust the threshold as necessary
        // LAMP
        // Simulated sunrise: red → orange → yellow → white
        // smoothColorFade(10, 0, 0,   255, 100, 0,   3000);  // Red to Orange
        // smoothColorFade(255, 100, 0, 255, 200, 0,   3000);  // Orange to Yellow
        // smoothColorFade(255, 200, 0, 255, 255, 180, 3000);  // Yellow to Warm White
        // delay(10000); // keep it on for a bit    
        for (int i = 0; i <= 255; i++) {
          analogWrite(rgbRed, i);
          analogWrite(rgbGreen, 255 - i);
          analogWrite(rgbBlue, i / 2);
          delay(20);
        }

      } else {
          // Servo motor
          // Sweep from 0° to 180°
          for (int pos = 0; pos <= 180; pos += 1) {
            myServo.write(pos);
            Serial.print("🔁 Moving to: ");
            Serial.print(pos);
            Serial.println("°");
            delay(10);
          }
        }
    }
    if (sleepStageIndex == 0) {
       lcd.setCursor(1, 0);
       lcd.print("Good morning, Merian :) ");
    }
    
    // Update the LCD with the current sleep stage
    updateLCD();
  }

  server.handleClient();
}
