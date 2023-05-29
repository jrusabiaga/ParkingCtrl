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
#define RXD1            18      //  RX del Serial1 usado para el TFMini
#define TXD1            19      //  TX del Serial1 usado para el TFMini
#define RXD2            16      //  RX del Serial2 usado para el A7670E
#define TXD2            17      //  TX del Serial2 usado para el A7670E

#define SHOW_TIMES      false   //  Muestra el tiempo de respuesta comandos AT
#define SHOW            true    //  Salidas cuando inicializa el A7670E

#define DIST_MIN        150     //  Distancia mínima hasta el coche
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

ESP32Time rtc(0);               //  Offset in seconds GMT


//  Variables operativas
long const bpsC = 115200;       //  Velocidad de conexión con la consola
long const bpsG = 115200;       //  Velocidad de conexión con el A7670E
long const bpsL = 115200;       //  Velocidad de conexión con el LÁSER
bool plazaOcupada = false;      //  Estado actual de la plaza
bool estadoAnte = true;         //  Estado de lectura anterior
bool arranca = true;            //  Solo cuando arranca en frio

//  Variables módulo A7670E
String ristra = "";             //  GSM received data
String SMS ="";                 //  Output SMS text
String lectura = "";            //  Expected answer from GSM module
uint32_t lapso = 0;             //  Contador de tiempo rutina loop()
String operador ="";            //  Nombre del operador de telefonía
String tipoRed = "";            //  Tipo de red de conexión ejem.:2G, 3G, 4G, etc
int flagSMS = 0;                //  SMS received flag

//  TFMini LIDAR Variables
int medida;                     //  La distancia medida por el TFMini Plus
int intensidad;                 //  Intensidad de la señal del TFMini Plus
float temperatura;              //  Temperatura del chip en el TFMini Plus
uint8_t chkSum;                 //  Suma de verificación
uint8_t cabecera1;              //  Encabezado de la data
uint8_t cabecera2;              //  Encabezado de la data
uint8_t lsbDist;                //  LSB de la distancia
uint8_t msbDist;                //  MSB de la distancia
uint8_t lsbInte;                //  LSB de la intensidad
uint8_t msbInte;                //  MSB de la intensidad
uint8_t lsbTemp;                //  LSB de la temperatura
uint8_t msbTemp;                //  MSB de la temperatura
uint8_t npi1;                   //  Ni la menor idea
uint8_t verifica;               //  Suma de verificación enviada por el TFMini
unsigned int charCabeza = 'Y';  //  Carácter usado para comienzo de datos
int byteNum = 1;                //  Lleva la cuenta del numero del byte

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

  //  Abrir puerto Serial del ESP32 para mensajes a la consola
  Serial.begin(bpsC);
  while (!Serial) {
    // wait for serial port. Needed for boards with native USB port
  }
  //  Needed to avoid double run on the terminal on UNO
  //	https://forum.arduino.cc/t/double-output-on-setup-observations/700756/5
  delay(200);

  //  Abrir puerto Serial1 del ESP32 para comunicar con TFMini
  Serial1.begin(bpsL, SERIAL_8N1, RXD1, TXD1);
  while (!Serial1) {
    // wait for serial port. Needed for boards with native USB port
  }

  //  Abrir puerto Serial2 del ESP32 para comunicar con A7076E
  Serial2.begin(bpsG, SERIAL_8N1, RXD2, TXD2);
  while (!Serial2) {
    // wait for serial port. Needed for boards with native USB port
  }


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
  Serial.println("Módulo A7670E desactivado");
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
    Serial.print("Verificando: ");
    lapso = millis();         //  Comienza el conteo de tiempo
  	getLidarData();           //  Se mide la distancia del obstáculo en la plaza

    if (medida >= DIST_MIN) {
      plazaOcupada = false; 
    } else {
      plazaOcupada = true; 
    }

    if (arranca) {
      arranca = false;
      Serial.print("La plaza está ");
      Serial.println(plazaOcupada ? "OCUPADA" : "LIBRE");
      estadoAnte = !plazaOcupada;
    }

    if (plazaOcupada == estadoAnte) {
      Serial.print("La plaza sigue ");
      Serial.println(plazaOcupada ? "OCUPADA" : "LIBRE");
    } else {
      Serial.print("La plaza ahora ");
      Serial.println(plazaOcupada ? "se OCUPÓ" : "esta LIBRE");

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
      Serial.println("Módulo A7670E desactivado");
      digitalWrite(MOSFET, LOW);

    }

    estadoAnte = plazaOcupada;
  }
}
