#include <Wire.h>
#include <LiquidCrystal_I2C.h> //Importar librerias para la pantalla LCD
#include <SPI.h>

LiquidCrystal_I2C lcd(0x27,16,2);


int count = 0; //Almacena el numero de cortes del haz de luz
int threshold = 20; //Umbral de detección de la radiación IR (sensibilidad de los sensores)
long runTime = 0; //Registra la duración de la prueba actual
bool start = false;
long standbyTime = 0;
bool frontStart = true;

const long SYNC_INTERVAL = 40000; //Milisegundos entre llamadas a la funcion flush()

String Sensor; //Almacena el valor de tipo 'string' del sensor actual
String Count; //Esta variable almacena el numero de cortes del haz de luz con tipo de dato 'string'

int minutes;
int seconds;
int errors = false;

//Define el tamaño de la matriz que almacena las asignaciones de pines para los sensores IR
const int rows = 2;
const int cols = 8;


int SensorMatrix[rows][cols] = {
//Esta matriz contendrá las asignaciones de pines para los sensores IR
	{0, 1, 2, 3, 4, 5, 6, 7},
	{8, 9, 10, 11, 12, 13, 14, 15}
};

int SensorStateMatrix[rows][cols] = {
//Esta matriz almacenará la lectura actual de cada sensor
	{0, 0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0, 0}
};

int lastSensorStateMatrix[rows][cols] = {
// Esta matriz almacenará la lectura anterior de cada sensor
	{0, 0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0, 0}
};

void setup() {
Serial.begin(9600); //Inicializa el monitor serial para escritura
initialize();
}

void(* resetFunc) (void) = 0;

void loop() {
	if (Serial.available()){
		char data = Serial.read();
		if (data == '0'){
			log_data(minutes, count);
			lcd.clear();
			lcd.setCursor(0,0);
			lcd.print("Tiempo: " + String(seconds) + "s");
			lcd.setCursor(0,1);
			lcd.print("Pasos: " + String(count));
			frontStart = false; //Indica que la prueba finalizo
			loopControl();
		}
	}
	scan_sensors(); //Llama a la funcion que escanea los sensores
	display(); //Llama a la funcion que muestra los resultados
}

//Esta funcion inicializa los sensores IR y la pantalla LCD
void initialize() {
//Inicializa la pantalla lcd para la visualización
	lcd.init();
	lcd.backlight();
	lcd.clear();
	lcd.print("Iniciando");
	delay(2000);
	lcd.clear();
	lcd.print("Esperando");
	
	loopControl();
	
//Prueba todos los sensores e informa un error si alguno de ellos no funciona
	lcd.clear();
	lcd.print("Probando");
	delay(2000);

	for(int i = 0; i < rows; i++){
		for(int j = 0; j < cols; j++){
			//Lee el valor actual del sensor correspondiente
			SensorStateMatrix[i][j] = analogRead(SensorMatrix[i][j]);
			//Si el valor del sensor (comprendido entre 0 y 1023) es menor que 'threshold' (umbral), cambia 'errors' a 'true'
			if (SensorStateMatrix[i][j] < threshold){
				//errors = true;
				delay(20);
				}
			}
		}
	if (errors) {
		notify("Sensores IR apagados"); //Muestra error si algun sensor falló
	}
	else {
		lcd.clear();
		lcd.print("Sensores IR OK");
		Serial.print("200");
		Serial.println();
		delay(2000);
	}
	lcd.clear();
	lcd.print("Configuracion completa");
	delay(2000);
	lcd.clear();
}

void scan_sensors(){
	//Esta función escaneará todos los sensores uno tras otro para detectar cortes del haz
	for(int i = 0; i < rows; i++){
		for(int j = 0; j < cols; j++){
			//Lee el valor actual del sensor correspondiente
			SensorStateMatrix[i][j] = analogRead(SensorMatrix[i][j]);
			//Si el haz es cortado (es decir, el valor es menor que el umbral), se incrementa la cuenta
			if (SensorStateMatrix[i][j] <= threshold && lastSensorStateMatrix[i][j] > threshold)
				count += 1;
			lastSensorStateMatrix[i][j] = SensorStateMatrix[i][j];
		delay(2);
		}
	}
	delay(50);
	return;
}

void display() {
	//Muestra los datos de la prueba en curso
	runTime = millis() - standbyTime;
	seconds = (runTime)/1000;
	lcd.setCursor(0,0);
	lcd.print("Tiempo: " + String(seconds) + " s");
	lcd.setCursor(0,1);
	lcd.print("Pasos:" + String(count));
	return;
}

void notify(char *str){
	Serial.println(str);
	lcd.clear();
	lcd.setCursor(0,0);
	lcd.print(str);
	while(1);
}

void log_data (int a, int b){
	Serial.print(millis() - standbyTime);
	Serial.print(", ");
	Serial.print(String(b));
	Serial.println();
}

void loopControl(){
	start = false;
	while (start == false){
		if (Serial.available()){
			char data = Serial.read();
			if (data == '1' && frontStart == true){
				start = true;
				standbyTime = millis();
			}
			if (data == '2')
				resetFunc();
		}
	}
}
