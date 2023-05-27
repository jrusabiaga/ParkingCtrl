//CSpell:ignore envia CREG atz ati cclk ctzu cgps csq ccid iccid ciccid
//CSpell:ignore cscs cmgf cnmi cgpsinfo volAlimentacion UTRAN CEREG CGEV
//CSpell:ignore pdn creset condicion conexion parametro
/****************************************************************************
       ____             _        ____ ____  __  __  ____  
      | __ )  ___  __ _(_)_ __  / ___/ ___||  \/  |/ /\ \ 
      |  _ \ / _ \/ _` | | '_ \| |  _\___ \| |\/| | |  | |
      | |_) |  __/ (_| | | | | | |_| |___) | |  | | |  | |
      |____/ \___|\__, |_|_| |_|\____|____/|_|  |_| |  | |
                  |___/                            \_\/_/ 
 ****************************************************************************/
void beginGSM(){
  String resp = "";           //  Respuesta recibida del GPRS
  bool arranco = false;       //  Cierto una vez terminado el arranque
  bool enFrio = false;        //  A7076E cierto si recién enciende o falso si ya estaba encendido
  bool simNotFound = false;   //  No se encuentra tarjeta SIM
  bool once = true;           //  Primera vez
  bool conexion = false;      //  Si existe conexión con la red de telefonía

  //  Se asume que la velocidad por defecto es 115200 bps
  //  Cuando el módulo A7670E termina el arranque lo notifica con el message
  //  "PB DONE" así que: se espera eso
  digitalWrite(MOSFET, HIGH);
  //Serial.println("\nComunicación con el módulo A7670E...");
  //Serial.println("\nAbriendo un canal con el módulo A7670E...");
  Serial.println("\nIntentando comunicar con el módulo A7670E...");
  //miDelay(20);
  uint32_t lap = millis();

  //  Espera "PB DONE" en un máximo de 20 segundos
  while(millis() < lap + 30000){
    while(Serial2.available()) {                //  Respuesta del Modem
      char letra = Serial2.read();              //  Lo pilla letra a letra
      if(armaFrase(letra, resp)){               //  Ensambla la frase
        Serial.print(resp);
        Serial.println("");
        if(resp.indexOf("PB DONE") >= 0){
          arranco = true;
          break;
        }else if(resp.indexOf("READY") >= 0){
          enFrio = true;
          break;
        }else if(resp.indexOf("SIM REMOVED") >= 0){
          arranco = true;
          simNotFound = true;
          break;
        }
      }
    }
    if(simNotFound) break;
    //  En 3 segundos probamos a ver si el A7670E esta encendido, se envía
    //  "AT" y se espera que responda "OK" caso contrario significa que aun
    //  esta arrancando. Esto no se repite.
    if(millis() > lap + 12000 && once && !enFrio){
      Serial.println("Probando si el módulo ya estaba encendido");
      Serial2.println("AT");
      once = false;
      while(Serial2.available()) {            //  Respuesta del Modem
        char letra = Serial2.read();          //  Lectura letra a letra
        if(armaFrase(letra, resp)){           //  Ensambla una frase
          Serial.print(resp);
          Serial.println("");
          if(resp.indexOf("OK") >= 0){
            arranco = true;               //  Se asume que esta encendido
            break;
          }
        }
      }
    }
    if(arranco) break;
  }
  if(!arranco){       //  El módulo no respondió en el tiempo dado
    Serial.println("\n");
    Serial.println("     **** ERROR FATAL ****");
    Serial.println("No se encontró el Módulo GSM-GPRS");
    Serial.println("Ejecución terminada");
    for(;;);
  }
  if(simNotFound){
    Serial.println("\n");
    Serial.println("     **** ERROR FATAL ****");
    Serial.println("No se encontró la tarjeta SIM");
    Serial.println("Ejecución terminada");
    for(;;);
  }
  //Serial.println("\nMódulo A7670E encontrado...");
  Serial.println("\nComunicación establecida con el módulo A7670E");
  Serial.println("");
  //Serial.println("Esperando señal...");
  Serial.println("Esperando señal de la red de telefonía móvil...");

  //  El módulo ya esta encendido. Ahora se espera hasta que se conecte con
  //  la red y este en línea, para poder tener comunicaciones.
  //  Para esto se espera hasta 2 minutos
  updateSerial(500, false);
  lap = millis();
  while(millis() < lap + 120000){         //  2 minutos
    lectura = "+CREG:";
    bool basura = envia("AT+CREG?", lectura, "OK", "ERROR", 9000, false);
    uint8_t parametro = lectura.substring(9,10).toInt();
    //Serial.print(lectura);
    //Serial.print("\t parámetro = "); Serial.println(parámetro);
    //Serial.println(millis());
    if(parametro == 1 || parametro == 5){ //  1 esta en línea, 5 esta en roaming
      conexion = true;
      break;
    }
    miDelay(5);                           //  Se verifica el estado cada 5 segundos
  }
  if(!conexion){
    Serial.println("\n");
    Serial.println("     **** ERROR FATAL ****");
    Serial.println("No hay conexion con la red de telefonía");
    Serial.println("Se reiniciará en 10 segundos");
    miDelay(10);
    ESP.restart();
    for(;;);
  }

  Serial.println("");
  Serial.println("Módulo A7670E conectado a la red\n");

}

/****************************************************************************
         _       _ _    ____ ____  __  __  ____  
        (_)_ __ (_) |_ / ___/ ___||  \/  |/ /\ \ 
        | | '_ \| | __| |  _\___ \| |\/| | |  | |
        | | | | | | |_| |_| |___) | |  | | |  | |
        |_|_| |_|_|\__|\____|____/|_|  |_| |  | |
                                          \_\/_/ 
 ****************************************************************************/
void initGSM(){
  
  Serial.println("\n\nConfiguración inicial...");
  for(int i = 0; i < 70; i++) Serial.print("-"); Serial.println("");

  if(SHOW) Serial.println("\nRestaura configuración guardada del A7670E");
  lectura = "";
  bool basura = envia("ATZ0", lectura, "OK", "ERROR", 9000, SHOW);

  basura = envia("ATI", lectura, "OK", "ERROR", 9000, SHOW);

  if(SHOW) Serial.println("\nImprimir fecha y hora desde la memoria");
  basura = envia("AT+CCLK?", lectura, "OK", "ERROR", 9000, SHOW);

  if(SHOW) Serial.println("\nActivar actualización automática de hora y fecha");
  basura = envia("AT+CTZU=1", lectura, "OK", "ERROR", 9000, SHOW);

  if(SHOW) Serial.println("\nIdentificar la Operadora de telefonía");
  getOperator(SHOW);

  if(SHOW) Serial.println("\nIntensidad de la señal");
  basura = envia("AT+CSQ", lectura, "OK", "ERROR", 9000, SHOW);

  if(SHOW) Serial.println("\nNumero de serie de la SIM");
  lectura = "+ICCID:";
  basura = envia("AT+CICCID", lectura, "OK", "ERROR", 9000, SHOW);

  if(SHOW) Serial.println("\nNotificación de cambio en el estado de la conexión");
  basura = envia("AT+CREG=2", lectura, "OK", "ERROR", 9000, SHOW);

  if(SHOW) Serial.println("\nConfigurar GSM set de caracteres");
  miDelay(1);
  basura = envia("AT+CSCS=\"IRA\"", lectura, "OK", "ERROR", 200, SHOW);

  if(SHOW) Serial.println("\nConfigurar el formato de los SMSs a formato de texto");
  basura = envia("AT+CMGF=1", lectura, "OK", "ERROR", 350, SHOW);

  if(SHOW) Serial.println("\nConfigura SMSs no se guarden en memoria y");
  if(SHOW) Serial.println("reporte cuando lleguen");
  miDelay(1);
  if(SHOW) basura = envia("AT+CNMI=1", lectura, "OK", "ERROR", 300, SHOW);

  if(SHOW) Serial.println("\nActualizo RTC del ESP32");
  miDelay(1);
  lectura = "+CCLK:";
  basura = envia("AT+CCLK?", lectura, "OK", "ERROR", 200, SHOW);
  setRTC(lectura);  //  Set RTC del ESP32
  
  if(SHOW) Serial.println("\nGuardo configuración del módulo A7670E");
  lectura = "";
  basura = envia("AT&W0", lectura, "OK", "ERROR", 9000, SHOW);

}

/****************************************************************************
                  _               _                    ____  
         ___ ___ | |__   ___ _ __| |_ _   _ _ __ __ _ / /\ \ 
        / __/ _ \| '_ \ / _ \ '__| __| | | | '__/ _` | |  | |
       | (_| (_) | |_) |  __/ |  | |_| |_| | | | (_| | |  | |
        \___\___/|_.__/ \___|_|   \__|\__,_|_|  \__,_| |  | |
                                                      \_\/_/ 
 ****************************************************************************/
uint8_t cobertura(){
  //  Verificación de la clase de cobertura a la red, primero tiene
  //  que estar configurado el "network registration status" con el
  //  comando "AT+CREG=1": (Errata no hace falta)
  //      enable network registration unsolicited result code
  //      +CREG: <stat>.
  //
  //  Se interpreta la respuesta del comando AT+CREG? que su respuesta
  //  tiene el siguiente formato:
  //      +CREG: <n>,<stat>[,<lac>,<ci>]
  //  El valor de <stat> representa la conexión con la red, con los siguientes
  //  valores:
  //      0   not registered, ME is not currently searching a new operator
  //          to register to.
  //      1   registered, home network.
  //      2   not registered, but ME is currently searching a new operator
  //          to register to.
  //      3   registration denied. 
  //      4   unknown.
  //      5   registered, roaming.
  //      6   registered for "SMS only", home network (applicable only when
  //          E-UTRAN)  Serial.println("\n\nCobertura...");
  //
  //Serial.println("Cobertura...");
  uint8_t parametro = 10;
  uint8_t delim1 = 0;
  uint8_t delim2 = 0;
  //  Se ejecuta el comando y se interpreta la respuesta del A7670E
  //  Según el manual este comando puede tardar en responder hasta
  //  9 segundos.
  lectura = "+CREG:";
  bool basura = envia("AT+CREG?", lectura, "OK", "ERROR", 9000, false);
  //  Se toma el valor de <stat>
  parametro = lectura.substring(9,10).toInt();
  
  //  Debug del parse y ejemplos de respuesta
  //Serial.println(lectura);
  //Serial.print("=>");
  //Serial.print(parametro);
  //Serial.println("<=\n\n");
  //  0 = not registered, ME is not currently searching a new operator
  //  to register to.
  
  //+CEREG: 2,1,0957,0A89DD70
  //+CEREG: 2,11,0000,00000000
  //+CEREG: 1,0957,0A89DD70,7
  //+CGEV: EPS PDN ACT 1
  
  
  return parametro;
}
 
/****************************************************************************
                           _   __  __           _       _       ____  
        _ __ ___  ___  ___| |_|  \/  | ___   __| |_   _| | ___ / /\ \ 
       | '__/ _ \/ __|/ _ \ __| |\/| |/ _ \ / _` | | | | |/ _ \ |  | |
       | | |  __/\__ \  __/ |_| |  | | (_) | (_| | |_| | |  __/ |  | |
       |_|  \___||___/\___|\__|_|  |_|\___/ \__,_|\__,_|_|\___| |  | |
                                                              \_\/_/ 
 ****************************************************************************/
void resetModule(){ 
  uint32_t lap =  millis();
  Serial2.println("AT+CRESET");
  Serial.println("Paro");
  miDelay(10);
  Serial.println("Paso");
  //afterReset(lap);
}

/****************************************************************************
               __ _            ____                _    ____  
         __ _ / _| |_ ___ _ __|  _ \ ___  ___  ___| |_ / /\ \ 
        / _` | |_| __/ _ \ '__| |_) / _ \/ __|/ _ \ __| |  | |
       | (_| |  _| ||  __/ |  |  _ <  __/\__ \  __/ |_| |  | |
        \__,_|_|  \__\___|_|  |_| \_\___||___/\___|\__| |  | |
                                                       \_\/_/ 
 ****************************************************************************/
bool afterReset(uint32_t lap){
  bool condicion1 = false;      //  Cuando el GSM esta activo
  bool condicion2 = false;      //  Cuando el GSM esta conectado a la red
  String resp = "";
  while(millis() < lap + 600000){
    //  Si esta activo y conectado sale
    //if(condicion1 && condicion2){
    if(condicion1){
      //initGSM();
      break;
    }
    if (Serial2.available()) {
      char letra = Serial2.read();
      if(armaFrase(letra, resp)){
        Serial.println(resp);
        //  Ultima respuesta de los módulos SIM7600X y A7670E
        //  al arrancar
        if(resp == "PB DONE"){
          Serial.println("Condicion cumplida");
          condicion1 = true;
        }
        //  Pero no esta conectado a la red hasta que salga
        //  +CGEV: EPS PDN ACT 1
        if(resp.indexOf("+CGEV: EPS") > 0){
          Serial.print("Segunda condicion = ");
          Serial.println(resp.indexOf("+CGEV: EPS"));
          condicion2 = true;
        }
      }
    }
  }
  //return (condicion1 && condicion2);
  return (condicion1);
}

/*********************************************************************************
                _    ___                       _              ____  
      __ _  ___| |_ / _ \ _ __   ___ _ __ __ _| |_ ___  _ __ / /\ \ 
     / _` |/ _ \ __| | | | '_ \ / _ \ '__/ _` | __/ _ \| '__| |  | |
    | (_| |  __/ |_| |_| | |_) |  __/ | | (_| | || (_) | |  | |  | |
     \__, |\___|\__|\___/| .__/ \___|_|  \__,_|\__\___/|_|  | |  | |
     |___/               |_|                                 \_\/_/ 
 ********************************************************************************/
 // Función que devuelve el nombre del operado de telefonía móvil
void getOperator(bool verbal){
  lectura = "";
  bool basura = envia("AT+COPS=3,1", lectura, "OK", "ERROR", 60000, verbal);
  lectura = "COPS?";
  basura = envia("AT+COPS?", lectura, "OK", "ERROR", 60000, verbal);
  uint8_t praComillas = lectura.indexOf('"');
  uint8_t sdaComillas = lectura.indexOf('"', praComillas+1);
  operador = lectura.substring(praComillas + 1, sdaComillas);
  int tipo = lectura.substring(sdaComillas + 2, sdaComillas + 3).toInt();
  if(tipo == 7) tipoRed = "4G";
  if((tipo > 3 && tipo < 7) || tipo == 2 || tipo == 8) tipoRed = "3G";
  if(tipo < 2 || tipo == 3) tipoRed = "2G";
  if(verbal){
    Serial.print("Operador = ");
    Serial.print(operador);
    Serial.print("\tRed = ");
    Serial.println(tipoRed);
  }
}

