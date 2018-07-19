///////////////////////////////////////////////////////////////////////
// Project           : OASYS : Mixkey                                //
//                                                                   //
// Description       : Arduino Mixkey Support [ AIS NBIoT,UC20 ]     //
//                     ---------------SENSOR---------------------    //
//                     DHT12 , Rain Sensor , Ultarsonic , SSD1306    //
// Author            : Attaphon Wongwikitkan                         //
// Date created      : 2018/06/21                                    //
//                                                                   //
///////////////////////////////////////////////////////////////////////
///////OLED SETUP///////////////////////////
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
Adafruit_SSD1306 display(-1);
///////DHT12 SETUP/////////////////////////
#include <DHT12.h>
DHT12 dht12;
//////ULTRASONIC SETUP/////////////////////
#define Ultra_Pin A3
float UltraAlpha = 0.8;
//////RAIN SENSOR SETUP////////////////////
#define Rain_Pin 3
float RainCaribate = 0.20;
int RainCount = 0;
///////SENSOR VAR//////////////////////////
float Humi=0.00;
float Pressure=0.00;
float Temp=0.00;
float Rain=0.00;
int Ultra=0;
///////STATION VAR/////////////////////////
String StationID = "STI04";
//////STAT SETUP///////////////////////////
#include "QuickStats.h"
QuickStats stats;
//////TIME SETUP///////////////////////////
const long interval_send = 80000;  //millisecond
const long interval_readsensor = 5000;
const long interval_rain = 50;
unsigned long previousMillis_send = 0;
unsigned long previousMillis_readsensor = 0;
unsigned long previousMillis_rain = 0;
/////SENDDATA SETUP////////////////////////
String ApiUrl="http://www.crflood.com/crflood/get_data/cr.php?";
/////NBIoT SETUP///////////////////////////
#include "AIS_NB_BC95.h"
String apnName = "devkit.nb";
String serverIP = "128.199.91.90"; // Your Server IP
String serverPort = "41234"; // Your Server Port
AIS_NB_BC95 AISnb;
/////3G SETUP//////////////////////////////
//#include "TEE_UC20.h"
//#include "http.h"
//#include "internet.h"
//HTTP http;
//INTERNET net;
////SIM MY by CAT internet
//#define APN "internet"
//#define USER ""
//#define PASS ""
///////SETUP FUNC//////////////////////////
void setup() {
  Serial.begin(9600);
///////OLED SETUP////////////////////// /// 
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  ShowLOGO();
  delay(2000);
///////NETWORK SETUP///////////////////////
  ShowConnect();
///////NBIoT///////////////////////////////
  AISnb.debug = true;
  AISnb.setupDevice(serverPort);
///////3G/////////////////////////////////
//  gsm.begin(&Serial1,9600);
//  gsm.Event_debug = debug;
//  gsm.PowerOn(); 
//  while(gsm.WaitReady()){}
//  net.DisConnect();
//  net.Configure(APN,USER,PASS);
//  net.Connect();
//////////////////////////////////////////
  ShowConnected();
///////DHT12 SETUP/////////////////////////
  dht12.begin();
//////ULTRASONIC SETUP/////////////////////
  pinMode(Ultra_Pin, INPUT);
//////RAIN SETUP///////////////////////////
  attachInterrupt(digitalPinToInterrupt(Rain_Pin), RainIn, FALLING);
//////TIME SETUP///////////////////////////
  previousMillis_send = millis();
  previousMillis_readsensor = millis();
//////LED Rotation/////////////////////////
  display.setRotation(3);
}
///////LOOP START/////////////////////////
void loop() {
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis_send >= interval_send){
    SendData();
    previousMillis_send = currentMillis;
    ShowSendData();
  }
  if (currentMillis - previousMillis_readsensor >= interval_readsensor){
    ReadSensor();
    previousMillis_readsensor = currentMillis;
  }
  ShowSensorData();
}
///////SEND DATA FUNCTION/////////////////
void SendData(){
  String SendString = ApiUrl;
  SendString=SendString+"U="+String(Ultra)+"&";
  SendString=SendString+"T="+String(Temp)+"&";
  SendString=SendString+"H="+String(Humi)+"&";
  SendString=SendString+"R="+String(Rain)+"&";
  SendString=SendString+"B=12.00&";
  SendString=SendString+"P=NRCT&";
  SendString=SendString+"M="+StationID;
  /////////////////////////////////////////
  //NBIoT
  UDPSend udp = AISnb.sendUDPmsgStr(serverIP, serverPort,SendString);
  //3G
//  http.begin(1);
//  http.url(SendString);
//  http.get();
  /////////////////////////////////////////
  ResetRain();
}
///////READ SENSOR FUNCTION///////////////
///////   MAIN ///////////////////////////
void ReadSensor(){
  ReadDHT12();
  ReadUltra();
  ReadRain();
}
/////////// DHT12 ////////////////////////
void ReadDHT12(){
  DHT12::ReadStatus chk = dht12.readStatus();
    Serial.println("--------Read DHT12--------");
    switch (chk) {
    case DHT12::OK:
      Humi = dht12.readHumidity();
      Temp = dht12.readTemperature();
      Serial.print("Temperature : ");
      Serial.println(Temp);
      Serial.print("Humidity : ");
      Serial.println(Humi);
      break;
    default:
      Serial.println("Not Found DHT12");
      break;
    }
}
/////////// ULTRASONIC////////////////////
void ReadUltra(){
  float UltraDataSet[20];
  for(int i=0 ; i<20 ; i++){
    UltraDataSet[i] = (analogRead(Ultra_Pin)*2);
    delay(100);
  }
  Ultra = (Ultra*(1-UltraAlpha))+(stats.mode(UltraDataSet,20,10.00)*UltraAlpha);
}
////////// RAIN /////////////////////////
void RainIn(){
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis_rain >= interval_rain){
    RainCount++;
    previousMillis_rain = currentMillis;
  }
}
void ReadRain(){
  Rain = RainCount*RainCaribate;
}
void ResetRain(){
  RainCount=0;
}
///////DISPLAY FUNCTION//////////////////
void ShowConnected(){
  display.clearDisplay();
  display.drawConnectedLogo();
  display.setCursor(30,52);
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.print("Connected");
  display.display();
}
void ShowSendData(){
  display.clearDisplay();
  display.drawConnectedLogo();
  display.setCursor(30,52);
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.print("Send Data");
  display.display();
}
void ShowConnect(){
  display.clearDisplay();
  display.drawConnectLogo();
  display.setCursor(7,52);
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.print("Connecting Network");
  display.display();
}
void ShowLOGO(){
  display.clearDisplay();
  display.drawLogo();
  display.display();
}
void ShowText(String Text){
  display.clearDisplay();
  display.setCursor(0,0);
  display.setTextColor(WHITE);
  display.setTextSize(2);
  display.println(Text);
  display.display();
}
void ShowSensorData(){
  display.clearDisplay(); 
  display.setTextColor(WHITE,BLACK);   //Text is white ,background is black
  display.setCursor(0, 0);
  display.setTextSize(2);
  display.print("CRF");
  display.setTextSize(1);
  display.setCursor(33, 8);
  display.println("lood");
  display.println("----------");
  display.println("  "+StationID+"  ");
  display.println("----------");
  display.print((interval_send-(millis()-previousMillis_send))/1000);
  display.println(" sec");
  display.println("----------");
  display.setTextSize(2);
  display.print(Ultra);
  display.setTextSize(1);
  display.println("");
  display.println("");
  display.println("    cm    ");
  display.println("----------");
  display.setTextSize(1);
  display.print("T:");
  display.print(Temp);
  display.println(" C");
  display.print("H:");
  display.print(Humi);
  display.println(" %");
  display.print("P:");
  display.print(Pressure);
  display.println(" hPa");
  display.print("R:");
  display.print(Rain);
  display.println(" mm");
  display.println("----------");
  
  display.display();
}
void debug(String data)
{
  Serial.println(data);
}
