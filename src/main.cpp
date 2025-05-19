#include <Arduino.h>
#include <TextWrapper.h> // TextWrapper for OLED display
#include "WiFiCredentialsManager.h" 
#include <ArduinoJson.h>
#include "ExampleFunctions.h" // Provides the functions used in the examples.

#define API_KEY "AIzaSyBluqS7p9GVlAT-ONZnggJyaUIiIAWxV24"
#define USER_EMAIL "paderesbryan08@gmail.com"
#define USER_PASSWORD "12345678"
#define DATABASE_URL "https://mobileappiotcontrollerdb-default-rtdb.asia-southeast1.firebasedatabase.app/"

// for OLED
U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);
TextWrapper* wrapper;
WiFiCredentialsManager wifiManager;

// LED pin definitions
const byte led1 = 2;  // Switch1
const byte led2 = 4;  // Switch2
const byte led3 = 5;  // Switch3
const byte led4 = 18; // Switch4
const byte led5 = 19; // Switch5

// For Temperature sensor
#define vRef 3.3                 // ADC reference voltage
#define ADC_Resolution 4095.0   // 12-bit resolution
const int lm35_pin = 32;        // Analog pin connected to LM35
const float temperatureOffset = 11.0;  // Calibration offset

void processData(AsyncResult &aResult);

SSL_CLIENT ssl_client, stream_ssl_client;

using AsyncClient = AsyncClientClass;
AsyncClient aClient(ssl_client), streamClient(stream_ssl_client);

UserAuth user_auth(API_KEY, USER_EMAIL, USER_PASSWORD, 3000 /* expire period in seconds (<3600) */);
FirebaseApp app;
RealtimeDatabase Database;
AsyncResult streamResult;

unsigned long ms = 0;

void sendMessageToOled(const String& message) {
  u8g2.begin();
  u8g2.clearBuffer();
  wrapper = new TextWrapper(u8g2);
  wrapper->WrapAndDisplayText(message.c_str(), 0, 10, 12);
  u8g2.sendBuffer();
}
void setup()
{    
    // Set the status callback
    wifiManager.setStatusCallback(sendMessageToOled);
    wifiManager.begin();

    String ssid, password;
    if (wifiManager.loadCredentials(ssid, password)) {
        if (wifiManager.connect(ssid.c_str(), password.c_str())) {
            sendMessageToOled("Connected to WiFi successfully.");
        } else {
            sendMessageToOled("Failed to connect. Starting AP mode...");
            wifiManager.startAP();
        }
    } else {
        sendMessageToOled("No saved credentials. Starting AP mode...");
        wifiManager.startAP();
    }
    
    // Initialize LED pins
    pinMode(led1, OUTPUT);
    pinMode(led2, OUTPUT);
    pinMode(led3, OUTPUT);
    pinMode(led4, OUTPUT);
    pinMode(led5, OUTPUT);

    // Initialize Firebase
    set_ssl_client_insecure_and_buffer(ssl_client);
    set_ssl_client_insecure_and_buffer(stream_ssl_client);

    initializeApp(aClient, app, getAuth(user_auth), auth_debug_print, "authTask");

    app.getApp<RealtimeDatabase>(Database);
    Database.url(DATABASE_URL);

    // Set SSE filters for stream events
    streamClient.setSSEFilters("get,put,patch,keep-alive,cancel,auth_revoked");

    // Start stream at /IoTControllerDB
    Database.get(streamClient, "/IoTControllerDB", processData, true /* SSE mode */, "streamTask");
    sendMessageToOled("Success!");
}

int getTemperatureInCelsius() {
    const int sampleCount = 10;
    int total = 0;
    
    unsigned long startMillis = millis();
    
    for (int i = 0; i < sampleCount; i++) {
        total += analogRead(lm35_pin);
        while (millis() - startMillis < 10);  // Non-blocking delay of 10ms
        startMillis = millis();  // Reset the millis for the next delay
    }
    
    float averageReading = total / float(sampleCount);
    float voltage = (averageReading / ADC_Resolution) * vRef;
    float temperatureC = (voltage / 0.01) + temperatureOffset;
    return temperatureC;
}


void loop()
{
    // Maintain Firebase authentication and async tasks
    app.loop();

    // Periodically send temperature values (every 2 seconds)
    if (app.ready() && millis() - ms > 2000)
    {
        ms = millis();

        // Send temperature as a float
        Database.set<float>(aClient, "/IoTControllerDB/Device2/Temperature/Value", getTemperatureInCelsius(), processData, "setTempTask");
    }
}


void processData(AsyncResult &aResult)
{
    if (!aResult.available())
        return;

    RealtimeDatabaseResult &RTDB = aResult.to<RealtimeDatabaseResult>();
    if (RTDB.isStream())
    {
        // Process SwitchStates
        if (RTDB.dataPath() == "/Device1/SwitchStates")
        {
            String data = RTDB.to<String>();
            if (data != "null")
            {           
                StaticJsonDocument<256> doc;
                DeserializationError error = deserializeJson(doc, data);
                if (!error)
                {
                    bool switch1 = doc["Switch1"];
                    bool switch2 = doc["Switch2"];
                    bool switch3 = doc["Switch3"];
                    bool switch4 = doc["Switch4"];
                    bool switch5 = doc["Switch5"];
                    digitalWrite(led1, switch1 ? HIGH : LOW);
                    digitalWrite(led2, switch2 ? HIGH : LOW);
                    digitalWrite(led3, switch3 ? HIGH : LOW);
                    digitalWrite(led4, switch4 ? HIGH : LOW);
                    digitalWrite(led5, switch5 ? HIGH : LOW);
                }
            }
        }
        else if (RTDB.dataPath().startsWith("/Device1/SwitchStates/"))
        {
            bool state = RTDB.to<bool>();
            if (RTDB.dataPath().endsWith("/Switch1"))
            {
                digitalWrite(led1, state ? HIGH : LOW);
            }
            else if (RTDB.dataPath().endsWith("/Switch2"))
            {
                digitalWrite(led2, state ? HIGH : LOW);
            }
            else if (RTDB.dataPath().endsWith("/Switch3"))
            {
                digitalWrite(led3, state ? HIGH : LOW);
            }
            else if (RTDB.dataPath().endsWith("/Switch4"))
            {
                digitalWrite(led4, state ? HIGH : LOW);
            }
            else if (RTDB.dataPath().endsWith("/Switch5"))
            {
                digitalWrite(led5, state ? HIGH : LOW);
            }
        }

        // Process TextMessage
        if (RTDB.dataPath() == "/Device3/TextMessage" || RTDB.dataPath() == "/Device3/TextMessage/Value")
        {
            String data = RTDB.to<String>();
            if (data != "null")
            {
                StaticJsonDocument<128> doc;
                DeserializationError error = deserializeJson(doc, data);
                if (!error && doc.containsKey("Value"))
                {
                    String textMessage = doc["Value"].as<String>();
                    sendMessageToOled(textMessage.c_str());
                }
            }
        }
    }
}