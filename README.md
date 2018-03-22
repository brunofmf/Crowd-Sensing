# Probe-request-counter
A probe request counter using ESP8266 NodeMCU and Arduino IDE

Just open the sketch on Arduino IDE and, <b>optionally</b>, redefine some variables:
- ARRAY_SIZE (size of the Probe Data Array)
- FIREBASE_HOST, FIREBASE_AUTH and FIREBASE_PUSH (Firebase setup)
- STATION_NETWORK and STATION_PASSWORD (WiFi credentials - if not defined the board will only work as an AP)

Then, on the Tools tab define the correct options (baud, board, port, ...) and upload it to the board.

You may open the Serial Monitor to visualize received probes. 
It also accepts the following commands:
- <b>Stop</b> - stops the handlers that capture the data
- <b>Restart</b> - restarts the handlers if they have been stopped
- <b>Count</b> - prints the number of distinct detected devices
- <b>Print</b> - prints the entire list of detected devices
- <b>Clear</b> - clears the entire list of detected devices
- <b>Send</b> - sends the list of detected devices in JSON to the specified Firebase server
- <b>Start Timer</b> - starts a timer that sends the list of detected devices every *sendTimer* seconds (default: 45s), in JSON, to the specified Firebase server
- <b>Stop Timer</b> - stop the timer
