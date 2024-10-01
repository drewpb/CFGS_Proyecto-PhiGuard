#include "Arduino.h"
//#include <RH_ASK.h>
//#include <SPI.h> 
#include <StringSplitter.h>
#include "PCF8575.h"

PCF8575 PCF(0x21);

///==============================================================================///
///=============================     VARIABLES     ==============================///
///==============================================================================///
struct DriverTB6612FNG_14{
  const uint8_t pwma_4;
  const uint8_t ain2_4;
  const uint8_t ain1_4;
  const uint8_t bin1_1;
  const uint8_t bin2_1;
  const uint8_t pwmb_1;
};  DriverTB6612FNG_14 driver_14 = {11, 17 ,16, 15, 14, 10}; 
///
struct DriverTB6612FNG_23{
  const uint8_t pwma_3;
  const uint8_t ain2_3;
  const uint8_t ain1_3;
  const uint8_t bin1_2;
  const uint8_t bin2_2;
  const uint8_t pwmb_2;
};  DriverTB6612FNG_23 driver_23 = {9, 8, 7, 5, 4, 3};
///


///==============================================================================///
///=============================     FUNCIONES     ==============================///
///==============================================================================///
/// RUTINA DE INTERRUPCIÖN
const uint8_t IRQPIN = 2;
volatile bool irq_flag = false;

void pcf_irq(){
  irq_flag = true;
}
///
void ledControl(){
  for (int i=0; i<=6; i++){
    digitalWrite(LED_BUILTIN,1);
    delay(160);
    digitalWrite(LED_BUILTIN,0);
    delay(160);
  }
}
///
void pinmodeDrivers(){
  pinMode( driver_14.pwma_4, OUTPUT);
  pinMode( driver_14.ain2_4, OUTPUT);
  pinMode( driver_14.ain1_4, OUTPUT);
  pinMode( driver_14.bin1_1, OUTPUT);
  pinMode( driver_14.bin2_1, OUTPUT);
  pinMode( driver_14.pwmb_1, OUTPUT);
  
  pinMode( driver_23.pwma_3, OUTPUT);
  pinMode( driver_23.ain2_3, OUTPUT);
  pinMode( driver_23.ain1_3, OUTPUT);
  pinMode( driver_23.bin1_2, OUTPUT);
  pinMode( driver_23.bin2_2, OUTPUT);
  pinMode( driver_23.pwmb_2, OUTPUT);
}

///################################################################################################################################
///############ SENTIDOS DE GIRO POR RUEDA INDIVIDUAL #############################################################################
void sentPos(uint8_t num, uint8_t pot){ // Poner numero de rueda y potencia del motor(0-255)
  switch (num){
    case 1:
      digitalWrite(driver_14.bin1_1,1); digitalWrite(driver_14.bin2_1,0);
      analogWrite(driver_14.pwmb_1, pot); break;
    case 2:
      digitalWrite(driver_23.bin1_2,1); digitalWrite(driver_23.bin2_2,0);
      analogWrite(driver_23.pwmb_2, pot); break;
    case 3:
      digitalWrite(driver_23.ain1_3,1); digitalWrite(driver_23.ain2_3,0);
      analogWrite(driver_23.pwma_3, pot); break;
    case 4:
      digitalWrite(driver_14.ain1_4,1); digitalWrite(driver_14.ain2_4,0);
      analogWrite(driver_14.pwma_4, pot); break;
  };  return;
}
void sentNeg(uint8_t num, uint8_t pot){ // Poner numero de rueda y potencia del motor(0-255)
  switch (num){
    case 1:
      digitalWrite(driver_14.bin1_1,0); digitalWrite(driver_14.bin2_1,1);
      analogWrite(driver_14.pwmb_1, pot); break;
    case 2:
      digitalWrite(driver_23.bin1_2,0); digitalWrite(driver_23.bin2_2,1);
      analogWrite(driver_23.pwmb_2, pot); break;
    case 3:
      digitalWrite(driver_23.ain1_3,0); digitalWrite(driver_23.ain2_3,1);
      analogWrite(driver_23.pwma_3, pot); break;
    case 4:
      digitalWrite(driver_14.ain1_4,0); digitalWrite(driver_14.ain2_4,1);
      analogWrite(driver_14.pwma_4, pot); break;
  };  return;
}
///################################################################################################################################
///################################ SENSORES IR ###################################################################################
struct IrSensor{
    bool gpio0 = false;
    bool gpio2 = false;
    bool gpio4 = false;
    bool gpio6 = false;
    bool gpio8 = false;
    bool gpio10 = false;
    bool gpio12 = false;
    bool gpio14 = false;
}; IrSensor IrS;

///#################################################################################
///############################# DIRECCIONES POSIBLES ##############################
///------------------------------------------------------------------------------------------------------------------------------------------------------------------|
///------------------------------------------------------------------------------------------------------------------------------------------------------------------|
struct Direccion {
  uint8_t dir;
  bool sensor_dir_princ;
  bool sensor_dir_sec1;
  bool sensor_dir_sec2;
  bool sensor_dir_opuesto;
};

Direccion direcciones[10];

// Estructura para la tabla de asociación
struct Asociacion {
  const char* nombreVariable;
  bool* direccionVariable;
};

// Definición de la tabla de asociación
Asociacion tablaAsociacion[] = {
  {"IrS.gpio0", &IrS.gpio0},
  {"IrS.gpio2", &IrS.gpio2},
 {"IrS.gpio4", &IrS.gpio4},
  {"IrS.gpio6", &IrS.gpio6},
 {"IrS.gpio8", &IrS.gpio8},
  {"IrS.gpio10", &IrS.gpio10},
 {"IrS.gpio12", &IrS.gpio12},
  {"IrS.gpio14", &IrS.gpio14},
};

void asignarVariable(const char* nombreVariable, bool* direccionVariable) {
  for (const auto& asociacion : tablaAsociacion) {
    if (strcmp(asociacion.nombreVariable, nombreVariable) == 0) {
      *direccionVariable = *asociacion.direccionVariable;
      break;
    }
  }
}

void sensoresConfig() {
  /*
   * MOV HACIA DELANTE Y SUS SENSORES CORRESPONIDENTES
   */
  direcciones[0].dir = 31;
  asignarVariable("IrS.gpio0", &direcciones[0].sensor_dir_princ);
  asignarVariable("IrS.gpio14", &direcciones[0].sensor_dir_sec1);
  asignarVariable("IrS.gpio2", &direcciones[0].sensor_dir_sec2);
  asignarVariable("IrS.gpio8", &direcciones[0].sensor_dir_opuesto);
  /*
   * MOV HACIA ATRÁS Y SUS SENSORES CORRESPONIDENTES
   */
  direcciones[1].dir = 32;
  asignarVariable("IrS.gpio8", &direcciones[1].sensor_dir_princ);
  asignarVariable("IrS.gpio6", &direcciones[1].sensor_dir_sec1);
  asignarVariable("IrS.gpio10", &direcciones[1].sensor_dir_sec2);
  asignarVariable("IrS.gpio0", &direcciones[1].sensor_dir_opuesto);
  /*
   * MOV HACIA DERECHA Y SUS SENSORES CORRESPONIDENTES
   */
  direcciones[2].dir = 33;
  asignarVariable("IrS.gpio4", &direcciones[2].sensor_dir_princ);
  asignarVariable("IrS.gpio2", &direcciones[2].sensor_dir_sec1);
  asignarVariable("IrS.gpio6", &direcciones[2].sensor_dir_sec2);
  asignarVariable("IrS.gpio12", &direcciones[2].sensor_dir_opuesto);
  /*
   * MOV HACIA IZQUIERDA Y SUS SENSORES CORRESPONIDENTES
   */
  direcciones[3].dir = 34;
  asignarVariable("IrS.gpio12", &direcciones[3].sensor_dir_princ);
  asignarVariable("IrS.gpio14", &direcciones[3].sensor_dir_sec1);
  asignarVariable("IrS.gpio10", &direcciones[3].sensor_dir_sec2);
  asignarVariable("IrS.gpio4", &direcciones[3].sensor_dir_opuesto);
  /*
   * MOV HACIA DIAG.1 Y SUS SENSORES CORRESPONIDENTES
   */
  direcciones[4].dir = 51;
  asignarVariable("IrS.gpio2", &direcciones[4].sensor_dir_princ);
  asignarVariable("IrS.gpio0", &direcciones[4].sensor_dir_sec1);
  asignarVariable("IrS.gpio4", &direcciones[4].sensor_dir_sec2);
  asignarVariable("IrS.gpio10", &direcciones[4].sensor_dir_opuesto);
  /*
   * MOV HACIA DIAG.2 Y SUS SENSORES CORRESPONIDENTES
   */
  direcciones[5].dir = 52;
  asignarVariable("IrS.gpio6", &direcciones[5].sensor_dir_princ);
  asignarVariable("IrS.gpio4", &direcciones[5].sensor_dir_sec1);
  asignarVariable("IrS.gpio8", &direcciones[5].sensor_dir_sec2);
  asignarVariable("IrS.gpio14", &direcciones[5].sensor_dir_opuesto);
  /*
   * MOV HACIA DIAG.3 Y SUS SENSORES CORRESPONIDENTES
   */
  direcciones[6].dir = 53;
  asignarVariable("IrS.gpio10", &direcciones[6].sensor_dir_princ);
  asignarVariable("IrS.gpio12", &direcciones[6].sensor_dir_sec1);
  asignarVariable("IrS.gpio8", &direcciones[6].sensor_dir_sec2);
  asignarVariable("IrS.gpio2", &direcciones[6].sensor_dir_opuesto);
  /*
   * MOV HACIA DIAG.4 Y SUS SENSORES CORRESPONIDENTES
   */
  direcciones[7].dir = 54;
  asignarVariable("IrS.gpio14", &direcciones[7].sensor_dir_princ);
  asignarVariable("IrS.gpio12", &direcciones[7].sensor_dir_sec1);
  asignarVariable("IrS.gpio0", &direcciones[7].sensor_dir_sec2);
  asignarVariable("IrS.gpio6", &direcciones[7].sensor_dir_opuesto);
  /*
   * MOV ROT ++ Y SUS SENSORES CORRESPONIDENTES, EN ESTE CASO LOS SENSORES SECUNDARIOS 
   */
  direcciones[8].dir = 41;
  asignarVariable("IrS.gpio2", &direcciones[8].sensor_dir_sec1);
  asignarVariable("IrS.gpio10", &direcciones[8].sensor_dir_sec2);
  /*
   * MOV ROT -- Y SUS SENSORES CORRESPONIDENTES, EN ESTE CASO LOS SENSORES SECUNDARIOS 
   */
  direcciones[9].dir = 42;
  asignarVariable("IrS.gpio14", &direcciones[9].sensor_dir_sec1);
  asignarVariable("IrS.gpio6", &direcciones[9].sensor_dir_sec2);
  // Resto del código para las demás direcciones...
}

uint8_t ejecutarDirecciones(uint8_t x_dir){
  uint8_t y_dir = 101; //CODIGO POR DEFECTO SI NO REGISTRA NINGUN SENSOR ACTIVO
    // Acceder a los elementos de la estructura
    
    uint8_t dir = direcciones[x_dir].dir;
    bool sensor_dir_princ = direcciones[x_dir].sensor_dir_princ;
    bool sensor_dir_sec1 = direcciones[x_dir].sensor_dir_sec1;
    bool sensor_dir_sec2 = direcciones[x_dir].sensor_dir_sec2;
    bool sensor_dir_opuesto = direcciones[x_dir].sensor_dir_opuesto;
        
    // Imprimir los valores en el monitor serial
    /*Serial.print("Direccion ");
    Serial.print(x_dir);
    Serial.print(": ");
    Serial.print("\t dir: "); Serial.println(dir);
    Serial.print("\t sensor_dir_princ: "); Serial.println(sensor_dir_princ);
    Serial.print("\t sensor_dir_sec1: "); Serial.println(sensor_dir_sec1);
    Serial.print("\t sensor_dir_sec2: "); Serial.println(sensor_dir_sec2);
    Serial.print("\t sensor_dir_opuesto: "); Serial.println(sensor_dir_opuesto);
    Serial.println("================================\t");*/
    if (x_dir == 0 and sensor_dir_princ != true){ 
      y_dir = 0;
      //Serial.println("PA'LANTE Q VOYY");
    } else if (x_dir == 1 and sensor_dir_princ != true){ 
      y_dir = 1;
      //Serial.println("PA'TRAS Q VOYY");
    } else if (x_dir == 2 and sensor_dir_princ != true){ 
      y_dir = 2;
      //Serial.println("PA'DER Q VOYY");
    } else if (x_dir == 3 and sensor_dir_princ != true){ 
      y_dir = 3;
      //Serial.println("PA'IZQ Q VOYY");
    } else if (x_dir == 4 and sensor_dir_princ != true){ 
      y_dir = 4;
      //Serial.println("PA'DIAG-1 Q VOYY");
    } else if (x_dir == 5 and sensor_dir_princ != true){ 
      y_dir = 5;
      //Serial.println("PA'DIAG-2 Q VOYY");
    } else if (x_dir == 6 and sensor_dir_princ != true){ 
      y_dir = 6;
      //Serial.println("PA'DIAG-3 Q VOYY");
    } else if (x_dir == 7 and sensor_dir_princ != true){ 
      y_dir = 7;
      //Serial.println("PA'DIAG-4 Q VOYY");
    }
    //Serial.println("\t================================");
    return y_dir;
}
///------------------------------------------------------------------------------------------------------------------------------------------------------------------|
///------------------------------------------------------------------------------------------------------------------------------------------------------------------|

struct MovAuto{
  uint8_t lastMov_codex;
  uint8_t actualMov_codex;
  bool actualMov_sensor;
  uint8_t newMov_codex;
  bool newMov_sensor;
};

void funcionGeneralIR(){
  struct MovAuto mvautoData;
    uint8_t x_dir = 0;
// while(true)
  if (irq_flag) {
    //Serial.print("\t===\t===\t"); Serial.println(irq_flag);
    irq_flag = false;
    uint16_t x = PCF.read16();
    Serial.print("X: "); Serial.println(x);
    // Comprobar qué sensor se activó
    if ((x & (1 << 0)) == 0) {
      Serial.println("Sensor 0 activado");
      IrS.gpio0 = true;
    } else { IrS.gpio0 = false; }
    if ((x & (1 << 2)) == 0) {
      Serial.println("Sensor 2 activado");
      IrS.gpio2 = true;
    } else { IrS.gpio2 = false; }
    if ((x & (1 << 4)) == 0) {
      Serial.println("Sensor 4 activado");
      IrS.gpio4 = true;
    } else { IrS.gpio4 = false; }
    if ((x & (1 << 6)) == 0) {
      Serial.println("Sensor 6 activado");
      IrS.gpio6 = true;
    } else { IrS.gpio6 = false; }
    if ((x & (1 << 8)) == 0) {
      Serial.println("Sensor 8 activado");
      IrS.gpio8 = true;
    } else { IrS.gpio8 = false; }
    if ((x & (1 << 10)) == 0) {
      Serial.println("Sensor 10 activado");
      IrS.gpio10 = true;
    } else { IrS.gpio10 = false; }
    if ((x & (1 << 12)) == 0) {
      Serial.println("Sensor 12 activado");
      IrS.gpio12 = true;
    } else { IrS.gpio12 = false; }
    if ((x & (1 << 14)) == 0) {
      Serial.println("Sensor 14 activado");
      IrS.gpio14 = true;
    } else { IrS.gpio14 = false; }
    
    //ejecutarDirecciones(0);
  } 

  /*
  
  if (newMov_sensor == false){
    execMov(newMove_codex);
  }
  */
  
  // Haz otras cosas aquí
  delay(12);
  }
///------------------------------------------------------------------------------------------------------------------------------------------------------------------
///------------------------------------------------------------------------------------------------------------------------------------------------------------------
void dirFront(uint8_t pot){
  sentPos(4, pot);  sentPos(2, pot);
  sentPos(3, pot);  sentPos(1, pot);
  return;
  }
void dirBack(uint8_t pot){
  sentNeg(4, pot);  sentNeg(2, pot);
  sentNeg(1, pot);  sentNeg(3, pot);
  return;
  }
void dirRight(uint8_t pot){
  sentNeg(1, pot);  sentPos(2, pot);
  sentPos(4, pot);  sentNeg(3, pot);
  return;
  }
void dirLeft(uint8_t pot){
  sentNeg(4, pot);  sentPos(3, pot);
  sentPos(1, pot);  sentNeg(2, pot);
  return;
  }
///-----------------------------------------------------------------------
void dirStop(uint8_t rueda){
  switch(rueda){
    case 0: //Para todas las ruedas
    digitalWrite(driver_14.bin1_1,0); digitalWrite(driver_14.bin2_1,0);
    digitalWrite(driver_23.bin1_2,0); digitalWrite(driver_23.bin2_2,0);
    digitalWrite(driver_23.ain1_3,0); digitalWrite(driver_23.ain2_3,0);
    digitalWrite(driver_14.ain1_4,0); digitalWrite(driver_14.ain2_4,0);
    break;
    case 1: //Para solo la rueda 1
    digitalWrite(driver_14.bin1_1,0); digitalWrite(driver_14.bin2_1,0); break;
    case 2: //Para solo la rueda 2
    digitalWrite(driver_23.bin1_2,0); digitalWrite(driver_23.bin2_2,0); break;
    case 3: //Para solo la rueda 3
    digitalWrite(driver_23.ain1_3,0); digitalWrite(driver_23.ain2_3,0); break;
    case 4: //Para solo la rueda 4
    digitalWrite(driver_14.ain1_4,0); digitalWrite(driver_14.ain2_4,0); break;
  };  return;
  }
///-----------------------------------------------------------------------
// Se mueve en la diagonal, en dirección a los cuadrantes; 1, 2, 3, 4 (sentido agujas del reloj) 
void dirDiag(uint8_t cuadrante, uint8_t pot){   
  //Serial.println("Entrado en dirDiag, cuadrante:  " + cuadrante);
  switch (cuadrante){
    case 1: //Primer cuadrante
    for (int i=1; i<=3; i+=2){  dirStop(i); }
    for (int i=2; i<=4; i+=2){  sentPos(i, pot); }
    break;
    ///
    case 2: //Segundo cuadrante
    for (int i=2; i<=4; i+=2){  dirStop(i); }
    for (int i=1; i<=3; i+=2){  sentNeg(i, pot);  }
    break;
    ///
    case 3: //Tercer cuadrante
    for (int i=1; i<=3; i+=2){  dirStop(i); }
    for (int i=4; i>=2; i-=2){  sentNeg(i, pot); }
    break;
    ///
    case 4: //Cuarto cuadrante
    for (int i=2; i<=4; i+=2){  dirStop(i); }
    for (int i=3; i>=1; i-=2){  sentPos(i, pot); }
    break;
  };  return;
}
///-----------------------------------------------------------------------
// Movimiento de rotacion en torno al eje central del coche
void dirRot(uint8_t sentido, uint8_t pot){
  //Serial.println("Entrado en dirRot, sentido: " + sentido);
  switch (sentido){
    case 1: //Sentido positivo de la as agujas del reloj
    for (int i=1; i<=2; i++){  sentNeg(i, pot); }
    for (int i=3; i<=4; i++){  sentPos(i, pot); }
    break;
    ///
    case 2: //Sentido negativo de la as agujas del reloj
    for (int i=4; i>=3; i--){  sentNeg(i, pot); }
    for (int i=2; i>=1; i--){  sentPos(i, pot); }
    break;
  };  return;
}
///-----------------------------------------------------------------------
// Movimiento de rotacion en torno a una rueda
void dirConcerWheel(uint8_t cuadrante, uint8_t pot){   
  switch (cuadrante){
    case 1: //Primer cuadrante
    for (int i=1; i<=2; i++){  dirStop(i); }
    for (int i=3; i<=4; i++){  sentNeg(i, pot); }
    break;
    ///
    case 2: //Segundo cuadrante
    for (int i=1; i<=2; i++){  dirStop(i); }
    for (int i=4; i>=3; i--){  sentPos(i, pot);  }
    break;
    ///
    case 3: //Tercer cuadrante
    for (int i=3; i<=4; i++){  dirStop(i); }
    for (int i=1; i<=2; i++){  sentPos(i, pot); }
    break;
    ///
    case 4: //Cuarto cuadrante
    for (int i=3; i<=4; i++){  dirStop(i); }
    for (int i=2; i>=1; i--){  sentNeg(i, pot); }
    break;
  };  return;
}
///-----------------------------------------------------------------------
// Movimiento de rotacion en torno a un eje
void dirConcerAxis(uint8_t eje, String sentido, uint8_t pot){  // Ejes; 41, 32
  switch (eje){
    case 41: // Eje delantero; 41
    for (int i=1; i<=4; i+=3){  dirStop(i); }
    if (sentido == "pos"){  sentPos(3, pot);  sentNeg(2, pot);  }
    else if (sentido == "neg"){ sentPos(2, pot);  sentNeg(3, pot);  }
    break;
    ///
    case 32: // Eje trasero; 32
    for (int i=2; i<=3; i++){  dirStop(i); }
    if (sentido == "pos"){  sentPos(4, pot);  sentNeg(1, pot);  }
    else if (sentido == "neg"){ sentPos(1, pot);  sentNeg(4, pot);  }
    break;
  };  return;
}
///#################################################################################
///########################### SECUENCIA DE MOVIMIENTO #############################
void secuenciaMov(){
  digitalWrite(LED_BUILTIN,1);
  dirRot(1, 230);
  delay(1618);
  dirStop(0);///-------
  delay(300);
  dirRot(2, 230);
  delay(1618);
  digitalWrite(LED_BUILTIN,0);

  dirStop(0);///---------------------------- ROTAR EN TORNO AL EJE CENTRAL
  delay(900);
  
  digitalWrite(LED_BUILTIN,1);
  dirFront(200);
  delay(1200);
  digitalWrite(LED_BUILTIN,0);
  
  dirStop(0); ///---------------------------- RECTO PALANTE
  delay(600);

  digitalWrite(LED_BUILTIN,1);
  dirLeft(200);
  delay(1200);
  digitalWrite(LED_BUILTIN,0);

  dirStop(0);///---------------------------- RECTO IZQUIERDA
  delay(600);
  
  digitalWrite(LED_BUILTIN,1);
  dirBack(200);
  delay(1200);
  digitalWrite(LED_BUILTIN,0);

  dirStop(0);///---------------------------- RECTO PATRAS
  delay(600);

  digitalWrite(LED_BUILTIN,1);
  dirRight(200);
  delay(1200);
  digitalWrite(LED_BUILTIN,0);

  dirStop(0);///---------------------------- RECTO DERECTA
  delay(600);
  
  digitalWrite(LED_BUILTIN,1);
  for (int i=1; i<5; i++){ 
    Serial.println(i); 
    dirDiag(i, 230);
    delay(1200);
    dirStop(0);
    delay(300);
    }  
  digitalWrite(LED_BUILTIN,0);
  
  dirStop(0);///---------------------------- CUATRO DIAGONALES
  delay(900);

  digitalWrite(LED_BUILTIN,1);
  for (int i=1; i<=2; i++){ 
    dirConcerWheel(i, 200);
    delay(1200);
    dirStop(0);
    delay(300);///-------
    }  
  for (int i=4; i>=3; i--){ 
    dirConcerWheel(i, 200);
    delay(1200);
    dirStop(0);
    delay(300);///-------
    }  
  digitalWrite(LED_BUILTIN,0);

  dirStop(0);///---------------------------- ROTAR EN TORNO AL EJE RUEDA
  delay(600);

  dirConcerAxis(41, "pos", 200);
  delay(1200);
  dirStop(0);
  delay(300);
  dirConcerAxis(32, "neg", 200);
  delay(1200);
  dirStop(0);
  delay(300);
  dirConcerAxis(41, "neg", 200);
  delay(1200);
  dirStop(0);
  delay(300);
  dirConcerAxis(32, "pos", 200);
  delay(1200);
  
  dirStop(0);///---------------------------- ROTAR EN TORNO AL EJE VERTICAL 
  delay(900);

  ledControl();
}

///#################################################################################
///########################### RECEPCIÓN RF: MOVIMIENTO ############################
//RH_ASK askRx; // Crear un objeto de la clase ASK
///
struct MovVelVar{
  uint8_t mov = 0;
  uint8_t vel = 0;
  bool ok = false;
}; 


///#################################################################################
///##################### MMOVIMIENTOS ORDENADOS: TECLADO Y RPI #####################
void ejecutarMovCodex(uint8_t codex_mov, uint8_t vel_ = 6){
  uint8_t vel = ((vel_ - 6) * (250 - 90) / (10 - 6)) + 90;
  Serial.print("VEL: "); Serial.println(vel);
  Serial.print("DIR: "); Serial.println(codex_mov);
  switch (codex_mov){
      case 20: // NO MOV.
      dirStop(0); break;
      case 31: // MOV. DELANTERO
      dirFront(vel); break; 
      case 32: // MOV. TRASERO
      dirBack(vel); break; 
      case 33: // MOV. DERECHA
      dirRight(vel); break; 
      case 34: // MOV. IZQUIERDA
      dirLeft(vel); break;       
      case 41: // ROT. EJE CENTRO DEL COCHE EN SENTIDO POSITIVO
      dirRot(1, vel); break;
      case 42: // ROT. EJE CENTRO DEL COCHE EN SENTIDO NEGATIVO
      dirRot(2, vel); break;
      case 51: // MOV. DIAGONAL NOR-ESTE
      dirDiag(1, vel); break;
      case 52: // MOV. DIAGONAL SUR-ESTE
      dirDiag(2, vel); break;
      case 53: // MOV. DIAGONAL SUR-OESTE
      dirDiag(3, vel); break;
      case 54: // MOV. DIAGONAL NOR-OESTE
      dirDiag(4, vel); break;
      /// ----------------- MOVIMIENTOS ESPECIALES ----------------- ///
      case 61: // ROT. EJE RUEDA 1 DEL COCHE
        //Serial.print("=========== ROT RUEDA 1 === | VEL: ");
        //Serial.println(vel);
        break;
      case 62: // ROT. EJE RUEDA 2 DEL COCHE
        //Serial.print("=========== ROT RUEDA 2 === | VEL: ");
        //Serial.println(vel);
        break;
      case 63: // ROT. EJE RUEDA 3 DEL COCHE
        //Serial.print("=========== ROT RUEDA 3 === | VEL: ");
        //Serial.println(vel);
        break;
      case 64: // ROT. EJE RUEDA 4 DEL COCHE
        //Serial.print("=========== ROT RUEDA 4 === | VEL: ");
        //Serial.println(vel);
        break;
      case 71: // ROT. EJE DELANTERO SENTIDO POSITIVO
        //Serial.print("=========== ROT. EJE DEL. POSITIVO === | VEL: ");
        //Serial.println(vel);
        break;
      case 72: // ROT. EJE DELANTERO SENTIDO NEGATIVO
        //Serial.print("=========== ROT. EJE DEL. NEGATIVO === | VEL: ");
        //Serial.println(vel);
        break;
      case 73: // ROT. EJE TRASERO SENTIDO POSITIVO
        //Serial.print("=========== ROT. EJE TRAS. POSITIVO === | VEL: ");
        //Serial.println(vel);
        break;
      case 74: // ROT. EJE TRASERO SENTIDO NEGATIVO
        //Serial.print("=========== ROT. EJE TRAS. NEGATIVO === | VEL: ");
        //Serial.println(vel);
        break;
        };  
}


/*
void ejecutarMov(){
  MovVelVar data = recibirMensaje();
  if (data.ok == true){
         
    }
    delay(24);
}
*/

///#################################################################################
///#################################################################################
///#################################################################################
/* ================================================================================================================================= 
   ===============================================  FUNCIÓN GENERAL DEL MOVIMIENTO AUTONOMO ======================================== 
   ================================================================================================================================= */
   
void funcionGeneralMovManual(uint16_t numero){
      char DataCode_M[10] = "";
      int8_t indiceCoincidencia = -1;
      Serial.println("EJECUTANDO EL MODO MANUAL");
      bool sensor_dir_princ;
      uint8_t vel = numero / 100;
      uint8_t dir = numero - (vel * 100);
      if (dir == 41 or dir == 42){
         ejecutarMovCodex(dir, 7); delay(500); ejecutarMovCodex(20);
      } else {
      indiceCoincidencia = buscarCoincidenciaDir(dir);
      if (indiceCoincidencia == -1 and dir == 20){ ejecutarMovCodex(20); }
      else if (indiceCoincidencia == -1 and dir != 20){
        Serial.println("¡¡¡ indiceCoincidencia == -1 and dir != 20 !!!");
      } else if (indiceCoincidencia != -1){
        funcionGeneralIR();
        sensoresConfig();
        if (direcciones[indiceCoincidencia].sensor_dir_princ == false){
        uint16_t num2; strcpy(DataCode_M, "");
        Serial.println("");
        ejecutarMovCodex(dir, vel);
        //funcionVerificacion(indiceCoincidencia);
        char number2;
        while (direcciones[indiceCoincidencia].sensor_dir_princ != true){
          funcionGeneralIR();
          sensoresConfig();
          if (Serial.available()){ 
            number2 = Serial.read(); 
            if (number2 == '\n'){
              num2 = static_cast<uint16_t>(strtoul(DataCode_M, NULL, 10));
              strcpy(DataCode_M, "");
              if (num2 != numero){ Serial.print("¡¡!! CAMBIANDO A DIR:"); Serial.println(num2);
                if (num2 == 20){
                  ejecutarMovCodex(20);
                }
              break; 
              }
              
            } else { strncat(DataCode_M, &number2, 1); }
          }
          delay(6);
        }
        }
        if (direcciones[indiceCoincidencia].sensor_dir_princ == true){
          ejecutarMovCodex(20);
          Serial.println("Parando Stop");
        } else{ delay(1); }
      }
    }
}

/* ================================================================================================================================= 
   ================================================================================================================================= 
   ================================================================================================================================= */
   
/* ================================================================================================================================= 
   ===============================================  FUNCIÓN GENERAL DEL MOVIMIENTO AUTONOMO ======================================== 
   ================================================================================================================================= */
void salidasLaterales(uint8_t xy_dir){
  funcionGeneralIR();
  sensoresConfig();
  bool sensor_dir_princ = direcciones[xy_dir].sensor_dir_princ;
  bool sensor_dir_princ_izq = direcciones[3].sensor_dir_princ;
  bool sensor_dir_princ_der = direcciones[2].sensor_dir_princ;
  ejecutarMovCodex(20);
  if (sensor_dir_princ == true){
    while (sensor_dir_princ_izq != true){
      funcionGeneralIR();
      sensoresConfig();
      sensor_dir_princ_izq = direcciones[3].sensor_dir_princ;
      sensor_dir_princ = direcciones[xy_dir].sensor_dir_princ;
      ejecutarMovCodex(34);
      delay(10);
      if (sensor_dir_princ == false){ break; }
    }
    if (sensor_dir_princ_izq == true){
      ejecutarMovCodex(20);
      while (sensor_dir_princ_der != true){
        funcionGeneralIR();
        sensoresConfig();
        sensor_dir_princ_der = direcciones[2].sensor_dir_princ;
        sensor_dir_princ = direcciones[xy_dir].sensor_dir_princ;
        ejecutarMovCodex(33);
        delay(10);
        if (sensor_dir_princ == false){ break; }
    } 
    if (sensor_dir_princ_der == true){
      //ejecutarMovCodex(34);
      //delay(300);
      while (sensor_dir_princ == true){
            funcionGeneralIR();
            sensoresConfig();
            sensor_dir_princ = direcciones[xy_dir].sensor_dir_princ;
            ejecutarMovCodex(42); 
            delay(160*6); }
    }
    }
  }
}
/* =============================================================== */

int8_t buscarCoincidenciaSensorSec(const bool* sensorDirSec) {
    for (int i = 0; i < 8; i++) {
      if (direcciones[i].sensor_dir_princ == *sensorDirSec) {
        return i;
      }
    }
  return -1; // Si no se encontró ninguna coincidencia
}
/* =============================================================== */
uint8_t lastDir;

void funcionGeneralMovAutonomo(){
  Serial.println("============================================= FLAG_0"); 
  struct MovAuto mvautoData;
  int8_t ajus_sens_sec1;
  int8_t ajus_sens_sec2;
  uint8_t dir;
  bool sensor_dir_princ;
  bool sensor_dir_sec1;
  bool sensor_dir_sec2;
  bool sensor_dir_opuesto; 
  uint8_t xy_dir;
  char DataCode_1[10] = ""; 
  funcionGeneralIR();
  sensoresConfig();
  
  for (uint8_t x_dir=0; x_dir<=7; x_dir++){ 
    uint8_t y_dir = ejecutarDirecciones(x_dir);     /*  DEVUELVE LA DIRECCIÓN LIBRE DE OBSTACULOS  */
    //Serial.print("Y_DIR: "); Serial.println(y_dir);
    if (y_dir == 101) {     /*  SI LA DIRECCION DEVUELTA ES EL CÓDIGO 101, LOS 8 SENSORES ESTÁN BLOQUEADOS  */
      Serial.println("NO PUEDO MOVERMEE");
    } else if(y_dir == x_dir){      /*  SI EL NUMERO DE LA DIRRECIÓN SOLICITADA COINCIDE CON LA DEVUELTA*/
      Serial.print("SALIENDO POR DIRECCION - Y_DIR: "); Serial.println(y_dir);      /*  SE ACTIVA LA DIRECCIÓN LIBRE DE OBSTACULOS  */
      xy_dir = y_dir;     /*  SE GUARDA LA DIRECCIÓN LIBRE DE OBSTACULOS  */
      dir = direcciones[x_dir].dir;     /*  SE ACTUALIZA LA EL 'CÓDIGO DE DIRECCIÓN' LIBRE DE OBSTACULOS  */
      sensor_dir_princ = direcciones[x_dir].sensor_dir_princ;     /*  SE ACTUALIZA LA LECTURA DEL 'SENSOR PRINCIPAL' DE DIRECCIÓN LIBRE DE OBSTACULOS  */
      sensor_dir_sec1 = direcciones[x_dir].sensor_dir_sec1;     /*  SE ACTUALIZA LA LECTURA DEL 'SENSOR SEC-1' DE DIRECCIÓN LIBRE DE OBSTACULOS  */
      sensor_dir_sec2 = direcciones[x_dir].sensor_dir_sec2;     /*  SE ACTUALIZA LA LECTURA DEL 'SENSOR SEC-2' DE DIRECCIÓN LIBRE DE OBSTACULOS  */
      sensor_dir_opuesto = direcciones[x_dir].sensor_dir_opuesto;     /*  SE ACTUALIZA LA LECTURA DEL 'SENSOR OPUESTO' A LA DIRECCIÓN LIBRE DE OBSTACULOS  */
      //mvautoData.actualMov_codex = 
      if (x_dir == 0){
        ajus_sens_sec2 = +3;
        ajus_sens_sec1 = -3;
      } else if (x_dir == 1){
        ajus_sens_sec2 = +1;
        ajus_sens_sec1 = -1;
      }
      break;
    }
  } 
  while(sensor_dir_princ != true){   /*  SE VERIFICA QUE EL SENSOR PRINCIPAL NO SE ACTIVE */
    if (Serial.available()){   /*  LEEMOS EL SERIAL POR SI SE CAMBIA A MODO MANUAL  */
          uint16_t numero; 
          char number = Serial.read(); 
          Serial.print("FLAG_SERIAL, NUMBER:"); Serial.println(number);
          if (number == '\n'){
            numero = static_cast<uint16_t>(strtoul(DataCode_1, NULL, 10));
            Serial.print("FLAG_SERIAL, NUMERO:"); Serial.println(numero);
            strcpy(DataCode_1, "");
          } else { strncat(DataCode_1, &number, 1); }
           Serial.print("FLAG_SERIAL, DATACODE (after):");
           Serial.println(DataCode_1);
        if (numero <= 2222 and numero > 0){Serial.print("FLAG_SERIAL-SALIENDO, NUMERO:"); Serial.println(numero); break; }   /*  SI LLEGA UN CODIGO MENOR DE 2222, SE ACABA EL MODO AUTO */
        } 
    funcionGeneralIR();
    sensoresConfig();
    dir = direcciones[xy_dir].dir;
    sensor_dir_princ = direcciones[xy_dir].sensor_dir_princ;
    sensor_dir_sec1 = direcciones[xy_dir].sensor_dir_sec1;
    sensor_dir_sec2 = direcciones[xy_dir].sensor_dir_sec2;
    sensor_dir_opuesto = direcciones[xy_dir].sensor_dir_opuesto;
    if (sensor_dir_sec1 == true or sensor_dir_sec2 == true){
      //Serial.print("@@@@@@@@@@@======@@@@@@@@@@");
      if (sensor_dir_sec1 == true and sensor_dir_sec2 == true){
        //Serial.println("MOV. ACTUAL FULL STOP IR SENSOR");
        //Serial.println("ACTIVANDO MOV. ROT");
        ejecutarMovCodex(41);
        delay(161);
        ejecutarMovCodex(20);
      } else if(sensor_dir_sec1 == true){     /* SI SE ACTIVA SOLO EL SENSOR SECUNDARIO SEC1 */
        const bool* direccionSec1 = &direcciones[xy_dir].sensor_dir_sec1;
        uint8_t indiceCoincidencia = buscarCoincidenciaSensorSec(direccionSec1);
        
        if (indiceCoincidencia != -1) {
          // Se encontró una coincidencia
          //Serial.print("Ejecutando la dirección : "); Serial.println(direcciones[indiceCoincidencia + ajus_sens_sec1].dir);
          ejecutarMovCodex(direcciones[indiceCoincidencia + ajus_sens_sec1].dir);
          
        } else {
          // No se encontró ninguna coincidencia
          //Serial.println("No se encontró ninguna coincidencia");
        }             
      } else if(sensor_dir_sec2 == true){     /* SI SE ACTIVA SOLO EL SENSOR SECUNDARIO SEC2 */
        const bool* direccionSec2 = &direcciones[xy_dir].sensor_dir_sec2;
        uint8_t indiceCoincidencia = buscarCoincidenciaSensorSec(direccionSec2);
        
        if (indiceCoincidencia != -1) {
          // Se encontró una coincidencia
          //Serial.print("Saliendo por la dirección : "); Serial.println(direcciones[indiceCoincidencia + ajus_sens_sec2].dir); 
          ejecutarMovCodex(direcciones[indiceCoincidencia + ajus_sens_sec2].dir);
        } else {
          // No se encontró ninguna coincidencia
          //Serial.println("No se encontró ninguna coincidencia");
        }
        
      }
    } else{
      Serial.print("MovAutonomo Direccion: "); Serial.println(dir);
      ejecutarMovCodex(dir);
    }
    
    delay(60);
  }
  salidasLaterales(xy_dir);
}
/* ================================================================================================================================= 
   ================================================================================================================================= 
   ================================================================================================================================= */
void setup() {
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  PCF.begin();
  pinMode(IRQPIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(IRQPIN), pcf_irq, FALLING);
  pinmodeDrivers();
  ledControl();
}

///#################################################################################
int8_t buscarCoincidenciaDir(uint8_t dir) {
    for (int i = 0; i < 8; i++) {
        if (dir == direcciones[i].dir) {
            return i;
        }
    }
    return -1; // Si no se encontró ninguna coincidencia
}


char DataCode[10] = ""; 
void loop() {
  uint16_t numero;
  funcionGeneralIR();
  sensoresConfig();
  char number;
  if (Serial.available()){
    //Serial.println("FLAG_1");
    number = Serial.read();
    if (number == '\n'){
      numero = static_cast<uint16_t>(strtoul(DataCode, NULL, 10));
      Serial.println(numero);
      strcpy(DataCode, "");
     if (numero == 3333){
      Serial.print("EJECUTANDO EL MOVIMIENTO AUTONOMO");
      funcionGeneralMovAutonomo();
    } else if (numero >= 19 and numero<= 1234) {
      Serial.print("EJECUTANDO EL MOVIMIENTO MANUAL");
      funcionGeneralMovManual(numero);
    } else if (numero == 4444){
      Serial.print("EJECUTANDO UNA SECUENCIA DE MOVIMIENTO");
      secuenciaMov();
    }
  } else { strncat(DataCode, &number, 1); }    // Añadir el nuevo caracter al conjunto existente
  }
  delay(18);
//funcionGeneralIR();
  //secuenciaMov();
}

///#######################################################################
///#######################################################################
