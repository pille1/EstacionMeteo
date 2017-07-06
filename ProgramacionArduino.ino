//-------------------------------------PROYECTO ESTACION METEOROLÓGICA-------------------------------------
//-----------------------------------Francisco Miguel Jiménez Fernández------------------------------------
//-----------------------------------------------01/06/2017------------------------------------------------

//--------------------------CARGAMOS LIBRERIAS Y ESTABLECEMOS CONSTANTES NECESARIAS------------------------------- 
//Para el módulo DHT22
//Librerias necesarias
#include <DHT.h>
#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321

//Para el módulo ethernet
//Librerias necesarias
#include <Ethernet.h>
#include <SPI.h>

// Para la pantalla 16x2
//Librerias necesarias
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x3F,16,2); //Direccion i2c de pantalla LCD

//Para el módulo BMP180
//Librerias necesarias
#include <Adafruit_BMP085.h>      //Cargamos la librería BMP085
Adafruit_BMP085 bmp;              //Iniciamos una instancia de la librería BMP085
 

//Conexión de módulo Ethernet
byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFF, 0xEE}; // Direccion MAC
byte ip[] = { 192,168,1,124 }; // Direccion IP del Arduino
byte server[] = { 192,168,1,123 }; // Direccion IP del servidor
EthernetClient client;

//Modulo DHT22 
const int DHTPin = 31;     // Pin al que lo conectamos
DHT dht(DHTPin, DHTTYPE);

//Módulo gotas lluvia
//Mínimas y máximas lecturas del sensor
const int sensorMin = 0;
const int sensorMax = 1024;

//--------------------------INICIAMOS LOS DISTINTOS MÓDULOS-------------------------- 
void setup() {
 Serial.begin(9600); //Monitor Serie
 Serial.println("Proyecto Miguel");
 dht.begin();
 //Iniciamos el fondo retroiluminado
  lcd.backlight();
  //Iniciamos la pantalla
  lcd.init();
 Ethernet.begin(mac, ip); // Inicializamos el Ethernet Shield
 delay(1000);
 if (!bmp.begin()) {  //Si hay un error al iniciar la librería BPM180
  Serial.println("No se puede iniciar el sensor BMP085, compruebe las conexiones!"); //Mostramos un mensaje
  while (1) {} //Detenemos la ejecución
  }
}

//--------------------------BUCLE DE EJECUCIÓN--------------------------
void loop() {
 // Esperar 2 segundos entre medidas
 delay(2000);
 //Limpiamos la LCD
 lcd.clear();
 //Código de ubicacion, pertenece al Puerto de Santa María
 //LO CAMBIARE CON UN BUCLE FOR PARA QUE CADA VEZ ENVIE DATOS A UNA UBICACION DISTINTA
 int ubicacion = 1;
 //Metemos en p la presión desde BMP180
 float p = bmp.readPressure()/100;
 //Metemos en h la humedad desde DHT22
 float h = dht.readHumidity();
 //Metemos en t la temperatura desde DHT22
 float t = dht.readTemperature();
 //Metemos en r la variable de lluvia de A0 (Sensor de lluvia)
 int r = analogRead(A0);

//Mapear el rango del sensor de lluvia (cuatro opciones):
  int rango= map(r, sensorMin, sensorMax, 0, 3);
  
//-------------------------------------ERRORES------------------------------------- 
 if (isnan(h) || isnan(t)) {
 Serial.println("Error al leer del sensor DHT");
 return;
 }
  if (isnan(p)) {
 Serial.println("Error al leer del sensor BMP180");
 return;
 }
  if (isnan(rango)) {
 Serial.println("Error al leer del sensor de lluvia");
 return;
 }
 
//-------------------------------------MOSTRAR DATOS POR SERIAL Y POR LCD-------------------------------------
//Las instrucciones Serial.print escriben por el monitor serie
//Las instrucciones lcd.print escriben en la pantalla LCD
 Serial.print("Humedad: ");
 Serial.print(h);
 lcd.print("Humedad: ");//Escribimos en la primera linea del LCD
 lcd.print(h);
 lcd.print(" %");
 lcd.setCursor(0,1); //Vamos a la linea de abajo del LCD
 Serial.println(" %\t");
 Serial.print("Temperatura: ");
 Serial.print(t);
 Serial.println(" *C ");
 lcd.print("Temp: ");//Escribimos en la segunda linea del LCD
 lcd.print(t);
 lcd.print(" C");
 delay(4000); // Esperamos antes de borrar
 lcd.clear(); //Limpiamos la pantalla LCD
 Serial.println("Presion: "); 
 Serial.print(p);
 Serial.println(" hPa ");
 lcd.print("Pres: ");//Escribimos en la primera linea del LCD
 lcd.print(p);
 lcd.print(" hPa");
 lcd.setCursor(0,1); //Vamos a la linea de abajo del LCD
 // range value:
 switch (rango) {
 case 0:    // Sensor muy mojado
    Serial.println("Lluvia fuerte");
    lcd.print("Lluvia fuerte");
    break;
 case 1:    // Sensor mojándose
    Serial.println("Lluvia debil");
    lcd.print("Lluvia debil");
    break;
 case 2:    // Sensor seco.
    Serial.println("Seco");
    lcd.print("Seco");
    break;
  }
  
//-------------------------------------ENVÍO DE DATOS-------------------------------------
 Serial.println("Conectando...");
  if (client.connect(server, 80)>0) {  // Conexion con el servidor
    client.print("GET /EstacionMeteo/insertaMedidas.php?temperatura="); // Enviamos la temperatura por GET
    client.print(t);
    client.print("&humedad="); // Enviamos la humedad por GET
    client.print(h);
    client.print("&lluvia="); // Enviamos si llueve o no por GET
    switch (rango) {
      case 0:    // Sensor muy mojado      
        client.print("Lluvia%20fuerte");
      break;
      case 1:    // Sensor mojándose
        client.print("Lluvia%20debil");
      break;
      case 2:    // Sensor seco.
        client.print("Seco");
      break;
    }
    client.print("&presion="); //Enviamos la presion por GET
    client.print(p);
    client.print("&ubicacion="); //Enviamos el idubicacion por GET (CAMBIANDO ESTE VALOR SIMULO LAS DISTINTAS UBICACIONES)
    //LO CAMBIARE CON UN BUCLE FOR PARA QUE CADA VEZ ENVIE DATOS A UNA UBICACION DISTINTA
    client.print(ubicacion);
    client.println(" HTTP/1.0");
    client.println("User-Agent: Arduino 1.0");
    client.println();
    Serial.println("Conectado");
  } else {
    Serial.println("Fallo en la conexion");
  }
  if (!client.connected()) {
    Serial.println("Desconectado!");
  }
  client.stop();
  client.flush();
  
}

