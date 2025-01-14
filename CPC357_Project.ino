#include "VOneMqttClient.h"
#include "DHT.h"

//define device id
const char* DHT11Sensor = "a1ac243b-8cb9-4e19-987a-a8632019e452";   
const char* IRSensor = "fc69e0eb-8b09-41b1-a1e7-53c8abe9793f";   
const char* MQ2Sensor = "30c19a0f-a3d9-492f-8770-9c404e7bca5b"; 
const char* LEDLight1 = "dc1429c9-cdf1-4298-83aa-4c17e066f361"; 
const char* LEDLight2 = "06d97b35-cb0e-426d-b884-311661128cdc"; 
const char* LEDLight3 = "0cea1779-6df6-4e4b-92b4-83ffd726c452"; 

//Used Pins
const int dht11Pin = 42;         //Right side Maker Port
const int irPin = 4;           //Left side Maker Port
const int mq2Pin = A2;      //Middle Maker Port
const int ledPin1 = 9;        // RED
const int ledPin2 = 10;        // YELLOW
const int ledPin3 = 5;        // GREEN

//input sensor
#define DHTTYPE DHT11
DHT dht(dht11Pin, DHTTYPE);

//Create an instance of VOneMqttClient
VOneMqttClient voneClient;

unsigned long lastMsgTime = 0;
int greenFlag = 0;
int yellowFlag = 0;
int redFlag = 0;

void setup_wifi() {

  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WIFI_SSID);

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void triggerActuator_callback(const char* actuatorDeviceId, const char* actuatorCommand) {
  Serial.print("Actuator Command Received for Device: ");
  Serial.print(actuatorDeviceId);
  Serial.print(" : ");
  Serial.println(actuatorCommand);

  String errorMsg = "";

  JSONVar commandObjct = JSON.parse(actuatorCommand);
  JSONVar keys = commandObjct.keys();

  if (String(actuatorDeviceId) == LEDLight1) {
    String key = "";
    bool commandValue = "";
    for (int i = 0; i < keys.length(); i++) {
      key = (const char*)keys[i];
      commandValue = (bool)commandObjct[keys[i]];
      Serial.print("Key : ");
      Serial.println(key.c_str());
      Serial.print("value : ");
      Serial.println(commandValue);
    }

    if (commandValue == true) {
      Serial.println("LED ON");
      digitalWrite(ledPin1, true);
    } else {
      Serial.println("LED OFF");
      digitalWrite(ledPin1, false);
    }
    voneClient.publishActuatorStatusEvent(actuatorDeviceId, actuatorCommand, true);
  }

  if (String(actuatorDeviceId) == LEDLight2) {
    String key = "";
    bool commandValue = "";
    for (int i = 0; i < keys.length(); i++) {
      key = (const char*)keys[i];
      commandValue = (bool)commandObjct[keys[i]];
      Serial.print("Key : ");
      Serial.println(key.c_str());
      Serial.print("value : ");
      Serial.println(commandValue);
    }

    if (commandValue == true) {
      Serial.println("LED ON");
      digitalWrite(ledPin2, true);
    } else {
      Serial.println("LED OFF");
      digitalWrite(ledPin2, false);
    }
    voneClient.publishActuatorStatusEvent(actuatorDeviceId, actuatorCommand, true);
  }
  if (String(actuatorDeviceId) == LEDLight3) {
    String key = "";
    bool commandValue = "";
    for (int i = 0; i < keys.length(); i++) {
      key = (const char*)keys[i];
      commandValue = (bool)commandObjct[keys[i]];
      Serial.print("Key : ");
      Serial.println(key.c_str());
      Serial.print("value : ");
      Serial.println(commandValue);
    }

    if (commandValue == true) {
      Serial.println("LED ON");
      digitalWrite(ledPin3, true);
      greenFlag = 1;
    } else {
      Serial.println("LED OFF");
      digitalWrite(ledPin3, false);
      greenFlag = 2;
    }
    voneClient.publishActuatorStatusEvent(actuatorDeviceId, actuatorCommand, true);
  }
}

void motion_detect() { 
  digitalWrite(ledPin3, HIGH); 
 
  Serial.println("Someone is here...TURN ON the light..."); 
} 

void motion_undetect() { 
  digitalWrite(ledPin3, LOW); 
 
  Serial.println("Someone is left...Turn OFF the light..."); 
} 

void setup() {
  setup_wifi();
  voneClient.setup();
  Serial.println("Trigger");
  voneClient.registerActuatorCallback(triggerActuator_callback);

  //sensor
  dht.begin();
  pinMode(irPin, INPUT);
  pinMode(ledPin1, OUTPUT);
  pinMode(ledPin2, OUTPUT);
  pinMode(ledPin3, OUTPUT);
}

void loop() {
  Serial.begin(9600);

  if (Serial.available() > 0) {
    char signal = Serial.read(); 

    if (signal == '1') {  // Intruder or Anomality detected
      digitalWrite(ledPin1, HIGH); 
    } 
    else if (signal == '0') {  // Authorized person and Normal Condition
      digitalWrite(ledPin1, LOW); 
    }
  }

  if (!voneClient.connected()) {
    voneClient.reconnect();
    String errorMsg = "DHTSensor Fail";
    voneClient.publishDeviceStatusEvent(DHT11Sensor, true);
    voneClient.publishDeviceStatusEvent(IRSensor, true);
    voneClient.publishDeviceStatusEvent(MQ2Sensor, true);
  }
  voneClient.loop();


  unsigned long cur = millis();
  if (cur - lastMsgTime > INTERVAL) {
    lastMsgTime = cur;

    float humidity = dht.readHumidity();
    int temperature = dht.readTemperature();

    JSONVar payloadObject;
    payloadObject["Humidity"] = humidity;
    payloadObject["Temperature"] = temperature;
    voneClient.publishTelemetryData(DHT11Sensor, payloadObject);

    int InfraredVal = !digitalRead(irPin);
    voneClient.publishTelemetryData(IRSensor, "Obstacle", InfraredVal);

    if (InfraredVal == 1) { 
      motion_detect();
    } else {
      delay(3000);
      motion_undetect();
    }

    float gas_level = analogRead(mq2Pin);
    voneClient.publishTelemetryData(MQ2Sensor, "Gas detector", gas_level);

    Serial.print(gas_level);
    Serial.print(",");
    Serial.print(temperature);
    Serial.print(",");
    Serial.println(humidity);
  }
}