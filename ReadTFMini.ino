//CSpell: ignore miny
/****************************************************************************
                   _   _     _     _            ____        _         ____  
         __ _  ___| |_| |   (_) __| | __ _ _ __|  _ \  __ _| |_ __ _ / /\ \ 
        / _` |/ _ \ __| |   | |/ _` |/ _` | '__| | | |/ _` | __/ _` | |  | |
       | (_| |  __/ |_| |___| | (_| | (_| | |  | |_| | (_| | || (_| | |  | |
        \__, |\___|\__|_____|_|\__,_|\__,_|_|  |____/ \__,_|\__\__,_| |  | |
        |___/                                                        \_\/_/ 
****************************************************************************/
//  Lee todos los parámetros el sensor TFMini
void getLidarData(){
  //  Se elimina el contenido del buffer para tener una lectura actualizada
  while (Serial1.available()) {
    Serial1.read();
  }
  delay(100);

  //  Se empieza con la lectura en tiempo real
  while(1){
    if (Serial1.available()) { // Se procesa Si existen bytes en el buffer

	  //El primer byte de la cabecera
      if (byteNum == 1) {
        cabecera1 = Serial1.read();
        if (cabecera1 == charCabeza){
          chkSum = cabecera1;
          byteNum = 2;
        }

	  //El segundo byte de la cabecera
      } else if (byteNum == 2) {
        cabecera2 = Serial1.read();
        if (cabecera2 == charCabeza) {
          chkSum += cabecera2;
          byteNum = 3;
        } else {
          byteNum = 1;
        }

	  //El primer byte de la distancia, Hexadecimal de menor valor LSB
      } else if (byteNum == 3) {
        lsbDist = Serial1.read();
        chkSum += lsbDist;
        byteNum = 4;

	  //El segundo byte de la distancia, Hexadecimal de mayor valor MSB
      } else if (byteNum == 4) {
        msbDist = Serial1.read();
        chkSum += msbDist;
        byteNum = 5;

	  //El primer byte de la intensidad, Hexadecimal de menor valor LSB
      } else if (byteNum == 5) {
        lsbInte = Serial1.read();
        chkSum += lsbInte;
        byteNum = 6;

	  //El segundo byte de la intensidad Hexadecimal de mayor valor MSB
      } else if (byteNum == 6) {
        msbInte = Serial1.read();
        chkSum += msbInte;
        byteNum = 7;

	  //El primer byte de la temperatura, Hexadecimal de menor valor LSB
      } else if (byteNum == 7) {
        lsbTemp = Serial1.read();
        chkSum += lsbTemp;
        byteNum = 8;

	  //El segundo byte de la temperatura, Hexadecimal de mayor valor MSB
      } else if (byteNum == 8) {
        msbTemp = Serial1.read();
        chkSum += msbTemp;
        byteNum = 9;

	  //Hexadecimal de verificación enviado por el TFMini Plus Check Sum
      } else if (byteNum == 9) {
        verifica = Serial1.read();
        if (verifica == chkSum) {
          medida = lsbDist + (msbDist * 256);       //La distancia medida
          intensidad = lsbInte + (msbInte * 256);   //La potencia de la señal
          temperatura = lsbTemp + (msbTemp * 256);  //La temperatura en Celsius
          temperatura = (temperatura / 8) - 256;
          Serial.print("Distancia = ");
          Serial.print(medida);                 //Distancia medida por el TFMini Plus
          Serial.print(" cm. ");
          //Serial.print(" Intensidad = ");
          //Serial.print(intensidad);           //Potencia de la señal recibida
          //Serial.print('\n');
          //Serial.print("Temperatura = ");
          //Serial.print(temperatura);
          //Serial.println(" Grados Celsius");  //Temperatura interna del TFMini Plus
          while (Serial1.available()) {
            Serial1.read();                     //Blanquea todo hasta empezar una 
                                                //nueva cadena de comunicación
          }
          delay(100);
        }
        byteNum = 1;
        break;
      }
    } //  Sale del if
  } //  Sale del while
}
