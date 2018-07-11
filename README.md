# Crowd-Sensing
A Crowd Sensor using ESP8266 NodeMCU and Arduino IDE

## Main Achievements
1. A probe request counter for crowd sensing using ESP8266;
2. Firebase integration being only required the FIREBASE_HOST and the FIREBASE_AUTH;
3. Struct to JSON, being then the JsonObject sent to Firebase;
4. Timers and newSightings detection functions;
5. Interaction with Serial Monitor via commands.

## Setup Notes
Just open the sketch on Arduino IDE and, <i>optionally</i>, redefine some variables:
- FIREBASE_HOST, FIREBASE_AUTH and FIREBASE_PUSH (Firebase setup - it will still print probes if no setup is made for Firebase)
- STATION_NETWORK and STATION_PASSWORD (WiFi credentials - if not defined the board will only work as an AP)

Then, on the Tools tab define the correct options (baud, board, port, ...) and upload the sketch to the board. You should use an upload speed of, at least, 57600 otherwise strange timeouts will occur when uploading the sketch! 
You will also need to install, on Arduino IDE, FirebaseArduino lib.  

![alt text](screenshots/ESP8266ModuleConfigurationArduinoIDE.png "Configuration data on the Tools tab")  

## Available Commands
You may open the Serial Monitor to visualize detailed information. 
It also accepts the following commands:
- <b>Stop</b> - stops the handlers that capture the data
- <b>Restart</b> - restarts the handlers if they have been stopped
- <b>Count</b> - prints the number of distinct detected devices
- <b>Print</b> - prints the entire list of detected devices
- <b>Clear</b> - clears the entire list of detected devices
- <b>Send</b> - sends the list of detected devices in JSON to the specified Firebase database
- <b>Start Timer</b> - starts a timer that sends the list of detected devices every <i>sendTimer</i> seconds (default: 45s), in JSON, to the specified Firebase database
- <b>Stop Timer</b> - stops the timer
