# Probe-request-counter
A probe request counter using ESP8266 NodeMCU and Arduino IDE

Just open the sketch on Arduino IDE and redefine some variables such as:
- (<b>mandatory</b>) clientNetwork and clientPassword (or comment that code and it will not connect to any network but it will still work as an AP and count the probe requests)
- (<b>optional</b>) arraySize, ssid, password, serverURL and sightingsInterval

Then, on the Tools tab define the correct options (baud, board, port, ...) and upload it to the board.

You may open the Serial Monitor to visualize received probes. 
It also accepts the following commands:
- <b>Stop</b> - stops the handlers that capture the data
- <b>Restart</b> - restarts the handlers if they have been stopped
- <b>Count</b> - prints the number of distinct detected devices
- <b>Print</b> - prints the entire list of detected devices
- <b>Send</b> - sends the list of detected devices in JSON to the specified server
