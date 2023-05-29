//CSpell:ignore CPMUTEMP 
/****************************************************************************
                                 _____                    ____  
       __ _ _ __ _ __ ___   __ _|  ___| __ __ _ ___  ___ / /\ \ 
      / _` | '__| '_ ` _ \ / _` | |_ | '__/ _` / __|/ _ \ |  | |
     | (_| | |  | | | | | | (_| |  _|| | | (_| \__ \  __/ |  | |
      \__,_|_|  |_| |_| |_|\__,_|_|  |_|  \__,_|___/\___| |  | |
                                                         \_\/_/ 
 ****************************************************************************/
bool armaFrase(char letra, String &ans){
  //Serial.println("Entro en armaLaFrase");
  /*
  if(!isprint(letra)){
    Serial.println("Esto no se puede");
    return false;
  }
  */
    //if(lectura == "+CPMUTEMP:") Serial.print("+CPMUTEMP: 0");
  #define CARRIAGE_RETURN 13
  #define NEWLINE 10
  
  static bool finalizo = false;     //  CR al principio de la frase inicial
  static bool iniUno = false;       //  CR al principio de la frase inicial
  bool final = false;

    //if(lectura == "+CPMUTEMP:") Serial.print("+CPMUTEMP: 0");

  if(finalizo){
    //if(lectura == "+CPMUTEMP:") Serial.print("+CPMUTEMP: 1");
    finalizo = false;
    ans = "";
  }
  if(letra == CARRIAGE_RETURN){       //  Primer CR   Respuesta normal
    //if(lectura == "+CPMUTEMP:") Serial.print("+CPMUTEMP: 2");
    iniUno = true;
  }else if(letra == NEWLINE && iniUno){  //  Ultimo LF
    //  Termino la respuesta y la puedo retornar
    //if(lectura == "+CPMUTEMP:") Serial.print("+CPMUTEMP: 3");
    ans.trim();
    iniUno = false;
    final = true;
    finalizo = true;
  }else{
    //if(lectura == "+CPMUTEMP:") Serial.print("+CPMUTEMP: 4");
    ans.concat(letra);
  }
  return final;
}
