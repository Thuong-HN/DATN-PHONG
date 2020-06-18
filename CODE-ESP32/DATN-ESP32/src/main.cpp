/*
  DEV by ThuongHuynh
*/

#include <Arduino.h>
#include <WiFi.h>
#include <SPIFFS.h>
#include <DHT.h>

TaskHandle_t Task1;
#define DHTPIN 13
#define DHTTYPE DHT22 
DHT dht(DHTPIN, DHTTYPE);
#define DAC1 25 
#define Button 14
#define buzzer 5
#define duration 5000
#define A 15
#define B 16
#define C 17
#define D 18
#define E 19
#define Fs 21
#define G 22
#define DP 27

#define disp1 23
#define disp2 4
#define disp3 12

#define numbersegments { \
{0,0,0,0,0,0,1,1},\
{1,0,0,1,1,1,1,1},\
{0,0,1,0,0,1,0,1},\
{0,0,0,0,1,1,0,1},\
{1,0,0,1,1,0,0,1},\
{0,1,0,0,1,0,0,1},\
{0,1,0,0,0,0,0,1},\
{0,0,0,1,1,1,1,1},\
{0,0,0,0,0,0,0,1},\
{0,0,0,0,1,0,0,1},\
}
byte numbers[10][8] = numbersegments; 
const int segments[8] = {A, B, C, D, E, Fs, G, DP};
int dem_button, stt_buzzer,stt_button;
unsigned long t1;
int tmax_tempe = 6000, tmax_buzzer = 500;
String tempFlash;
String temp_dieuchinh;
int dac = 0;
const char* ssid = "NoPass";
const char* password =  "calihotel12345679";

WiFiServer wifiServer(80);
WiFiClient client;
int domovan = 0, stt_auto;

#define dac0    40    //  0%
#define dac25   80    //  25%
#define dac50   115   //  50%
#define dac75   135   //  75%
#define dac100  180   //  100%

unsigned long t2;
String str_domovan = "0", str_chedo = "auto";
String control = "0", mode = "auto";
float old_nhietdo,doam,t,nhietdo;
String temp_gialap ;
//********* VI *************
IPAddress local_IP(192, 168, 1, 184);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 0, 0);
IPAddress primaryDNS(8, 8, 8, 8);   
IPAddress secondaryDNS(8, 8, 4, 4); 



void read_Flash_Temp() 
{
  File file_temp = SPIFFS.open("/ReadTemp.txt");
  if (!file_temp) 
  {
    Serial.println("Failed to open file for reading");
    return;
  }
 
  if(file_temp.available()) 
  {
    tempFlash = file_temp.readStringUntil('\n');
    
  } 
  if(tempFlash == ""){
    tempFlash = "25";
  }
  file_temp.close();
  
}
void write_Flash_Temp(String &tempx) 
{
  File file_temp = SPIFFS.open("/ReadTemp.txt", FILE_WRITE);
  if(!file_temp){
     Serial.println("There was an error opening the file for writing");
     return;
  }
  file_temp.print(tempx);
  file_temp.print("\n"); // xuong dong
  file_temp.close(); 
}
void read_Flash_Mode() 
{
  File file = SPIFFS.open("/ReadFlash.txt");
  if (!file) 
  {
    Serial.println("Failed to open file for reading");
    return;
  }
  String content[2];
  int row = 0;
  while(file.available()) 
  {
    content[row++] = file.readStringUntil('\n');
    if (row == 2) break;
  } 
  file.close();
  if(content[0] != ""){          
    str_domovan = content[0];
  }
  if(content[1] != ""){
    str_chedo = content[1];
 
  }
  
}
void write_Flash_Mode(String &movan, String &chedo) 
{
  File file = SPIFFS.open("/ReadFlash.txt", FILE_WRITE);
  if(!file){
     Serial.println("There was an error opening the file for writing");
     return;
  }
  file.print(movan);
  file.print("\n"); // xuong dong
  
  file.print(chedo);  
  file.print("\n"); // xuong dong
  
  file.close(); 
}
void Temp_auto(String &gettemp)
{
  
  
  
    if(stt_auto == 0)
    {
      int nhietdo_sosanh = gettemp.toInt() - temp_dieuchinh.toInt();
      Serial.print("nhiet do so sanh: ");Serial.println(nhietdo_sosanh);
      stt_auto = 1;
      if( nhietdo_sosanh > 3)
      {
        dac = 180;control = "100";dem_button = 0;
      }
      if(nhietdo_sosanh == 3){
        dac = 135;control = "75";dem_button = 4;
      }
      if(nhietdo_sosanh == 2){
        dac = 115;control = "50";dem_button = 3;
      }
      if(nhietdo_sosanh == 1){
        dac = 80;control = "25";dem_button = 2;
      }
      if(nhietdo_sosanh <= 0){
        dac = 40;control = "0";dem_button = 1;
      }
      dacWrite(DAC1, dac);Serial.println(control);
      t2 = millis();stt_buzzer = 0;
      digitalWrite(buzzer, HIGH);
      String send = "sendRealtime,"+String(nhietdo) + "," + String(doam)+ "%" + "," + String(control)+ "%" + "," + String(mode);
      client.print(send);
      client.flush();
      write_Flash_Mode(control,mode);
    } 
    
}

void setsegments(int number, int digit, int ontime)
  { 
    for (int seg=0; seg<8; seg++)
    { 
      if(numbers[number][seg]==0)
      { 
        digitalWrite(segments[seg], LOW);
      }
      else 
      {
        digitalWrite(segments[seg], HIGH);
      }
    }
    digitalWrite(digit, HIGH);
    delayMicroseconds(ontime);
    digitalWrite(digit, LOW);
  }
void Task1code( void * pvParameters ){
  
  for(;;)
  {
    if(control == "0"){
        
        setsegments(0, disp1, duration);
        setsegments(0, disp2, duration);
      }
      if(control == "25"){
        setsegments(2, disp1, duration);
        setsegments(5, disp2, duration);
      }
      if(control == "50"){
        setsegments(5, disp1, duration);
        setsegments(0, disp2, duration);                  // **** HIỂN THỊ TRẠNG THÁI ĐIỀU KHIỂN LÊN LED 7 ĐOẠN ***
      }
      if(control == "75"){
        setsegments(7, disp1, duration);
        setsegments(5, disp2, duration);
      }
      if(control == "100"){
        setsegments(1, disp1, duration);
        setsegments(0, disp2, duration);
        setsegments(0, disp3, duration);
      }
      //Serial.println(digitalRead(Button));
      if(digitalRead(Button) == 0 )                       // **** CHỜ NÚT NHẤN MODE ***
      {
        delay(100);
        if(digitalRead(Button) == 0)
        {
          t2 = millis();stt_buzzer = 0;stt_button=1;
          digitalWrite(buzzer, HIGH);
          dem_button++;
          switch (dem_button)
          {
          case 1:
            control = "0";
            dacWrite(DAC1, dac0);
            
            break;
          case 2:
            control = "25";
            dacWrite(DAC1, dac25);
            
            break;
          case 3:
            control = "50";
            dacWrite(DAC1, dac50);
            
            break;
          case 4:
            control = "75";
            dacWrite(DAC1, dac75);
            
            break;
          case 5:
            control = "100";
            dacWrite(DAC1, dac100);
            
            dem_button=0;
            break;
          default:
            break;
          }
          // String send = "sendRealtime,"+String(nhietdo) + "," + String(doam)+ "%" + "," + String(control)+ "%";
          // client.print(send);
          // client.flush();
          mode = "manual";
          write_Flash_Mode(control,mode);
        }
      }
      
    
      vTaskDelay(2);
  } 
}
void setup() {
  //Serial.begin(9600);
  if(!SPIFFS.begin(true)){
      Serial.println("An Error has occurred while mounting SPIFFS"); 
    }


  if (!WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS)) 
  {
      Serial.println("STA Failed to configure wifi 1");
  }
  
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println(".");
  }
  
  Serial.print("Connected to IP: ");
  Serial.println(WiFi.localIP());
  wifiServer.begin();
  delay(100);

  dht.begin();

  for(int t = 0; t <8;t++){
    pinMode(segments[t], OUTPUT);
    digitalWrite(segments[t], LOW);
  }
  pinMode(Button, INPUT_PULLUP);
  pinMode(disp1, OUTPUT);
  pinMode(disp2, OUTPUT);  
  pinMode(disp3, OUTPUT);
  pinMode(buzzer, OUTPUT);

  digitalWrite(buzzer, LOW);
  digitalWrite(disp1, LOW);
  digitalWrite(disp2, LOW);
  digitalWrite(disp3, LOW);
  read_Flash_Mode();
  control = str_domovan;
  if(control == "0")
  {
    
    dacWrite(DAC1, dac0);
    dem_button = 1;         // 1
  }
  if(control == "25"){
    dacWrite(DAC1, dac25);
    dem_button = 2;         // 2
              
  }
  if(control == "50"){
    dacWrite(DAC1, dac50);
    dem_button = 3;         // 3
  }
  if(control == "75"){
    dacWrite(DAC1, dac75);
    dem_button = 4;         // 4
  }
  if(control == "100"){
    dacWrite(DAC1, dac100);
    dem_button = 0;         // 5
  }
  xTaskCreatePinnedToCore(                                    // **** CHO PHÉP XỬ LÝ TRÊN CORE 0 ESP32 ***
      Task1code, /* Tên hàm sẽ xử lý trên core được chọn */
      "Task1", /* Tên của tác vụ */
      10000,  /* Kích thướt của tác vụ */
      NULL,  /* Đối số đầu vào bạn muốn đưa vào bên trong hàm xử lý ở trên */
      3,  /* Mức độ ưu tiên xử lý (0 là thấp nhất) */
      &Task1,  /* Tác vụ bạn khai báo ở phía trên */
      0); /* Chọn core bạn muốn chạy tác vụ (0 hoặc 1) */

  t1 = millis();
}
 
//=======================================================================
//                    Main Program Loop
//=======================================================================
void loop() 
{
  
  //********* TCP/IP ***********
  client = wifiServer.available();
  if (client )                                            
  { 
    Serial.println("Client Connected ...");
    read_Flash_Mode();
    read_Flash_Temp();
    
    nhietdo = dht.readTemperature();
    temp_gialap = String(nhietdo);
    temp_dieuchinh = tempFlash;stt_auto=0;
    client.print("sendFlash,"+str_domovan+"%,"+str_chedo+","+tempFlash+","+nhietdo);
    control = str_domovan, mode = str_chedo;
    while (client.connected())                                // **** CONNECTED ***
    {
        //******** ĐỌC NHỆT ĐỘ PHÒNG ***********
        if ( (unsigned long)(millis()-t1) >= tmax_tempe)
        {
            doam = dht.readHumidity();
            nhietdo = dht.readTemperature();
            //t = dht.computeHeatIndex(t, doam , false);
            if (isnan(doam) || isnan(nhietdo)) {
              Serial.println(F("Failed to read from DHT sensor!"));
              nhietdo = 0.0; doam = 0.0;
                      
            }
            Serial.print(F("Humidity: "));
            Serial.print(doam);
            Serial.print(F("  Temperature: "));
            Serial.print(nhietdo);
            Serial.println(F("°C "));

            String send = "sendRealtime,"+String(nhietdo) + "," + String(doam)+ "%" + "," + String(control)+ "%" + "," + String(mode);
            client.print(send);
            client.flush();
            t1 = millis();
        }
        
        //********* READ DATA ***************
        String getData = client.readString();
        
        if(getData != "")                                     // **** NHẬN DỮ LIỆU ĐIỀU KHIỂN TỪ APP ***
        {
          Serial.println(getData);
          
          if(getData.indexOf("autoMode") != -1){
            mode = "auto";
            Serial.println("AUTO MODE --------------");
          }
          if(getData.indexOf("manualMode") != -1){
            mode = "manual";
            Serial.println("MANUAL MODE --------------");
          }

          if(getData.indexOf("Control") != -1)                // **** CONTROL- LEVEL ***
          {
              t2 = millis();stt_buzzer = 0;
              digitalWrite(buzzer, HIGH);
              if(getData.indexOf("0.0%") != -1){
                dacWrite(DAC1, dac0);
                control = "0";dem_button = 1;
                Serial.println("CONTROL MODE: 0% --------------");
              }
              if(getData.indexOf("25%") != -1){
                dacWrite(DAC1, dac25);
                control = "25";dem_button = 2;
                Serial.println("CONTROL MODE: 25% --------------");
              }
              if(getData.indexOf("50%") != -1){
                dacWrite(DAC1, dac50);
                control = "50";dem_button = 3;
                Serial.println("CONTROL MODE: 50% --------------");
              }
              if(getData.indexOf("75%") != -1){
                dacWrite(DAC1, dac75);
                control = "75";dem_button = 4;
                Serial.println("CONTROL MODE: 75% --------------");
              }
              if(getData.indexOf("100%") != -1){
                dacWrite(DAC1, dac100);
                control = "100";dem_button = 0;
                Serial.println("CONTROL MODE: 100% --------------");
              }
          }
          
          write_Flash_Mode(control,mode);
          
        }
        // **********************************

        // ****** NẾU LÀ CHẾ ĐỘ AUTO THÌ ĐK VAN THEO NHIỆT ĐỘ PHÒNG *********
        if(mode == "auto")
        {   
                                                           // **** CONTROL - AUTO ***
          if(getData.indexOf("temp") != -1){
              temp_dieuchinh = getData.substring(4);
              Serial.print("NHIET DO DIEU CHINH: ");Serial.println(temp_dieuchinh); 
              write_Flash_Temp(temp_dieuchinh);
              getData = "";stt_auto=0;
          }
          
          if(getData.indexOf("set") != -1){
              temp_gialap = getData.substring(3);
              Serial.print("NHIET DO GIA LAP: ");Serial.println(temp_gialap); 
              stt_auto = 0;
              // if(temp_gialap == temp){
              //   control = "0"; dacWrite(DAC1, dac0);getData = "";stt_auto=1;dem_button = 1;
              //   t2 = millis();stt_buzzer = 0;
              //   digitalWrite(buzzer, HIGH);
              //   String send = "sendRealtime,"+String(nhietdo) + "," + String(doam)+ "%" + "," + String(control)+ "%" + "," + String(mode);
              //   client.print(send);
              //   client.flush();
              // }
          }
          Temp_auto(temp_gialap);      
          
        }
        if(stt_button == 1)                                                           // **** TRẠNG THÁI NÚT NHẤN MODE ***
        {
          String send = "sendRealtime,"+String(nhietdo) + "," + String(doam)+ "%" + "," + String(control)+ "%" + "," + String(mode);
            client.print(send);
            client.flush();
            mode = "manual";
            write_Flash_Mode(control,mode);
            stt_button = 0; stt_auto = 0;
        }
        if ( ( (unsigned long)(millis()-t2) >= tmax_buzzer) && stt_buzzer == 0)              // **** BẬT BUZZER TRONG 5S ***
        {
            digitalWrite(buzzer, LOW);
            stt_buzzer = 1;
        }

        delay(5);
    }
    client.stop();
    Serial.println("Client disconnected");
  }
   
  if ( ( (unsigned long)(millis()-t2) >= tmax_buzzer) && stt_buzzer == 0)                    // **** BẬT BUZZER TRONG 5S ***
  {
      digitalWrite(buzzer, LOW);
      stt_buzzer = 1;
  }


  delay(10);
}

