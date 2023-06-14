//CSpell: ignore inte, movil
#define FILE_NAME "ParkingCtrl.ino"
/****************************************************************************
	Control inteligente de Puesto de parking

Description	:	Sistema de control de aparcamiento utilizando un ESP32, un sensor
              láser ToF y un módulo A7670E puede detectar el estado de ocupación
              de una plaza de aparcamiento cada minuto. Si se produce un cambio
              en el estado, se activará un MOSFET que encenderá el módulo A7670E.
              Una vez activo y conectado a la red, enviará un SMS a cada número
              preestablecido en la lista de clientes, para notificar el estado
              de la plaza de aparcamiento. Luego, el módulo A7670E se desactivará
              hasta el próximo cambio en el estado de la plaza.

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
#include <SPIFFS.h>             //  Manejo de ficheros en memoria flash

#define LED_BUILTIN     2       //  LED sobre el MCU
#define MOSFET          4       //  Pin que controla la alimentación del A7670E
#define RXD1            18      //  RX del Serial1 usado para el TFMini
#define TXD1            19      //  TX del Serial1 usado para el TFMini
#define RXD2            16      //  RX del Serial2 usado para el A7670E
#define TXD2            17      //  TX del Serial2 usado para el A7670E

#define SHOW_TIMES      false   //  Muestra el tiempo de respuesta comandos AT
#define SHOW            true    //  Salidas cuando inicializa el A7670E

#define DIST_MIN        150     //  Distancia mínima hasta el coche
#define NUM_CONFIRMA    2       //  Número de veces que confirma el estado del parking
#define INTERVALO       60000   //  Intervalo para verificar la plaza 1 min.

#define NUMERO      "647475070" //  Número de contacto Móvil 1
//#define NUMERO      "603372696" //  Número de contacto Movil 2

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
uint8_t confirmacion = 0;       //  Veces que se confirma el estado de la plaza
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

//  Registro usado para almacenar clientes en fichero .dat
struct Registro {
  int numero;
  char nombre[16];
  char movil[10];
};

int const numCli = 10;          //  Número de clientes

//  El número de veces que puedes escribir en la memoria flash de un ESP32 está
//  limitado, el número exacto depende del dispositivo específico y la
//  biblioteca utilizada. La memoria flash está diseñada para aproximadamente
//  100,000 a 1,000,000 de operaciones de escritura.

//  Cambiar el valor de "nuevaLista" a true cuando se modifique la lista y una
//  vez ejecutado el sketch, cambiar a false y volver a compilar y subir el
//  sketch para extender la vida util de la memoria flash.
bool const nuevaLista = false;

// Crear una lista de registros
//Registro registros[10] = {
Registro registros[numCli] = {
  {1, "Juan", NUMERO},          //  Registro 0
  {2, "Maria", NUMERO},         //  Registro 1
  {3, "Pedro", NUMERO},         //  Registro 2
  {4, "Laura", NUMERO},         //  Registro 3
  {5, "Lucas", NUMERO},         //  Registro 4
  {6, "Ana", NUMERO},           //  Registro 5
  {7, "Luis", NUMERO},          //  Registro 6
  {8, "Sofia", NUMERO},         //  Registro 7
  {9, "Diego", NUMERO},         //  Registro 8
  {10, "Valeria", NUMERO}       //  Registro 9
};

/***************************************************************************
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
  Serial.println(F("Sistema utiliza un sensor láser ToF para detectar el estado de"));
  Serial.println(F("ocupación de una plaza cada minuto, activando un módulo A7670E"));
  Serial.println(F("que envía un SMS a cada cliente de la lista. El módulo es"));
  Serial.println(F("desactivado hasta el próximo cambio, para reducir el consumo."));
  Serial.println(F("\n\n"));

  //  Inicializa SPIFFS y crear fichero de clientes
  if(nuevaLista) dbClienteIni();

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

    //  Verifica si hay algún coche aparcado en la plaza, para esto: 
    //  utiliza el sensor de distancia
    if (medida >= DIST_MIN) {
      plazaOcupada = false; 
    } else {
      plazaOcupada = true; 
    }

    //  La primera vez que se ejecuta el loop(): Arranque en frio del MCU
    if (arranca) {
      arranca = false;
      Serial.print("La plaza está ");
      Serial.println(plazaOcupada ? "OCUPADA" : "LIBRE");
      estadoAnte = !plazaOcupada;
    }
    //  Comienza el monitoreo de la plaza y la comunicación via SMSs
    if (plazaOcupada == estadoAnte) {
      confirmacion = 0;
      Serial.print("La plaza sigue ");
      Serial.println(plazaOcupada ? "OCUPADA" : "LIBRE");
    } else {
      Serial.print("La plaza ahora ");
      Serial.println(plazaOcupada ? "se OCUPÓ" : "esta LIBRE");
      confirmacion ++;
      //  Solo se confirma cuando por NUM_CONFIRMA veces 
      if(confirmacion == NUM_CONFIRMA){
        //  El Modem GSM puede tardar mas de 25 segundos en arrancar
        beginGSM();               //  Enciende el módulo A7670E y espera por el
        initGSM();                //  Configuración inicial del módulo
        
        Serial.print("***********************  COMIENZA  ***********************");
        Serial.println("");

        //  Se envían los SMS
        for(int i = 0; i < numCli; i++){
          //  Se ensambla el mensaje saliente
          SMS = "Hola  ";
          SMS += registros[i].nombre;
          SMS += "\nPlaza ";
          SMS += plazaOcupada ? "OCUPADA\n" : "LIBRE\n";
          SMS += rtc.getTime("%B %d %H:%M:%S\n");
          //Serial.println(SMS);
          sendSMS(registros[i].nombre, registros[i].movil, SMS);
          miDelay(3);
        }
        Serial.print("***********************  TERMINA  ***********************");
        Serial.println("");

        //  Desactivar el módulo A7670E para ahorrar batería
        Serial.println("Módulo A7670E desactivado");
        digitalWrite(MOSFET, LOW);

        //  Cambia el estadoAnterior a el estado actual
        estadoAnte = plazaOcupada;
      }
    }

  }
}
