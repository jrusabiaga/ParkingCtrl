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

  //Aquí podríamos hacer una comprobación de cobertura en el momento del envío del SMS, 
  //Pero es redundante ya que esto lo hacemos cada vez que encendemos el módulo
/*  
  while(cobertura() != 1){
    //Todo lo que no sea "1" es que no tenemos cobertura, luego hacemos un reseteo a ver si espabila
    resetModule();
    
    // La próxima linea no hace falta, es solo una comprobación doble (módulo activo + módulo registrado en red)
    uint32_t lapso = millis();
    //afterReset(lapso);
  }
*/

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

      //Si el módulo no responde y se queda en un loop infinito
      //Se reinicia y configura
      if(sigue){
        Serial.println("\nNo responde RESET del módulo");
        //  Se apaga el módulo A7670E
        Serial.println("Módulo A7670E desactivado");
        digitalWrite(MOSFET, LOW);
        miDelay(4);

        //Se reinicia y configura el módulo nuevamente, puede tardar mas de 25 segundos en arrancar
        beginGSM();//Enciende el módulo y espera por el
        initGSM();//Configuración inicial del módulo      
      }
      miDelay(1);
    }

    /*
      //CHECKOINT: Si llego a este punto ya he obtenido el símbolo ">" y puedo escribir el texto
    */ 
    //Serial.print("\nCuerpo del Mensaje ");
    //Serial.println(Serial2.print(SMS));
    Serial2.print(SMS);//Se envía el texto del mensaje
    updateSerial(800, false);
    //Serial.print("\nFin de Mensaje ");
    //Serial.println(Serial2.print((char)26));
    Serial2.print((char)26);    // Código ASCII 0x1A de CTRL+Z señala fin del texto
    Serial.println("¡enviado!");


    miDelay(5);
    //Aquí es donde el módulo responde sobre cómo ha ido el envío del SMS. 
    //Le damos 40 segundos máximo para ello. Si los sobrepasa, reseteamos el módulo
    salio = updateSerial2(40000 - (millis() - tiempo), false);

    //  Imprimimos la respuesta del módulo
    Serial.println("Respuesta del A7670E --------------------------------------------------------");
    Serial.println(ristra);
    Serial.println("FIN -------------------------------------------------------------------------");
    Serial.println("");

    //  Si el comando de envío y no responde (en menos de 40 segundos) algo pasa en el módulo
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

