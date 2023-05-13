

/**
 * ================================================================================
 *          DECLARAÇÃO DE BIBLIOTECAS:
 * ================================================================================
 */

// #include <SPI.h>
// #include "Arduino.h"
#include <WiFi.h>
#include "heltec.h"
#include "images.h"

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
WiFiServer server(80);

// Define a porta de controle para o led.
#define pinLedSala 16

// Define a banda do monitor.
#define BAND    868E6  //you can set band here directly,e.g. 868E6,915E6

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
  Heltec.display -> drawXbm(0,5,logo_width,logo_height,(const unsigned char *)logo_bits);
  
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
    
  // Inicia o display.
  Heltec.begin(true /*DisplayEnable Enable*/, true /*LoRa Enable*/, true /*Serial Enable*/, true /*LoRa use PABOOST*/, BAND /*LoRa RF working band*/);

  // Executa a função logoHeltec().
  logoHeltec();

  // Executa a função logoHeltec().
  wifiSetUp();

  // Define porta como saída.
  pinMode(pinLedSala , OUTPUT);

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
  // Recebe a conexão do cliente.
  WiFiClient client = server.available();   // listen for incoming clients

  // Se existe cliente conectado.
  if (client) 
  {
    // Exibe mensagem para o usuário.
    Serial.println("Novo cliente");  

    // Cria uma String para manter a comunicação de dados com o cliente.
    String currentLine = "";

    // Looping enquanto a conexão estiver ativa.
    while (client.connected()) 
    {
      // Se há bytes de dados a serem lidos pelo cliente.
      if (client.available()) 
      {        
        // Leia um byte, então.
        char c = client.read();

        // Imprime fora do serial monitor.
        Serial.write(c);

        // Se o byte for uma nova linha,
        if (c == '\n') 
        {
          // Se a linha atual estiver em branco, você terá dois caracteres de nova linha em uma linha.
          // Esse é o fim da solicitação HTTP do cliente, então envia uma resposta.
          if (currentLine.length() == 0) 
          {
            // Os cabeçalhos HTTP sempre começam com um código de resposta (por exemplo, HTTP/1.1 200 OK)
            // e um tipo de conteúdo para que o cliente saiba o que está por vir, então uma linha em branco.
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println();

            // Modelo de código antigo.
            /**
            // O conteúdo de resposta HTTP após o cabeçalho.
            client.print("Click <a href=\"/ON\">aqui</a> para ligar o pino 16 do Led.<br>");
            client.print("Click <a href=\"/OFF\">aqui</a> para desligar o pino 16 do Led.<br>");

            // Check to see if the client request was "GET /H" or "GET /L":
            if (currentLine.endsWith("GET /ON")) 
            {
              digitalWrite(pinLed , HIGH);               // GET /H turns the LED on
            }
            
            if (currentLine.endsWith("GET /OFF")) 
            {
              digitalWrite(pinLed , LOW);                // GET /L turns the LED off
            }
            */

            // Modelo de Tabela.
            /**
            <table>
            <tr>
                <th>Coluna 1</th>
                <th>Coluna 2</th>
            </tr>
  
            <tr>
              <td>Linha 1, Coluna 1</td>
              <td>Linha 1, Coluna 2</td>
            </tr>
  
            <tr>
              <td>Linha 2, Coluna 1</td>
              <td>Linha 2, Coluna 2</td>
            </tr>
  
            <tr>
              <td>Linha 3, Coluna 1</td>
              <td>Linha 3, Coluna 2</td>
            </tr>
          </table>
          */
            client.println("<html>");
            client.println("<title>Casa</title>");
            client.println("<meta charset=\"UTF-8\"/>");
            client.println("<form>");
            client.println("<h3>CONTROLE CASA INTELIGENTE</h3>");
            client.println("<br>");
            client.println("*** Controle Iluminação: ***");
            client.println("<br><br>");

            client.println("<table border=\"2\">");
              client.println("<tr>");
                client.println("<th>ON:</th>");
                client.println("<th>OFF:</th>");
                client.println("<th>STATUS:</th>");
              client.println("</tr>");

            client.println("<tr>");
                client.println("<td><input type=\"radio\" name=\"led16\" value=\"ON\">Luz Sala</td>");
                client.println("<td><input type=\"radio\" name=\"led16\" value=\"OFF\">Luz Sala</td>");
                client.println("<td><center>");
                client.println(digitalRead(pinLedSala));
                client.println("</center></td>");
            client.println("</tr>");
           
            client.println("<tr>");
                client.println("<td><input type=\"radio\" name=\"led16\" value=\"ON\">Luz Escritório</td>");
                client.println("<td><input type=\"radio\" name=\"led16\" value=\"OFF\">Luz Escritório</td>");
                client.println("<td><center>");
                client.println(digitalRead(pinLedSala));
                client.println("</center></td>");
            client.println("</tr>");
           
            client.println("</table>");
            client.println("<br><br>");
            client.println("<input type=\"submit\" value=\"Enviar\">");
            client.println("</form>");
            client.println("</html>");

            // O HTTP responde o final da solicitação com uma linha em branco.
            client.println();
            
            // Interrompe o looping com um break.
            break;
            
          } 
          else 
          {   
            // Se você tiver uma nova linha, limpe currentLine.
            currentLine = "";
          }
        } 
        else if (c != '\r') 
        {  
          // Se você tiver qualquer outra coisa além de um caractere de retorno, adiciona ao final de currentLine.
          currentLine += c;
        }


        // Verifica se a requisição do cliente contém o valor de led enviado pelo radio button, e implementa.
        if (currentLine.indexOf("led16=ON") != -1) 
        {
          digitalWrite(pinLedSala , HIGH); // Liga o LED
        }
        
        if (currentLine.indexOf("led16=OFF") != -1) 
        {
          digitalWrite(pinLedSala , LOW); // Desliga o LED
        }
        
      }
    }
    
    // Fecha a conexão.
    client.stop();
    Serial.println("Cliente desconectado!");
  }
}
