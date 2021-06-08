#include <DHT.h>
#include <ESP8266WiFi.h>
#include "HX711.h"
HX711 scale(12, 14);
float calibration_factor = 2230;
float units;
float ounces;
const int buzzerPin = 2;
const int flexPin = A0;
String apiKey = "AULUV71GF2VCKFIZ";
const char *ssid = "xtz";
const char *pass = "nopass123";
const char* server = "api.thingspeak.com";
#define DHTPIN 0
int ob1 = 16;
int ob2 = 5;
const int trigP = 13;
const int echoP = 15;
int vs =4;
long duration,m;
int d,cm;
int c=0;
DHT dht(DHTPIN, DHT11);
int readPing(){
delay(70);
digitalWrite(trigP,LOW); delayMicroseconds(5);
digitalWrite(trigP,HIGH); delayMicroseconds(10);
digitalWrite(trigP,LOW);
duration = pulseIn(echoP,HIGH);
cm=(duration/2)/29.1;
if (cm==0){
cm=250;
}
if (cm>=250)
{cm=250;}
return cm;
}
51
float weight() {
scale.set_scale(calibration_factor);
Serial.print("Reading: ");
units = scale.get_units(), 10;
if (units < 0)
{
units = 0.00;
}
ounces = units * 0.035274;
Serial.print(units);
Serial.print(" grams");
Serial.print(" calibration_factor: ");
Serial.print(calibration_factor);
Serial.println();
if(Serial.available())
{
char temp = Serial.read();
if(temp == '+' || temp == 'a')
calibration_factor += 1;
else if(temp == '-' || temp == 'z')
calibration_factor -= 1;
}
return units;
}
WiFiClient client;
void setup()
{
delay(10);
dht.begin();
Serial.println("Connecting to ");
Serial.println(ssid);
WiFi.begin(ssid, pass);
while (WiFi.status() != WL_CONNECTED)
{
delay(500);
Serial.print(".");
}
Serial.println("");
Serial.println("WiFi connected");
pinMode(trigP, OUTPUT);
pinMode(echoP, INPUT);
52
pinMode(vs, INPUT);
pinMode(ob1, INPUT);
pinMode(ob2, INPUT);
pinMode(buzzerPin,OUTPUT);
Serial.begin(9600);
Serial.println("HX711 calibration sketch");
Serial.println("Remove all weight from scale");
Serial.println("After readings begin, place known weight on scale");
Serial.println("Press + or a to increase calibration factor");
Serial.println("Press - or z to decrease calibration factor");
scale.set_scale();
scale.tare();
long zero_factor = scale.read_average();
Serial.print("Zero factor: ");
Serial.println(zero_factor);
}
void loop()
{
float h = dht.readHumidity();
float t = dht.readTemperature();
int f;
d = readPing();
float wt = weight();
m=digitalRead(vs);
f = analogRead(flexPin);
Serial.println(f);
delay(100);
if(digitalRead(ob1)==LOW){
c++;
}
if(digitalRead(ob2)==LOW){
c--;
}
if(c<0){
c=0; }
if (isnan(h) || isnan(t) || isnan(d)|| isnan(m)|| isnan(c)|| isnan(f)|| isnan(wt))
{
Serial.println("Failed to read from DHT sensor!");
return;
}
53
if (client.connect(server,80))
{
String postStr = apiKey;
postStr +="&field1=";
postStr += String(t);
postStr +="&field2=";
postStr += String(h);
postStr +="&field3=";
postStr += String(d);
postStr +="&field4=";
postStr += String(m);
postStr +="&field5=";
postStr += String(c);
postStr +="&field6=";
postStr += String(f);
postStr +="&field7=";
postStr += String(wt);
postStr += "\r\n\r\n";
client.print("POST /update HTTP/1.1\n");
client.print("Host: api.thingspeak.com\n");
client.print("Connection: close\n");
client.print("X-THINGSPEAKAPIKEY: "+apiKey+"\n");
client.print("Content-Type: application/x-www-form-urlencoded\n");
client.print("Content-Length: ");
client.print(postStr.length());
client.print("\n\n");
client.print(postStr);
Serial.print("Temperature: ");
Serial.print(t);
Serial.print(" degrees Celcius,Water level,Height: ");
Serial.print(d);
Serial.print(" Vibration: ");
Serial.print(m);
Serial.print(" No of Vehicle: ");
Serial.print(c);
Serial.print(" Humidity: ");
Serial.print(h);
Serial.print(" Bending: ");
Serial.print(f);
Serial.print(" Bridge Load: ");
Serial.print(wt);
Serial.println("%. Send to Thingspeak.");
}
client.stop();
Serial.println("Waiting...");
if ( t > 60 || d > 20 || m > 0 || c > 10 || f > 20 || wt > 20)
{
54
digitalWrite(buzzerPin,HIGH);
Serial.println("Bridge is in Danger...");
Serial.println("Alert has been send to base station...");
Serial.println("Close the bridge...");
}
else{
digitalWrite(buzzerPin,LOW);
Serial.println("Bridge is Safe...");
Serial.println("Everything is fine...");
Serial.println("Data send to the base station...");
}
delay(200);
}
