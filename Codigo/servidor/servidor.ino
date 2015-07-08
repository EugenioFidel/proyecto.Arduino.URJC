#include <Twitter.h>
#include <SPI.h>              
#include <Ethernet.h>
#include <DHT.h>

#define MAXTEMP  28
#define MAXHUME  20
#define HUMVAL    2
#define HUMAL     3
#define TEMPVAL   5
#define TEMPAL    4
#define PINDHT    7


byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
IPAddress ip(192,168,1,144); // Ip del servidor 192.168.1.144 
IPAddress myDns(8,8,8,8); 
EthernetServer server(80);  // Arrancamos el servidor en el puerto estandard 80
DHT dht( PINDHT, DHT11);
char buffer[140];
Twitter twitter("3095700296-qHmjpweAY2Q2p3pxIUEU49QmJTy6ExXuFXetc27");  //Token del que nos provee Twitter para identificarnos como usuarios

void setup()
{
   pinMode(HUMVAL,OUTPUT);
   pinMode(HUMAL,OUTPUT);
   pinMode(TEMPVAL,OUTPUT);
   pinMode(TEMPAL,OUTPUT);
   dht.begin();
   Serial.begin(9600);
   while (!Serial) ;        

   Ethernet.begin(mac, ip,myDns);
   delay(3000);
   server.begin();          // Inicia el servidor web
   Serial.print("Servidor Web en la direccion: ");
   Serial.println(Ethernet.localIP());
}

void loop(){
  EthernetClient client = server.available();  // Cuando se produce una petición al puerto 80 de la ip local 192.168.1.144   
   if (client) 
    { Serial.println("CONECTADO");
      boolean currentLineIsBlank = true;  // Las peticiones HTTP finalizan con linea en blanco
      while (client.connected())
        { if (client.available())
             {  char c = client.read();
                Serial.write(c);   // mostramos gestión http por la consola
                // A partir de aquí mandamos nuestra respuesta
               if (c == '\n' && currentLineIsBlank) 
                  {   // Enviar una respuesta tipica
                      client.println("HTTP/1.1 200 OK");             
                      client.println("Content-Type: text/html");
                      client.println("Connection: close");
                      client.println("Refresh: 20");            // Actualizar cada minuto
                      client.println();
                      client.println("<!DOCTYPE HTML>");
                      client.println("<html>");

                      float h = dht.readHumidity();           // Leer el sensor
                      float t = dht.readTemperature();
                      Serial.println(t);
                      Serial.println(h);
                      // Creamos la página web
                      client.print("<head><title>Observacion ambiental</title></head>");
                      client.print("<body><IMG SRC=\"https://encrypted-tbn3.gstatic.com/images?q=tbn:ANd9GcTUN_2J_jYXp4s96GZEAd16L0Wk-wfY7bWBiYuzOtH29Eqv2YaU\">");
                      client.print("<h1> Humedad y temperatura en el local</h1><p>Temperatura: ");
                      client.print(t);     // Aqui va la temperatura
                      client.print(" grados Celsius</p>");
                      client.print("<p>Humedad:  ");
                      client.print(h);    // Aqui va la humedad
                      client.print(" por ciento</p>");
                      client.print("<p><em> La pagina se actualiza cada 20 segundos.</em></p></body></html>");
                      GestionarLeds(t,h);
                        
                      break;
                }
            if (c == '\n')
                currentLineIsBlank = true;          // nueva linea
            else if (c != '\r')
                currentLineIsBlank = false;
          }
        }
        }else{
          
             float h = dht.readHumidity();           // Leer el sensor
             float t = dht.readTemperature();
             GestionarLeds(t,h);
         }
     delay(10);         // Para asegurarnos de que los datos se envia
     client.stop();     // Cerramos la conexion
     delay(20000);
 }  


//Procedimiento que recibe un Array de crctes y lo envía a Twitter usando la libreria Twitter.h 
void tweet(char msg[]){
  char textoTweet[144];
  sprintf(textoTweet,"%s",msg);
  if (twitter.post(textoTweet)){    
    int status = twitter.wait(&Serial);
    if (status == 200){
      Serial.println(F("Tweet enviado"));
    }else{
      Serial.print(F("Fallo subida tweet:cdg:"));
      Serial.println(status);
    }
  }else{
    Serial.println(F("Fallo conexión."));
  }
}

void GestionarLeds(float t,float h){
  if(t>MAXTEMP){
                         digitalWrite(TEMPVAL,LOW);
                         digitalWrite(TEMPAL,HIGH);
                         char temperatura[6];
                         dtostrf(t,5, 2, temperatura);
                         temperatura[5]='\0';                                                 
                         sprintf(buffer,"Alarma ambiental. Temperatura %s grados.- cod - %d",temperatura,millis());
                         Serial.println(buffer);
                         delay (1000);
                         tweet(buffer);
  }else{
     digitalWrite(TEMPVAL,HIGH); //PONEMOS HUMEDAD EN VALIDO
     digitalWrite(TEMPAL,LOW);  
   }
  
   if(h>MAXHUME){
                         digitalWrite(HUMVAL,LOW);
                         digitalWrite(HUMAL,HIGH);
                         char humedad[6];
                         dtostrf(h,5, 2, humedad);
                         humedad[5]='\0';                                                 
                         sprintf(buffer,"Alarma ambiental. Humedad %s por ciento.- cod - %d",humedad,millis());
                         Serial.println(buffer);
                         delay (1000);
                         tweet(buffer);
   }else{
      digitalWrite(HUMVAL,HIGH); //PONEMOS HUMEDAD EN VALIDO
      digitalWrite(HUMAL,LOW);  
   }
}
