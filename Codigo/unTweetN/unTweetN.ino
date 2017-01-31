

/* Control de Ip's en red local:
 *Programa que controla si las ip's presentes en un LAN se encuentran registradas en una tabla de control.
 *De no estarlo se genera un mensaje de alerta que se envía a la red social Twitter, con expresión de la IP
 *no registrada
 */
    #include <SD.h>
    #include <SPI.h>
    #include <Ethernet.h>    
    #include <Twitter.h>
    #include <ICMPPing.h>
                                                       
   #define FBYTE 192
   #define SBYTE 168
   #define TBYTE 1
   #define FRBYTE 144
   
   #define NUMPAQUETES 4
   
   byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };                    //mac para la placa Arduino
   byte ipConection[]  = {FBYTE,SBYTE,TBYTE,FRBYTE};                       //IP para la placa Arduino
   IPAddress myDns(8,8,8,8);                                               //Servidor DNS, establecemos la dirección de DNS's de google.com     
   Twitter twitter("<<Tu token aquí>>");				   //Token del que nos provee Twitter para identificarnos como usuarios
   SOCKET pingSocket = 0;                                                  //socken para establecer comunicación en la LAN
   char buffer [64];                                                       //un array de caracteres para envio de mensajes a la consola Serie
   ICMPPing ping(pingSocket, (uint16_t)random(0, 255));                    //objeto de la clase ICMPPing para hacer pings en la red local
   EthernetClient client;
   int chipSelect=4;                                                       //este puerto será utilizado para la conexión con la tarjeta SD
   int ipsLeidas[5][4];                                                    //un array de enteros de 5x4 que almacenará un máximo de cinco direcciones ip autorizadas en la lan
   int numIps=0;                                                           //número de ips leidas del fichero de ips autorizadas


   void setup(){
      Ethernet.begin(mac,ipConection,myDns); 
      delay(100);
      Serial.begin(9600);
      delay(1000);
      Serial.print(F("Conectado con IP: "));
      for (byte i = 0; i < 4; i++){
        Serial.print(ipConection[i], DEC);
        //Serial.print(ipConection[i]);
        Serial.print(F(".")); 
      }
    Serial.println();
     // Si ha habido error al leer la tarjeta informamos por el puerto serie.
    if (!SD.begin(chipSelect)){
      Serial.println(F("Error al leer la tarjeta."));
    }
  // Abrimos el archivo.
	File dataFile = SD.open("ipsAuth3.txt"); 
	if (dataFile) {                 //abro if
		//Mientras el fichero tenga contenido
		while (dataFile.available()) {      //abro while
			for(byte i=0;i<4;i++){                //abro for
				char crct;           
				byte index=0; 
				char valor[4];        
				while((crct=dataFile.read())!='.'){   //abro while
					valor[index]=crct;
					valor[index+1]='\0';
					index++; 
				}                                    //cierro while
				ipsLeidas[numIps][i]=atoi(valor);            
			}                                    //cierro for
			Serial.print(F("ip leida del fichero: "));
			Serial.print(ipsLeidas[numIps][0]);
			Serial.print(F("."));
			Serial.print(ipsLeidas[numIps][1]);
			Serial.print(F("."));
			Serial.print(ipsLeidas[numIps][2]);
			Serial.print(F("."));
			Serial.println(ipsLeidas[numIps][3]);   
			numIps++;
		}	//cierro while 
		dataFile.close();	//cierro el fichero
        }else {
		Serial.println(F("Error al abrir .txt"));    
	} //cierro if

        
}
 
void loop()
{  
  for(int i=1;i<256;i++){ //for
      IPAddress pingAddr(FBYTE,SBYTE,TBYTE,i);
      ICMPEchoReply echoReply = ping(pingAddr, NUMPAQUETES);
      if (echoReply.status == SUCCESS){   //if ping resulto  
            sprintf(buffer,"Reply[%d] from:%d.%d.%d.%d:bytes=%d time=%ldms TTL=%d",
            echoReply.data.seq,
            echoReply.addr[0],
            echoReply.addr[1],
            echoReply.addr[2],
            echoReply.addr[3],
            REQ_DATASIZE,
            millis() - echoReply.data.time,
            echoReply.ttl);
           
            if(!(comprobarIP(echoReply.addr))){    // si la ipE está en el fichero de ips           
               Serial.println(F("Ip NO autorizada"));   
               sprintf(buffer,"Ip no autorizada en:%d.%d.%d.%d, cd.-%d", FBYTE,SBYTE,TBYTE,i,millis()); 
               tweet(buffer);
            }else{
                    Serial.println(F("IP AUTORIZADA"));
            } //cierre if  si la ipE está en el fichero de ips
        
      }else{
        sprintf(buffer, "Echo request failed;%d.%d.%d.%d",FBYTE,SBYTE,TBYTE,i);
      }// cierre if ping resuelto
      //imprimo el buffer
      Serial.println(buffer);      
  } //cierre for
 
  
}

  boolean comprobarIP(IPAddress a){
       
        Serial.print(F("\n\nPing atendido en ip: "));
        //Serial.print(echoReply[0]);
        Serial.print(a[0]);
        Serial.print(F("."));
        //Serial.print(echoReply[1]);
        Serial.print(a[1]);
        Serial.print(F("."));
        //Serial.print(echoReply[2]);
        Serial.print(a[2]);
        Serial.print(F("."));
        //Serial.print(echoReply[3]);
        Serial.print(a[3]);
        Serial.println();
	for(byte i=0;i<numIps;i++){
		boolean chivato=true;
		for(byte j=0;j<4;j++){
			if(!((a[j])==ipsLeidas[i][j])){
				chivato=false;
			}
		}
		if(chivato){
			return chivato;
		}
	}
	return false;
 }

//Función que recibe un Array de crctes y lo envía a Twitter usando la libreria Twitter.h 
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
