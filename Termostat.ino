/*---Termostat s propojenim na Google docs sheet-----*/
/*---------------------------------------------------*/

/*
  **Vstupy**
  D2 GPIO4 - SW1 aut/man
  D3 GPIO0 - SW2 on/off
  D9 GPIO3 - Teplomer

  **Vystupy**
  D0 GPIO16 - LED
  D1 GPIO5 - Rele

  **Displej**
  D5 GPIO14 - SCLK displeje
  D6 GPIO12 - data/command displeje
  D7 GPIO13 - SDIN displeje
  D8 GPIO15 - /CS displeje
  D4 GPIO2  - /RES displeje
 */

/*------------------Popis pinů displeje--------------*/
/*
1              VDD            2.7-3.3V
2              SCLK           Clock input
3              SDIN           Serial data
4              data/command   Data or command
5              /CS            Chip select
6              OSC            NC
7              GND
8              /RES           Reset

*/
/*---------------------------------------------------*/
/*
 *  Displej má 6 řádků a 14 znaků na řádek
 */
/*---------------------------------------------------*/
/*---------------------------------------------------*/


#include <ESP8266WiFi.h>
#include <HTTPSRedirect.h>
#include <DHT.h>
#include <string.h>
//#include <stdio.h>
#include <PCD8544.h>


#define relay 5
#define LED D0
#define SW_aut_man 4
#define SW_ON_OFF 0
#define DHTPIN 3

// Uncomment whatever DHT sensor type you're using!
#define DHTTYPE DHT11  // DHT 11
//#define DHTTYPE DHT21  // DHT 21
//#define DHTTYPE DHT22  // DHT 22

DHT dht(DHTPIN,DHTTYPE);

//LCD
static PCD8544 lcd;

int chart = 0;


const char WEBSITE[] = "api.pushingbox.com";  //pushingbox API server
const String devid = "vCBF80EA0AE7D345";   //device ID from Pushingbox

const char* ssid;
const char* password;

const char* ssid1 = "AndroidAP_Tomas";
const char* password1 = "****";

const char* ssid2 = "Tomas_Plzen";
const char* password2 = "****";

const char* ssid3 = "AP_COMTREND";
const char* password3 = "****";


const char* host = "script.google.com";
const char* googleRedirHost = "script.googleusercontent.com";
const char *GScriptId = "****";

const int httpsPort = 443;

// http://askubuntu.com/questions/156620/how-to-verify-the-ssl-fingerprint-by-command-line-wget-curl/
// echo | openssl s_client -connect script.google.com:443 |& openssl x509 -fingerprint -noout
// www.grc.com doesn't seem to get the right fingerprint
// SHA1 fingerprint of the certificate
const char* fingerprint = "13 2C 74 E3 CB 4D 30 8F 4C 18 42 2E DC 8C 95 92 E7 E6 B5 A0";


IPAddress ip(192, 168, 2, 30);
IPAddress gateway(192, 168, 2, 1);
IPAddress subnet(255, 255, 255, 0);

WiFiClient client;  //Instantiate WiFi object

//********************************************************************//
//------------------------------SETUP---------------------------------//
//********************************************************************//
void setup() {
  ESP.wdtDisable();         // watchdog disable

  Serial.begin(9600);       // See the connection status in Serial Monitor

  lcd.begin(84, 48);        // PCD8544-compatible displays may have a different resolution...
  lcd.setCursor(0, 0);
  lcd.print("****Radek_0***");
  lcd.setCursor(0, 1);
  lcd.print("****Radek_1***");
  lcd.setCursor(0, 2);
  lcd.print("****Radek_2***");
  lcd.setCursor(0, 3);
  lcd.print("****Radek_3***");
  lcd.setCursor(0, 4);
  lcd.print("****Radek_4***");
  lcd.setCursor(0, 5);
  lcd.print("****Radek_5***");
  delay(1000);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("*Inicializace*");

  pinMode(SW_aut_man, INPUT_PULLUP);
  pinMode(SW_ON_OFF, INPUT_PULLUP);
  pinMode(LED, OUTPUT);
  digitalWrite(LED, HIGH);     // 3.3V

  ESP.wdtEnable(10000);     // zapne soft wdt po 10s
}

//********************************************************************//
//-------------------------------WIFI---------------------------------//
//********************************************************************//
void wifiConnect(const char* ssid, const char* password) {
  WiFi.disconnect(true);
  Serial.print("Pripojuji se k: ");
  Serial.println(ssid);

  lcd.setCursor(0, 0);
  lcd.print((String) ssid + ":O");

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi pripojeno");
  Serial.print("IP adresa NodeMCU: ");
  Serial.println(WiFi.localIP());

  lcd.setCursor(0, 0);
  lcd.print((String) ssid + ":X");
  lcd.setCursor(0, 1);
  lcd.print("IP");
  lcd.print(WiFi.localIP());

}

void wifiSetIP(){
  WiFi.mode(WIFI_STA);                  // Set WiFi mode to station (client)
  WiFi.config(ip, gateway, subnet);
}

//********************************************************************//
//-------------------------------HTTP---------------------------------//
//********************************************************************//
bool httpReqPushingBox(float temp, float humidity, int manON, int manOFF) {
      if (client.connect(WEBSITE, 80))
      {
           client.print("GET /pushingbox?devid=" + devid
         + "&temp="           + (String) temp
         + "&humidity="       + (String) humidity
         + "&manON="          + (String) manON
         + "&manOFF="         + (String) manOFF
           );

        client.println(" HTTP/1.1");
        client.print("Host: ");
        client.println(WEBSITE);
        client.println("User-Agent: ESP8266/1.0");
        client.println("Connection: close");
        client.println();

        while(client.available()){
            String line = client.readStringUntil('\r');
            Serial.print(line);
        }

        return true;
      }
      else return false;
}

bool httpReqHTTPSinit() {

        HTTPSRedirect client(httpsPort);
        Serial.print("Connecting to ");
        Serial.println(host);

        lcd.setCursor(0, 2);
        lcd.print("Google:pripo..");

        bool flag = false;
        for (int i=0; i<5; i++){
          int retval = client.connect(host, httpsPort);
          if (retval == 1) {
             flag = true;
             break;
          }
          else  {
            Serial.println("Connection failed. Retrying...");
            lcd.setCursor(0, 2);
            lcd.print("Google:nedostu");
          }
        }

        Serial.flush();
        if (!flag){
          Serial.print("Could not connect to server: ");
          Serial.println(host);
          Serial.println("Exiting...");
          lcd.setCursor(0, 2);
          lcd.print("Google:chyba");
          return false;
        }

        Serial.flush();
        if (client.verify(fingerprint, host)) {
          Serial.println("Certificate match.");
          lcd.setCursor(0, 2);
          lcd.print("Google:OK");
          return true;
        } else {
          Serial.println("Certificate mis-match");
          lcd.setCursor(0, 2);
          lcd.print("Google:mis-mat");
          return false;
        }


}

bool httpReqHTTPS(float temp, float humidity, int manON, int manOFF, int postLOG) {
  HTTPSRedirect client(httpsPort);
  //if (!client.connected() || client.connected()) {  //znova připojit za každé okolnosti
  if (!client.connected()) {
    //client.connect(host, httpsPort);
    lcd.setCursor(0, 2);
    lcd.print("Google:pripo..");
    for (int i=0; i<5; i++){
      int retval = client.connect(host, httpsPort);
      if (retval == 1)
        break;
    }
    lcd.setCursor(0, 2);
    lcd.clearLine();
    lcd.print("Google:OK");
  }

  String answer = client.readRedir(String("/macros/s/") + GScriptId
                  + "/exec?temp=" + temp + "&humidity=" + humidity
                  + "&manON=" + manON + "&manOFF=" + manOFF + "&postLOG=" + postLOG,
                  host, googleRedirHost);

    lcd.setCursor(0, 2);
    lcd.clearLine();
    lcd.print("Google:Vycteno");

//  String answer = client.readRedir(String("/macros/s/") + GScriptId + "/exec?temp=" + temp + "&humidity=" + humidity + "&manON=" + manON + "&manOFF=" + manOFF + "&postLOG=" + postLOG, host, googleRedirHost);

  Serial.println("Precteno z tabulky: " + answer);
  //Serial.println("Delka odpovedi: " + (String) answer.length());
  //Serial.println("Nalezeno: " + (String) answer.find("Pozadavek"));

  if (answer == "Pozadavek na topeni je:1\r") {
    //Serial.println("Vyhodnoceno jako pozadavek na zapnuti topeni");
    return true;
  }
  else {
    //Serial.println("Vyhodnoceno jako pozadavek na vypnuti topeni");
    return false;
  }
}

//********************************************************************//
//------------------------------RELE----------------------------------//
//********************************************************************//
void heatingON() {
  Serial.println("Zapinam topeni");
  pinMode(relay, OUTPUT);
  digitalWrite(relay, LOW);     // 0V
  digitalWrite(LED, LOW);       // 0V

  lcd.setCursor(chart, 4);
  lcd.print("->");
  chart++;
  if (chart >=83)
    chart =0;
}

void heatingOFF() {
  Serial.println("Vypinam topeni");
  pinMode(relay, INPUT);
  digitalWrite(LED, HIGH);     // 3.3V

  lcd.setCursor(chart, 4);
  lcd.print("_>");
  chart++;
  if (chart >=83)
    chart =0;
}


//********************************************************************//
//-------------------------------LCD----------------------------------//
//********************************************************************//
void lcd_init() {

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("WiFi:SSID:NC");
  lcd.setCursor(0, 1);
  lcd.print("IP:0.0.0.0");
  lcd.setCursor(0, 2);
  lcd.print("Google:NC");
  //lcd.setCursor(0, 3);
  //lcd.print("Rezim a stav");
  //lcd.setCursor(0, 4);
  //lcd.print("Historie stavu");
  lcd.setCursor(0, 5);
  lcd.print("Udaje z koupel");
}

void lcd_autHeatON() {
  lcd.setCursor(0, 3);
  //lcd.clearLine();
  lcd.print("Rezim:AUT  ZAP");
}

void lcd_autHeatOFF() {
  lcd.setCursor(0, 3);
  //lcd.clearLine();
  lcd.print("Rezim:AUT  VYP");
}

void lcd_manHeatON() {
  lcd.setCursor(0, 3);
  //lcd.clearLine();
  lcd.print("Rezim:MAN  ZAP");
}

void lcd_manHeatOFF() {
  lcd.setCursor(0, 3);
  //lcd.clearLine();
  lcd.print("Rezim:MAN  VYP");
}

void lcd_temp(float temp, float humidity) {
  lcd.setCursor(0, 5);
  lcd.clearLine();
  /*
  lcd.print("T:");
  lcd.print((String) ((int) temp));
  lcd.setCursor(3, 5);
  lcd.print("*C");
  lcd.setCursor(8, 5);
  lcd.print("H:");
  lcd.print((String) ((int) humidity));
  lcd.setCursor(13, 5);
  lcd.print("%");
  */
  String message;
  message = "T:" + ((String) ((float) temp)) + "  H:" + ((String) ((int) humidity)) + "%";
  lcd.print(message);
}


//********************************************************************//
//-------------------------------MAIN---------------------------------//
//********************************************************************//
// the loop function runs over and over again forever
void loop() {

  bool state = false;
  int loop60 = 0;
  int loop600 = 0;
  float temp, humidity, t, h;
  int manON, manOFF, autMode, manMode, postLOG;

  temp = 24.5;
  humidity = 30.5;
  manON = 0;
  manOFF = 0;
  autMode = 0;
  manMode = 0;

  Serial.println("Program startuje...");
  lcd_init();

  wifiSetIP();

  //ssid = ssid1;
  //password = password1;

  //ssid = ssid2;
  //password = password2;

  ssid = ssid2;
  password = password2;

  wifiConnect(ssid, password);

  httpReqHTTPSinit();

  Serial.println("Smycka programu start");
  while(1) {
    Serial.println("V zakladni smycce");
    //lcd.setCursor(0, 3);
    //lcd.print("Rezim:AUT wait");

    for (;(loop600 < 600);loop60++,loop600++) {
      Serial.println("Cekam: " + (String) loop60 + "s/60s| " + (String) loop600 +  "s/600s");

      lcd.setCursor(0, 2);
      lcd.clearLine();
      lcd.print("Google:" + (String) loop60 + "/" + (String) loop600);
      lcd.setCursor(0, 0);
      lcd.clearLine();
      lcd.print("WiFi signal:");
      lcd.setCursor(0, 1);
      lcd.clearLine();
      lcd.print(WiFi.RSSI());


      // Reading temperature or humidity takes about 250 milliseconds!
      // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
      float h = dht.readHumidity();
      // Read temperature as Celsius (the default)
      float t = dht.readTemperature();

      // Updatuj pouze pokud bylo cteni uspesne
      if (!(isnan(h))) {
        humidity = h;
      }

      if (!(isnan(t))) {
        temp = t;
      }


      lcd_temp(temp, humidity);

      if (digitalRead(SW_aut_man)) {
        autMode = 1;
        manMode = 0;
        //lcd.setCursor(0, 3);
        //lcd.print("Rezim:AUT");

        if (state == true)
          lcd_autHeatON();
        else
          lcd_autHeatOFF();

      }
      else {
        autMode = 0;
        manMode = 1;
        //lcd.setCursor(0, 3);
        //lcd.print("Rezim:MAN");
      }

      if (digitalRead(SW_ON_OFF)) {
        manON = 1;
        manOFF = 0;
        if (manMode == 1) {
          heatingON();
          lcd_manHeatON();
        }
      }
      else {
        manON = 0;
        manOFF = 1;
        if (manMode == 1) {
          heatingOFF();
          lcd_manHeatOFF();
        }
      }



      if (loop60 == 60) {
        loop60 = 0;
        if (WiFi.status() != WL_CONNECTED)
          wifiConnect(ssid, password);  //reconnect WiFi pokud nutno
        state = httpReqHTTPS(temp, humidity, manON, manOFF, 0);

        if (autMode == 1) {
          if ((state == true) && (temp  < 26)) {  //pojistka, aby se nepretopilo pri chybe v Google docs
            heatingON();
            lcd_autHeatON();
          }
          else {
            heatingOFF();
            lcd_autHeatOFF();
          }
        }

      }
      delay(700);
      ESP.wdtFeed();
    }
    loop600 = 0;
    loop60 = 0;
    if (WiFi.status() != WL_CONNECTED)
      wifiConnect(ssid, password);  //reconnect WiFi pokud nutno
    state = httpReqHTTPS(temp, humidity, manON, manOFF, 1);

    if (autMode == 1) {
      if ((state == true) && (temp  < 26)) {  //pojistka, aby se nepretopilo pri chybe v Google docs
        heatingON();
        lcd_autHeatON();
      }
      else {
        heatingOFF();
        lcd_autHeatOFF();
      }
    }
  }
}


//End of the program
//Comment 1
