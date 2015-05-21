
#include <stdio.h>

// Ethernet shield
#include <SPI.h>
#include <utility/w5100.h>
#include <Dns.h>
#include <Ethernet.h>
#include <EthernetClient.h>
#include <EthernetServer.h>
#include <EthernetUdp.h>
#include <util.h>
// Web server
#include <Flash.h>
#include <SD.h>
#include <TinyWebServer.h>

// LCD shield
#include <LiquidCrystal.h>

// DHT11 Temperature sensor
#include "DHT.h"
DHT dht;

// Light lux sensor 
#include <Wire.h>
#include <BH1750.h>
BH1750 lightMeter;

// LCD
LiquidCrystal lcd(8, 9, 3, 5, 6, 7);


// network
byte mac[] = { 0xDE, 0xAD, 0xBE, 0x1F, 0x3E, 0x5C };
byte ip[] = { 10, 0, 0, 10 };
byte dns_server[] = { 8, 8, 8, 8 };
byte gateway[] = { 10, 0, 0, 138 }; 
byte subnet[] = { 255, 255, 255, 0 };
unsigned int localPort = 8888;      // local port to listen for UDP packets

// A UDP instance to let us send and receive packets over UDP
EthernetUDP Udp;


// Senror pins
#define SOIL1 15

// Init the DS1302
#include <DS1302RTC.h>
#include <Time.h>
// Set pins:  CE, IO,CLK
DS1302RTC RTC(22, 23, 24);

// Alarms
#include <TimeAlarms.h>


// Relays
#define R1 40
#define R2 41
#define R3 42
#define R4 43
#define R5 44
#define R6 45

static char timeStamp[9] = "HH:MM:SS";
static char dateStamp[11] = "DD-MM-YYYY";

int R1State = LOW;

// Web server
boolean file_handler(TinyWebServer& web_server);
boolean index_handler(TinyWebServer& web_server);
boolean has_filesystem = true;
// SD
Sd2Card card;
SdVolume volume;
SdFile root;
SdFile file;

const int sdChipSelect = 4;            // SD card chipSelect

boolean has_ip_address = false;

TinyWebServer::PathHandler handlers[] = {
  {"/", TinyWebServer::GET, &index_handler },
  {"/" "*", TinyWebServer::GET, &file_handler },
  {NULL},
};
TinyWebServer web = TinyWebServer(handlers, NULL);


void setup()
{
  Serial.begin(9600);
  Serial.println("Initializing network...");
  Ethernet.begin(mac, ip, dns_server, gateway, subnet);
  Serial.println("Network initialized...");
  // get time from ntp/rtc
  setNTP();
  //start LCD
  lcd.begin(16, 2);
  // Start lux sensor  
  lightMeter.begin();
  // Start temperature & humidity sensor 
  dht.setup(2); // data pin 2
  // Start soil humidity sensor
  //pinMode(SOIL1, INPUT); 

  // set relays
  pinMode(R1,OUTPUT);
  pinMode(R2,OUTPUT);
  pinMode(R3,OUTPUT);
  pinMode(R4,OUTPUT);
  pinMode(R5,OUTPUT);
  pinMode(R6,OUTPUT);

  //setting alarms
  genTimeStamp();
  Alarm.timerRepeat(1, genTimeStamp);
  genDateStamp();
  Alarm.alarmRepeat(0,0,0, genDateStamp);
  Alarm.timerRepeat(1, lcdDashboard);
  Alarm.timerRepeat(1, getSensors);

  //Alarm.timerRepeat(10, R1Toggle);
  
  //Web server
  Serial << F("Free RAM: ") << FreeRam() << "\n";
  
  pinMode(53, OUTPUT); // set the SS pin as an output (necessary!)
  //digitalWrite(53, HIGH); // but turn off the W5100 chip!
  // initialize the SD card
  
  Serial << F("Setting up SD card...\n");

  if (!card.init(SPI_FULL_SPEED, 4)) {
    Serial << F("card failed\n");
    has_filesystem = false;
  }
  // initialize a FAT volume
  if (!volume.init(&card)) {
    Serial << F("vol.init failed!\n");
    has_filesystem = false;
  }
  if (!root.openRoot(&volume)) {
    Serial << F("openRoot failed!\n");
    has_filesystem = false;
  }

}


// global sensor data

int air_hum = 0;
int air_tmp = 0;
int soi_hum = 0;
uint16_t lux = 0;
static char printSensors[30];


void loop()
{
  Serial.print(dateStamp);
  Serial.print(" ");
  Serial.print(timeStamp); 
  Serial.print(" ");
  Serial.println(printSensors);
  web.process();
  Alarm.delay(dht.getMinimumSamplingPeriod()+100);
}


void getSensors()
{
  air_hum = dht.getHumidity();
  air_tmp = dht.getTemperature();
  soi_hum = analogRead(SOIL1);
  lux = lightMeter.readLightLevel();
  sprintf(printSensors,"%dl %dC %d%% %d%",lux,air_tmp,air_hum,soi_hum);
}

void genTimeStamp()
{
    sprintf(timeStamp,"%02d:%02d:%02d",hour(),minute(),second());
}

void genDateStamp()
{
    sprintf(dateStamp,"%02d-%02d-%04d",day(),month(),year());
}

void lcdDashboard()
{
   lcd.setCursor(0,0);
   lcd.print(timeStamp);
   lcd.setCursor(0,1);
   lcd.print(printSensors);
}

void on(byte pin)
{
    digitalWrite(pin, HIGH);
}

void off(byte pin)
{
    digitalWrite(pin, LOW);
}

void R1Toggle()
{ 
  if ( R1State == LOW ) {
    R1State = HIGH; 
  } else { 
    R1State = LOW;
  }
  digitalWrite(R1, R1State);
}


