#include "FirebaseESP8266.h"  // Install Firebase ESP8266 library
#include <ESP8266WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

#define FIREBASE_HOST "chloeiot-default-rtdb.firebaseio.com" //Without http:// or https:// schemes
#define FIREBASE_AUTH "X4wkoaKbSrITcHQ075Tkb4VutJsROgI4iz3IkvG4"
#define WIFI_SSID "ANU"                           //NAMA WIFI
#define WIFI_PASSWORD "ifafani157"                //PASS WIFI

#define SensorPin A0    //PIN analog untuk sensor soil moisture

// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");

int sensorValue = 0; 
int kelembapan=0;
String StrKelembapan;
const int relay1 = D7;
int x = 0;


//Define FirebaseESP8266 data object
FirebaseData firebaseData,indikator,historyx;
FirebaseJson json;

void setup()
{  
  Serial.begin(9600);
  randomSeed(analogRead(0));
  
   pinMode(relay1, OUTPUT);
     
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Firebase.reconnectWiFi(true);

  // Initialize a NTPClient to get time
  timeClient.begin();
  // Set offset time in seconds to adjust for your timezone, for example:
  // GMT +1 = 3600
  // GMT +8 = 28800
  // GMT -1 = -3600
  // GMT 0 = 0
  timeClient.setTimeOffset(25200);

}

        void sensorUpdate()                                                                           //untuk kirim data kelembapan realtime
        {
          sensorValue = analogRead(SensorPin); 
          kelembapan = (sensorValue*100)/1023;
                            
          if (Firebase.setInt(firebaseData, "/DRealtime/humidity", kelembapan))
          {
            Serial.println("PASSED");
            Serial.println("PATH: " + firebaseData.dataPath());
            Serial.println("TYPE: " + firebaseData.dataType());
            Serial.println("ETag: " + firebaseData.ETag());
            Serial.println("Suhunya" +sensorValue);
            Serial.println("------------------------------------");
            Serial.println();
          }
          else
          {
            Serial.println("FAILED");
            Serial.println("REASON: " + firebaseData.errorReason());
            Serial.println("------------------------------------");
            Serial.println();
          }
          
        }
           //SIRAM
            int f = 1;
            void siram()                                                            //Untuk siram manual lewat tombol--------------------------------------
            {
              
                Firebase.getInt(indikator,"/InWatering/Value");
                int x3=indikator.intData();
              
                if(x3==2){
                  digitalWrite(relay1,LOW); //nyala
                  delay(3000);
                  Firebase.setInt(firebaseData, "/InWatering/Value", f); 
                }
                if(x3==1){
                  digitalWrite(relay1,HIGH); //mati
                }
            }

            void history()                                                                    //History-------------------------------------------------
            {              
              delay(5000);
               sensorValue = analogRead(SensorPin); 
               kelembapan = (sensorValue*100)/1023;
               timeClient.update();
                                    
                  
                  unsigned long epochTime = timeClient.getEpochTime();                                 
                  String formattedTime = timeClient.getFormattedTime();
                                         
                  int currentHour = timeClient.getHours();
                     String jam ="";
                    
                    if(currentHour<10){
                
                      jam="0"+String(currentHour);
                      
                    }else{
                      jam =String(currentHour);
                      };
                      
//                    String jam = (String)currentHour;
                                  
                  int currentMinute = timeClient.getMinutes();  
                     String menit ="";              
                    if(currentMinute<10){
                      
                      menit="0"+String(currentMinute);
                      
                    }else{
                      menit =String(currentMinute);
                      };

//                    String menit = (String)currentMinute;
                    
                  String jamfix= jam+ ":" + menit;
                    
                  Serial.println(jamfix); 
                                                      
                  //Get a time structure
                  struct tm *ptm = gmtime ((time_t *)&epochTime); 
                
                  int monthDay = ptm->tm_mday;
                                  
                    String tanggal ="";
                    
                    if(monthDay<10){
                
                      tanggal="0"+String(monthDay);
                      
                    }else{
                      tanggal =String(monthDay);
                      };
                
                  int currentMonth = ptm->tm_mon+1;
                //  Serial.print("Month: ");
                //  Serial.println(currentMonth);
                    String bulan ="";
                
                     if(currentMonth<10){
                
                      bulan="0"+String(currentMonth);
                      
                    }else{
                       bulan =String(currentMonth);
                    };                             
                
                  int currentYear = ptm->tm_year+1900;             
                
                  //Print complete date:
                  String currentDate = tanggal + "-" + bulan + "-" + String(currentYear);
                  String currentDate1= String(currentYear)+ "-" + bulan + "-" + tanggal;
                  Serial.print("Current date: "); 
                  Serial.println(currentDate);                
                  Serial.println("");

                  String Pkey=currentDate1+"_"+jamfix;
                  
                  StrKelembapan = String(kelembapan)+"%";
                  
                if(kelembapan>60){
                  digitalWrite(relay1,LOW); //nyala
                  delay(3000);
                  digitalWrite(relay1,HIGH); //mati
                                  
                  Firebase.setString(historyx, "/History/"+Pkey+"/date", currentDate);                                               
                  Firebase.setString(historyx, "/History/"+Pkey+"/soilMoisture", StrKelembapan);                                               
                  Firebase.setString(historyx, "/History/"+Pkey+"/time", jamfix);                                               
                  }
            }
            
void loop() {
  sensorUpdate();
  siram();
  history();
  delay(500);
}
