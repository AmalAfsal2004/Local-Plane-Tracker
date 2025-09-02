# Plane Tracker Display
Living under or near a busy flight corridor can lead to increasing curiosity. What airline is currently flying over me? What plane is it? How fast is it going? As an aviation enthusiast, these are all questions I've asked myself various times. This project utilizes an ESP32 and 64x64 Hub75 RGB Matrix to find the closest plane to my location, and display its information. I've included the information of the plane's airline name, aircraft callsign, aircraft type, ground speed, transponder/squawk code, altitude, and distance between yourself and the plane in miles.

## Physical Components
- ESP-WROOM-32 ESP32 ESP-32S Development Board 2.4GHz Dual-Mode WiFi + Bluetooth (Brand: AITRIP)
- 64x64 HUB 75 RGB MATRIX
- Two 5v Power Supplies to power Display and ESP32 (Will most likely switch to one 5V Power Supply with more amps in the future)
- Power Supply Breadboard (Connect one of the power supplies to this, which then powers the ESP32)
- Breadboard (For the Power Supply Breadboard to connect to)
- Jumper Cables, female to and female to male

<p align = "center">
<img src = "https://github.com/AmalAfsal2004/Local-Plane-Tracker/blob/main/images/kor_air(1).jpg?raw=true"/>
</p>

## APIs
This program utilizes two ADSB APIs: adsb.lol & adsbdb \
Links: \
https://api.adsb.lol/docs \
https://www.adsbdb.com/ \
I decided to use two APIs because while adsb.lol returns information about plane registration, ground speed, transponder/squawk code, altitude, and distance to user, it lacks the airline name and descriptive information about the aircraft type (adsb.lol has aircraft type information but its been simplified quite a lot, ie: B738 vs 737-800). adsbdb has a working database of aircraft, but more specifically Airline Name and a descriptive plane type. By using the registration and callsign information from adsb.lol, I could plug it into the adsbdb API which returns me the airline name and aircraft type. 

Disclaimer: I am by no means trying to diminish the value that these two ADSB data vendors provide, especially for freeware. I am simply trying to make the best out of both products to enhance my project.
