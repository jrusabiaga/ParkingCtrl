//CSpell: ignore minu
/****************************************************************************
                        _       _       ____            _       _  ____  
        _   _ _ __   __| | __ _| |_ ___/ ___|  ___ _ __(_) __ _| |/ /\ \ 
       | | | | '_ \ / _` |/ _` | __/ _ \___ \ / _ \ '__| |/ _` | | |  | |
       | |_| | |_) | (_| | (_| | ||  __/___) |  __/ |  | | (_| | | |  | |
        \__,_| .__/ \__,_|\__,_|\__\___|____/ \___|_|  |_|\__,_|_| |  | |
             |_|                                                  \_\/_/ 
 ****************************************************************************/
void updateSerial(unsigned long howLongToWait, bool imprime){
  delay(howLongToWait);
  
  while ((Serial2.available()))  {
    char character = Serial2.read();
      if(imprime){
        //Serial.write(character);// Send all data to Serial Port.
        Serial.print(character);
        //Serial.print("~");
      }
      ristra.concat(character);

      //  Esto cuando se recibe un SMS, no creo que sea necesario
      pos=ristra.indexOf("+CMT");   // SMS Received
      if (pos >= 0) {
        flagSMS=1;                  // Flag Activated
      }
    //}
  }
} // end updateSerial

/****************************************************************************
                        _        ____  
         ___ _ ____   _(_) __ _ / /\ \ 
        / _ \ '_ \ \ / / |/ _` | |  | |
       |  __/ | | \ V /| | (_| | |  | |
        \___|_| |_|\_/ |_|\__,_| |  | |
                                \_\/_/ 
 ****************************************************************************/
bool envia(String at, String &retorno, String okResp, String errorStr, uint32_t timeOut, bool imprime){
        
  if(timeOut == 0) timeOut = 3600000;
  String resp = "";
  char letraIn;
  
  //  Envía el comando AT
  Serial2.println(at);
  delay(5);
  uint32_t lap =  millis();                     //  Inicia un contador     
  while(millis() < lap + timeOut){              
    if (Serial2.available()) {                  //  Respuesta del Modem
      char letra = Serial2.read();              //  Lo pilla letra a letra
      if(armaFrase(letra, resp)){               //  Ensambla la frase
        if(resp.indexOf(retorno) >= 0){
          retorno = resp;                       //  Retorna la frase solicitada
        }else if(resp.indexOf(errorStr) >= 0){
          retorno = resp;                       //  Retorna el ERROR
        }else{
          if(!(resp.indexOf("OK") >= 0)){
            resp.trim();
            if(resp.length() > 0){
              retorno = resp;                   //  Retorna el ERROR
            }
          }
        }
        if(imprime){                            //  Si quiere salida a pantalla
          Serial.print(resp);
          if(SHOW_TIMES){
            Serial.print("\t\t");
            Serial.print(millis() - lap);
          }
          Serial.println("");
        }
        if((resp == okResp)) return true;
      }
    }
    //delay(10);
  }
}

/****************************************************************************
                  _ ____       _              ____  
        _ __ ___ (_)  _ \  ___| | __ _ _   _ / /\ \ 
       | '_ ` _ \| | | | |/ _ \ |/ _` | | | | |  | |
       | | | | | | | |_| |  __/ | (_| | |_| | |  | |
       |_| |_| |_|_|____/ \___|_|\__,_|\__, | |  | |
                                       |___/ \_\/_/ 
 ****************************************************************************/
//  Demora segundos enteros
void miDelay(uint8_t segs){
  //Serial.print("Espera ");
  //Serial.print(segs);
  //Serial.println(" segs.");
  for(int i = 0; i < segs; i++){
    uint32_t lap = millis() + 1000;
    while(1){
      if(lap < millis()) break;
    }
  }
}

/*********************************************************************************
                 _   ____ _____ ____  ____  
        ___  ___| |_|  _ \_   _/ ___|/ /\ \ 
       / __|/ _ \ __| |_) || || |   | |  | |
       \__ \  __/ |_|  _ < | || |___| |  | |
       |___/\___|\__|_| \_\|_| \____| |  | |
                                     \_\/_/ 
 ********************************************************************************/
//  Inicializa el circuito RTC del ESP32 
void setRTC(String &lectura){
  //  Toma la hora reportada por el GSM modem y la introduce en el RTC del ESP32    
  int ano = (lectura.substring(8, 10).toInt() + 2000) ;
  int mes = lectura.substring(11, 13).toInt();
  int dia = lectura.substring(14, 16).toInt();
  int hora = lectura.substring(17, 19).toInt();
  int minu = lectura.substring(20, 22).toInt();
  int seg = lectura.substring(23, 25).toInt();
  Serial.print(dia);
  Serial.print("/");
  Serial.print(mes);
  Serial.print("/");
  Serial.println(ano);
  rtc.setTime(seg, minu, hora, dia, mes, ano);
  //rtc.offset = 0; // change offset value
}

/*********************************************************************************
                                 _     _____ ____   ____  
         ___ _ __ _ __ ___  _ __| |   | ____|  _ \ / /\ \ 
        / _ \ '__| '__/ _ \| '__| |   |  _| | | | | |  | |
       |  __/ |  | | | (_) | |  | |___| |___| |_| | |  | |
        \___|_|  |_|  \___/|_|  |_____|_____|____/| |  | |
                                                   \_\/_/ 
*********************************************************************************/
//  Enciende el LED sobre el MCU un numero de veces para señalizar algun error
void errorLED(uint8_t veces){
  uint32_t tiemON = 300;
  uint32_t tiemOFF = 150;
  uint32_t tiemRepite = 800;
  for(int i = 0; i < veces; i++){
    digitalWrite(LED_BUILTIN, HIGH);      //  Enciende el LED
    delay(tiemON);
    digitalWrite(LED_BUILTIN, LOW);       //  Apaga el LED
    delay(tiemOFF);
  }
  delay(tiemRepite);
}