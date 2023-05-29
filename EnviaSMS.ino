/*********************************************************************************
                           _ ____  __  __ ____   ____  
        ___  ___ _ __   __| / ___||  \/  / ___| / /\ \ 
       / __|/ _ \ '_ \ / _` \___ \| |\/| \___ \| |  | |
       \__ \  __/ | | | (_| |___) | |  | |___) | |  | |
       |___/\___|_| |_|\__,_|____/|_|  |_|____/| |  | |
                                                \_\/_/ 
 ********************************************************************************/
//  Envía el SMS al número
void sendSMS(String numero, String SMS){

  //  Si se pierde la cobertura nunca saldrá de este bucle
  //  Atención para reportar este error con el LED_ERR y estado SIN_COBERTURA
  while(cobertura() != 1){
    resetModule();
    uint32_t lapso = millis();
    
    // Atención a la próxima linea, creo que no hace falta
    afterReset(lapso);
  }

  Serial.print("Envía SMS: "); 
  Serial.println(numero); 
  
  Serial2.print("AT+CMGS=\"" + numero + "\"\r");
  updateSerial(800, true);
  Serial2.print(SMS);         //  Se envía el texto del mensaje
  updateSerial(800, true);
  Serial2.print((char)26);    // Código ASCII 0x1A de CTRL+Z señala fin del texto
  
  updateSerial(1000, true);
  miDelay(5);
  Serial.println(" Enviado");


  //  Una vez enviados los SMS se limpian las variables
  SMS = "";
  //  End sendSMS
}

