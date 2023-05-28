#define FILE_NAME "ParkingCtrl.ino"
/****************************************************************************
	Control inteligente de Puesto de parking

Description	:	Sistema de control de aparcamiento utilizando un ESP32, un sensor
              láser ToF y un módulo A7670E puede detectar el estado de ocupación
              de una plaza de aparcamiento cada minuto. Si se produce un cambio
              en el estado, se activará un MOSFET que encenderá el módulo A7670E.
              Una vez activo y conectado a la red, enviará un SMS a un número
              preestablecido para notificar el estado de la plaza de aparcamiento.
              Luego, el módulo A7670E se desactivará hasta el próximo cambio en el
              estado de la plaza.

              Este sistema permite monitorear de forma periódica el estado de
              ocupación de la plaza de aparcamiento y enviar notificaciones en
              tiempo real solo cuando se produce un cambio, lo que ayuda a
              optimizar el uso del módulo A7670E y minimizar el consumo de
              energía.
              
	Author		:	
	Created		:	
Modification:	
	Copyright	: 
	Basado en	:	

		Notes		:	

Components	:	


*****************************************************************************/
#include <ESP32Time.h>          //  ESP32 Time functionality

#define LED_BUILTIN     2       //  LED sobre el MCU
#define MOSFET          4       //  Pin que controla la alimentación del A7670E
#define RXD1            18      //  RX del Serial1 usado para el TFMiny
#define TXD1            19      //  TX del Serial1 usado para el TFMiny
#define RXD2            16      //  RX del Serial2 usado para el A7670E
#define TXD2            17      //  TX del Serial2 usado para el A7670E

#define SHOW_TIMES      false   //  Muestra el tiempo de respuesta comandos AT
#define SHOW            true    //  Salidas cuando inicializa el A7670E

#define INTERVALO       60000   //  Intervalo para verificar la plaza 1 min.
//#define NUMERO   "+34647475070" //  Número de contacto Móvil 1
#define NUMERO   "+34603372696" //  Número de contacto Movil 2

/****************************************************************************
			  ____ _     ___  ____    _    _     ____
			 / ___| |   / _ \| __ )  / \  | |   / ___|
			| |  _| |  | | | |  _ \ / _ \ | |   \___ \
			| |_| | |__| |_| | |_) / ___ \| |___ ___) |
			 \____|_____\___/|____/_/   \_|_____|____/

*****************************************************************************/
//  Main thread Variables

ESP32Time rtc(0);           //  Offset in seconds GMT

long const bps = 115200;

bool plazaOcupada = false;  //  Estado actual de la plaza
bool estadoAnte = false;    //  Estado de lectura anterior
bool arranca = true;        //  Solo cuando arranca en frio

String ristra = "";         //  GSM received data
String hld = "";            //  Holds SMS received
String sender = "";         //  Phone number that send the SMS
String dateTime = "";       //  Received SMS time and date
String cmd = "";            //  SMS text received
String SMS ="";             //  Output SMS text
String smsError = "";       //  Error messages text
String lectura = "";        //  Expected answer from GSM module
uint32_t lapso = 0;         //  Contador de tiempo rutina loop()

String operador ="";        //  Nombre del operador de telefonía
String tipoRed = "";        //  Tipo de red de conexión ejem.:2G, 3G, 4G, etc
int tempInterna = 0;        //  Temperatura interna del Modem GSM

int flagSMS = 0;            //  SMS received flag
int pos;                    //  SMS begin position 

//  TFMini LIDAR Variables
unsigned char inLidar[9];   //  Data recibida del TFMini
unsigned char checkSum;     //  Suma de comprobación data recibida
int apuntaByte = 0x01;      //  Puntero a un byte data recibida (inLidar[])
int dist;                   //  Distancia medida hasta un obstáculo en la plaza
int intensidad;             //  Intensidad de la señal del TFMini
float temperatura;

//************************************************
//  Para probar por falta del TFMini
//************************************************
bool complete = true;   //  Becomes true when enter is press 
String strVar = "";     //  String to keep the text entered



/****************************************************************************
								 _                ____
			  ___  ___| |_ _   _ _ __  / /\ \
			 / __|/ _ \ __| | | | '_ \| |  | |
			 \__ \  __/ |_| |_| | |_) | |  | |
			 |___/\___|\__|\__,_| .__/| |  | |
													|_|		 \_\/_/
****************************************************************************/
void setup() {

  //  On board led
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  //  MOSFET que alimenta al módulo A7670E
  pinMode(MOSFET, OUTPUT);
  digitalWrite(MOSFET, LOW);

  //  Abrir puerto Serial1 del ESP32 para comunicar con TFMiny
  Serial1.begin(115200, SERIAL_8N1, RXD1, TXD1);

  //  Abrir puerto Serial2 del ESP32 para comunicar con A7076E
  Serial2.begin(115200, SERIAL_8N1, RXD2, TXD2);

  //  Abrir puerto Serial del ESP32 para mensajes a la consola
  Serial.begin(bps);
  while (!Serial) {
    // wait for serial port. Needed for boards with native USB port
  }
  //  Needed to avoid double run on the terminal on UNO
  //	https://forum.arduino.cc/t/double-output-on-setup-observations/700756/5
  delay(200);


  //  Sketch ID on the terminal
  Serial.println(F(FILE_NAME));
  for(int i = 0; i < 70; i++) Serial.print("-"); Serial.println("");
  Serial.println(F("Sistema utiliza un sensor láser ToF para detectar el estado de ocupación de una plaza"));
  Serial.println(F("cada minuto, activando un módulo A7670E que envía un SMS al número preestablecido antes"));
  Serial.println(F("de ser desactivado hasta el próximo cambio."));
  Serial.println(F("\n\n"));

  /************************ INICIALIZANDO módulo A7076E *********************************/
  //  El Modem GSM puede tardar mas de 25 segundos en arrancar
  beginGSM();               //  Enciende el módulo A7670E y espera por el
  initGSM();                //  Configuración inicial del módulo 

  Serial.println("");
  Serial.println("******************** INICIALIZADO ********************");

  //  Desactivar el módulo A7670E hasta que sea necesario
  Serial.println("Desactivando módulo A7670E");
  digitalWrite(MOSFET, LOW);
  miDelay(1);

  //lapso = -INTERVALO;         //  Se utiliza para contar el tiempo en el loop()


}


/****************************************************************************
			 _                    ____
			| | ___   ___  _ __  / /\ \
			| |/ _ \ / _ \| '_ \| |  | |
			| | (_) | (_) | |_) | |  | |
			|_|\___/ \___/| .__/| |  | |
						  			|_|		 \_\/_/
****************************************************************************/
void loop() {

  //  Cada INTERVALO milisegundos se verifica si hay coche ocupando la plaza
  if( millis() > (lapso + INTERVALO) || lapso == 0){
    Serial.println("Chequeando plaza de parking");
    lapso = millis();         //  Comienza el conteo de tiempo
  	getLidarData();           //  Se mide la distancia del obstáculo en la plaza

    //***********************************
    //  Para probar por falta del TFMini
    //***********************************
    /*
    if (complete) {
      //  Solo se aceptan numero de 0 al 10
      String prtStrVar = strVar;
      prtStrVar.trim();

      //  Valida los valores
      //CSpell: ignore obstaculo
      int ditObstaculo = prtStrVar.toInt();
      if(ditObstaculo >= 0 && ditObstaculo <= 1000) dist = ditObstaculo;

      //  Inicializar variables para proxima la vez
      strVar = "";
      complete = false;
    }
    //***********************************
    //  Final de para probar
    //***********************************
    */
    if (dist >= 150) {
      plazaOcupada = false; 
    } else {
      plazaOcupada = true; 
    }
    if (plazaOcupada == estadoAnte) {
      Serial.print("La plaza sigue ");
      Serial.println(plazaOcupada ? "OCUPADA\n" : "LIBRE\n");
    } else {
      if (arranca) {
        arranca = false;
        Serial.print("La plaza está ");
        Serial.println(plazaOcupada ? "OCUPADA\n" : "LIBRE\n");
      } else {
        Serial.print("La plaza ahora ");
        Serial.println(plazaOcupada ? "se OCUPÓ\n" : "esta LIBRE\n");
      }

      //  El Modem GSM puede tardar mas de 25 segundos en arrancar
      beginGSM();               //  Enciende el módulo A7670E y espera por el
      initGSM();                //  Configuración inicial del módulo
      

      //  Se ensambla el mensaje saliente
      SMS = "Plaza ";
      SMS += plazaOcupada ? "OCUPADA\n" : "LIBRE\n";
      SMS += rtc.getTime("%B %d %H:%M:%S\n");
      Serial.println(SMS);

      //  Se envía el SMS
      sendSMS(NUMERO, SMS);
      //miDelay(3);

      //  Desactivar el módulo A7670E hasta que sea necesario
      Serial.println("Desactivando módulo A7670E");
      digitalWrite(MOSFET, LOW);

      estadoAnte = plazaOcupada;
    }
  }
}

/****************************************************************************
                      _       _ _____                 _    ____
        ___  ___ _ __(_) __ _| | ____|_   _____ _ __ | |_ / /\ \
       / __|/ _ \ '__| |/ _` | |  _| \ \ / / _ \ '_ \| __| |  | |
       \__ \  __/ |  | | (_| | | |___ \ V /  __/ | | | |_| |  | |
       |___/\___|_|  |_|\__,_|_|_____| \_/ \___|_| |_|\__| |  | |
                                                          \_\/_/
 ****************************************************************************/
//  Para probar por falta del TFMini
void serialEvent() {
  while (Serial.available()) {
    char charIn = Serial.read();        //  Lee un carácter del buffer
    strVar += charIn;                   //  Añade el carácter a la var receptora
    if (charIn == 10 || charIn == 13) { //  Cualquier tipo de fin de línea
      delay(5);                         //  Espera por el USB
      if (!Serial.available()) {
        complete = true;                //  Señaliza que la comunicación terminó
      }
    }
    if (strVar.length() > 10) complete = true;
  }
}
