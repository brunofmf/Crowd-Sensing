/*************************************************
    This sketch was written and developed by Bruno Fernandes in March 2018.
    
    Portions of this code are partially based in others such as
    WiFiEvents (by Markus Sattler and Ivan Grokhotkov)
    and FirebaseDemo_ESP8266 (by Google Inc).

    MIT Licensed
*************************************************/
#include <ESP8266WiFi.h>
#include <FirebaseArduino.h>
#include <stdio.h>
#include "user_interface.h"

/** Probe Data Array Size **/
#define ARRAY_SIZE        50

/** Available Commands **/
#define CMD_RESTART       "Restart"
#define CMD_STOP          "Stop"
#define CMD_COUNT         "Count"
#define CMD_PRINT         "Print"
#define CMD_CLEAR         "Clear"
#define CMD_SEND          "Send"
#define CMD_START_TIMER   "Start Timer"
#define CMD_STOP_TIMER    "Stop Timer"

/** Firebase Setup **/
#define FIREBASE_HOST     "dummy.firebaseio.com"  //Set Firebase Host
#define FIREBASE_AUTH     "dummy_auth"            //Set Firebase Auth
#define FIREBASE_PUSH     "ProbeData"

/** WiFi Connection Data **/
#define AP_SSID           "ap-density"
#define AP_PASSWORD       "ap-Pa$$word"
#define STATION_NETWORK   "dummy_net"             //Set station network
#define STATION_PASSWORD  "dummy_pass"            //Set station password

/** Probe Data Struct **/
struct probeData {
  String mac;
  String rssi;
  long previousMillisDetected;
} probeArray[ARRAY_SIZE];

/** Control Variables **/
int currIndex           = 0;
int dumpVersion         = 1;
bool handlersStopped    = false;
String command;
bool isConnected        = false;
bool timerIsActive      = false;
bool sendNow            = false;

/** Time Variables **/
long sightingsInterval  = 60000; //1 minute
long connectionWait     = 15000; //15 seconds
long sendTimer          = 45000; //45 seconds

/** Os Timer **/
os_timer_t theTimer;

/** Event Handlers **/
WiFiEventHandler stationConnectedHandler;
WiFiEventHandler stationDisconnectedHandler;
WiFiEventHandler probeRequestCaptureDataHandler;
WiFiEventHandler probeRequestPrintHandler;

/** Sketch Functions **/
void setup() {
  Serial.begin(9600);
  
  //Don't save WiFi configuration in flash - optional
  WiFi.persistent(false);

  Serial.println(F("*** ESP8266 Probe Request Capture by Bruno Fernandes ***")); 

  //Set up an access point
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP(AP_SSID, AP_PASSWORD);

  //Connect to the network
  long startTime = millis();
  WiFi.begin(STATION_NETWORK, STATION_PASSWORD);
  
  //Wait, at most, connectionWait seconds
  while ( (WiFi.status() != WL_CONNECTED) && (millis() - startTime < connectionWait) ) {
    Serial.print(".");
    delay(500);
  }
  if(WiFi.status() == WL_CONNECTED) {
    Serial.print(" Connected to "); Serial.print(STATION_NETWORK);
    Serial.print("; IP address: "); Serial.print(WiFi.localIP());
    Serial.println(" ......");
    isConnected = true;
  } else{
    Serial.print(" Connection to "); Serial.print(STATION_NETWORK);
    Serial.println(" failed! Board will only work as an AP......");
  }

  if(isConnected){
    //coonect to Firebase
    Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
    Serial.print(F("*** Secure Firebase connection estabilished using HOST: ")); Serial.print(FIREBASE_HOST);
    Serial.print(F(" and using AUTH: ")); Serial.print(FIREBASE_AUTH); Serial.println(" ***");
  } else {    
    Serial.println(F("*** No Firebase Connection was estabilished! ***"));
  }
  
  //Setup timer
  os_timer_setfn(&theTimer, timerCallback, NULL);
  os_timer_arm(&theTimer, sendTimer, true);

  // Register event handlers. Callback functions will be called as long as these handler objects exist.
  // Call "onStationConnected" each time a station connects
  stationConnectedHandler = WiFi.onSoftAPModeStationConnected(&onStationConnected);
  // Call "onStationDisconnected" each time a station disconnects
  stationDisconnectedHandler = WiFi.onSoftAPModeStationDisconnected(&onStationDisconnected);
  // Call "onProbeRequestCaptureData" and "onProbeRequestPrint" each time a probe request is received.
  probeRequestPrintHandler = WiFi.onSoftAPModeProbeRequestReceived(&onProbeRequestPrint);
  probeRequestCaptureDataHandler = WiFi.onSoftAPModeProbeRequestReceived(&onProbeRequestCaptureData);
}

void loop() {
  if(Serial.available() > 0) {
    //read incoming data as string
    command = Serial.readString();
    Serial.println(command);
    if(command.equalsIgnoreCase(CMD_RESTART)){
      restartHandlers();
    } else if(command.equalsIgnoreCase(CMD_STOP)){
      stopHandlers();
    } else if(command.equalsIgnoreCase(CMD_COUNT)){
      Serial.println(currIndex);  
      delay(1000);      
    } else if(command.equalsIgnoreCase(CMD_PRINT)){
      printProbeArray();
      delay(1000);  
    } else if(command.equalsIgnoreCase(CMD_CLEAR)){
      clearData();
      delay(1000);  
    } else if(command.equalsIgnoreCase(CMD_SEND)){
      sendDataCmd();
      delay(1000); 
    } else if(command.equalsIgnoreCase(CMD_START_TIMER)){
      startTimer(); 
    } else if(command.equalsIgnoreCase(CMD_STOP_TIMER)){
      stopTimer();
    } else{
      Serial.println(F("Unknown command!"));
    }
  }
  //If timer told us it is time to send Data
  if(sendNow){
    sendDataFirebase(true);
    sendNow = false;
  }
}

void onStationConnected(const WiFiEventSoftAPModeStationConnected& evt) {
  Serial.print(F("Station connected: "));
  Serial.println(macToString(evt.mac));
}

void onStationDisconnected(const WiFiEventSoftAPModeStationDisconnected& evt) {
  Serial.print(F("Station disconnected: "));
  Serial.println(macToString(evt.mac));
}

void onProbeRequestCaptureData(const WiFiEventSoftAPModeProbeRequestReceived& evt) {
  // We can't use "delay" or other blocking functions in the event handler.
  // Therefore we should set a flag and then check it inside "loop" function.
  if(currIndex < ARRAY_SIZE){
    if(newSighting(evt)){
      probeArray[currIndex].mac = macToString(evt.mac);
      probeArray[currIndex].rssi = evt.rssi;
      probeArray[currIndex++].previousMillisDetected = millis();
    }
  } else{
    Serial.println(F("*** Array Limit Achieved!! Send and clear it to process more probe requests! ***"));    
  }
}

void onProbeRequestPrint(const WiFiEventSoftAPModeProbeRequestReceived& evt) {
  Serial.print("Probe request from: ");
  Serial.print(macToString(evt.mac));
  Serial.print("; RSSI: ");
  Serial.print(evt.rssi);
  Serial.print("; Millis Last Detected: ");
  Serial.println(millis());
}

void timerCallback(void* z){
  if(timerIsActive)
    sendNow = true;
}

void sendDataCmd(){
  if(isConnected){ //Send Data to Firebase only if station connected
    sendDataFirebase(false);
  } else{
    Serial.println(F("*** It is not possible to send data! There is no connection - board is only working as AP ***"));
  }
}

void sendDataFirebase(bool clearD){
  DynamicJsonBuffer jsonBuffer; //The default initial size for DynamicJsonBuffer is 256. It allocates a new block twice bigger than the previous one.
  JsonObject& root = jsonBuffer.createObject(); //Create the Json object
  JsonObject& tempTime = root.createNestedObject("timestamp");
  tempTime[".sv"] = "timestamp";
  JsonArray& probes = root.createNestedArray("probes" + String(dumpVersion++)); //Create child probes array
  //Fill JsonArray with data
  for(int i = 0; i < currIndex; i++){
    JsonObject& probe = probes.createNestedObject();
    probe["mac"] = probeArray[i].mac;
    probe["rssi"] = probeArray[i].rssi;
    probe["previousMillisDetected"] = probeArray[i].previousMillisDetected;
  }
  //Push JSON to Firebase
  Firebase.push(FIREBASE_PUSH, root);
  //Handle error or success
  if (Firebase.failed()){
    Serial.print(F("*** Failed to publish Probe Data! Firebase error: "));
    Serial.print(Firebase.error()); Serial.println(" ***");
  } else{
    if(clearD){
      Serial.println(F("*** Probe Data successfully published in Firebase! Data will be cleared... ***"));
      clearData();
    } else{
      Serial.println(F("*** Probe Data successfully published in Firebase! NO data was cleared... ***"));
    }
  }
}

void clearData(){
  for(int i = 0; i < currIndex; i++){
    probeArray[i].mac = "";
    probeArray[i].rssi = "";
    probeArray[i].previousMillisDetected = 0;
  }
  currIndex = 0;
  Serial.println(F("*** Probe Data successfully cleared! ***"));
}

void stopHandlers(){
  if(!handlersStopped){
    //printProbeArray();
    handlersStopped = true;
    probeRequestPrintHandler = WiFiEventHandler();
    probeRequestCaptureDataHandler = WiFiEventHandler();
    Serial.println(F("*** Services Stopped! ***"));
    timerIsActive = false;
  } else{
    Serial.println(F("*** Services Already Stopped! ***"));
  }
}

void restartHandlers(){
  if(handlersStopped){
    //Restart Probe Request Capture
    handlersStopped = false;
    //Restart handlers
    probeRequestPrintHandler = WiFi.onSoftAPModeProbeRequestReceived(&onProbeRequestPrint);
    probeRequestCaptureDataHandler = WiFi.onSoftAPModeProbeRequestReceived(&onProbeRequestCaptureData);
    Serial.println(F("*** Services Restarted! ***"));
  } else{
    Serial.println(F("*** Services are already running! ***"));
  }
}

void startTimer(){
  if(isConnected && !handlersStopped){ //Send data to Firebase every sendTimer seconds - only if station is connected and handlers not stopped
    timerIsActive = true;
    Serial.println(F("*** Timer started! ***"));
  } else{
    Serial.println(F("*** Not possible to set timer! There is no connection - board is only working as AP OR handlers are Stopped ***"));
  }  
}

void stopTimer(){
  if(isConnected){
    timerIsActive = false;
    Serial.println(F("*** Timer stopped! ***"));
  } else{
    Serial.println(F("*** Not possible to set timer! There is no connection - board is only working as AP ***"));
  }  
}

bool newSighting(const WiFiEventSoftAPModeProbeRequestReceived& evt){
  String mac = macToString(evt.mac);
  long currTime = millis();
  //start by the end as array is ordered - new sightings are at the end - break as soon as it finds mac at the list
  for(int i = currIndex-1; i>=0; i--){
    //if mac has already been captured
    if(mac.equals(probeArray[i].mac)){
      //lets check if enough time has passed (sightingsInterval milliseconds since last sighting)
      if(currTime-probeArray[i].previousMillisDetected < sightingsInterval){
        return false;
      }      
      break;
    }
  }
  return true;
}

void printProbeArray(){
  Serial.println("*** Print detected devices: ***");
    for(int i = 0; i < currIndex; i++){
      Serial.print("MAC: ");
      Serial.print(probeArray[i].mac);
      Serial.print("; RSSI: ");
      Serial.print(probeArray[i].rssi);
      Serial.print("; Millis Last Detected: ");
      Serial.println(probeArray[i].previousMillisDetected);
      Serial.print("--- index: ");Serial.print(i);Serial.println(" ---");
    }
  Serial.println("*** All has been printed ***");
}

String macToString(const unsigned char* mac){
  char buf[20];
  snprintf(buf, sizeof(buf), "%02x:%02x:%02x:%02x:%02x:%02x",mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  return String(buf);
}
