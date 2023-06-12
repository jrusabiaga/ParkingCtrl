//CSpell: ignore CMGS
/*********************************************************************************
                           _ ____  __  __ ____   ____  
        ___  ___ _ __   __| / ___||  \/  / ___| / /\ \ 
       / __|/ _ \ '_ \ / _` \___ \| |\/| \___ \| |  | |
       \__ \  __/ | | | (_| |___) | |  | |___) | |  | |
       |___/\___|_| |_|\__,_|____/|_|  |_|____/| |  | |
                                                \_\/_/ 
 ********************************************************************************/
//  Envía el SMS al número
void sendSMS(String nombre, String numero, String SMS){

  bool salio = false;

  //  Si se pierde la cobertura nunca saldrá de este bucle
  //  Atención para reportar este error con el LED_ERR y estado SIN_COBERTURA
  while(cobertura() != 1){
    resetModule();
    uint32_t lapso = millis();
    
    // Atención a la próxima linea, creo que no hace falta
    afterReset(lapso);
  }

  while(!salio){
    uint32_t tiempo = 0;
    bool sigue = true;
    ristra = "";

    Serial.print("Envía SMS: "); 
    Serial.print(numero); 
    Serial.print(" a "); 
    Serial.println(nombre); 
    
    while(sigue){
      tiempo = millis();
      //Serial.print("\nComando para enviar SMS ");
      //Serial.println(Serial2.print("AT+CMGS=\"" + numero + "\"\r"));
      Serial2.print("AT+CMGS=\"" + numero + "\"\r");
      sigue = !updateSerial3(800, ">", false);

      //  Si el módulo no responde y se queda en un loop infinito
      //  Se reinicia y configura
      if(sigue){
        Serial.println("\nNo responde RESET del módulo");
        //  Se apaga el módulo A7670E
        Serial.println("Módulo A7670E desactivado");
        digitalWrite(MOSFET, LOW);
        miDelay(4);

        //  Se reinicia y configura el módulo nuevamente
        //  El Modem GSM puede tardar mas de 25 segundos en arrancar
        beginGSM();               //  Enciende el módulo A7670E y espera por el
        initGSM();                //  Configuración inicial del módulo      
      }
      miDelay(1);
    }

    //Serial.print("\nCuerpo del Mensaje ");
    //Serial.println(Serial2.print(SMS));
    Serial2.print(SMS);
    //Serial2.print(SMS);         //  Se envía el texto del mensaje
    updateSerial(800, false);
    //Serial.print("\nFin de Mensaje ");
    //Serial.println(Serial2.print((char)26));
    Serial2.print((char)26);    // Código ASCII 0x1A de CTRL+Z señala fin del texto
    Serial.println("¡enviado!");


    miDelay(5);
    //  Verificar que no pase mas de 40 seg. 
    salio = updateSerial2(40000 - (millis() - tiempo), false);

    //  Impresión de lo que se a recibido del módulo A7670E
    Serial.println("Respuesta del A7670E --------------------------------------------------------");
    Serial.println(ristra);
    Serial.println("FIN -------------------------------------------------------------------------");
    Serial.println("");

    //  Si el comando de envío y no responde algo pasa en el módulo
    if(!salio){
      Serial.println("\nNo responde RESET del módulo");
      //  Se apaga el módulo A7670E
      Serial.println("Módulo A7670E desactivado");
      digitalWrite(MOSFET, LOW);
      miDelay(4);

      //  Se reinicia y configura el módulo nuevamente
      //  El Modem GSM puede tardar mas de 25 segundos en arrancar
      beginGSM();               //  Enciende el módulo A7670E y espera por el
      initGSM();                //  Configuración inicial del módulo      
    }
  }

  //  Una vez enviados los SMS se limpian las variables
  SMS = "";
  //  End sendSMS
}

