#include <Arduino.h>

//#define WIFICODE 0

#ifdef WIFICODE
//OTA
  #include <WiFi.h>
  #include <WiFiClient.h>
  #include <WebServer.h>
  #include <ESPmDNS.h>
  #include <Update.h>
  #include <web.h>
#endif
//Board
#include "board_def.h"

//GPS
#include <TinyGPS++.h>
//Memoria EEPROM
#include <EEPROM.h>
#define EEPROM_SIZE 1

#define TARGET_LAT 40.783902
#define TARGET_LON 32.306219
//Variable Estado
int Estado_Juego;
String command;
//Includes Epaper
#include <GxEPD.h>
#include <GxIO/GxIO_SPI/GxIO_SPI.h>
#include <GxIO/GxIO.h>

#include <Fonts/FreeMonoBold12pt7b.h>
#include <Fonts/FreeSerif12pt7b.h>
#define DEFALUT_FONT FreeMonoBold12pt7b

//graficos
#include "BitmapGraphics.h"
#include <output2\Iris_1_0.h>
#include <output2\Iris_1_1.h>
#include <output2\Iris_1_2.h>
#include <output2\Iris_1_3.h>
#include <output2\Iris_1_3_1_3.h>
#include <output2\Iris_1_3_1_5.h>


#include <Fonts/FreeSans9pt7b.h>

String  Image0 = "gImage_Iris_1_0";
String  Image1 = "gImage_Iris_1_0";
const unsigned char * Images[] = 
{
	gImage_Iris_1_0,
	gImage_Iris_1_1,
	gImage_Iris_1_2,
  gImage_Iris_1_3
};
GxIO_Class io(SPI, ELINK_SS, ELINK_DC, ELINK_RESET);
GxEPD_Class display(io, ELINK_RESET, ELINK_BUSY);

#ifdef WIFICODE
// Variables OTA
const char* host = "Cocoloco";
const char* ssid = "globalme";
const char* password = "perico124";


WebServer server(80);
#endif
//Variables GPS
static const int RXPin = 32, TXPin = 25;
static const uint32_t GPSBaud = 9600;

//variables millis
unsigned long startMillis;  //some global variables available anywhere in the program
unsigned long currentMillis = 0;
unsigned long GPSMillis;

void displayInfo();
// HHardwareSerial Serial2(2);

// The TinyGPS++ object
TinyGPSPlus gps;
void showPartialUpdate(String Text);

long previousMillis = 0; 

int Current_Status=000; //Iinicio Estado
int Last_Status=000;
int Next_Status = 000;

int Cuenta_Imagen=0;
int Entrada=0;
unsigned long distanceKm; 

int MedidaHall=0;
unsigned long Touch =100;


void displayInit(void){
    static bool isInit = false;
    if (isInit) {
        return;
    }
    isInit = true;
    display.init(115200);
    
    display.eraseDisplay();
    display.setTextColor(GxEPD_BLACK);
    display.setFont(&DEFALUT_FONT);
    //display.setTextSize(0);
    //display.fillScreen(GxEPD_WHITE);
    display.setRotation(0);
    display.update();
    display.drawExampleBitmap(gImage_Iris_1_0, 0, 0, 200, 200, GxEPD_BLACK);
   // display.drawExampleBitmap(gImage_Iris_1_3_1_3,0, 0, 200, 200, GxEPD_BLACK);
    display.update();
}

void setup(void) {
  Serial.begin(115200);
  EEPROM.begin(EEPROM_SIZE);
  Estado_Juego=EEPROM.read(0);

  //Init Epaper
  displayInit();
  //display.drawExampleBitmap(gImage_Iris_1_0, 0, 0, 200, 200, GxEPD_BLACK);
  //display.update();

#ifdef WIFICODE
  // Connect to WiFi network
  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Conectado a ");
  Serial.println(ssid);
  Serial.print("Direccion IP: ");
  Serial.println(WiFi.localIP());

  /*use mdns for host name resolution*/
  if (!MDNS.begin(host)) { //http://Cocoloco.local
    Serial.println("Error setting up MDNS responder!");
    while (1) {
      delay(1000);
    }
  }
  Serial.println("mDNS responder started");
  /*return index page which is stored in serverIndex */
  server.on("/", HTTP_GET, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/html", loginIndex);
  });
  server.on("/serverIndex", HTTP_GET, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/html", serverIndex);
  });
  /*handling uploading firmware file */
  server.on("/update", HTTP_POST, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
    ESP.restart();
  }, []() {
    HTTPUpload& upload = server.upload();
    if (upload.status == UPLOAD_FILE_START) {
      Serial.printf("Update: %s\n", upload.filename.c_str());
      if (!Update.begin(UPDATE_SIZE_UNKNOWN)) { //start with max available size
        Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_WRITE) {
      /* flashing firmware to ESP*/
      if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
        Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_END) {
      if (Update.end(true)) { //true to set the size to the current progress
        Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
      } else {
        Update.printError(Serial);
      }
    }
  });
  server.begin();

#endif




// Inicio GPS
 //Serial.begin(115200, SERIAL_8N1);
  Serial2.begin(GPSBaud, SERIAL_8N1, RXPin , TXPin);
  Serial.println("Serial Txd is on pin: "+String(TX));
  Serial.println("Serial Rxd is on pin: "+String(RX));


}
//Variables de bucle

void loop() {

  unsigned long currentMillis = millis();
  
#ifdef WIFICODE
  server.handleClient();
#endif
 // gestion del tiempo
  currentMillis = millis();

  if(Serial.available()){
        command = Serial.readString();
       // Estado_Juego= command.toInt();
       // EEPROM.write(0, Estado_Juego);
       // EEPROM.commit();

    }
  switch (Estado_Juego){
    case 0:
   // Serial.println("Estado 0");
    if ((currentMillis - previousMillis) >=2000)
    {
      Serial.println("Estado 0 ");
      Next_Status=1;
      previousMillis = currentMillis;
      
    }
    break;
    case 1: //Nivel 1 del juego
      switch (Current_Status){
        case 0:
          Next_Status=1;
        
        break;
        case 1: //0.1 Espera pulso
        //if (command.equals("a") or MedidaHall >=13  or (Touch <=70 and Touch>=10)) {
        if (command.equals("a") || Touch<20 ) {
            Next_Status=2;
            command = "NA";
            MedidaHall=0;
           Touch=100;
          }
        else if ((currentMillis - previousMillis) >=1000)
        {
            MedidaHall = hallRead();
            Serial.println("Hall value: " + String(hallRead()));
            Touch = touchRead(T4);
            Serial.println("Touch  value: " + String(Touch));
            Serial.println("Estado 1");
            previousMillis = currentMillis;
        }
        break;
        case 2: //0.2 Abrir Ojo
        if (Cuenta_Imagen <4) {
          if ((currentMillis - previousMillis) >=1000)
          {
            Serial.println("Abriendo ojo. Cuenta: " + String(Cuenta_Imagen));
            display.drawExampleBitmap(Images[Cuenta_Imagen], sizeof(gImage_gui), GxEPD::bm_default | GxEPD::bm_partial_update);
            Cuenta_Imagen=Cuenta_Imagen+1;
            previousMillis = currentMillis;

          }
        }
        else {
          Next_Status=3;
          Entrada=1; // Condición de primera entrada para solo refrescar el display una vez en siguiende paso
        }
        break;
        case 3: //0.3 Leer GPS y Display Loading
          if (Entrada>0){
            display.drawExampleBitmap(gImage_Iris_1_3_1_3, sizeof(gImage_Iris_1_3_1_3),GxEPD::bm_default |GxEPD::bm_partial_update); //Muestra Localizando en display
            Entrada=0;
          }
          if (gps.encode(Serial2.read())) {
          if (gps.location.isValid())
          {
            Next_Status=4;
            Entrada=1;
            previousMillis = currentMillis;
          }
          }
          else if ((currentMillis - previousMillis) >=45000) 
          { Serial.print(F("Location: ")); 
            Serial.print(F("INVALID"));
            Next_Status=5;
            Entrada=1;
            previousMillis = currentMillis;
            
          }
          // if ((currentMillis - previousMillis) >=5000) //Temporizador para pasar a siguente stage
          //   {
          //     Next_Status=5;
          //     previousMillis = currentMillis;
          //   }
        break;
        case 4: //0.4 Mostrar distancia y espera 
        if (Entrada>0){
            display.drawExampleBitmap(gImage_Iris_1_3, sizeof(gImage_gui),  GxEPD::bm_default | GxEPD::bm_partial_update); 
            distanceKm = ((unsigned long)TinyGPSPlus::distanceBetween(gps.location.lat(),gps.location.lng(),TARGET_LAT,TARGET_LON)/1000);
            Serial.print(F("Distancia: ")); 
            Serial.print(String(distanceKm));
            Serial.print(F("Location: ")); 
            Serial.print(gps.location.lat(), 6);
            Serial.print(F(","));
            Serial.print(gps.location.lng(), 6);
            showPartialUpdate(String(distanceKm) + " Km");
            Entrada=0;
          }
            
          if ((currentMillis - previousMillis) >=5000)
            {
              Next_Status=6;
              previousMillis = currentMillis;
            }
        break;
        case 5: // 0.5 Display "GpS no valido sal fuera"
          //showPartialUpdate("GpS no \r valido sal\r fuera");
           if (Entrada>0){
            display.drawExampleBitmap(gImage_Iris_1_3_1_5, sizeof(gImage_gui),  GxEPD::bm_default | GxEPD::bm_partial_update); 
            Entrada=0;
          }
          if ((currentMillis - previousMillis) >=5000)
            {
              Next_Status=6;
              previousMillis = currentMillis;
            }
        break;
        case 6: //0.6 Cerrar ojo
          if (Cuenta_Imagen >0) {
            if ((currentMillis - previousMillis) >=2000)
            {
              Cuenta_Imagen=Cuenta_Imagen-1;
              Serial.println("Abriendo ojo. Cuenta: " + String(Cuenta_Imagen));
              display.drawExampleBitmap(Images[Cuenta_Imagen], sizeof(gImage_gui), GxEPD::bm_default | GxEPD::bm_partial_update);
              previousMillis = currentMillis;

            }
          }
          else {
          Next_Status=0;
        }
        break;
      }
    // if ((currentMillis - previousMillis) >=2000)
    //     {
    //       Serial.println("Estado 1");
    //       Estado_Juego=EEPROM.read(0);
    //       Serial.println("Estado juego: "+ String(Estado_Juego));
    //       previousMillis = currentMillis;
    //       Serial.println("Current Status: " + String(Current_Status));
    //       Serial.println("Next Status: " + String(Next_Status));

    //     }
    Current_Status=Next_Status;  
    Last_Status=Current_Status;  
    break; //Estado juego 1
    default:
    if ((currentMillis - previousMillis) >=2000)
    {
      Serial.println("Estado Default");
      Estado_Juego=EEPROM.read(0);
      Serial.println("Estado: "+ String(Estado_Juego));
      previousMillis = currentMillis;
    }
      break;
  }

 // delay(1);
}


void showPartialUpdate(String Text)
{
  String TextString = String(Text);
  const char* name = "FreeSans9pt7b";
  const GFXfont* f = &FreeSans9pt7b;
 
  uint16_t box_x = 60;
  uint16_t box_y = 60;
  uint16_t box_w = 100;
  uint16_t box_h = 50;
  uint16_t cursor_y = box_y + 9;


  display.setFont(f);
  display.setTextColor(GxEPD_BLACK);
  display.setRotation(3);
  display.fillRect(box_x, box_y, box_w, box_h, GxEPD_WHITE);
  display.setCursor(box_x, cursor_y+35);
  display.print(TextString); 
  display.updateWindow(box_x, box_y, box_w, box_h, true);
}
