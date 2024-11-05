#include <WiFi.h>
#include <PubSubClient.h>
#include <MFRC522.h>
#include <HTTPClient.h>

#define SS_PIN 5
#define RST_PIN 0
MFRC522 rfid(SS_PIN, RST_PIN);

const char* ssid = "Converge_AZU9";
const char* password = "497362SW";
const char* server = "http://192.168.1.2:84/esp_rfid_project/query.php";

WiFiClient espClient;
PubSubClient client(espClient);

// Declare the functions at the top
void connectWiFi();
void sendToServer(String rfidValue);
String urlEncode(String str);

void setup() {
    Serial.begin(115200);
    SPI.begin();  // Init SPI bus
    rfid.PCD_Init();  // Init MFRC522

    connectWiFi();  // Call the function to connect to WiFi
}

void loop() {
    // Indicate that the device is scanning
    Serial.println("Scanning for RFID...");

    // Check if a new card is present
    if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial()) return;

    // Build the RFID value string from the card
    String rfid_value = "";
    for (byte i = 0; i < rfid.uid.size; i++) {
        rfid_value += String(rfid.uid.uidByte[i], HEX);
    }

    // Indicate that the card has been scanned and is being sent
    Serial.print("RFID detected: ");
    Serial.println(rfid_value);
    Serial.println("Sending RFID value to server...");

    // Send the RFID value to the PHP server
    sendToServer(rfid_value);  // Call the function to send the RFID value to the server

    delay(2000);  // Delay for readability
}

void connectWiFi() {
    // Connect to WiFi network
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("Connecting to WiFi...");
    }
    Serial.println("Connected to WiFi");
    Serial.print("WiFi IP Address: ");
    Serial.println(WiFi.localIP());
}

void sendToServer(String rfidValue) {
    // Check WiFi connection before making the request
    if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;  // Declare HTTP client object

        // Make sure this URL matches your PHP server
        http.begin(server);

        http.addHeader("Content-Type", "application/x-www-form-urlencoded");

        // Prepare the POST request payload
        String httpRequestData = "rfid=" + rfidValue;  // The data sent via POST
        Serial.println("Sending RFID data to server: " + httpRequestData);

        // Send the POST request
        int httpResponseCode = http.POST(httpRequestData);  // Perform the POST request with the payload

        // Check the response code
        if (httpResponseCode > 0) {
            String response = http.getString();  // Get the response from the server
            Serial.print("HTTP Response code: ");
            Serial.println(httpResponseCode);
            Serial.println("Server Response: ");
            Serial.println(response);
        } else {
            Serial.print("Error on HTTP request: ");
            Serial.println(httpResponseCode);
        }

        http.end();  
    } else {
        Serial.println("WiFi not connected");
    }
}
 //URL encoding//
String urlEncode(String str) {
    String encodedString = "";
    char c;
    char code0;
    char code1;
    for (int i = 0; i < str.length(); i++) {
        c = str.charAt(i);
        if (c == ' ') {
            encodedString += '+';
        } else if (isalnum(c)) {
            encodedString += c;
        } else {
            code1 = (c & 0xf) + '0';
            if ((c & 0xf) > 9) {
                code1 = (c & 0xf) - 10 + 'A';
            }
            c = (c >> 4) & 0xf;
            code0 = c + '0';
            if (c > 9) {
                code0 = c - 10 + 'A';
            }
            encodedString += '%';
            encodedString += code0;
            encodedString += code1;
        }
    }
    return encodedString;
}
