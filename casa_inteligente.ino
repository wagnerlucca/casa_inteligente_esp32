/**
 * ================================================================================
 *          DECLARAÇÃO DE BIBLIOTECAS:
 * ================================================================================
 */

// #include <SPI.h>
// #include "Arduino.h"
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <WiFi.h>
#include "heltec.h"
// #include "images.h"

/**
 * ================================================================================
 *          DECLARAÇÃO DE VARIÁVEIS:
 * ================================================================================
 */

// Define o nome da rede Wifi que se pretende conectar o dispositivo.
const char* ssid = "MATRIX";      

// Define a senha para se conectar na rede Wifi.
const char* password = "sentapua1945"; 

// Define a porta do servidor web.
AsyncWebServer server(80);
// WiFiServer server(80);

// Define a porta de controle para o led.
// #define pinLedSala 16

// Define a banda do monitor.
#define BAND 868E6

const char* PARAM_INPUT_1 = "output";
const char* PARAM_INPUT_2 = "state";

const char index_html[] PROGMEM = R"rawliteral(
  <!DOCTYPE HTML>
  <html>
    <head>
      <title>ESP Web Server</title>
      <meta charset=\"UTF-8\" name="viewport" content="width=device-width, initial-scale=1">
      <link rel="icon" href="data:,">
  
      <style>
        html {font-family: Arial; display: inline-block; text-align: center;}
        h2 {font-size: 3.0rem;}
        p {font-size: 3.0rem;}
        body {max-width: 600px; margin:0px auto; padding-bottom: 25px;}
        .switch {position: relative; display: inline-block; width: 120px; height: 68px} 
        .switch input {display: none}
        .slider {position: absolute; top: 0; left: 0; right: 0; bottom: 0; background-color: #ccc; border-radius: 6px}
        .slider:before {position: absolute; content: ""; height: 52px; width: 52px; left: 8px; bottom: 8px; background-color: #fff; -webkit-transition: .4s; transition: .4s; border-radius: 3px}
        input:checked+.slider {background-color: #27AE60}
        input:checked+.slider:before {-webkit-transform: translateX(52px); -ms-transform: translateX(52px); transform: translateX(52px)}
      </style>
    </head>
    
    <body>
      <h2>ESP Web Server</h2>
      %BUTTONPLACEHOLDER%
      
      <script>
        function toggleCheckbox(element) 
        {
          var xhr = new XMLHttpRequest();
          
          if(element.checked)
          {
            xhr.open("GET", "/update?output="+element.id+"&state=1", true); 
          }
          else 
          { 
            xhr.open("GET", "/update?output="+element.id+"&state=0", true); 
          }
          
          xhr.send();
        }
      </script>
    </body>
  </html>
)rawliteral";

// Replaces placeholder with button section in your web page
String processor(const String& var){
  //Serial.println(var);
  if(var == "BUTTONPLACEHOLDER"){
    String buttons = "";
    buttons += "<h4>Output - GPIO 16 (Sala)</h4><label class=\"switch\"><input type=\"checkbox\" onchange=\"toggleCheckbox(this)\" id=\"16\" " + outputState(16) + "><span class=\"slider\"></span></label>";
    buttons += "<h4>Output - GPIO 4 (Cozinha)</h4><label class=\"switch\"><input type=\"checkbox\" onchange=\"toggleCheckbox(this)\" id=\"4\" " + outputState(4) + "><span class=\"slider\"></span></label>";
    buttons += "<h4>Output - GPIO 33 (Quarto)</h4><label class=\"switch\"><input type=\"checkbox\" onchange=\"toggleCheckbox(this)\" id=\"33\" " + outputState(33) + "><span class=\"slider\"></span></label>";
    return buttons;
  }
  return String();
}

String outputState(int output){
  if(digitalRead(output)){
    return "checked";
  }
  else {
    return "";
  }
}

/**
 * ================================================================================
 *          FUNÇÃO LOGOHELTEC():
 * ================================================================================
 */

void logoHeltec()
{
  // Limpa a tela.
  Heltec.display -> clear();
  
  // Carrega a logo da Heltec.
  // Precisa do arquivo "images.h" na pasta do projeto.
  // Heltec.display -> drawXbm(0,5,logo_width,logo_height,(const unsigned char *)logo_bits);
  
  // Gera a saída carregada na tela.
  Heltec.display -> display();

  // Aguarda um tempo.
  delay(1000);

  // Limpa a tela.
  Heltec.display -> clear();
}

/**
 * ================================================================================
 *          FUNÇÃO WIFICONNECT():
 * ================================================================================
 */
void wifiSetUp()
{
  // Desconecta de concexões prévias realizadas pelo dispositivo.
  WiFi.disconnect(true);

  // Aguarda um tempo.
  delay(100);

  // Seta a tipo de conexão, standard.
  WiFi.mode(WIFI_STA);

  // Configura a auto conexão a rede Wifi.
  WiFi.setAutoConnect(true);

  // Aguarda um tempo.
  delay(100);
  
  // Exibe mensagem no display.
  Heltec.display -> drawString(0, 0, "Conectando...");
  Heltec.display -> drawString(0, 15, "SSID: " + (String)ssid);
  Heltec.display -> display();
  delay(1000);

  //Inicia a conexão com a rede, especificando o nome da rede e a senha de acesso.
  WiFi.begin(ssid , password);

  // Contador que armazena os "." que são exibidos na tela durante a tentativa de conexão.
  byte cont = 0;

  // Testa a conexão.
  while (WiFi.status() != WL_CONNECTED && cont < 40) 
  {
    // Exibe mensagem no display.
    Heltec.display -> drawString(cont, 25, ".");
    Heltec.display -> display();
    
    // Incrementa o contador.
    cont = cont + 2;

    // Aguarda um tempo.
    delay(500);
  }

  if(WiFi.status() == WL_CONNECTED)
  {
    // Limpa a tela.
    Heltec.display -> clear();
    
    // Quando a conexão for estabelecida.
    // Exibe mensagem no display.
    Heltec.display -> drawString(0, 0, "SSID: " + (String)ssid);
    Heltec.display -> drawString(0, 12, "STATUS: Conectado.");
    Heltec.display -> drawString(0, 24, "IP: " + WiFi.localIP().toString());
  
    // Imprime a potência da conexão estabelecida.
    long rssi = WiFi.RSSI();

    Heltec.display -> drawString(0, 36, "SINAL (RSSI): " + (String)rssi + " dBm");
    Heltec.display -> display();
  }
  else
  {
    Heltec.display -> drawString(0, 40, "Conexão...Falhou!");
    Heltec.display -> display();
  }
}

/**
 * ================================================================================
 *          FUNÇÃO SETUP():
 * ================================================================================
 */
 
void setup()
{
  // Define a velocidade de comunicação com a porta serial.
  // Serial.begin(115200);

  pinMode(16 , OUTPUT);
  digitalWrite(16 , LOW);
  // pinMode(4 , OUTPUT);
  // digitalWrite(4 , LOW);
  // pinMode(33 , OUTPUT);
  // digitalWrite(33 , LOW);
    
  // Inicia o display.
  Heltec.begin(true /*DisplayEnable Enable*/, true /*LoRa Enable*/, true /*Serial Enable*/, true /*LoRa use PABOOST*/, BAND /*LoRa RF working band*/);

  // Executa a função logoHeltec().
  // logoHeltec();

  // Executa a função wifiSetUp().
  wifiSetUp();

// ================================

  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html, processor);
  });

  // Send a GET request to <ESP_IP>/update?output=<inputMessage1>&state=<inputMessage2>
  server.on("/update", HTTP_GET, [] (AsyncWebServerRequest *request) {
    String inputMessage1;
    String inputMessage2;
    // GET input1 value on <ESP_IP>/update?output=<inputMessage1>&state=<inputMessage2>
    if (request->hasParam(PARAM_INPUT_1) && request->hasParam(PARAM_INPUT_2)) {
      inputMessage1 = request->getParam(PARAM_INPUT_1)->value();
      inputMessage2 = request->getParam(PARAM_INPUT_2)->value();
      digitalWrite(inputMessage1.toInt(), inputMessage2.toInt());
    }
    else {
      inputMessage1 = "No message sent";
      inputMessage2 = "No message sent";
    }
    Serial.print("GPIO: ");
    Serial.print(inputMessage1);
    Serial.print(" - Set to: ");
    Serial.println(inputMessage2);
    request->send(200, "text/plain", "OK");
  });

  // Inicia o serviço Web.
  server.begin();
}

/**
 * ================================================================================
 *          FUNÇÃO LOOP():
 * ================================================================================
 */
 
void loop() 
{

}