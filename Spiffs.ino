//CSpell: ignore movil
/****************************************************************************
            _ _      ____ _ _            _       ___       _  ____  
         __| | |__  / ___| (_) ___ _ __ | |_ ___|_ _|_ __ (_)/ /\ \ 
        / _` | '_ \| |   | | |/ _ \ '_ \| __/ _ \| || '_ \| | |  | |
       | (_| | |_) | |___| | |  __/ | | | ||  __/| || | | | | |  | |
        \__,_|_.__/ \____|_|_|\___|_| |_|\__\___|___|_| |_|_| |  | |
                                                             \_\/_/ 
****************************************************************************/
//  Inicializa, actualiza fichero de clientes en memoria flash
void dbClienteIni(){

  //  Inicializa SPIFFS para crear una lista de clientes en memoria
  //  flash
  if (!SPIFFS.begin(true)) {
    Serial.println("Error al montar SPIFFS");
    return;
  }

  //  Borrar el fichero anterior
  borraFichero(SPIFFS, "/lista.dat");

  // Guardar la lista en el sistema de archivos SPIFFS
  salvaRegs(registros, 10);

  Serial.println("Lista guardada correctamente");

  // Mostrar todos los registros
  listaRegs();
}

/****************************************************************************
                  _            ____                 ____  
        ___  __ _| |_   ____ _|  _ \ ___  __ _ ___ / /\ \ 
       / __|/ _` | \ \ / / _` | |_) / _ \/ _` / __| |  | |
       \__ \ (_| | |\ V / (_| |  _ <  __/ (_| \__ \ |  | |
       |___/\__,_|_| \_/ \__,_|_| \_\___|\__, |___/ |  | |
                                         |___/     \_\/_/ 
****************************************************************************/
//  Salve los registros en memoria flash
void salvaRegs(Registro registros[], int cantidad) {
  File file = SPIFFS.open("/lista.dat", "w");
  if (!file) {
    Serial.println("Error al abrir el archivo");
    return;
  }

  file.write((uint8_t*)registros, cantidad * sizeof(Registro));
  file.close();
}

/****************************************************************************
        _ _     _        ____                 ____  
       | (_)___| |_ __ _|  _ \ ___  __ _ ___ / /\ \ 
       | | / __| __/ _` | |_) / _ \/ _` / __| |  | |
       | | \__ \ || (_| |  _ <  __/ (_| \__ \ |  | |
       |_|_|___/\__\__,_|_| \_\___|\__, |___/ |  | |
                                   |___/     \_\/_/ 
****************************************************************************/
//  Imprime por el terminal una lista de clientes
void listaRegs() {
  File file = SPIFFS.open("/lista.dat", "r");
  if (!file) {
    Serial.println("Error al abrir el archivo");
    return;
  }

  //registros[cantidad];
  int cantidad = file.read((uint8_t*)registros, sizeof(registros));
  file.close();

  Serial.println("----- Registros -----");
  for (int i = 0; i < cantidad / sizeof(Registro); i++) {
    //Serial.print("Número: ");
    if(registros[i].numero != 0){
      Serial.print(registros[i].numero);
      Serial.print(", ");
      Serial.print(registros[i].nombre);
      Serial.print(", ");
      Serial.println(registros[i].movil);
      //Serial.println("---------------------");
    }
  }
}

/****************************************************************************
        _                          _____ _      _                     ____  
       | |__   ___  _ __ _ __ __ _|  ___(_) ___| |__   ___ _ __ ___  / /\ \ 
       | '_ \ / _ \| '__| '__/ _` | |_  | |/ __| '_ \ / _ \ '__/ _ \| |  | |
       | |_) | (_) | |  | | | (_| |  _| | | (__| | | |  __/ | | (_) | |  | |
       |_.__/ \___/|_|  |_|  \__,_|_|   |_|\___|_| |_|\___|_|  \___/| |  | |
                                                                     \_\/_/ 
****************************************************************************/
//  Elimina el fichero de clientes de la memoria flash
void borraFichero(fs::FS &fs, const char * path){
    Serial.printf("Deleting file: %s\r\n", path);
    if(fs.remove(path)){
        Serial.println("- file deleted");
    } else {
        Serial.println("- delete failed");
    }
}

