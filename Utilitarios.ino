//CSpell: ignore minu CMGS
/****************************************************************************
                        _       _       ____            _       _ ____   ____  
        _   _ _ __   __| | __ _| |_ ___/ ___|  ___ _ __(_) __ _| |___ \ / /\ \ 
       | | | | '_ \ / _` |/ _` | __/ _ \___ \ / _ \ '__| |/ _` | | __) | |  | |
       | |_| | |_) | (_| | (_| | ||  __/___) |  __/ |  | | (_| | |/ __/| |  | |
        \__,_| .__/ \__,_|\__,_|\__\___|____/ \___|_|  |_|\__,_|_|_____| |  | |
             |_|                                                        \_\/_/ 
 ****************************************************************************/
//void updateSerial2(unsigned long howLongToWait, bool imprime){
bool updateSerial2(unsigned long howLongToWait, bool imprime){
  uint32_t tiempo = millis() + howLongToWait;
  String termina = "OK";
  String resp = "+CMGS:";
  String respErr = "+CMS ERROR:";
  bool recResp = false;       //  Se hace cierto cuando se recibe la "resp"
  bool recTermina = false;    //  Se hace cierto cuando se recibe el "termina"
  bool recRespErr = false;    //  Se hace cierto cuando se recibe el "respErr"
  int pos = 0;                //  Posición donde se encuentra el texto

  
  if(imprime) Serial.print("updateSerial2 =>");
  while(tiempo > millis() && (!recResp && !recTermina)){
    while ((Serial2.available()))  {
      char character = Serial2.read();
      if(imprime){
        Serial.print(character);
      }
      ristra.concat(character);

      //  Espera las respuestas correctas
      if(!recResp){
        pos = ristra.indexOf(resp);
        if (pos >= 0) {
          if(imprime){
            Serial.print(":");
            Serial.print("\n-·" + ristra + "·- resp pos: ");
            Serial.println(pos);
          }
          recResp = true;
          pos = 0;
        }
      }
      if(!recTermina){
        pos = ristra.indexOf(termina);
        if (pos >= 0) {
          if(imprime){
            Serial.print("·");
            Serial.print("\n-·" + ristra + "·- ok pos: ");
            Serial.println(pos);
          }
          recTermina = true;
          pos = 0;
        }
      }
      //  En caso de "+CMS ERROR: Network timeout" el mensaje no salio
      if(!recRespErr){
        pos = ristra.indexOf(respErr);
        if (pos >= 0) {
          if(imprime) Serial.print("X");
          recRespErr = true;
          recResp = false;
          recTermina = false;
          pos = 0;
        }
      }
    }
  }
  if(imprime) Serial.print("<= updateSerial2\n");
  if(imprime && (!recResp || !recTermina)) Serial.print("Timeout...\n");
  return (recResp && recTermina);
} // end updateSerial2

/****************************************************************************
                        _       _       ____            _       _ _____  ____  
        _   _ _ __   __| | __ _| |_ ___/ ___|  ___ _ __(_) __ _| |___ / / /\ \ 
       | | | | '_ \ / _` |/ _` | __/ _ \___ \ / _ \ '__| |/ _` | | |_ \| |  | |
       | |_| | |_) | (_| | (_| | ||  __/___) |  __/ |  | | (_| | |___) | |  | |
        \__,_| .__/ \__,_|\__,_|\__\___|____/ \___|_|  |_|\__,_|_|____/| |  | |
             |_|                                                        \_\/_/ 
 ****************************************************************************/
bool updateSerial3(unsigned long howLongToWait, String esperaPor, bool imprime){
  uint32_t tiempo = millis() + howLongToWait;
  bool respCorrecta = false;
  int pos = 0;                //  Posición donde se encuentra el texto
  
  if(imprime) (void)Serial.print("->>>");
  while(tiempo > millis() && !respCorrecta){
    while ((Serial2.available()))  {
      char character = Serial2.read();
      if(imprime){
        Serial.print(character);
      }
      ristra.concat(character);

      //  Espera las respuestas correctas
      if(!respCorrecta){
        pos = ristra.indexOf(esperaPor);
        if (pos >= 0) {
          if(imprime){
            Serial.print(":");
            Serial.print("\n-·" + ristra + "·- resp pos: ");
            Serial.println(pos);
          }
          respCorrecta = true;
          pos = 0;
        }
      }
    }
  }
  if(imprime){
    Serial.print("<<<- ");
    Serial.println(respCorrecta ? "true": "false");
  }
  return respCorrecta ? true: false;
} // end updateSerial3

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
  
  if(imprime) Serial.print("->>");
  while ((Serial2.available()))  {
    char character = Serial2.read();
      if(imprime){
        Serial.print(character);
      }
      ristra.concat(character);
  }
  if(imprime) Serial.print("<<-\n");
} // end updateSerial

/****************************************************************************
                        _        ____  
         ___ _ ____   _(_) __ _ / /\ \ 
        / _ \ '_ \ \ / / |/ _` | |  | |
       |  __/ | | \ V /| | (_| | |  | |
        \___|_| |_|\_/ |_|\__,_| |  | |
                                \_\/_/ 
 ****************************************************************************/
//  Envia comando AT y espera por respuesta del módulo
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
//  Enciende el LED sobre el MCU un número de veces para señalizar algún error
void errorLED(uint8_t veces){
  uint32_t timeON = 300;
  uint32_t timeOFF = 150;
  uint32_t timeRepite = 800;
  for(int i = 0; i < veces; i++){
    digitalWrite(LED_BUILTIN, HIGH);      //  Enciende el LED
    delay(timeON);
    digitalWrite(LED_BUILTIN, LOW);       //  Apaga el LED
    delay(timeOFF);
  }
  delay(timeRepite);
}