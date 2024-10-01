/*
Serial.begin()   //if using UART0
SerialPort.begin (BaudRate, SerialMode, RX_pin, TX_pin)
*/

#define RXD2 16
#define TXD2 17

#define RXD1 9
#define TXD1 10

unsigned long previousMillis = 0;
const unsigned long interval = 3000;

void setup()  
{
  Serial.begin(115200);
  Serial1.begin(115200, SERIAL_8N1, RXD1, TXD1);
  Serial2.begin(115200, SERIAL_8N1, RXD2, TXD2); 
  pinMode(LED_BUILTIN, OUTPUT);
}
float tiempo;
void loop()
{
  if (Serial2.available()) {
    Serial.print("Tiempo desdeel utlimo mensaje: "); Serial.println(millis()-previousMillis);
    char number = Serial2.read();
    Serial1.print(number);
    Serial.print(number);
    previousMillis = millis();
     }

 
 /*  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    // Enviar la cadena "_0_20_0_" al esclavo (Arduino Nano)
    SerialPort_1.print("_0_20_0_"); 
  }*/
  
}
