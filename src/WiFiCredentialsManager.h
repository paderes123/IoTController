#ifndef WIFI_CREDENTIALS_MANAGER_H
#define WIFI_CREDENTIALS_MANAGER_H

#include <WiFi.h>
#include <WebServer.h>
#include <Preferences.h>
#include <functional>

class WiFiCredentialsManager {
public:
    WiFiCredentialsManager();

    void begin();
    void end();
    bool loadCredentials(String &ssid, String &password);
    void saveCredentials(const String &ssid, const String &password);
    void clearCredentials();
    bool connect(const char* ssid, const char* password);
    void startAP();

    // Set a callback for real-time status messages
    void setStatusCallback(std::function<void(const String&)> callback);

private:
    void handleRoot();
    void handleSave();

    Preferences preferences;
    WebServer server;
    std::function<void(const String&)> statusCallback; // Callback for status messages

    const char* AP_SSID = "IoT Remote Control";
    const char* AP_PASSWORD = "12345678";

    const char* html = R"(
    <!DOCTYPE html>
    <html lang="en">
    <head>
        <meta charset="UTF-8">
        <title>IoT Controller Setup</title>
        <style>
        body {
            font-family: Arial, sans-serif;
            max-width: 400px;
            margin: 50px auto;
            padding: 20px;
            background-color: #f4f4f4;
            border-radius: 8px;
            box-shadow: 0 0 10px rgba(0,0,0,0.1);
        }
        h2 {
            text-align: center;
        }
        label {
            display: block;
            margin-top: 15px;
            font-weight: bold;
        }
        input[type="text"],
        input[type="password"] {
            width: 100%;
            padding: 8px;
            margin-top: 5px;
            box-sizing: border-box;
        }
        input[type="submit"] {
            margin-top: 20px;
            width: 100%;
            padding: 10px;
            background-color: #4CAF50;
            color: white;
            border: none;
            border-radius: 4px;
            font-size: 16px;
            cursor: pointer;
        }
        input[type="submit"]:hover {
            background-color: #45a049;
        }
        </style>
    </head>
    <body>
        <h2>IoT Controller WiFi Setup</h2>
        <form action="/save" method="POST">
        <label for="ssid">SSID</label>
        <input type="text" id="ssid" name="ssid" required>
    
        <label for="password">Password</label>
        <input type="password" id="password" name="password">
    
        <input type="submit" value="Save">
        </form>
    </body>
    </html>
    )";   
};

#endif // WIFI_MANAGER_H