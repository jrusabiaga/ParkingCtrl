/****************************************************************************
            _ _      ____ _ _            _       ___       _  ____  
         __| | |__  / ___| (_) ___ _ __ | |_ ___|_ _|_ __ (_)/ /\ \ 
        / _` | '_ \| |   | | |/ _ \ '_ \| __/ _ \| || '_ \| | |  | |
       | (_| | |_) | |___| | |  __/ | | | ||  __/| || | | | | |  | |
        \__,_|_.__/ \____|_|_|\___|_| |_|\__\___|___|_| |_|_| |  | |
                                                             \_\/_/ 
****************************************************************************/
void dbClienteIni(){

  //  Inicializa SPIFFS para crear una lista de clientes en memoria
  //  flash
  if (!SPIFFS.begin(true)) {
    Serial.println("Error al montar SPIFFS");
    return;
  }

  /*
  // Crear una lista de registros
  //Registro registros[10] = {
  registros[10] = {
    {1, "Juan", "123456789"},
    {2, "María", "987654321"},
    {3, "Pedro", "456789123"},
    {4, "Laura", "321098765"},
    {5, "Lucas", "456789012"},
    {6, "Ana", "890123456"},
    {7, "Luis", "654321098"},
    {8, "Sofía", "901234567"},
    {9, "Diego", "789012345"},
    {10, "Valeria", "234567890"}
  };
  */

  //  Borrar el fichero anterior
  borraFichero(SPIFFS, "/lista.dat");


  // Guardar la lista en el sistema de archivos SPIFFS
  salvaRegs(registros, 10);

  Serial.println("Lista guardada correctamente");

  // Mostrar todos los registros
  listaRegs();

  // Borrar el registro con número 2
  //borraReg(2);

  // Mostrar los registros actualizados después de borrar uno
  //listaRegs();
}

/****************************************************************************
                  _            ____                 ____  
        ___  __ _| |_   ____ _|  _ \ ___  __ _ ___ / /\ \ 
       / __|/ _` | \ \ / / _` | |_) / _ \/ _` / __| |  | |
       \__ \ (_| | |\ V / (_| |  _ <  __/ (_| \__ \ |  | |
       |___/\__,_|_| \_/ \__,_|_| \_\___|\__, |___/ |  | |
                                         |___/     \_\/_/ 
****************************************************************************/
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
void borraFichero(fs::FS &fs, const char * path){
    Serial.printf("Deleting file: %s\r\n", path);
    if(fs.remove(path)){
        Serial.println("- file deleted");
    } else {
        Serial.println("- delete failed");
    }
}

/****************************************************************************
        _                               ____             ____  
       | |__   ___  _ __ _ __ __ _ _ __|  _ \ ___  __ _ / /\ \ 
       | '_ \ / _ \| '__| '__/ _` | '__| |_) / _ \/ _` | |  | |
       | |_) | (_) | |  | | | (_| | |  |  _ <  __/ (_| | |  | |
       |_.__/ \___/|_|  |_|  \__,_|_|  |_| \_\___|\__, | |  | |
                                                  |___/ \_\/_/ 
****************************************************************************/
void borraReg(int numero) {
  File file = SPIFFS.open("/lista.dat", "r");
  if (!file) {
    Serial.println("Error al abrir el archivo");
    return;
  }

  Registro registros[10];
  int cantidad = file.read((uint8_t*)registros, sizeof(registros));
  file.close();

  for (int i = 0; i < cantidad / sizeof(Registro); i++) {
    if (registros[i].numero == numero) {
      // Desplazar los registros restantes para eliminar el registro encontrado
      for (int j = i; j < cantidad / sizeof(Registro) - 1; j++) {
        registros[j] = registros[j + 1];
      }
      cantidad -= sizeof(Registro);
      break;
    }
  }

  // Guardar los registros actualizados
  salvaRegs(registros, cantidad / sizeof(Registro));
}
