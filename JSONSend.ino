//LIBRARIES FOR OVER THE AIR UPDATES
#include <Arduino.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ElegantOTA.h>
#include <WebSerial.h>

//ESSENTIAL LIBRARIES
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>
#include <Fonts/Picopixel.h>
#include <Fonts/TomThumb.h>

//HEADER FILE THAT STORES YOUR PRIVATE INFO
#include <local_info.h>

//WiFi user and password
const char* ssid = SSID;
const char* password = PASSWORD;

/*
* Non-Blocking timer delay variables
* Timer delay currently set to 10 seconds
* As per the developers of the API, keep it to 1 request per second maximum
*/
unsigned long lastTime = 0;
unsigned long timerDelay = 10000;

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

//For OTA serial monitor and sketch pushing
AsyncWebServer server(80); 

void setup() {
  Serial.begin(115200);
  delay(3000);  // Give time to open Serial Monitor
  
  WiFi.begin(ssid, password);
  Serial_n_Webln("Connecting");
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial_n_Web(".");
  }
  Serial_n_Webln("");
  Serial_n_Web("Connected to WiFi network with IP Address: ");
  Serial_n_Webln(WiFi.localIP());

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
  delay(1000);

  static_background();

  //MAX DISPLAY LENGTH FOR CARRIER NAME IS GOING TO BE 9

 //===========================================================================
 //Anthing below this line, until the end of the setup loop, is for OTA Functionality
  WebSerial.begin(&server); //ONLINE SERIAL MONITOR
  WebSerial.onMessage([&](uint8_t *data, size_t len) {
    Serial.printf("Received %u bytes from WebSerial: ", len);
    Serial.write(data, len);
    Serial.println();
    WebSerial.println("Received Data...");
    String d = "";
    for(size_t i=0; i < len; i++){
      d += char(data[i]);
    }
    WebSerial.println(d);
  });
  WebSerial.setAuthentication(OTA_USR , OTA_PASS);

  ElegantOTA.begin(&server);   
  ElegantOTA.setAutoReboot(true);
  ElegantOTA.setAuth(OTA_USR , OTA_PASS);

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/plain", "Hi! I am ESP32.");
  });

  server.begin();
  Serial_n_Webln("HTTP server started");

}

//Because ADSB_API_ONE has my personal coordinates, I put the String in a non-included header file
//The link to this API is below
// "https://api.adsb.lol/v2/closest/{lat}/{lon}/{radius}"
// Plug in your latitude, longitute, and radius up to 250 miles, and then uncomment the code.
// String ADSB_API_ONE = "https://api.adsb.lol/v2/closest/{lat}/{lon}/{radius}"

void loop() {

  ElegantOTA.loop(); //OVER THE AIR UPDATE FUNCTIONALITY 

  //Every 10 seconds, according to timer delay
  if ((millis() - lastTime) > timerDelay) {
    lastTime = millis();
    if (WiFi.status() == WL_CONNECTED) {
      HTTPClient client_one; //For API ONE
      HTTPClient client_two; //For API TWO;
      client_one.begin(ADSB_API_ONE);
      int httpCode_one = client_one.GET();

      if (httpCode_one > 0) {
        get_api_one(client_one, client_two, httpCode_one);
      }
      else {
        //If connecting to API 1 fails
        dma_display->clearScreen();
        static_background();
        dma_display->setTextColor(myWHITE);
        dma_display->setTextSize(1);
        dma_display->setFont(&TomThumb); //VERT HAS OFFSET OF 5
        dma_display->setCursor(4,17);
        dma_display->print("API Fetch Fail!");
        dma_display->setCursor(4,24);
        dma_display->print("Check WiFi!");        
        Serial_n_Webln("Could not GET from API 1!");
        Serial_n_Webln(httpCode_one);
      }
    }
  }

}