/*
  
  RotoCam
  
  Projektdetails unter
  https://github.com/HenrikAalto/RotaCam
  
  Vielen Dank an Rui Santos für die Umsetzung des WebCam-Moduls https://RandomNerdTutorials.com

  Bitte das ESP-Modul ggf. anpassen. 
  Ich habe bisher ausschließlich AI-Thinker Module benutzt.

  Diese Software steht jedem der es möchte zur Veränderung und Weiterentwicklung zur Verfügung.
  Ich gebe keinerlei Garantien für Funktionalität und/oder Zuverlässigkeit des Quellcodes

*/

#include "esp_camera.h"
#include "WiFi.h"
#include "esp_timer.h"
#include "img_converters.h"
#include "Arduino.h"
#include "fb_gfx.h"
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
#include "dl_lib.h"
#include "esp_http_server.h"

const char* Version = "V0.1.1 (04-06-2019)";   

// Das Programm ermöglicht zwei WiFi-Konfigurationen
// Beim Start wird erst versucht SSID1 zu verbinden, gelingt das nicht wird SSDI2 versucht
const char* ssid1 = "MySSID1";
const char* password1 = "PassphraseOfWiFi1";
const char* ssid2 = "MySSID2";
const char* password2 = "PassphraseOfWiFi2";

const int FlashPin =  4;                  // Pin für das Blitzlicht
const int FlashChannel = 5;               // PWM-Kanal des Blitzes

const int myServoPin =  12;               // Pin für das Servo
const long myServoMax =  114;             // Maximalwinkel für das Servo
const long myServoMin =  13;              // Minimalwinkel für das Servo
const int myServoChannel = 6;             // PWM-Kanal des Servos
const int myServoVerstellbereich = 205;   // Gradzahl um die der Servo verstellt werden kann
const int myServoPause = 25;              // Standard Pausenzeit nach Verstellung des Servos

// Variablen

long gegenwaertiger_Zustand = 1;          // Variable für den vollständigen Automaten
long zukuenftigertiger_Zustand = 1;       // Variable für den vollständigen Automaten

int LogLevel = 0;                         // Variable um Log Level einzustellen (0=keine, 1=Zustandsausgabe über Seriell)

int inByte = 0;                           // Variable zu Anzeige über welche Schnittstelle Daten eintreffen
int flash = 0;                            //Blitzlicht 0=aus 1=ein

int MyServoPosition = myServoMin;         //Servoposition

uint8_t TelnetSession;                    // Sitzungsnr. Telnet

int i;
char command;
char inputString [5];

#define MAX_SRV_CLIENTS 1 

#define PART_BOUNDARY "123456789000000000000987654321"

#define CAMERA_MODEL_AI_THINKER
//#define CAMERA_MODEL_M5STACK_PSRAM
//#define CAMERA_MODEL_WROVER_KIT

#if defined(CAMERA_MODEL_WROVER_KIT)
  #define PWDN_GPIO_NUM    -1
  #define RESET_GPIO_NUM   -1
  #define XCLK_GPIO_NUM    21
  #define SIOD_GPIO_NUM    26
  #define SIOC_GPIO_NUM    27  
  #define Y9_GPIO_NUM      35
  #define Y8_GPIO_NUM      34
  #define Y7_GPIO_NUM      39
  #define Y6_GPIO_NUM      36
  #define Y5_GPIO_NUM      19
  #define Y4_GPIO_NUM      18
  #define Y3_GPIO_NUM       5
  #define Y2_GPIO_NUM       4
  #define VSYNC_GPIO_NUM   25
  #define HREF_GPIO_NUM    23
  #define PCLK_GPIO_NUM    22

#elif defined(CAMERA_MODEL_M5STACK_PSRAM)
  #define PWDN_GPIO_NUM     -1
  #define RESET_GPIO_NUM    15
  #define XCLK_GPIO_NUM     27
  #define SIOD_GPIO_NUM     25
  #define SIOC_GPIO_NUM     23 
  #define Y9_GPIO_NUM       19
  #define Y8_GPIO_NUM       36
  #define Y7_GPIO_NUM       18
  #define Y6_GPIO_NUM       39
  #define Y5_GPIO_NUM        5
  #define Y4_GPIO_NUM       34
  #define Y3_GPIO_NUM       35
  #define Y2_GPIO_NUM       32
  #define VSYNC_GPIO_NUM    22
  #define HREF_GPIO_NUM     26
  #define PCLK_GPIO_NUM     21

#elif defined(CAMERA_MODEL_AI_THINKER)
  #define PWDN_GPIO_NUM     32
  #define RESET_GPIO_NUM    -1
  #define XCLK_GPIO_NUM      0
  #define SIOD_GPIO_NUM     26
  #define SIOC_GPIO_NUM     27
  #define Y9_GPIO_NUM       35
  #define Y8_GPIO_NUM       34
  #define Y7_GPIO_NUM       39
  #define Y6_GPIO_NUM       36
  #define Y5_GPIO_NUM       21
  #define Y4_GPIO_NUM       19
  #define Y3_GPIO_NUM       18
  #define Y2_GPIO_NUM        5
  #define VSYNC_GPIO_NUM    25
  #define HREF_GPIO_NUM     23
  #define PCLK_GPIO_NUM     22
#else
  #error "Kein Kameramodul ausgewählt!"
#endif

static const char* _STREAM_CONTENT_TYPE = "multipart/x-mixed-replace;boundary=" PART_BOUNDARY;
static const char* _STREAM_BOUNDARY = "\r\n--" PART_BOUNDARY "\r\n";
static const char* _STREAM_PART = "Content-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n";

httpd_handle_t stream_httpd = NULL;

static esp_err_t stream_handler(httpd_req_t *req){
  camera_fb_t * fb = NULL;
  esp_err_t res = ESP_OK;
  size_t _jpg_buf_len = 0;
  uint8_t * _jpg_buf = NULL;
  char * part_buf[64];

  res = httpd_resp_set_type(req, _STREAM_CONTENT_TYPE);
  if(res != ESP_OK){
    return res;
  }

  while(true){
    fb = esp_camera_fb_get();
    if (!fb) {
      Serial.println("Bilderfassung fehlgeschlagen!");
      res = ESP_FAIL;
    } else {
      if(fb->width > 400){
        if(fb->format != PIXFORMAT_JPEG){
          bool jpeg_converted = frame2jpg(fb, 80, &_jpg_buf, &_jpg_buf_len);
          esp_camera_fb_return(fb);
          fb = NULL;
          if(!jpeg_converted){
            Serial.println("JPEG Kompression fehlgeschlagen!");
            res = ESP_FAIL;
          }
        } else {
          _jpg_buf_len = fb->len;
          _jpg_buf = fb->buf;
        }
      }
    }
    if(res == ESP_OK){
      size_t hlen = snprintf((char *)part_buf, 64, _STREAM_PART, _jpg_buf_len);
      res = httpd_resp_send_chunk(req, (const char *)part_buf, hlen);
    }
    if(res == ESP_OK){
      res = httpd_resp_send_chunk(req, (const char *)_jpg_buf, _jpg_buf_len);
    }
    if(res == ESP_OK){
      res = httpd_resp_send_chunk(req, _STREAM_BOUNDARY, strlen(_STREAM_BOUNDARY));
    }
    if(fb){
      esp_camera_fb_return(fb);
      fb = NULL;
      _jpg_buf = NULL;
    } else if(_jpg_buf){
      free(_jpg_buf);
      _jpg_buf = NULL;
    }
    if(res != ESP_OK){
      break;
    }
    //Serial.printf("MJPG: %uB\n",(uint32_t)(_jpg_buf_len));
  }
  return res;
}

WiFiServer TelnetServer(23);
WiFiClient TelnetServerClients[MAX_SRV_CLIENTS];

void startCameraServer(){
  httpd_config_t config = HTTPD_DEFAULT_CONFIG();
  config.server_port = 80;

  httpd_uri_t index_uri = {
    .uri       = "/",
    .method    = HTTP_GET,
    .handler   = stream_handler,
    .user_ctx  = NULL
  };
  
  //Serial.printf("Starte Webserver an Port: '%d'\n", config.server_port);
  if (httpd_start(&stream_httpd, &config) == ESP_OK) {
    httpd_register_uri_handler(stream_httpd, &index_uri);
  }
}

void SetServoPos(float winkel)
{
    int i = 0;
    
    // Übersetzen des Winkels in eine Pulsweite anhand Max/Min
    uint32_t duty = ((winkel/(myServoVerstellbereich))*(myServoMax-myServoMin)) + myServoMin; 
    
    if (inByte == 2) {
      Serial.print("Servo-Winkel: ");
        }
    if (inByte == 1) {
      TelnetServerClients[TelnetSession].print("Servo-Winkel: ");
    }
    if (MyServoPosition > duty) {
      for (i = MyServoPosition ; i >= duty; i -= 1) { 
        ledcWrite(myServoChannel, i);  
       delay(myServoPause); 
      }
    } else {
      for (i = MyServoPosition ; i <= duty; i += 1) { 
        ledcWrite(myServoChannel, i);  
       delay(myServoPause); 
      }
    }
    if (inByte == 2) {
      Serial.print(winkel); 
      Serial.print(" (");   
      Serial.print(duty);
      Serial.println(") Grad (Zahlwert)");
    }
    if (inByte == 1) {
      TelnetServerClients[TelnetSession].print(winkel); 
      TelnetServerClients[TelnetSession].print(" (");   
      TelnetServerClients[TelnetSession].print(duty);
      TelnetServerClients[TelnetSession].println(") Grad (Zahlwert)");
    }

    MyServoPosition = duty;
    delay(200);
//    ledcSetup(myServoChannel, 0, 8);
}

void setup() {
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); //disable brownout detector
 
  Serial.begin(115200);
  Serial.setDebugOutput(false);
}

void loop() {
  switch (gegenwaertiger_Zustand) {
    
    case 1:   //Kamera initialisieren
      {
        camera_config_t config;
        config.ledc_channel = LEDC_CHANNEL_0;
        config.ledc_timer = LEDC_TIMER_0;
        config.pin_d0 = Y2_GPIO_NUM;
        config.pin_d1 = Y3_GPIO_NUM;
        config.pin_d2 = Y4_GPIO_NUM;
        config.pin_d3 = Y5_GPIO_NUM;
        config.pin_d4 = Y6_GPIO_NUM;
        config.pin_d5 = Y7_GPIO_NUM;
        config.pin_d6 = Y8_GPIO_NUM;
        config.pin_d7 = Y9_GPIO_NUM;
        config.pin_xclk = XCLK_GPIO_NUM;
        config.pin_pclk = PCLK_GPIO_NUM;
        config.pin_vsync = VSYNC_GPIO_NUM;
        config.pin_href = HREF_GPIO_NUM;
        config.pin_sscb_sda = SIOD_GPIO_NUM;
        config.pin_sscb_scl = SIOC_GPIO_NUM;
        config.pin_pwdn = PWDN_GPIO_NUM;
        config.pin_reset = RESET_GPIO_NUM;
        config.xclk_freq_hz = 20000000;
        config.pixel_format = PIXFORMAT_JPEG; 
        config.frame_size = FRAMESIZE_UXGA;
        config.jpeg_quality = 10;
        config.fb_count = 2;
  
        esp_err_t err = esp_camera_init(&config);
        if (err != ESP_OK) {
          Serial.printf("Kamerainitialisierung fehlgeschlagen mit Error 0x%x", err);
          zukuenftigertiger_Zustand = 999;
        } else {
          zukuenftigertiger_Zustand = 2;
        }
      }
      break;

    case 2:   // PWM für Blitzlichr und Servo initialisieren
      {
        //Servo:  chanel 1, 166Hz, 8Bit;
        ledcSetup(myServoChannel, 166, 8);
        ledcAttachPin(myServoPin, myServoChannel);
        ledcWrite(myServoChannel, myServoMin);   //Servo in 0 Grad Stellung
        delay(1000);
              
        //Blitzlicht: Kanal 2, 500Hz, 8Bit;
        ledcSetup(FlashChannel, 500, 8);
        ledcAttachPin(FlashPin, FlashChannel);
        ledcWrite(FlashChannel, 0);            // LED aus

        zukuenftigertiger_Zustand = 3;
      }
      break;
  
    case 3:   // Initialisierung Blitze und Servo testen 
      { 
        Serial.print("Blitz-LED-Test...");
        ledcWrite(FlashChannel, 255); // LED zum Start an
        delay(500);
        ledcWrite(FlashChannel, 0);  // LED aus
        Serial.println(" fertig.");
       
        Serial.println("Servo-Test...");
        SetServoPos(180);
        delay(200);
        SetServoPos(10);
        Serial.println("fertig.");
        
        zukuenftigertiger_Zustand = 4;
      }
      break;
      
    case 4:   // erste WiFi-Verbindung aufbauen
      {
        int i=0;
        
        // Wi-Fi connection
        Serial.print("WiFi Verbindung zu ");
        Serial.print(ssid1);
        Serial.print(" wird aufgebaut...");
        WiFi.begin(ssid1, password1);
        while ((WiFi.status() != WL_CONNECTED) and (i != 10)) {
          ledcWrite(FlashChannel, 1); // LED an
          delay(10);
          ledcWrite(FlashChannel, 0); // LED aus
          delay(500);
          i=i+1;
          Serial.print(".");
        }
        if (WiFi.status() == WL_CONNECTED) {
          Serial.println(" fertig.");
          zukuenftigertiger_Zustand = 8;
        } else {
          Serial.println(" fehlgeschlagen!!!");
          zukuenftigertiger_Zustand = 5;
        }
        
      }
      break;
      
    case 5:   // Wifi abschalten wenn erste Verbindung nicht zustande kam
      {
        Serial.print("WiFi wird abgeschaltet...");
        WiFi.mode(WIFI_OFF);
        delay(2000);
        Serial.println(" fertig.");
        zukuenftigertiger_Zustand = 6;
      }
      break;

    case 6:   // zweite WiFi-Verbindung  aufbauen
      {
        int i=0;
        
        // Wi-Fi connection
        Serial.print("WiFi Verbindung zu ");
        Serial.print(ssid2);
        Serial.print(" wird aufgebaut...");
        WiFi.begin(ssid2, password2);
        while ((WiFi.status() != WL_CONNECTED) and (i != 10)) {
          ledcWrite(FlashChannel, 10); // LED an
          delay(10);
          ledcWrite(FlashChannel, 0); // LED aus
          delay(500);
          i=i+1;
          Serial.print(".");
        }
        if (WiFi.status() == WL_CONNECTED) {
          Serial.println(" fertig.");
          zukuenftigertiger_Zustand = 8;
        } else {
          Serial.println(" fehlgeschlagen!!!");
          zukuenftigertiger_Zustand = 7;
        }
        
      }
      break;

    case 7:   // Wifi abschalten wenn zweite Verbindung nicht zustande kam
      {
        Serial.print("WiFi wird abgeschaltet...");
        WiFi.mode(WIFI_OFF);
        delay(2000);
        Serial.println(" fertig.");
        zukuenftigertiger_Zustand = 999;
      }
      break;

    case 8:   // Videostream einschalten
      {
        Serial.print("Kamerastream wird gestartet...");
        startCameraServer();
        Serial.print(" fertig.    (http://");
        Serial.print(WiFi.localIP());
        Serial.println(")");
        zukuenftigertiger_Zustand = 9;
      }
      break;

    case 9:   // Telnetserver einschalten
      {
        Serial.print("Telnetserver wird gestartet...");
        TelnetServer.begin();
        TelnetServer.setNoDelay(true);
        Serial.print(" fertig.    (");
        Serial.print(WiFi.localIP());
        Serial.println(")");
        zukuenftigertiger_Zustand = 90;
      }
      break;

    case 90:    // Testen ob noch ungenutzte Sitzungen offen stehen und diese ggf. schließen
      {   inByte = 0;     
          for(i = 0; i < MAX_SRV_CLIENTS; i++)
          {
            if (TelnetServerClients[i] && !TelnetServerClients[i].connected())
            {
              Serial.print("Client disconnected ... terminate session "); Serial.println(i+1); 
              TelnetServerClients[i].stop();
            }
          }
          zukuenftigertiger_Zustand = 91;
      }
      break;

     case 91:    // Prüfen ob Clients mit dem TelnetServer Verbindung aufgenommen haben
       { 
         if (TelnetServer.hasClient())
         {
            zukuenftigertiger_Zustand = 92;
         } else {
            zukuenftigertiger_Zustand = 94;   // Seriellen Input auswerten
         } 
       }
      break;

    case 92:    // Sitzungen prüfen uund neue Clienten begrüßen
      {                            
        for(i = 0; i < MAX_SRV_CLIENTS; i++) // Nach neuen Sitzungen suchen und diese begrüßen
        {
          Serial.print("Teste Telnet-Sitzungen "); Serial.println(i+1);
          
          // find free socket
          if (!TelnetServerClients[i])
          {
            TelnetServerClients[i] = TelnetServer.available(); 
            
            Serial.print("Neue Telnetcliens haben Verbindung aufgebaut "); Serial.println(i+1);
            
            TelnetServerClients[i].flush();  // Input buffer leeren
            TelnetServerClients[i].print("ESP32 - Cam Controller ");
            TelnetServerClients[i].println(Version);
           
            TelnetServerClients[i].print("Millisekunden seit dem Start des Moduls: ");
            TelnetServerClients[i].println(millis());
            
            TelnetServerClients[i].print("Freies RAM: ");
            TelnetServerClients[i].println(ESP.getFreeHeap());
      
            TelnetServerClients[i].println("----------------------------------------------------------------");
            zukuenftigertiger_Zustand = 94;
            break;
          }
          else
          {
            Serial.println("Sitzung wird bereits genutzt.");
            zukuenftigertiger_Zustand = 93;
          }
        }
      }
      break;

    case 93:    // Maximale Sitzungszahl erreicht, neue ablehnen
      {
        Serial.println("Maximale Anzahl der Telnet-Sitzung erreicht!");
        TelnetServer.available().stop();                           
        zukuenftigertiger_Zustand = 94;      }
      break;

    case 94:   // Daten aus den Telnetsitzungn lesen.
      {
        zukuenftigertiger_Zustand = 95;  // Vordefinierter zukünftiger zustand für den Fall, dass keine Daten vorliegen
        for(i = 0; i < MAX_SRV_CLIENTS; i++)
          {
            if (TelnetServerClients[i] && TelnetServerClients[i].connected())
            {
              if(TelnetServerClients[i].available())
              { 
                command = TelnetServerClients[i].read();
                int c = 0;
                while(TelnetServerClients[i].available())
                {
                  inputString[c] = TelnetServerClients[i].read();
                  c++;
                }
                inByte=1;
                zukuenftigertiger_Zustand = 100;
              }
            }
          }
       }
      break;

    case 95:   // Serielle Schnittstelle auf eingehende Daten überwachen
      {
        if (Serial.available()) {
          command = Serial.read();
          int c = 0;
          while(Serial.available())
          {
            inputString [c] = Serial.read();
            c++;
          }
          inByte=2;
          zukuenftigertiger_Zustand = 100;
        }
        else {
          zukuenftigertiger_Zustand = 90;
        }       
      }
      break;
      
    case 100:    // Auf der seriellen Schnittstelle eingetroffene Zeichen auswerten
      {
        switch (command) {
          case 'B':
            {
              zukuenftigertiger_Zustand = 110;  //Kommando für Flash
            }
            break;

          case 'i':
            {
              zukuenftigertiger_Zustand = 999;  //Kommando für Reboot
            }
            break;

          case 'R':
            {
              zukuenftigertiger_Zustand = 210;  //Kommando für Rotation
            }
            break;

          case 'l':
            {
              zukuenftigertiger_Zustand = 998;  //Kommando im Logs zu aktivieren/deaktivieren
            }
            break;
            
          case 'q':
            {
              zukuenftigertiger_Zustand = 800;  //Kommando zum beenden der Telnetsession
            }
            break;
            
          case '?':
            {
              if (inByte == 2) {                
                Serial.print("ESP32 - Cam Controller ");
                Serial.println(Version);
                Serial.println("______________________________________________________");
                Serial.println("Befehle");
                Serial.println("Bxxx    - Blitz    (Intensität 0 < xxx < 100)");
                Serial.print("Rxxx    - Rotation (Winkel 0 < xxx < ");
                Serial.print(myServoVerstellbereich);
                Serial.println(")");
                Serial.println("l       - Zustandslog aktivieren/deaktivieren");
                Serial.println("i       - Initialisierung");
                Serial.println("?       - Diese Liste");
              }              
              if (inByte == 1) {
                TelnetServerClients[TelnetSession].print("ESP32 - Cam Controller ");
                TelnetServerClients[TelnetSession].print(Version);
                TelnetServerClients[TelnetSession].println("");
                TelnetServerClients[TelnetSession].println("______________________________________________________");
                TelnetServerClients[TelnetSession].println("Befehle");
                TelnetServerClients[TelnetSession].println("Bxxx    - Blitz    (Intensität 0 < xxx < 100)");
                TelnetServerClients[TelnetSession].print("Rxxx    - Rotation (Winkel 0 < xxx < ");
                TelnetServerClients[TelnetSession].print(myServoVerstellbereich);
                TelnetServerClients[TelnetSession].println(")");
                TelnetServerClients[TelnetSession].println("q       - Telnet-Sitzung beenden");
                TelnetServerClients[TelnetSession].println("l       - Zustandslog aktivieren/deaktivieren");
                TelnetServerClients[TelnetSession].println("i       - Initialisierung");
                TelnetServerClients[TelnetSession].println("?       - Diese Liste");                              
              }
              zukuenftigertiger_Zustand = 90;
            }
            break;
            
          default: {
              // Ungültiger Sequenzbeginn -> ignorieren
              zukuenftigertiger_Zustand = 90;
            }
            break;
        }
      }
      break;
      
    case 110:    // Blitz-LED steuern
      {
        int op = atoi(inputString);
        
        if (op <= 100) {                  
          uint32_t duty = (op*254)/100;                           
          ledcWrite(FlashChannel, duty);
          if (inByte == 2) {
            Serial.print("Blitzlicht auf ");
            Serial.print(op);
            Serial.println("% gesetzt");
          }
          if (inByte == 1) {
            TelnetServerClients[TelnetSession].print("Blitzlicht auf ");
            TelnetServerClients[TelnetSession].print(op);
            TelnetServerClients[TelnetSession].println("% gesetzt"); 
          }
        } else {
          if (inByte == 2) {
            Serial.print("Wert (");
            Serial.print(op);
            Serial.println(") außerhalb des möglichen Bereichs!"); 
          }
          if (inByte == 1) {
            TelnetServerClients[TelnetSession].print("Wert (");
            TelnetServerClients[TelnetSession].print(op);
            TelnetServerClients[TelnetSession].println(") außerhalb des möglichen Bereichs!"); 
          }
        }
        zukuenftigertiger_Zustand = 90;
      }
      break;

    case 210:    // Servo Steuern
      {
        int op = atoi(inputString);
        
        if (op <= myServoVerstellbereich) {
          SetServoPos(op);
        } else {
          if (inByte == 2) {
            Serial.print("Wert (");
            Serial.print(op);
            Serial.println(") außerhalb des möglichen Bereichs!"); 
          }
          if (inByte == 1) {
            TelnetServerClients[TelnetSession].print("Wert (");
            TelnetServerClients[TelnetSession].print(op);
            TelnetServerClients[TelnetSession].println(") außerhalb des möglichen Bereichs!"); 
          }
        }
        zukuenftigertiger_Zustand = 90;
      }
      break;

    case 800:    // Sitzung beenden
      {
        if (inByte == 2) {
          Serial.print("Telnetsitzung beendet");
        }
        if (inByte == 1) {
          TelnetServerClients[TelnetSession].println("Sitzung beendet.");
        }
        TelnetServerClients[TelnetSession].stop();
        zukuenftigertiger_Zustand = 90;
      }
      break;


    case 998:   // Logs aktivieren (kleiner Debugmodus)
      { 
        if (LogLevel == 0) {
          LogLevel = 1; 
          if (inByte == 2) {
            Serial.print("Zustands Log aktiviert");
          }
          if (inByte == 1) {
            TelnetServerClients[TelnetSession].print("Zustands Log aktiviert");
          }
        } else {
          LogLevel = 0;
          if (inByte == 2) {
            Serial.print("Zustands Log deaktiviert");
          }
          if (inByte == 1) {
            TelnetServerClients[TelnetSession].print("Zustands Log deaktiviert");
          }        }
        zukuenftigertiger_Zustand = 90;   
      }
      break;

    case 999:   // Re-Initialisierung auslösen
      { 
        Serial.println("---> Re-Initialisierung.  [RESET]");  
        ESP.restart();        
      }
      break;
      
    default: {    // Ungültiger Zustand eingetreten !!!
        Serial.println("ACHTUNG ungueltiger Zustand in Hauptmodul erkannt.");
        delay (3000);
        zukuenftigertiger_Zustand = 999; //Nicht behandelbarer Fehler - Reboot auslösen
      }
      break;
  }
  gegenwaertiger_Zustand = zukuenftigertiger_Zustand;
  if (LogLevel == 1) {
    if (inByte == 2) {
      Serial.print("Zukünftiger Zustand: ");
      Serial.println(zukuenftigertiger_Zustand);
    }
    if (inByte == 1) {
      TelnetServerClients[TelnetSession].print("Zukünftiger Zustand: ");
      TelnetServerClients[TelnetSession].println(zukuenftigertiger_Zustand);
    }
    delay(100);
  }
}
