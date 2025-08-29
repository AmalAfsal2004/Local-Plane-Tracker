
void Serial_n_Webln(auto text) {
  Serial.println(text);
  WebSerial.println(text);
}
/*
* print() for both online and real life Serial Monitor
*/
void Serial_n_Web(auto text) {
  Serial.print(text);
  WebSerial.print(text);
}

void static_background() {
  //MAIN AIRCRAFT INFO BOX
  dma_display->setFont(&TomThumb);
  dma_display->setTextWrap(false);
  dma_display->fillRect(2,2,60,23,myBLUE);

  //DEPARTURE ARRIVAL
  dma_display->fillRect(2,28,18,9,myBLUE);
  dma_display->fillRect(44,28,18,9,myBLUE);

  dma_display->setTextColor(myYELLOW);
  dma_display->setCursor(7,35);
  dma_display->print("GS");
  dma_display->setCursor(46, 35);
  dma_display->print("SQK.");

/*
  //ARROW
  dma_display->drawLine(21, 32, 42, 32, myWHITE);
  dma_display->drawLine(38,28,42,32,myWHITE);
  dma_display->drawLine(38,36,42,32,myWHITE);
*/

  //ALTITUDE + DISTANCE TO US
  dma_display->fillRect(2,46,18,9,myBLUE);
  dma_display->fillRect(44,46,18,9,myBLUE);

  dma_display->setCursor(4, 53);
  dma_display->setTextColor(myYELLOW);
  dma_display->print("ALT.");
  dma_display->setCursor(46, 53);
  dma_display->print("DIS.");
}

void get_api_one(HTTPClient &client_one, HTTPClient &client_two ,int httpCode){
  String airline_name;
  const char* callsign = nullptr;
  String aircraft_type;
  const char* registration = nullptr;
  const char* squawk = nullptr;
  int gs;
  int alt_baro;
  float distance;

  String payload = client_one.getString();
  DynamicJsonDocument doc(1536);
  deserializeJson(doc,payload);

  if (doc["ac"].size() != 0) {
    callsign = doc["ac"][0]["flight"];
    aircraft_type = doc["ac"][0]["t"].as<String>();
    registration = doc["ac"][0]["r"];
    squawk = doc["ac"][0]["squawk"];
    gs = doc["ac"][0]["gs"];
    alt_baro = doc["ac"][0]["alt_baro"];
    distance = doc["ac"][0]["dst"];

    String ADSB_API_TWO = "https://api.adsbdb.com/v0/aircraft/" + String(registration) + "?callsign=" + String(callsign);
    client_two.begin(ADSB_API_TWO);
    int httpCode_two = client_two.GET();

    if (httpCode_two > 0) {
      get_api_two(client_two, airline_name, aircraft_type);
    }
    else {
      Serial_n_Webln("Failed to connect to API 2!");
      Serial_n_Webln(httpCode_two);
    }

    Serial_n_Webln("Airline Name: ");
    Serial_n_Web(airline_name);

    Serial_n_Webln("Callsign: ");
    Serial_n_Web(callsign);

    Serial_n_Webln("Aircraft Type: ");
    Serial_n_Web(aircraft_type);

    Serial_n_Webln("Registration: ");
    Serial_n_Web(registration);

    Serial_n_Webln("Squawk: ");
    Serial_n_Web(squawk);

    Serial_n_Webln("Ground Speed: ");
    Serial_n_Web(gs);

    Serial_n_Webln("Altitude: ");
    Serial_n_Web(alt_baro);

    Serial_n_Webln("Distance: ");
    Serial_n_Web(distance);

    dma_display->clearScreen();
    static_background();

    dma_display->setFont(NULL);
    dma_display->setCursor(4, 3);
    dma_display->setTextSize(1);
    dma_display->setTextColor(myYELLOW);
    dma_display->print(airline_name);
    dma_display->setTextSize(1);
    dma_display->setFont(&TomThumb); //VERT HAS OFFSET OF 5
    dma_display->setCursor(4,17);
    dma_display->print(callsign);
    dma_display->setCursor(4,24);
    dma_display->print(aircraft_type);

    dma_display->setTextColor(myWHITE);
    dma_display->setCursor(5,44);
    dma_display->print(gs);
    dma_display->setCursor(45, 44);
    dma_display->print(squawk);

    dma_display->setCursor(2, 62);
    dma_display->print(alt_baro);
    dma_display->setCursor(46, 62);
    dma_display->print(distance);

  }
  else {
    Serial_n_Webln("No Planes!");
    dma_display->clearScreen();
    static_background();
    dma_display->setFont(NULL);
    dma_display->setCursor(4, 3);
    dma_display->setTextSize(1);
    dma_display->setTextColor(myYELLOW);
    dma_display->print("No Planes!");
  }
}

void get_api_two(HTTPClient &client_two, String &airline_name, String &aircraft_type) {

  String payload = client_two.getString();
  DynamicJsonDocument doc(1536);
  deserializeJson(doc,payload);

  if (doc["response"] != "unknown callsign") {
    if (doc["response"]["flightroute"]["airline"].size() == 0) {
      airline_name = "Private";
    }
    else {
      airline_name = doc["response"]["flightroute"]["airline"]["name"].as<String>();
    }
    aircraft_type = doc["response"]["aircraft"]["type"].as<String>();

    if (airline_name.length() >= 10) {
      airline_name = airline_name.substring(0,9) + ".";
    }

    if (aircraft_type.length() >= 16) {
      airline_name = airline_name.substring(0,15) + ".";
    }
  }
  else {
    airline_name = "Private"; //Most likely the case
  }
}