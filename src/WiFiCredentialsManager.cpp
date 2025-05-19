#include "WiFiCredentialsManager.h"

WiFiCredentialsManager::WiFiCredentialsManager() : server(80) {
    // Default callback does nothing
    statusCallback = [](const String&) {};
}

void WiFiCredentialsManager::setStatusCallback(std::function<void(const String&)> callback) {
    statusCallback = callback;
}

void WiFiCredentialsManager::begin() {
    preferences.begin("wifi-config", false);
    statusCallback("Preferences initialized");
}

void WiFiCredentialsManager::end() {
    preferences.end();
    statusCallback("Preferences closed");
}

bool WiFiCredentialsManager::loadCredentials(String &ssid, String &password) {
    ssid = preferences.getString("ssid", "");
    password = preferences.getString("password", "");
    bool credentialsValid = !(ssid == "" || password == "");
    if (credentialsValid) {
        statusCallback("Loaded credentials: SSID=" + ssid);
    } else {
        statusCallback("No saved credentials found");
    }
    return credentialsValid;
}

void WiFiCredentialsManager::saveCredentials(const String &ssid, const String &password) {
    preferences.putString("ssid", ssid);
    preferences.putString("password", password);
    statusCallback("Saved credentials: SSID=" + ssid);
}

void WiFiCredentialsManager::clearCredentials() {
    preferences.clear();
    statusCallback("Cleared credentials");
}

bool WiFiCredentialsManager::connect(const char* ssid, const char* password) {
    WiFi.begin(ssid, password);
    statusCallback(String("Connecting to WiFi: ") + ssid);
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
        statusCallback("loading...");
        delay(500);
        attempts++;
    }
    if (WiFi.status() == WL_CONNECTED) {
        statusCallback("Connected to WiFi");
        statusCallback(String("IP: ") + WiFi.localIP().toString());
        return true;
    } else {
        statusCallback("Failed to connect to WiFi");
        WiFi.disconnect(true);
        return false;
    }
}

void WiFiCredentialsManager::startAP() {
    WiFi.disconnect(true);
    WiFi.mode(WIFI_AP);
    if (WiFi.softAP(AP_SSID, AP_PASSWORD)) {
        statusCallback("WiFi Manager started");
        statusCallback(String("AP IP: ") + WiFi.softAPIP().toString());
    } else {
        statusCallback("Failed to start AP");
        return;
    }

    server.on("/", std::bind(&WiFiCredentialsManager::handleRoot, this));
    server.on("/save", std::bind(&WiFiCredentialsManager::handleSave, this));
    server.begin();
    statusCallback("Web server started");

    while (true) {
        server.handleClient();
        delay(10);
    }
}

void WiFiCredentialsManager::handleRoot() {
    server.send(200, "text/html", html);
}

void WiFiCredentialsManager::handleSave() {
    String ssid = server.arg("ssid");
    String password = server.arg("password");
    if (connect(ssid.c_str(), password.c_str())) {
        saveCredentials(ssid, password);
        server.send(200, "text/html", "<h2>Credentials Saved. Restarting...</h2>");
        delay(1000);
        ESP.restart();
    } else {
        server.send(200, "text/html", "<h2>Invalid Credentials. Please try again.</h2>");
    }
}