/*  
    ThermoBeer
    Copyright (C) 2016 Simeón Ruiz Romero <http://www.simeonruiz.es>

    ThermoBeer is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    ThermoBeer is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#include <OneWire.h>
#include <DallasTemperature.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <DHT.h>

#define ENVSENS 4   // Pin del sensor de ambiente
#define TEMPROBE 2  // Pin de la sonda de temperatura
#define TEMPSELECTOR A1 // Pin del potenciometro selector
#define TOLSELECTOR A0  // Pin del potenciometro tolerancia

#define HEATER 6
#define COOLER 5

#define DHTTYPE DHT11


//Variables and objects
LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);

OneWire beerProbeWire(TEMPROBE);

DallasTemperature beerProbe(&beerProbeWire);
DHT environmentSensor(ENVSENS, DHTTYPE);  // Declarar sensor ambiente
float envTemp;
float prevEnvTemp;
float beerTemp;
float prevBeerTemp;
int selectTemp;
int prevSelectTemp;
int selectTolerance;
int prevSelectTolerance;
int tempSelectorValue;
int tolerSelectorValue;
byte heating;
byte cooling;
byte prevCooling;
byte prevHeating;


//Functions

// Funcion para imprimir un texto en una posicion concreta del LCD
void printText(String text, int line, int column){
  lcd.setCursor(column,line); 
  lcd.print(text);
  
}



void setup() {
  delay(1000);
  environmentSensor.begin();
  beerProbe.begin();

  pinMode(HEATER, OUTPUT);
  pinMode(COOLER, OUTPUT);

  heating = 0;
  cooling = 0;
  lcd.begin(16,2);
  lcd.backlight();

  prevSelectTemp = 0;
  prevSelectTolerance = 0;
  prevEnvTemp = 0;
  prevBeerTemp = 0;
  prevHeating = 0;
  prevCooling = 0;
  delay(1000);
}

void loop() {

  //leemos los valores de los potenciometros.
  tempSelectorValue = analogRead(TEMPSELECTOR);
  tolerSelectorValue = analogRead(TOLSELECTOR);

  //puesto que los valores de los potenciometros se corresponderan a un nivel de resolucion de la entrada analogica
  //los mapeamos al rango que deseamos (entre 8 y 26 para la seleccion de temperatura y 0 y 2 para el margen)
  selectTemp = map(tempSelectorValue, 0, 1023, 8, 26);
  selectTolerance = map (tolerSelectorValue, 0, 1023, 0, 2);

  //Obtenemos las temperaturas.
  envTemp = environmentSensor.readTemperature();
  beerProbe.requestTemperatures();
  beerTemp = beerProbe.getTempCByIndex(0);

  //MOSTRAR TODAS LAS TEMPERATURAS


  
  if(beerTemp >= selectTemp + selectTolerance){
    //Si hemos alcanzado la temperatura deseada (más el margen seleccionado)
    if(heating == 1){
      //Si ya estabamos calentando, paramos
      heating = 0;
    }else{
      //Si no estabamos calentando, es porque fuera hace mucho calor;
      //Enfriamos el ambiente
      cooling = 1;
      heating = 0;
    }
  }

  if (beerTemp < selectTemp - selectTolerance){
    //Si hemos bajado de la temperatura deseada (más el margen seleccionado, esta vez por debajo)
    if(cooling == 1){
      //Si ya estabamos enfriando, paramos
      cooling = 0;
    }else{
      //Si no estabamos enfriando, es porque fuera hace mucho frio;
      //Calentamos el ambiente
      heating = 1;
      cooling = 0;
    }
  }



  //Comprobamos si algun valor ha sido modificado respecto a la vuelta anterior; esto se hace para no refrescar el lcd ni modificar las salidas de los reles
  //a cada vuelta del programa para evitar el "titileo" del lcd y posibles problemas en los reles (aunque esto último es muy poco propenso a suceder por culpa del refresco)
  if( envTemp != prevEnvTemp || beerTemp != prevBeerTemp || selectTemp != prevSelectTemp || selectTolerance != prevSelectTolerance || prevCooling != cooling || prevHeating != heating){

    //Si hemos parado o comenzado a calentar, actualizamos la salida del calentador
    if (heating != prevHeating){
      digitalWrite(HEATER, heating);
    }

    //Si hemos parado o comenzado a enfriar, actualizamos la salida del enfriador
    if (cooling != prevCooling){
      digitalWrite(COOLER, cooling);
    }

    //Actualizamos las variables previas para la siguiente vuelta
    prevEnvTemp = envTemp;
    prevBeerTemp = beerTemp;
    prevSelectTemp = selectTemp;
    prevSelectTolerance = selectTolerance;
    prevHeating = heating;
    prevCooling = cooling;
    
    //Si llegamos a este punto es por que algo ha cambiado, debemos cambiar el contenido del lcd
    lcd.clear();
    
    //Mostrar seleccionada
    //(char)223 imprime el caracter º al lado de la C mayuscula
    printText(String(selectTemp) +(char)223 + "C", 1, 0);
    
    //Mostrar tolerancia seleccionada
    printText(String(selectTolerance) +(char)223 + "C", 1, 6); 
    //Mostrar temperatura ambiente
    printText(String(envTemp) +(char)223 + "C",0, 0);
    //Mostrar sonda
    printText(String(beerTemp) +(char)223 + "C",0, 9);


    //Indicar si estamos enfriando o calentando, para saber que relé está conectado.
    //Esto es en caso de que no tengamos ningún led para indicarlo.
    if (heating == 1){
      printText("Cal.", 1, 12);
    }
    if (cooling == 2){
      printText("Enf.", 1, 12);
    }
    
  }

  
  
}
