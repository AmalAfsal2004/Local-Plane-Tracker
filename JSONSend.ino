
#include <WiFi.h>
#include <HTTPClient.h>
#include <TinyGPSPlus.h>
#include <ArduinoJson.h>
#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>
#include <Fonts/Picopixel.h>
#include <Fonts/TomThumb.h>
#include <local_info.h>


// Now that wiring is fixed:
#define RXD2 16  // GPS TX goes here
#define TXD2 17  // GPS RX goes here
#define GPS_BAUD 9600

const char* ssid = SSID;
const char* password = PASSWORD;


unsigned long lastTime = 0;
unsigned long timerDelay = 5000;

HardwareSerial gpsSerial(2);
TinyGPSPlus gps;


//----------------------------------------Defines the connected PIN between P3 and ESP32.
#define R1_PIN 4
#define G1_PIN 2
#define B1_PIN 32
#define R2_PIN 33
#define G2_PIN 25
#define B2_PIN 26

#define A_PIN 14
#define B_PIN 12
#define C_PIN 23
#define D_PIN 22
#define E_PIN 27 //--> required for 1/32 scan panels, like 64x64px. Any available pin would do, i.e. IO32.

#define LAT_PIN 19
#define OE_PIN 18
#define CLK_PIN 21
//----------------------------------------

//----------------------------------------Defines the P5 Panel configuration.
#define PANEL_RES_X 64  //--> Number of pixels wide of each INDIVIDUAL panel module. 
#define PANEL_RES_Y 64  //--> Number of pixels tall of each INDIVIDUAL panel module.
#define PANEL_CHAIN 1   //--> Total number of panels chained one to another
//----------------------------------------

MatrixPanel_I2S_DMA *dma_display = nullptr;

//----------------------------------------Variable for color.
// color565(0, 0, 0); --> RGB color code. Use the "color picker" to use or find another color code.
uint16_t myBLACK = dma_display->color565(0, 0, 0);
uint16_t myWHITE = dma_display->color565(255, 255, 255);
uint16_t myRED = dma_display->color565(255, 0, 0);
uint16_t myGREEN = dma_display->color565(0, 255, 0);
uint16_t myBLUE = dma_display->color565(3, 43, 196);
uint16_t myYELLOW = dma_display->color565(252, 182, 0);
//----------------------------------------


void static_backround() {
  //MAIN AIRCRAFT INFO BOX
  dma_display->setFont(&TomThumb);
  dma_display->setTextWrap(false);
  dma_display->fillRect(2,2,60,23,myBLUE);

  //DEPARTURE ARRIVAL
  dma_display->fillRect(2,28,18,9,myBLUE);
  dma_display->fillRect(44,28,18,9,myBLUE);

  dma_display->setTextColor(myYELLOW);
  dma_display->setCursor(4,35);
  dma_display->print("DEP.");
  dma_display->setCursor(46, 35);
  dma_display->print("ARR.");

  //ARROW
  dma_display->drawLine(21, 32, 42, 32, myWHITE);
  dma_display->drawLine(38,28,42,32,myWHITE);
  dma_display->drawLine(38,36,42,32,myWHITE);

  //ALTITUDE + DISTANCE TO US
  dma_display->fillRect(2,46,18,9,myBLUE);
  dma_display->fillRect(44,46,18,9,myBLUE);

  dma_display->setCursor(4, 53);
  dma_display->setTextColor(myYELLOW);
  dma_display->print("ALT.");
  dma_display->setCursor(46, 53);
  dma_display->print("DIS.");
}


void setup() {
  Serial.begin(115200);
  gpsSerial.begin(GPS_BAUD, SERIAL_8N1, RXD2, TXD2);
  delay(3000);  // Give time to open Serial Monitor
  Serial.println("GPS Parser Started");

  
  WiFi.begin(ssid, password);
  Serial.println("Connecting");
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());

  //DISPLAY
    HUB75_I2S_CFG::i2s_pins _pins={R1_PIN, G1_PIN, B1_PIN, R2_PIN, G2_PIN, B2_PIN, A_PIN, B_PIN, C_PIN, D_PIN, E_PIN, LAT_PIN, OE_PIN, CLK_PIN};
  delay(10);

  //----------------------------------------Module configuration.
  HUB75_I2S_CFG mxconfig(
    PANEL_RES_X,   //--> module width.
    PANEL_RES_Y,   //--> module height.
    PANEL_CHAIN,   //--> Chain length.
    _pins          //--> pin mapping.
  );
  delay(10);
  //----------------------------------------

  // Set I2S clock speed.
  mxconfig.i2sspeed = HUB75_I2S_CFG::HZ_10M;  // I2S clock speed, better leave as-is unless you want to experiment.
  delay(10);

  //----------------------------------------Display Setup.
  dma_display = new MatrixPanel_I2S_DMA(mxconfig);
  dma_display->begin();
  dma_display->setBrightness8(6); //--> 0-255.
  //----------------------------------------
  
  dma_display->clearScreen();
  
  dma_display->fillScreen(myWHITE);
  delay(1000);
  dma_display->fillScreen(myRED);
  delay(1000);
  dma_display->fillScreen(myGREEN);
  delay(1000);
  dma_display->fillScreen(myBLUE);
  delay(1000);
  
  dma_display->clearScreen();
  delay(1000);

  static_backround();

}

void loop() {
  /*
    * Reads incoming GPS DATA and parses it
  */
  const char* flight_carrier = nullptr;
  const char* flight_number = nullptr;
  const char* aircraft_type = nullptr;
  const char* origin_airport = nullptr;
  const char* destination_airport = nullptr;
  int current_altitude_ft;
  float distance_miles;

  if ((millis() - lastTime) > timerDelay) {
    if (WiFi.status() == WL_CONNECTED) {
      WiFiClient client;
      HTTPClient http;
          while (gpsSerial.available() > 0) {
            gps.encode(gpsSerial.read());
          }
          if (gps.location.isUpdated()) { //Checks if new data is available
            JsonDocument doc;

            double latitude = gps.location.lat();
            double longitude = gps.location.lng();
            int rad = 30;

            doc["lat"] = latitude;
            doc["lon"] = longitude;
            doc["rad"] = 30;

            String output;

            serializeJson(doc,output);
            Serial.println(output);

            http.begin(client,serverName);
            http.addHeader("Content-Type", "application/json");

            int httpResponseCode = http.POST(output);

            Serial.print("HTTP Response Code: ");
            Serial.println(httpResponseCode);
            http.end();

            if (httpResponseCode > 0) {
                String response = http.getString();
                Serial.print("Response from server: ");
                Serial.println(response);  // Print server's response
                DynamicJsonDocument doc(1024);
                deserializeJson(doc,response);

                if (doc["closest_flight"]["flight_carrier"] == "Private Owner") {
                    flight_carrier = "Private";
                }
                else {
                    flight_carrier = doc["closest_flight"]["flight_carrier"];
                }
                flight_number = doc["closest_flight"]["flight_number"];
                aircraft_type = doc["closest_flight"]["aircraft_type"];
                origin_airport = doc["closest_flight"]["origin_airport"];
                destination_airport = doc["closest_flight"]["destination_airport"];
                current_altitude_ft = doc["closest_flight"]["current_altitude_ft"];
                distance_miles = doc["closest_flight"]["distance_miles"];

                Serial.println(current_altitude_ft);
                Serial.println(distance_miles);

                dma_display->fillRect(2,2,60,23,myBLACK);
                dma_display->fillRect(2,37,18,8,myBLACK);
                dma_display->fillRect(44,37,18,8,myBLACK);
                dma_display->fillRect(2,55,18,8,myBLACK);
                dma_display->fillRect(44,55,18,8,myBLACK);
        
                dma_display->fillRect(2,2,60,23,myBLUE);
                dma_display->setFont(NULL);
                dma_display->setCursor(4, 3);
                dma_display->setTextSize(1);
                dma_display->setTextColor(myYELLOW);
                dma_display->print(flight_carrier);
                dma_display->setFont(&TomThumb); //VERT HAS OFFSET OF 5
                dma_display->setCursor(4,17);
                dma_display->print(flight_number);
                dma_display->setCursor(4,24);
                dma_display->print(aircraft_type);

                dma_display->setTextColor(myWHITE);
                dma_display->setCursor(5,44);
                dma_display->print(origin_airport);
                dma_display->setCursor(47, 44);
                dma_display->print(destination_airport);

                dma_display->setTextColor(myWHITE);
                dma_display->setCursor(2, 62);
                dma_display->print(current_altitude_ft);
                dma_display->setCursor(46, 62);
                dma_display->print(distance_miles);
                d

            } else {
                Serial.println("Error in sending POST request");
            }  
          }
          else {
            Serial.println("WiFi Disconnected");
          }
          lastTime = millis();
    }
  }

}