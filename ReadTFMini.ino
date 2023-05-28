//CSpell: ignore miny
/****************************************************************************
                   _   _     _     _            ____        _         ____  
         __ _  ___| |_| |   (_) __| | __ _ _ __|  _ \  __ _| |_ __ _ / /\ \ 
        / _` |/ _ \ __| |   | |/ _` |/ _` | '__| | | |/ _` | __/ _` | |  | |
       | (_| |  __/ |_| |___| | (_| | (_| | |  | |_| | (_| | || (_| | |  | |
        \__, |\___|\__|_____|_|\__,_|\__,_|_|  |____/ \__,_|\__\__,_| |  | |
        |___/                                                        \_\/_/ 
****************************************************************************/
//  Lee todos los parámetros el sensor TFMiny
void getLidarData(){
  if (Serial1.available()){               //  Si se recibe data del TFMini
    Serial.println("Entro al LIDAR");
    if(apuntaByte == 0x01){               //  Si el puntero apunta al primer byte
      inLidar[0]=Serial1.read();
      if(inLidar[0] == 0x59){             //  Primer byte debe ser 0x59
        checkSum = inLidar[0];
        apuntaByte = 0x02;
      }
    }else if(apuntaByte == 0x02){         //  Si el puntero apunta al segundo byte
      inLidar[1]=Serial1.read();
      if(inLidar[1] == 0x59){             //  Segundo byte debe ser 0x59
        checkSum += inLidar[1];
        apuntaByte = 0x03;
      }else{
        apuntaByte = 0x01; 
      }
    }else if(apuntaByte == 0x03){         //  Byte menor de la distancia
      inLidar[2]=Serial1.read();
      checkSum += inLidar[2];
      apuntaByte = 0x04;
    }else if(apuntaByte == 0x04){         //  Byte mayor de la distancia
      inLidar[3]=Serial1.read();
      checkSum += inLidar[3];
      apuntaByte = 0x05;
    }else if(apuntaByte == 0x05){         //  Byte menor de la intensidad
      inLidar[4]=Serial1.read();
      checkSum += inLidar[4];
      apuntaByte = 0x06;
    }else if(apuntaByte == 0x06){         //  Byte mayor de la intensidad
      inLidar[5]=Serial1.read();
      checkSum += inLidar[5];
      apuntaByte = 0x07;
    }else if(apuntaByte == 0x07){         //  Byte menor de la temperatura
      inLidar[6]=Serial1.read();
      checkSum += inLidar[6];
      apuntaByte = 0x08;
    }else if(apuntaByte == 0x08){         //  Byte mayor de la temperatura
      inLidar[7]=Serial1.read();
      checkSum += inLidar[7];
      apuntaByte = 0x09;
    }else if(apuntaByte == 0x09){         //  Suma de comprobación
      inLidar[8]=Serial1.read();
      if(inLidar[8] == checkSum){
        dist = inLidar[2] + inLidar[3]*256;         //  Distancia medida
        intensidad = inLidar[4] + inLidar[5]*256;   //  Intensidad de la señal
        temperatura = inLidar[6] + inLidar[7] *256; //  Temperatura del TFMiny
        temperatura = temperatura/8 - 256;          //  Convierte a Celsius                         
        Serial.print("dist = ");
        Serial.print(dist);                         //  Muestra distancia
        Serial.print('\n');
        //Serial.print("intensidad = ");
        //Serial.print(intensidad);                   //  Muestra la intensidad
        //Serial.print('\n');
        //Serial.print("\t Temp. interna = ");
        //Serial.print(temperatura);
        //Serial.println(" grados Celsius");          //  Muestra Temperatura interna
        //  Limpieza del buffer de comunicaciones para que este vació
        //  para la proxima lectura
        while(Serial1.available()){Serial1.read();}
        delay(100);
      }
      apuntaByte = 0x01;
    }
  }
}
