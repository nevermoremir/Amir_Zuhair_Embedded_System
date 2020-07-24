
#define BLYNK_PRINT Serial    
#include <SPI.h>
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <SimpleTimer.h>
#include <DHT.h>
#include <Servo.h>
#include <Wire.h>
#include <Adafruit_ADS1015.h>

//initialize
Adafruit_ADS1115 ads;  /* Use this for the 16-bit version */

char auth[] = "RmwdfQkchIFmmZV-TNfJDJAHcFZEIHM6"; //Auth Token to connect with Blynk App
char ssid[] = "zakiah@unifi";  //Wifi_name
char pass[] = "123456789";  //Wifi_pass
#define DHTTYPE DHT11     // DHT 11
#define DHTPIN 2          // Digital pin 4
#define RAINPIN A0        //Analog pin 0
#define LDRPIN D0         // D0 pin 
int Rsensor;              //Rain Sensor Value
int LDR;                  //LDR value D
float h;                  //Humidity Value
float t;                  // Temperature Value
int16_t adc0;             // adc value A0 from LDR
int V_LDR;                // hold LDR VALUE 
int b;                    // Control Mode
int xcpy;                 // Copy of state motor
Servo servo;
DHT dht(DHTPIN, DHTTYPE);
SimpleTimer timer;

BLYNK_CONNECTED() // sync state from APP
{
  Blynk.syncVirtual(V3);
   Blynk.syncVirtual(V4);
 }
BLYNK_WRITE(V3) //Check if button mode at app is pressed
 {
   b=param.asInt(); //Triggred when changes made at button app
   
   Serial.println("b=");  //debug purpose
  Serial.println(b);
  
  }

BLYNK_WRITE(V4) // Check if button segmented input user is change
{
  if(b==0)  // Mode is Manual 
  {
   
    
  switch (param.asInt()) // if there are change to segmented button 
  {
    case 1 : {
            
                Serial.println("Open"); // debug purpose
                Motor(0);               //Call Motor fx and sed 0 to Open
                break;
             }
    case 2 : {
                Serial.println("Close");  // debug purpose
                Motor(1);                 //Call Motor fx and sed 0 to Open
                break;
             }
   }
  }
  else if (b==1)   //mode is auto ( for disable segmented button to be changed from user in auto mode)
  {
    if(xcpy==0)
    Blynk.virtualWrite(V4,1); // change state button to current motor position (Open)
    else if(xcpy==1)
    Blynk.virtualWrite(V4,2); // change state button to current motor position  (Close)
    }
}

void sendSensor() // Temp & Humid sensor function
{
  //read from sensor
   h = dht.readHumidity(); //humidity 
   t = dht.readTemperature(); // temperature in Celcius

  if (isnan(h) || isnan(t)) // check fail to connect
  {
    Serial.println("Failed to read from DHT sensor!");    
    return;
  }
 //Send value to Blynk Apps
  Blynk.setProperty(V6,"label","°C");
  Blynk.virtualWrite(V5, h);  //V5 is for Humidity
  Blynk.virtualWrite(V6, t);  //V6 is for Temperature
}



void sendRainSensor()  // Rain sensor function
{
Rsensor=analogRead(RAINPIN); // read analog input from A0 NodeMcu

Serial.print("Rainsensor="); // debug purpose
 Serial.println(Rsensor);
 
  if(Rsensor <950 && Rsensor >900) // Paramater for detecting droplet
{
  Blynk.virtualWrite(V1,"Gonna " );  //V1
  Blynk.virtualWrite(V2," Raining" );  //V2
 }
else if (Rsensor >=950)           // Parameter when there is no droplet
{
  Blynk.virtualWrite(V1,"Not " );  //V1
  Blynk.virtualWrite(V2," Raining" );  //V2
} 
else                              // Other than that is raining -> too much droplet
{
  Blynk.virtualWrite(V1,"Its " );  //V1
  Blynk.virtualWrite(V2," Raining" );  //V2
}

}


void Motor(int x) // Motor function with argument input x=1 :close x=0 :open
{
  xcpy=x;
  if (x==0)  // Open
  servo.write(0); // 0 degree
  else if(x==1) // Close
  servo.write(180);// 180 degree
  delay(1000);
 }

void sendLDR() // LDR function 
{
  adc0 = ads.readADC_SingleEnded(0); // obtain analog value LDR from ADS1115 (Exnteder Analog) 
 V_LDR= (adc0*0.1875)/3.3; // Change input to voltage and divide by 3.3 volt
  Serial.println("LDR="); // debug purpose
  Serial.println(V_LDR);
  if(V_LDR < 500)     // Parameter for daylight
    Blynk.virtualWrite(V0,"Daylight " );  //Write into LCD in Blynk App
   else if(V_LDR>500 && V_LDR<600)        // Paramter for evening
   Blynk.virtualWrite(V0,"Evening " );  //Write into LCD in Blynk App
   else                                   //else for night
    Blynk.virtualWrite(V0,"Night " );  //Write into LCD in Blynk App
}

void setup()
{
  Serial.begin(115200); // See the connection status in Serial Monitor
  ads.begin();
  servo.attach(14); //D5
  Blynk.begin(auth, ssid, pass); // set conection with Blynk App

  dht.begin(); // dht library set begin

  // Setup a function to be called every second (time interval)
  timer.setInterval(1000L, sendSensor);
  timer.setInterval(500L, sendRainSensor);
  timer.setInterval(1000L, sendLDR);
}

void loop()
{
  
  Blynk.run(); // Initiates Blynk
  timer.run(); // Initiates SimpleTimer
  
delay(1000);

if(b==1) // Mode Auto
{
 if(V_LDR <500 && Rsensor>=900) // parameter for Sunny day
 {
    Blynk.virtualWrite(V4,1); // Update current position motor at App
    Motor(0);                 // call motor and pass argument 0 -> Open
 }    
 else if(Rsensor <800)      // parameter for Rain day
 {
    Blynk.virtualWrite(V4,2); // Update current position motor at App
    Motor(1);                  // call motor and pass argument 1 -> Close
 }   
 else if(V_LDR>600)
 {
    Blynk.virtualWrite(V4,2);   // Update current position motor at App
    Motor(1);                   // call motor and pass argument 1 -> Close
 }
 else
 Serial.println("nothig change"); // Nothing change for debug purpose
}

}
