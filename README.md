# Probe-request-counter
A probe request counter using ESP8266 NodeMCU and Arduino IDE

Just open the sketch on Arduino IDE and, <b>optionally</b>, redefine some variables (it will still work if you don't redefine them):
- arraySize, clientNetwork, clientPassword, ssid, password, serverURL and sightingsInterval

Then, on the Tools tab define the correct options (baud, board, port, ...) and upload it to the board.

You may open the Serial Monitor to visualize received probes. 
It also accepts the following commands:
- <b>Stop</b> - stops the handlers that capture the data
- <b>Restart</b> - restarts the handlers if they have been stopped
- <b>Count</b> - prints the number of distinct detected devices
- <b>Print</b> - prints the entire list of detected devices
- <b>Send</b> - sends the list of detected devices in JSON to the specified server
