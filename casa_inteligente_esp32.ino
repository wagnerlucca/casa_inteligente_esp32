/**
 * ================================================================================
 *          DECLARAÇÃO DE BIBLIOTECAS:
 * ================================================================================
 */

#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <WiFi.h>
#include <LiquidCrystal_I2C.h>

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

// Define a porta de controle para o led.
// #define pinLedSala 16

// Define tamanho do LCD 20 x 4.
int lcdColumns = 20;
int lcdRows = 4;

// Inicializa o objeto da classe lcd.
LiquidCrystal_I2C lcd(0x27, lcdColumns, lcdRows);

byte _ssid[] = 
{
  B00000,
  B11111,
  B10101,
  B01110,
  B00100,
  B00100,
  B00100,
  B00100
};

byte _ip[] = 
{
  B11100,
  B01000,
  B01000,
  B01011,
  B11101,
  B00111,
  B00100,
  B00100
};

byte _sinal[] = 
{
  B00000,
  B00001,
  B00001,
  B00101,
  B00101,
  B10101,
  B10101,
  B10101
};

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

String outputState(int output)
{
  if(digitalRead(output))
  {
    return "checked";
  }
  else 
  {
    return "";
  }
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

  // Exibe mensagem no display. (LCD)
  lcd.clear();
  lcd.setCursor(0 , 0);
  lcd.print("Conectando...");
  lcd.setCursor(0 , 1);
  lcd.print("ssid: ");
  lcd.setCursor(6 , 1);  
  lcd.print((String)ssid);
  
  // Exibe mensagem no display. (Serial)
  Serial.println("Conectando...");
  Serial.print("ssid: ");
  Serial.println((String)ssid);  
  delay(1000);

  //Inicia a conexão com a rede, especificando o nome da rede e a senha de acesso.
  WiFi.begin(ssid , password);

  // Contador que armazena os "." que são exibidos na tela durante a tentativa de conexão.
  byte cont = 0;

  // Testa a conexão.
  while (WiFi.status() != WL_CONNECTED && cont < 20) 
  {
    // Exibe mensagem no display. (LCD)
    lcd.setCursor(cont , 2);
    lcd.print(".");
    
    // Exibe mensagem no display. (Serial)
    Serial.print(".");
        
    // Incrementa o contador.
    cont = cont + 1;

    // Aguarda um tempo.
    delay(500);
  }

  if(WiFi.status() == WL_CONNECTED)
  {
    // Exibe mensagem no display. (LCD)
    lcd.clear();
    
    lcd.setCursor(0 , 0);
    lcd.write(0);
    lcd.setCursor(1 , 0);
    lcd.print(" = " + (String)ssid);
    
    lcd.setCursor(0 , 1);
    lcd.write(1);
    lcd.setCursor(1 , 1);
    lcd.print(" = " + WiFi.localIP().toString());

    lcd.setCursor(0 , 2);
    lcd.write(2);
       
    // Imprime a potência da conexão estabelecida.
    long rssi = WiFi.RSSI();

    lcd.setCursor(1 , 2);
    lcd.print(" = " + (String)rssi + " dBm");
       
    // (Serial)
    Serial.print("IP: ");
    Serial.println(WiFi.localIP().toString()); 
  }
  else
  {
    // Exibe mensagem no display. (LCD)
    lcd.setCursor(0 , 2);  
    lcd.print("                    ");
    lcd.setCursor(0 , 3);  
    lcd.print("Conexao falhou!");

    // Exibe mensagem no display. (Serial)
    Serial.println("Conexao falhou!");
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
  Serial.begin(115200);

  pinMode(16 , OUTPUT);
  digitalWrite(16 , LOW);
  // pinMode(4 , OUTPUT);
  // digitalWrite(4 , LOW);
  // pinMode(33 , OUTPUT);
  // digitalWrite(33 , LOW);

  // Inicializa o LCD.
  lcd.init();
  
  // Ativa a Luz de Fundo do LCD.                      
  lcd.backlight(); 

  // Declara vetor de byte.
  lcd.createChar(0 , _ssid); 
  lcd.createChar(1 , _ip); 
  lcd.createChar(2 , _sinal); 
  
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
