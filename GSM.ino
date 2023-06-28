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
  String resp = "";           //  Respuesta recibida del A7670E
  bool arranco = false;       //  Cierto una vez terminado el arranque
  bool enFrio = false;        //  A7076E cierto si recién enciende o falso si ya estaba encendido
  bool simNotFound = false;   //  No se encuentra tarjeta SIM
  bool once = true;           //  Primera vez
  bool conexion = false;      //  Si existe conexión con la red de telefonía

  uint8_t arranques = 0;

  //  Se asume que la velocidad por defecto es 115200 bps
  //  Canudo el módulo A7670E termina el arranque lo notifica con el message
  //  "PB DONE" así que: se espera eso
  Serial.println("Módulo A7670E activado");
  digitalWrite(MOSFET, HIGH);
  Serial.println("\nIntentando comunicar con el módulo A7670E...");
  //miDelay(20);
  uint32_t lap = millis();

  //  Esperamos la llegada de la cadena "PB DONE" por un máximo de 2,5 minutos
  while(millis() < lap + 150000){
    while(Serial2.available()) {                //  Respuesta del Modem
      char letra = Serial2.read();              //  Lo pilla letra a letra
      if(armaFrase(letra, resp)){               //  Ensambla la frase
        Serial.print(resp);
        Serial.println("");
        if(resp.indexOf("PB DONE") >= 0){
          arranco = true;
          /*
          Serial.print("Final PB DONE --- arranques = ");
          Serial.print(arranques);
          Serial.print(" en: ");
          Serial.println((millis() - lap) / 1000.00);
          Serial.println("Quieto paro");
          for(;;);
          */
          break;
        }else if(resp.indexOf("*ATREADY:") >= 0){
          //  El módulo responde con "*ATREADY:" solo cuando arranca
          arranques ++;
          enFrio = true;
          //Serial.println("Arranque en frio");
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
      while(Serial2.available()) {   //  Respuesta del Modem
        char letra = Serial2.read(); //  Lectura letra a letra
        if(armaFrase(letra, resp)){  //  Ensambla una frase
          Serial.print(resp);
          Serial.println("");
          if(resp.indexOf("OK") >= 0){
            arranco = true;          //  Se asume que esta encendido
            break;
          }
        }
      }
    }
    if(arranco) break;
  }
  if(!arranco){                      //  El módulo no respondió en el tiempo dado
    Serial.println("\n");
    Serial.println("     **** ERROR FATAL ****");
    Serial.println("No se encontró el Módulo A7670E");
    Serial.println("Ejecución terminada");
    Serial.print("No respondio con \"PB DONE\" --- arranques = ");
    Serial.print(arranques);
    Serial.print(" en: ");
    Serial.print((millis() - lap) / 1000.00);
    Serial.println(" segundos");
    digitalWrite(MOSFET, LOW);
    for(;;){
      errorLED(1);
    }
  }
  if(simNotFound){                   //  El módulo reporta que no hay SIM
    Serial.println("\n");
    Serial.println("     **** ERROR FATAL ****");
    Serial.println("No se encontró la tarjeta SIM");
    Serial.println("Ejecución terminada");
    digitalWrite(MOSFET, LOW);
    for(;;){
      errorLED(2);
    }
  }
  Serial.println("\nComunicación establecida con el módulo A7670E");
  Serial.println("");
  Serial.println("2 min. de espera por la señal de la red de telefonía móvil...");

  //  El módulo ya esta encendido. Ahora se espera hasta que se conecte con
  //  la red y este en línea, para poder tener comunicaciones.
  //  Para esto se espera hasta 2 minutos
  updateSerial(500, false);
  lap = millis();
  while(millis() < lap + 120000){         //  2 minutos
    lectura = "+CREG:";
    bool basura = envia("AT+CREG?", lectura, "OK", "ERROR", 9000, false);
    uint8_t parametro = lectura.substring(9,10).toInt();
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
    Serial.println("Se reiniciará en 20 segundos");

    //  Apagado del módulo A7670E y cuenta atrás de 20 seg
    digitalWrite(MOSFET, LOW);
    lapso = millis() + 20000;
    while(millis() < lapso){
      errorLED(3);
    }
    ESP.restart();      //  Reinicio del MCU
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
//  Configuración del módulo A7670E
void initGSM(){
  
  //JAVI: primero pinto una frase y luego una raya con 71 caracteres "-", luego una linea en blanco
  Serial.println("\n\nConfiguración inicial...");
  for(int i = 0; i < 70; i++) Serial.print("-"); Serial.println("");

  if(SHOW) Serial.println("\nRestaura configuración guardada del A7670E");
  lectura = "";

  /*JAVI:
    La funcion envia() está en Utilitarios.ino. Tiene varios parámetros: 
      - Comando que enviamos (String)
      - String que nos devuelve el cacharro (pasado por referencia, para que vuelva)
      - Respuesta "buena" que esperamos del cacharro según diga el manual
          - P.ej. para el comando ATZ0, es "OK" 
      - Respuesta "mala" que esperamos del cacharro según diga el manual
          - P.ej. para el comando ATZ0, es "ERROR"
      - Timeout en milisegundos: si enviamos un 0 se asumirán 6 minutos
            (El módulo GPRS tarda como 3-4 minutos en arrancar)
      - Booleano para decidir si queremos ver la salida por pantalla
            (lo tenemos como cosntante (a TRUE) en ParkingCtrl.ino)
    Devuelve un booleano que solo será TRUE si la respuesta es la "buena" esperada, con la instrucción: 
       if((resp == okResp)) return true;
  */

  //El comando ATZ0 restaura la configuración de usuario en el módulo GPRS
    //Esta configuración es la que tiene guardada el comando AT&W
  bool basura = envia("ATZ0", lectura, "OK", "ERROR", 9000, SHOW);

  //El comando ATI pregunta al módulo GPRS por su información de prodcuto en este caso: 
    //Manufacturer: INCORPORATED
    //Model: A7670E-LASE
    //Revision: A7670M7_V1.11.1
    //IMEI: 862205057937775
    //+GCAP: +CGSM,+FCLASS,+DS
  basura = envia("ATI", lectura, "OK", "ERROR", 9000, SHOW);

  //El comando AT+CCLK gestiona la fecha/hora del módulo: 
      //Si enviamos AT+CCLK=? estamos haciendo el test del comando
      //Si enviamos AT+CCLK? estamos leyendo la fecha/hora del sistema
      //Si enviamos AT+CCLK=<time> estamos estableciendo la fecha/hora del sistema
  if(SHOW) Serial.println("\nImprimir fecha y hora desde la memoria");
  basura = envia("AT+CCLK?", lectura, "OK", "ERROR", 9000, SHOW);

  //El comando AT+CTZU gestiona la activación automática de la zona horaria vía NITZ (Network Identity and Time Zone)
    //Si enviamos AT+CTZU? estamos en modo lectura: nos dice si la tiene activada o no
    //Si enviamos AT+CTZU=<on/off> estamos en modo escritura: 
        //Si le ponemos un "1" lo activamos (según el manual)
        //El valor de fábrica es "0" (según el manual)
  if(SHOW) Serial.println("\nActivar actualización automática de hora y fecha");
  basura = envia("AT+CTZU=1", lectura, "OK", "ERROR", 9000, SHOW);

  //La función getOperator() está implementada aquí mismo en GSM.ino
  if(SHOW) Serial.println("\nIdentificar la Operadora de telefonía");
  getOperator(SHOW);

  //El comando AT+CSQ devuelve la intensidad de la señal captada y la tasa de errores
      //En nuestro caso nos devuelve normalmente: +CSQ: 16,99. Esto quiere decir: 
        //16 de intensidad, que según el manual son -53dBm (decibelio por miliwatio), que es bastante buena
          
        //99 de "channel bit error rate", que según el manual es desconocido o no detectable
            //En el momento del arranque no hay errores, siempre da 99
  if(SHOW) Serial.println("\nIntensidad de la señal");
  basura = envia("AT+CSQ", lectura, "OK", "ERROR", 9000, SHOW);

  //El comando AT+CICCID lee la información de la tarjeta SIM
  if(SHOW) Serial.println("\nNumero de serie de la SIM");
  //Según el manual, si devuelve por ejemplo la cadena "+ICCID: 89860318760238610932" es un OK
  lectura = "+ICCID:";
  basura = envia("AT+CICCID", lectura, "OK", "ERROR", 9000, SHOW);

  //El comando AT+CREG=2 hace, según manual: enable network registration and location information unsolicited
  if(SHOW) Serial.println("\nNotificación de cambio en el estado de la conexión");
  basura = envia("AT+CREG=2", lectura, "OK", "ERROR", 9000, SHOW);

  //EL comando AT+CSCS="IRA" setea el juego de caracteres a IRA (International Reference Alphabet)
  if(SHOW) Serial.println("\nConfigurar GSM set de caracteres");
  miDelay(1);
  basura = envia("AT+CSCS=\"IRA\"", lectura, "OK", "ERROR", 200, SHOW);

  //El comando AT+CMGF especifica el formato de los mensajes SMS
    //El modo=1 es modo texto, el modo=0 es binario (por defecto)
  if(SHOW) Serial.println("\nConfigurar el formato de los SMSs a formato de texto");
  basura = envia("AT+CMGF=1", lectura, "OK", "ERROR", 350, SHOW);

  //El comando AT+CNMI gestiona cómo recibir los mensajes en el TE (Terminal Equipment), 
    //en nuestro caso el ESP32. Con el mode=1 le decimos que no los guarde
  if(SHOW) Serial.println("\nConfigura SMSs no se guarden en memoria y");
  if(SHOW) Serial.println("reporte cuando lleguen");
  miDelay(1);
  if(SHOW) basura = envia("AT+CNMI=1", lectura, "OK", "ERROR", 300, SHOW);

  //El comando AT+CCLK? es para la lectura del reloj de sistema
  if(SHOW) Serial.println("\nActualiza RTC del ESP32");
  miDelay(1);
  lectura = "+CCLK:";
  basura = envia("AT+CCLK?", lectura, "OK", "ERROR", 200, SHOW);
  setRTC(lectura);  //  Set RTC del ESP32
  
  //El comando AT&W<value> solo tiene un <value> posible que es el 0 para salvar
    //Con esto salvamos la configuración de usuario que hemos ido seteando con 
    //los comandos ATE, ATQ, ATV, ATX, AT&C, AT&D,
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
  //  Verificación de la clase de cobertura a la red
  //  Se interpreta la respuesta del comando AT+CREG?, que tiene el siguiente formato:
  //      +CREG: <n>,<stat>[,<lac>,<ci>]
  //  El valor de <stat> representa la conexión con la red, con los siguientes valores:
  //      0   not registered, ME is not currently searching a new operator
  //          to register to.
  //      1   registered, home network.
  //      2   not registered, but ME is currently searching a new operator
  //          to register to.
  //      3   registration denied. 
  //      4   unknown.
  //      5   registered, roaming.
  //      6   registered for "SMS only", home network (applicable only when E-UTRAN)  

  //Serial.println("\n\nCobertura...");
  
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
  //El comando AT+CRESET hace un reseteo del módulo GPRS
  Serial2.println("AT+CRESET");
}

/****************************************************************************
               __ _            ____                _    ____  
         __ _ / _| |_ ___ _ __|  _ \ ___  ___  ___| |_ / /\ \ 
        / _` | |_| __/ _ \ '__| |_) / _ \/ __|/ _ \ __| |  | |
       | (_| |  _| ||  __/ |  |  _ <  __/\__ \  __/ |_| |  | |
        \__,_|_|  \__\___|_|  |_| \_\___||___/\___|\__| |  | |
                                                       \_\/_/ 
 ****************************************************************************/

//Esta función 
bool afterReset(uint32_t lap){
  bool condicion1 = false;      //  Cuando el GSM está activo
  bool condicion2 = false;      //  Cuando el GSM está conectado a la red
  String resp = "";

  //Este while() se ejecutará cada minuto mientras el módulo no esté activo 
  while(millis() < lap + 600000){
    //  Si esta activo y conectado sale
    if(condicion1){
      break;
    }
    if (Serial2.available()) {
      char letra = Serial2.read();
      if(armaFrase(letra, resp)){
        Serial.println(resp);
        //  Ultima respuesta del módulo GPRS al arrancar
        if(resp == "PB DONE"){
          Serial.println("Condicion cumplida");
          condicion1 = true;
        }
        //  El módulo no está conectado a ninguna red hasta que no obtengamos la cadena +CGEV: EPS PDN ACT 1
        if(resp.indexOf("+CGEV: EPS") > 0){
          Serial.print("Conetado a la red!");
          //Serial.println(resp.indexOf("+CGEV: EPS"));
          condicion2 = true;
        }
      }
    }
  }
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
  //Con la orden AT+COPS=3,1 le digo que quiero que me conteste el nombre del operador en modo texto corto
  bool basura = envia("AT+COPS=3,1", lectura, "OK", "ERROR", 60000, verbal);

  //Según el manual, si devuelve la cadena "COPS?" es que ha ido bien
  lectura = "COPS?";

//Con la orden AT+COPS?, le interrogo por el operador
  basura = envia("AT+COPS?", lectura, "OK", "ERROR", 60000, verbal);
  //Este comando nos devuelve esto: +COPS: 0,1,"vodafone ES",7
    //Cojo la primera y segunda aparición de las comillas en la cadena devuelta
  uint8_t praComillas = lectura.indexOf('"');
  uint8_t sdaComillas = lectura.indexOf('"', praComillas+1);
    //El operador aparece entre esas dos posiciones
  operador = lectura.substring(praComillas + 1, sdaComillas);
    //El tipo aparece entre 2 y 3 posiciones más allá de operador 
  int tipo = lectura.substring(sdaComillas + 2, sdaComillas + 3).toInt();

  //Según el manual: 
    //0: GSM (2G)
    //1: GSM Compact (2G)
    //2: UTRAN (3G) 
    //3: GSM with GPRS (2G)
    //4,5,6: diferentes modalidades de UTRAN (3G)
    //7: EUTRAN (4G)
    //8: UTRAN HSPA+ (4G+)

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

