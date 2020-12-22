/*
  DEV by ThuongHuynh
  ESP32 connect to Winform APP use TCP/IP
*/

#include <Arduino.h>
#include <SPIFFS.h>

/////////////// BLE ///////////////
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

#define SERVICE_UUID        "00c01523-6e7f-48a7-924b-95d3cc8c30f4"
#define CHARACTERISTIC_UUID "00c01525-6e7f-48a7-924b-95d3cc8c30f4"


TaskHandle_t Task1;

#define DAC1 25
#define btn_UP 14
#define btn_DOWN 5
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

#define numbersegments            \
  {                               \
    {0, 0, 0, 0, 0, 0, 1, 1},     \
        {1, 0, 0, 1, 1, 1, 1, 1}, \
        {0, 0, 1, 0, 0, 1, 0, 1}, \
        {0, 0, 0, 0, 1, 1, 0, 1}, \
        {1, 0, 0, 1, 1, 0, 0, 1}, \
        {0, 1, 0, 0, 1, 0, 0, 1}, \
        {0, 1, 0, 0, 0, 0, 0, 1}, \
        {0, 0, 0, 1, 1, 1, 1, 1}, \
        {0, 0, 0, 0, 0, 0, 0, 1}, \
        {0, 0, 0, 0, 1, 0, 0, 1}, \
  }
byte numbers[10][8] = numbersegments;
const int segments[8] = {A, B, C, D, E, Fs, G, DP};
int dem_button_UP = 5, stt_buzzer, stt_button,dem_button_DOWN = 5;
unsigned long t1;
int tmax_tempe = 3000, tmax_buzzer = 500;
String tempFlash;
String temp_dieuchinh;
int dac = 0;


// const char *ssid = "RD_DVES";
// const char *password = "dongvietdves";


#define dac0    40    //  0%
#define dac25   80   //  25%
#define dac50   115  //  50%
#define dac75   135  //  75%
#define dac100  180 //  100%


                              

String str_domovan = "0", str_chedo = "auto";
String control = "0", mode_control = "auto";



/////////// Function BLE //////

static BLEServer* pServer = NULL;
static BLECharacteristic* pCharacteristic = NULL;
static BLEAdvertising *pAdvertising = NULL;
static bool deviceConnected = false, flag_sleep = false, flag_scroll_off = false;

class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer,esp_ble_gatts_cb_param_t *param) {
      
      deviceConnected = true;
      
      BLEDevice::startAdvertising();  // Cho phép quảng bá BLE cho các client khác 
      Serial.print("Connected Client...");Serial.println( pServer->getConnectedCount());    
      //Serial.print("Connected ID...");Serial.println( pServer->getConnId());   
//      sprintf(
//        remoteAddress,
//        "%.2X:%.2X:%.2X:%.2X:%.2X:%.2X",        //************* ĐỌC ĐỊA CHỈ THIẾT BỊ KẾT NỐI *********
//        param->connect.remote_bda[0],
//        param->connect.remote_bda[1],
//        param->connect.remote_bda[2],
//        param->connect.remote_bda[3],
//        param->connect.remote_bda[4],
//        param->connect.remote_bda[5]
//      );
//      
//      Serial.print("ESP32 CONNECT TO: ");Serial.println(remoteAddress);

    };

    void onDisconnect(BLEServer* pServer) 
    {
      //pServer->disconnectClient();                                                     //*** DISCONNECT client theo ID mà lúc đầu kết nối ***
      //Serial.print("DisConnect...");Serial.println( pServer->getConnectedCount());
      Serial.print("DisConnect ID...");Serial.println( pServer->getConnId());  
      //deviceConnected = false;                                                         //*** Luôn giữ TRUE vì vòng lặp loop phải sử dụng ***

////      digitalWrite(Led_Disconnect, LOW);
//      if(messageTmp.indexOf("@up") != -1 && messageTmp.indexOf("***") == -1) // ******* NẾU ĐANG GỬI HÌNH MÀ APP BỊ ĐÓNG THÌ RESET ESP32 **** TRÁNH BỊ ĐÈ GIÁ TRỊ NHẬN TRƯỚC ĐÓ
//      {
//        ESP.restart();
//      }
      
    }
};

class MyCallbacks: public BLECharacteristicCallbacks 
{
    
    void onWrite(BLECharacteristic* pCharacteristic) 
    {
      std::string rxValue = pCharacteristic->getValue(); 
      
      Serial.print("CHUOI NHAN: ");Serial.println(rxValue.c_str());
      
                                                                // ************ SAU 1 GIỜ TỪ LÚC ĐIỀU KHIỂN LED SẼ TỰ TẮT ***********
      if (rxValue.length() <= 2) 
      {                                                                       // ************ ĐIỀU KHIỂN TỪ REMOTE HOẶC APP ***********
          
          String getControl = rxValue.c_str();
          if(getControl == "L"){                             // ******* ĐIỀU KHIỂN THANK TỪ REMOTE ********
             dacWrite(DAC1, dac0);
             pCharacteristic->setValue(std::string("TSLW"));
             control = "0";
             dem_button_UP = 1;
             
          }
          if(getControl == "T"){                             // ******* ĐIỀU KHIỂN THANK TỪ REMOTE ********
             dacWrite(DAC1, dac50);
             pCharacteristic->setValue(std::string("TSLW"));
             control = "50";
             dem_button_UP = 3;
             
          }
          if(getControl == "W"){                             // ******* ĐIỀU KHIỂN THANK TỪ REMOTE ********
             dacWrite(DAC1, dac75);
             pCharacteristic->setValue(std::string("TSLW"));
             control = "75";
             dem_button_UP = 4;
             
          }
          if(getControl == "S"){                             // ******* ĐIỀU KHIỂN THANK TỪ REMOTE ********
             dacWrite(DAC1, dac100);
             pCharacteristic->setValue(std::string("TSLW"));
             control = "100";
             dem_button_UP = 5;
             
          }
          
      }
    }
};


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
  while (file.available())
  {
    content[row++] = file.readStringUntil('\n');
    if (row == 2)
      break;
  }
  file.close();
  if (content[0] != "")
  {
    str_domovan = content[0];
  }
  if (content[1] != "")
  {
    str_chedo = content[1];
  }
}
void write_Flash_Mode(String &movan, String &chedo)
{
  File file = SPIFFS.open("/ReadFlash.txt", FILE_WRITE);
  if (!file)
  {
    Serial.println("There was an error opening the file for writing");
    return;
  }
  file.print(movan);
  file.print("\n"); // xuong dong

  file.print(chedo);
  file.print("\n"); // xuong dong

  file.close();
}


void setsegments(int number, int digit, int ontime)
{
  for (int seg = 0; seg < 8; seg++)
  {
    if (numbers[number][seg] == 0)
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

void Task1code(void *pvParameters)
{
  
  for (;;)
  {
    
    if (control == "0")
    {

      setsegments(0, disp1, duration);
      setsegments(0, disp2, duration);
    }
    if (control == "25")
    {
      setsegments(2, disp1, duration);
      setsegments(5, disp2, duration);
    }
    if (control == "50")
    {
      setsegments(5, disp1, duration);
      setsegments(0, disp2, duration); // **** HIỂN THỊ TRẠNG THÁI ĐIỀU KHIỂN LÊN LED 7 ĐOẠN ***
    }
    if (control == "75")
    {
      setsegments(7, disp1, duration);
      setsegments(5, disp2, duration);
    }
    if (control == "100")
    {
      setsegments(1, disp1, duration);
      setsegments(0, disp2, duration);
      setsegments(0, disp3, duration);
    }
    delay(10);
  }
}
void setup()
{
  Serial.begin(115200);
  if (!SPIFFS.begin(true))
  {
    Serial.println("An Error has occurred while mounting SPIFFS");
  }
  initBLE();

  for (int t = 0; t < 8; t++)
  {
    pinMode(segments[t], OUTPUT);
    digitalWrite(segments[t], LOW);
  }
  pinMode(btn_UP, INPUT_PULLUP);
  pinMode(btn_DOWN, INPUT_PULLUP);
  pinMode(disp1, OUTPUT);
  pinMode(disp2, OUTPUT);
  pinMode(disp3, OUTPUT);

  digitalWrite(disp1, LOW);
  digitalWrite(disp2, LOW);
  digitalWrite(disp3, LOW);
  read_Flash_Mode();
  control = str_domovan;
  Serial.println(control);
  if (control == "0")
  {

    dacWrite(DAC1, dac0);
    dem_button_UP = 1; // 1
  }
  if (control == "25")
  {
    dacWrite(DAC1, dac25);
    dem_button_UP = 2; // 2
  }
  if (control == "50")
  {
    dacWrite(DAC1, dac50);
    dem_button_UP = 3; // 3
  }
  if (control == "75")
  {
    dacWrite(DAC1, dac75);
    dem_button_UP = 4; // 4
  }
  if (control == "100")
  {
    dacWrite(DAC1, dac100);
    dem_button_UP = 5; // 5
  }
  
  xTaskCreatePinnedToCore( // **** CHO PHÉP XỬ LÝ TRÊN CORE 0 ESP32 ***
      Task1code,           /* Tên hàm sẽ xử lý trên core được chọn */
      "Task1",             /* Tên của tác vụ */
      10000,               /* Kích thướt của tác vụ */
      NULL,                /* Đối số đầu vào bạn muốn đưa vào bên trong hàm xử lý ở trên */
      3,                   /* Mức độ ưu tiên xử lý (0 là thấp nhất) */
      &Task1,              /* Tác vụ bạn khai báo ở phía trên */
      0);                  /* Chọn core bạn muốn chạy tác vụ (0 hoặc 1) */
 
  delay(100);
  
  t1 = millis();
}

//=======================================================================
//                    Main Program Loop
//=======================================================================
void loop()
{

  
    if (digitalRead(btn_UP) == 0 && dem_button_UP <5) // **** CHỜ NÚT NHẤN MODE ***
    {
      delay(20);
      if (digitalRead(btn_UP) == 1)
      {
        
//        delay(20);
//        if (digitalRead(btn_UP) == 1)
//        {
          dem_button_UP++;
          switch (dem_button_UP)
          {
          case 1:
            control = "0";
            dacWrite(DAC1, dac0);
            Serial.println("UP------------0");
            break;
          case 2:
            control = "25";
            dacWrite(DAC1, dac25);
            Serial.println("UP------------25");
            break;
          case 3:
            control = "50";
            dacWrite(DAC1, dac50);
            Serial.println("UP------------50");
            break;
          case 4:
            control = "75";
            dacWrite(DAC1, dac75);
            Serial.println("UP------------75");
            break;
          case 5:
            control = "100";
            dacWrite(DAC1, dac100);
            Serial.println("UP------------100");
            dem_button_UP = 5;
            break;
          default:
            break;
        }
        mode_control = "manual";
        write_Flash_Mode(control, mode_control);
//        }
      }
    }

    
     if (digitalRead(btn_DOWN) == 0 && dem_button_UP != 1) // **** CHỜ NÚT NHẤN MODE ***
    {
      delay(20);
      if (digitalRead(btn_DOWN) == 1)
      {
//       delay(20);
//        if (digitalRead(btn_DOWN) == 1)
//        {
          dem_button_UP--;
          switch (dem_button_UP)
          {
          case 1:
            control = "0";
            dacWrite(DAC1, dac0);
  //          Serial.println("DOWN------------0");
            break;
          case 2:
            control = "25";
            dacWrite(DAC1, dac25);
  //          Serial.println("DOWN------------25");
            break;
          case 3:
            control = "50";
            dacWrite(DAC1, dac50);
  //          Serial.println("DOWN------------50");
            break;
          case 4:
            control = "75";
            dacWrite(DAC1, dac75);
  //          Serial.println("DOWN------------75");
            break;
          
          default:
            break;
          }
          mode_control = "manual";
          write_Flash_Mode(control, mode_control);
//      }
      }
    }
    
}

void initBLE()
{
  esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT);           // *** GIẢI PHÓNG BT CLASSIC -> CHỈ DÙNG BLE
  esp_bt_controller_enable(ESP_BT_MODE_BLE);
  
  esp_err_t errRc=esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_DEFAULT,ESP_PWR_LVL_P9);
  esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_ADV, ESP_PWR_LVL_P9);
  esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_SCAN ,ESP_PWR_LVL_P9); 
 
  String devName = "EMOJI";
  String chipId = String((uint32_t)(ESP.getEfuseMac() >> 24), HEX);
  devName += '_';
  devName += chipId;
  BLEDevice::init(devName.c_str());
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());
  BLEService *pService = pServer->createService(SERVICE_UUID);
  pCharacteristic = pService->createCharacteristic(
                      CHARACTERISTIC_UUID,
                      BLECharacteristic::PROPERTY_READ   |
                      BLECharacteristic::PROPERTY_WRITE  
//                      BLECharacteristic::PROPERTY_NOTIFY |
//                      BLECharacteristic::PROPERTY_INDICATE
                    );
 
  pCharacteristic->addDescriptor(new BLE2902());
  pCharacteristic->setCallbacks(new MyCallbacks());
  pService->start();
  
  pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(false); // false
  //pAdvertising->setMinPreferred(0x0);  
  pAdvertising->setMinPreferred(0x06);  // functions that help with iPhone connections issue
  pAdvertising->setMinPreferred(0x12);  // Mức độ ưu tiên
  BLEDevice::startAdvertising();
  //Serial.print("CORE INITBLE");Serial.println(xPortGetCoreID());
}
