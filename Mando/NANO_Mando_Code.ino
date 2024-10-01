/**
 * Name:     ARDUINO NANO - KEYBOARD - MENUS_LCD
 * Autor:    ANDRÉS R. PHILIPPS BENÍTEZ
 * License:  CC BY-NC-SA 3.0
 * Date:     ABRIL/2022
 *
 * Arduino NANO, Pinout:
 *         _______________
 *        |      USB      |
 *        |13           12|
 *        |3V3          11|
 *        |AREF         10|
 *   APAD |A0            9| 
 *        |A1            8| 
 *        |A2            7| 
 *        |A3            6| 
 *        |A4            5| 
 *        |A5            4| 
 *        |          3/SCL|
 *        |          2/SDA|
 *        |5V          GND|
 *        |RST         RST|
 *        |GND   1/INT2/RX|
 *        |VIN   0/INT3/TX|
 *        |MISO         SS|
 *        |SCK        MOSI|
 *        |_______________|
 *
*/
 
 /**__________________________________________________________________________________________________________________________________________________________________________________________________________________
 *   ######   ---   ---   ---   ---   ---   ---   ---   ---   ---   ---   ---   ---   ---   ---   ---   ---   ---   ---   ---   ---  LIBRERIAS NECESARIAS PARA EL FUNCIONAMIENTO DEL CODIGO   ---   ---   ---   ######
 */

#include <LiquidCrystal_I2C.h>
#include<Keypad.h>
//#include <RH_ASK.h>
//#include <SPI.h>
#include <StringSplitter.h>
#include <SoftwareSerial.h>


/**_____________________________________________________________________________________________________________________________________________________________________________________
 *   ######   ---   ---   ---   ---   ---   ---   ---   ---   ---   ---   ---   ---   ---   ---   ---   ---   ---   ---   ---   ---  OBJETOS DE LAS LIBRERIAS   ---   ---   ---   ######
 */
 //SoftwareSerial xbee(0, 1); // Configuración de los pines RX y TX del módulo Xbee
 LiquidCrystal_I2C lcd(0x3F, 16, 2); // Configuracion del LCD: LiquidCrystal_I2C(lcd_Addr, lcd_cols, lcd_rows)
//RH_ASK askTx; // Crear un objeto de la clase ASK

/**_________________________________________________________________________________________________________________________________________________________________________________________________________________________________
 *   ######   ---   ---   ---   ---   ---   ---   ---   ---   ---   ---   ---   ---   ---   ---   ---   ---   ---   ---   ---   ---    MACROS, CONSTANTES, ENUMERADORES, ESTRUCTURAS Y VARIABLES GLOBALES   ---   ---   ---   ###### 
 */
////////////////////////////////////////////////////////////////////////////////////
#define pinLed  13
/// - TECLADO -   ------  ------  ------  ------  ------  ------
const byte pinesF[4] = {2,3,4,5};
const byte pinesC[6] = {6,7,8,9,10,11};
char teclas[4][6] = {
 {'1','2','3','@','<','>'},
 {'4','5','6','+','[',']'},
 {'7','8','9','-','!','?'},
 {'*','0','#','/','_',' '}  };
Keypad teclado = Keypad(makeKeymap(teclas), pinesF, pinesC, 4, 6);
char tecla; char palabra;
byte data_count = 0;
#define Code_Length 32
char Data[Code_Length]; 
/// --------------------------------------------------------------------------------------------
const byte flechaPos[8] PROGMEM = {  B00000, B00100, B01110, B10101, B00100, B00100, B00100, B00000  };
const byte flechaNeg[8] PROGMEM = {  B00000, B00100, B00100, B00100, B10101, B01110, B00100, B00000  };
const byte noMov[8] PROGMEM = {  B00000, B10001, B01010, B00100, B00100, B01010, B10001, B00000  };
/// --------------------------------------------------------------------------------------------



//String teclas;
/// - JOYSTICK -    ------  ------  ------  ------  ------  ------
#define pROTJSTICK  A1    // Pin analógico de la rotación del joystik
#define pBTTMJSTICK A0   // Pin analógico del pulsador del joystik
#define pEJEX       A3       // Pin analógico del eje vertical del joystik
#define pEJEY       A2       // Pin analogico del eje horizontal del joystik
enum Joystick{ Unknown, Ok, Up, Down, Left, Right, Back, Enter, Diag_1, Diag_2, Diag_3, Diag_4, Conc_1, Conc_2, Conc_3, Conc_4 } jystckPressed;

struct calibJoystick {
  uint16_t rot = 535;
  uint16_t ejey = 510;
  uint16_t ejex = 560;
}; calibJoystick calJoy;


/// - MENU Y SUBMENU -    ------  ------  ------  ------  ------  ------
#define ARRAYSIZE 16
/**_______________________________________________________________________________________________________________________________________________________________________________
 *  ######  ---   ---   ---   ---   ---   ---   ---   ---   ---   ---   ---   ---   ---   ---   ---   ---   ---   ---   ---   ---  FUNCIONES PERSONALES   ---   ---   ---   ######
 */
 ///
  /*----------------------------------------------------------------|
 *  ---   ---   ---   RELACIONADAS CON EL JOYSTICK   ---   ---   ---|
 */
//###################### --- LECTURA DEL JOYSTICK: QUÉ SE HA PULSADO --- ###########################    ------  ------  ------  ------  ------  ------
Joystick readJoystick(){
  uint16_t val_rotjoystick = analogRead(pROTJSTICK);
  uint16_t val_bttmjoystick = analogRead(pBTTMJSTICK);
  uint16_t val_ejey = analogRead(pEJEY);
  uint16_t val_ejex = analogRead(pEJEX);  
  /// 
  jystckPressed = Joystick::Unknown;
  /// MOVIMIENTO DE ROTACIÓN
  if (val_rotjoystick > (calJoy.rot + 200)){ jystckPressed = Joystick::Back; }
  else if (val_rotjoystick < (calJoy.rot - 200)){ jystckPressed = Joystick::Enter; }
  /// PULSACIÓN DE BOTÓN
  if (val_bttmjoystick == 0){ 
    if (val_ejex < (calJoy.ejex - 100) and val_ejey < (calJoy.ejey - 100)){  jystckPressed = Joystick::Conc_1; } // DERECHA + ARRIBA
    else if (val_ejex < (calJoy.ejex - 100) and val_ejey > (calJoy.ejey + 100)){  jystckPressed = Joystick::Conc_2; } // DERECHA + ABAJO
    else if (val_ejex > (calJoy.ejex + 100) and val_ejey > (calJoy.ejey + 100)){  jystckPressed = Joystick::Conc_3; } // IZQUIERDA + ABAJO
    else if (val_ejex > (calJoy.ejex + 100) and val_ejey < (calJoy.ejey - 100)){  jystckPressed = Joystick::Conc_4; } // IZQUIERDA + ARRIBA
    else { jystckPressed = Joystick::Ok; } 
    }
  /// MOVIMIENTO DEL EJE Y
  if (val_ejey > (calJoy.ejey + 120)){  jystckPressed = Joystick::Down; } /// 700
  else if (val_ejey < (calJoy.ejey - 120) ){  jystckPressed = Joystick::Up; } /// 370
  /// MOVIMIENT DEL EJE X
  if (val_ejex > (calJoy.ejex + 120)){  jystckPressed = Joystick::Left; }
  else if (val_ejex < (calJoy.ejex - 120) ){  jystckPressed = Joystick::Right;  }

  if (val_ejex < (calJoy.ejex - 100) and val_ejey < (calJoy.ejey - 100)){  jystckPressed = Joystick::Diag_1; } // DERECHA + ARRIBA
  else if (val_ejex < (calJoy.ejex - 100) and val_ejey > (calJoy.ejey + 100)){  jystckPressed = Joystick::Diag_2; } // DERECHA + ABAJO
  else if (val_ejex > (calJoy.ejex + 100) and val_ejey > (calJoy.ejey + 100)){  jystckPressed = Joystick::Diag_3; } // IZQUIERDA + ABAJO
  else if (val_ejex > (calJoy.ejex + 100) and val_ejey < (calJoy.ejey - 100)){  jystckPressed = Joystick::Diag_4; } // IZQUIERDA + ARRIBA
  /// DEVULEVE EL BOTON PRESIONADO
  return jystckPressed;
}
//###################### --- LECTURA DEL JOYSTICK: CON QUÉ POTENCIA --- ###########################   ------  ------  ------  ------  ------  ------
void readJoysitickPot(){
  uint8_t vel = 0; uint8_t vel_last = 0;
  uint8_t mov; uint8_t mov_last;
  bool cam_mode = false; uint8_t pan = 0; uint8_t til = 0; uint8_t pan_last = 0; uint8_t til_last = 0;
  //uint16_t count = 0;
  byte textPos[8]; byte textNeg[8]; byte textNoMov[8];
  memcpy_P(textPos, flechaPos, 8);  memcpy_P(textNeg, flechaNeg, 8);  memcpy_P(textNoMov, noMov, 8);
  while (true){
    //vel = 0; mov = "";
    //count++;
  jystckPressed = readJoystick();  
  if (cam_mode == false){
    if (jystckPressed == Joystick::Ok) {
    lcd.setCursor(2,0); lcd.print("OK >>   ");
   }  /// ----------------------------------------------------------------------- 
     else if( jystckPressed == Joystick::Back){
    lcd.setCursor(2,0); lcd.print("ROT --     ");
    vel = (analogRead(pROTJSTICK)*10)/1023;
    mov = 42; // "RotNeg"
    lcd.setCursor(0,1); lcd.print(F("VELOCIDAD:")); lcd.print(vel);
    printSentidoRuedas(textPos, textPos, textNeg, textNeg, 13);
   } else if( jystckPressed == Joystick::Enter){
    lcd.setCursor(2,0); lcd.print("ROT ++     ");
    vel = 10-((analogRead(pROTJSTICK)*10)/1023);
    mov = 41; // "RotPos" 
    lcd.setCursor(0,1); lcd.print(F("VELOCIDAD:")); lcd.print(vel);
    printSentidoRuedas(textNeg, textNeg, textPos, textPos, 13);
   } else if( jystckPressed == Joystick::Up){
    printSentidoRuedas(textPos, textPos, textPos, textPos, 13);
    lcd.setCursor(2,0); lcd.print("UP ++      ");
    vel = 10-((analogRead(pEJEY)*10)/1023);
    mov = 31;  // "Up"
    lcd.setCursor(0,1); lcd.print(F("VELOCIDAD:")); lcd.print(vel);
   } else if( jystckPressed == Joystick::Down){
    printSentidoRuedas(textNeg, textNeg, textNeg, textNeg, 13);
    lcd.setCursor(2,0); lcd.print("DOWN --    ");
    vel = (analogRead(pEJEY)*10)/1023;
    mov = 32;  // "Down"
    lcd.setCursor(0,1); lcd.print(F("VELOCIDAD:")); lcd.print(vel);
   } else if( jystckPressed == Joystick::Right){
    printSentidoRuedas(textNeg, textPos, textNeg, textPos, 13);
    lcd.setCursor(2,0); lcd.print("RIGHT >>   ");
    vel = 10-((analogRead(pEJEX)*10)/1023);
    mov = 33; // "Der"
    lcd.setCursor(0,1); lcd.print(F("VELOCIDAD:")); lcd.print(vel);
   } else if( jystckPressed == Joystick::Left){
    printSentidoRuedas(textPos, textNeg, textPos, textNeg, 13);
    lcd.setCursor(2,0); lcd.print(" << LEFT   ");
    vel = (analogRead(pEJEX)*10)/1023;
    mov = 34; // "Izq"
    lcd.setCursor(0,1); lcd.print(F("VELOCIDAD:")); lcd.print(vel);
   }  /// ---------------------------------------------------------------------------
     else if( jystckPressed == Joystick::Diag_1){
    printSentidoRuedas(textNoMov, textPos, textNoMov, textPos, 13);
    lcd.setCursor(2,0); lcd.print(" DIAG 1    ");
    vel = ((analogRead(pEJEX)+analogRead(pEJEY))-1070.932)/-70.932;
    mov = 51; // "Diag1"
    lcd.setCursor(0,1); lcd.print(F("VELOCIDAD:"));
    lcd.setCursor(10,1); lcd.print(vel);
   } else if( jystckPressed == Joystick::Diag_2){
    printSentidoRuedas(textNeg, textNoMov, textNeg, textNoMov, 13);
    lcd.setCursor(2,0); lcd.print(" DIAG 2    ");
    vel = 6;
    mov = 52; // "Diag2"
    lcd.setCursor(0,1); lcd.print(F("VELOCIDAD:"));
    lcd.setCursor(10,1); lcd.print((analogRead(pEJEX)*10)/1023);
   } else if( jystckPressed == Joystick::Diag_3){
    printSentidoRuedas(textNoMov, textNeg, textNoMov, textNeg, 13);
    lcd.setCursor(2,0); lcd.print(F(" DIAG 3    "));
    vel = ((analogRead(pEJEX)+analogRead(pEJEY)) - (calJoy.ejey + calJoy.ejex + 200))*9 / (1800 - (calJoy.ejey + calJoy.ejex + 200));
    mov = 53; // "Diag3"
    lcd.setCursor(0,1); lcd.print(F("VELOCIDAD:"));
    lcd.setCursor(10,1); lcd.print(vel);
   } else if( jystckPressed == Joystick::Diag_4){
    printSentidoRuedas(textPos, textNoMov, textPos, textNoMov, 13);
    lcd.setCursor(2,0); lcd.print(" DIAG 4    ");
    vel = 6;
    mov = 54; // "Diag4"
    lcd.setCursor(0,1); lcd.print(F("VELOCIDAD:"));
    lcd.setCursor(10,1); lcd.print((analogRead(pEJEX)*10)/1023);
   }  
   /// ---------------------------------------------------------------------------
   else {
    lcd.setCursor(2,0); lcd.print(F("NO MOVE    "));
    lcd.setCursor(0,1); lcd.print(F("             "));
    printSentidoRuedas(textNoMov, textNoMov, textNoMov, textNoMov, 13);
    mov = 20; vel = 0; // "noMov"
   }
   } else if( cam_mode == true){
      if( jystckPressed == Joystick::Back){
        lcd.setCursor(2,0); lcd.print(" <<< PAN IZQ  ");
        ++pan;
        lcd.setCursor(0,1); lcd.print(F("POSICION: ")); lcd.print(pan); lcd.setCursor(11,1); lcd.print(F("   "));
      } else if( jystckPressed == Joystick::Enter){
        lcd.setCursor(2,0); lcd.print(" PAN DER >>>  ");
        --pan;
        lcd.setCursor(0,1); lcd.print(F("POSICION: ")); lcd.print(pan); lcd.setCursor(11,1); lcd.print(F("   "));
      } else if( jystckPressed == Joystick::Up){
        lcd.setCursor(2,0); lcd.print("TIL UP +++  ");
        ++til;
        lcd.setCursor(0,1); lcd.print(F("POSICION: ")); lcd.print(til); lcd.setCursor(11,1); lcd.print(F("   "));
      } else if( jystckPressed == Joystick::Down){
        lcd.setCursor(2,0); lcd.print("TIL DOWN ---  ");
        --til;
        lcd.setCursor(0,1); lcd.print(F("POSICION: ")); lcd.print(til); lcd.setCursor(11,1); lcd.print(F("   "));
      } else {
        lcd.setCursor(2,0); lcd.print(F("  - STOP -   "));
        lcd.setCursor(0,1); lcd.print(F("  PAN:")); lcd.print(pan); lcd.setCursor(7,1); lcd.print(F("  TIL:")); lcd.print(til); lcd.setCursor(14,1); lcd.print(F("  "));
       }
       if(pan <= 1){ pan=1; }
       else if(pan >= 7){ pan=7; }
       if(til <= 1){ til=1; }
       else if(til >= 5){ til=5; }
   }   
   
   if ((mov != mov_last) or (vel != vel_last)){
    uint16_t code_VelMov = vel*100 + mov ;
    Serial.println(code_VelMov*10);
    Serial.flush();
    delay(666);
   }
   else if ((til != til_last) or (pan != pan_last)){
    uint8_t code_PanTil = pan*10 + til;
    Serial.println(code_PanTil*10+1);
    Serial.flush();
    delay(666);
   }
   delay(21);
   vel_last = vel;
   mov_last = mov;
   til_last = til;
   pan_last = pan;
   
  tecla = teclado.getKey(); 
  if (tecla){ 
    if (tecla == '_'){  break;  } 
    if (tecla == '#'){  
      lcd.setCursor(0,0); lcd.print(F("*"));
      cam_mode = true;
      }
    if (tecla == '*'){  
      lcd.setCursor(0,0); lcd.print(F(" "));
      cam_mode = false;
      }
    }
  }
}
  /*----------------------------------------------------|
 *  ---   ---   ---   FUNCIONES VARIAS   ---   ---   --- 
 */
void mensajeInicio(){
  // Imprime la informacion del proyecto:
    lcd.setCursor(0,0); lcd.print(F("KEYBOARD MENU"));
    lcd.setCursor(0,1); lcd.print(F(" PhiGuard "));
    delay(1618);  lcd.clear();
    lcd.setCursor(0,0); lcd.print(F(" INICIANDO  "));
    lcd.setCursor(0,1); for( int i=0 ; i<16 ; i++ ) { lcd.print(F(".")); delay(60); } 
    lcd.clear();
}
void printSentidoRuedas(byte fig1_4[], byte fig2_4[], byte fig3_4[], byte fig4_4[], uint8_t fila){
  lcd.createChar(4,fig4_4); lcd.createChar(1,fig1_4);
  lcd.createChar(3,fig3_4); lcd.createChar(2,fig2_4);
  lcd.setCursor(fila,0); lcd.write(4); lcd.write(1);
  lcd.setCursor(fila,1); lcd.write(3); lcd.write(2);
  return;
}
void joystickLCD(){
    lcd.setCursor(0,1); lcd.print(F("EX:")); lcd.print(analogRead(pEJEX));
    lcd.setCursor(8,1); lcd.print(F("EY:")); lcd.print(analogRead(pEJEY));
    lcd.setCursor(8,0); lcd.print(F("RT:")); lcd.print(analogRead(pROTJSTICK));
    lcd.setCursor(0,0); lcd.print(F("BT:")); lcd.print(analogRead(pBTTMJSTICK)); 
}
/*
void printMovCoche(char moveCoche, int fila){
  /// --------------------------------------------------------------------------------------------
  // Movimiento para adelante
  const byte goDown[4][8] = {{ B00000, B00000, B10000, B11000, B11100, B11110, B10000, B10000  },
                      { B10000, B10000, B10000, B10000, B10000, B10000, B00000, B00000  },
                      { B00001, B00001, B00001, B00001, B00001, B00001, B00000, B00000  },
                      { B00000, B00000, B00001, B00011, B00111, B01111, B00001, B00001  }};
  // Movimiento para atras
  const byte goUp[4][8] = {{ B00000, B00000, B10000, B10000, B10000, B10000, B10000, B10000  },
                      { B10000, B10000, B11110, B11100, B11000, B10000, B00000, B00000  },
                      { B00001, B00001, B01111, B00111, B00011, B00001, B00000, B00000  },
                      { B00000, B00000, B00001, B00001, B00001, B00001, B00001, B00001  }};   
  // Figura de rotacion en el eje central positiva
  const byte rotPos[4][8] = {{ B00000,  B00000, B00000,  B11010, B00110,  B01110,  B00001,  B00001  },
                      { B00001,  B00001, B00010,  B00100, B11000,  B00000,  B00000,  B00000  },
                      { B10000,  B10000, B01110,  B01100, B01011,  B00000,  B00000,  B00000  },
                      { B00000,  B00000, B00000,  B00011, B00100,  B01000,  B10000,  B10000  }};
  // Figura de rotacion en el eje central negativa
  const byte rotNeg[4][8] = {{ B00000, B00000, B00000, B11000, B00100, B00010, B00001, B00001  },
                      { B00001, B00001, B01110, B00110, B11010, B00000, B00000, B00000  },
                      { B10000, B10000, B01000, B00100, B00011, B00000, B00000, B00000  },
                      { B00000, B00000, B00000, B01011, B01100, B01110, B10000, B10000  }};
  // Figura de rotacion con eje en las ruedas
  const byte rotRueda_1[4][8] = {};
  byte rotRueda_2[4][8] = {};
  byte rotRueda_3[4][8] = {};
  byte rotRueda_4[4][8] = {};
  byte roAxis_41[4][8] = {};
  byte rotAxis_32[4][8] = {};
  /// --------------------------------------------------------------------------------------------
  lcd.createChar( 4, arrayMove[3]); lcd.createChar( 1, arrayMove[0]);
  lcd.createChar( 3, arrayMove[2]); lcd.createChar( 2, arrayMove[1]);
  lcd.setCursor( fila, 0); lcd.write(4); lcd.write(1);
  lcd.setCursor( fila, 1); lcd.write(3); lcd.write(2);
  return;
}*/
  /*----------------------------------------------------|
 *  ---   ---   ---   MENÚS Y SUBMENÚS   ---   ---   --- 
 */
//########################### --- MENÚS Y SUBMENÚS - "JOYSTICK" --- ##############################  ------  ------  ------  ------  ------  ------
void menuJoystick(){
  const String menuJoy[ARRAYSIZE] = {"", " - CALIBRACION ", " - CONT.MANUAL", "               "};
  uint8_t submenu = 1;
  ///-----------------
  while (jystckPressed != Joystick::Back){
  jystckPressed = readJoystick();
  lcd.setCursor(15,0); lcd.print(F("<"));
  while (jystckPressed != Joystick::Enter){
    jystckPressed = readJoystick();
    digitalWrite(pinLed, 1);
    if( jystckPressed == Joystick::Down){
      submenu++;
      if (submenu >= 2) { submenu = 2;  }
      delay(300);
    } else if( jystckPressed == Joystick::Up){
      submenu--;
      if (submenu <= 1) { submenu = 1;  }
      delay(300);
    } else if( jystckPressed == Joystick::Back){
      return;
    }
      
    lcd.setCursor(0,0); lcd.print(menuJoy[submenu]);
    lcd.setCursor(0,1); lcd.print(menuJoy[submenu + 1]);
  }
  lcd.clear();
  switch (submenu){
    case 1:
      mCalibracion();
      break;
    case 2:
      delay(120);
      mCondManual();
      break;
  };
  jystckPressed = Joystick::Up;
  lcd.clear();
  delay(300);
  }}

void mCalibracion(){ // SUBMENÚ DE CONDUCCIÓN AUTOMÁTICA  ------  ------  ------  ------  ------  ------
  while (true) {
    joystickLCD();
    tecla = teclado.getKey();
    if (tecla){
      lcd.setCursor(15,0); lcd.print(tecla);
      if (tecla == '_') { return; }
      else if (tecla == '*'){
        calJoy.rot = analogRead(pROTJSTICK);
        calJoy.ejey = analogRead(pEJEY);
        calJoy.ejex = analogRead(pEJEX);
        lcd.clear();
        for (int i=0; i<16; i++){
          lcd.setCursor(i,0); lcd.print(F("."));
          delay(33);  }
        lcd.clear();
        lcd.setCursor(0,0); lcd.print(F("OK.CALIB RT:")); lcd.print(calJoy.rot);
        lcd.setCursor(0,1); lcd.print(F("EX:")); lcd.print(calJoy.ejex);
        lcd.setCursor(8,1); lcd.print(F("EY:")); lcd.print(calJoy.ejey);
        delay(3000);
        return;
      }
    }
}}
void mCondManual(){ // SUBMENÚ DE CONDUCCIÓN MANUAL  ------  ------  ------  ------  ------  ------
  readJoysitickPot();
  delay(300);
  //tecla = teclado.getKey();    
}
//############################ --- MENÚS Y SUBMENÚS - "TECLADO" --- ##############################  ------  ------  ------  ------  ------  ------
void menuTeclado(){
  //const String menuTec[ARRAYSIZE] = {"", " - C.MANUAL    ", " - C.AUTO      ", " - C.PRUEBA    ", "               "};
  uint8_t posCol = 0; uint8_t posFil = 0;
  char DataReturn;  char pulsado;
  char DataCode[16] = ""; 
  uint8_t pan = 0; uint8_t til = 0; uint8_t pan_last = 0; uint8_t til_last = 0;
  unsigned long lstStopTime = 0;
  ///-------------
  while (jystckPressed != Joystick::Back){ 
    tecla = teclado.getKey();
    if (tecla){
      Data[data_count] = tecla; 
      strncat(DataCode, &tecla, 1);    // Añadir el nuevo caracter al conjunto existente
      if (strlen(DataCode) >= 12) { strcpy(DataCode, "");     // Reiniciar el conjunto a una cadena vacía
      } else if (tecla == '?'){ strcpy(DataCode, ""); }

      lcd.setCursor(posCol, posFil);  lcd.print(Data[data_count]); 
      data_count++; 
      posCol++;
      ///------------------------
      if (posCol == 16){
        posFil = 1;
        posCol = 0;
      }
     } //Serial.println(DataCode);
    if (strcmp(DataCode, "10</") == 0){   ///--------------
      Serial.println(2);
      Serial.flush();
      for (int i=0; i<13; i++) {
            lcd.setCursor(i,0); lcd.print(".");
            delay(60); }
      unsigned long lastTime = 0;
      strcpy(DataCode, "");
      
      while (teclado.getKey() != '_'){
        if (millis() - lastTime >= 1618){
          lastTime = millis();
          Serial.println(2);
          Serial.flush();
        } 
       lcd.setCursor(0,0); lcd.print(" ROBOT M.AUTO  ");
       lcd.setCursor(0,1); lcd.print(" PAN-TIL STOP  ");
      } lcd.clear(); posFil = 0; posCol = 0;
    } else if(strcmp(DataCode, "11</") == 0){   ///----------------------------------------------------------------
      strcpy(DataCode, "");
      while (true){
        jystckPressed = readJoystick();
        lcd.setCursor(0,0); lcd.print("M*");
        if( jystckPressed == Joystick::Back){
          lcd.setCursor(2,0); lcd.print(" <<< PAN IZQ  ");
          ++pan;
          lcd.setCursor(0,1); lcd.print(F("POSICION: ")); lcd.print(pan); lcd.setCursor(11,1); lcd.print(F("   "));
        } else if( jystckPressed == Joystick::Enter){
          lcd.setCursor(2,0); lcd.print(" PAN DER >>>  ");
          --pan;
          lcd.setCursor(0,1); lcd.print(F("POSICION: ")); lcd.print(pan); lcd.setCursor(11,1); lcd.print(F("   "));
        } else if( jystckPressed == Joystick::Up){
          lcd.setCursor(2,0); lcd.print(" TIL UP +++  ");
          ++til;
          lcd.setCursor(0,1); lcd.print(F("POSICION: ")); lcd.print(til); lcd.setCursor(11,1); lcd.print(F("   "));
        } else if( jystckPressed == Joystick::Down){
          lcd.setCursor(2,0); lcd.print(" TIL DOWN --- ");
          --til;
          lcd.setCursor(0,1); lcd.print(F("POSICION: ")); lcd.print(til); lcd.setCursor(11,1); lcd.print(F("   "));
        } else {
          lcd.setCursor(2,0); lcd.print(F("  - STOP -    "));
          lcd.setCursor(0,1); lcd.print(F("  PAN:")); lcd.print(pan); lcd.setCursor(7,1); lcd.print(F("  TIL:")); lcd.print(til); lcd.setCursor(14,1); lcd.print(F("  "));
         }
         if(pan <= 1){ pan=1; }
         else if(pan >= 7){ pan=7; }
         if(til <= 1){ til=1; }
         else if(til >= 5){ til=5; }

         if ((til != til_last) or (pan != pan_last)){
          uint8_t code_PanTil = pan*10 + til;
          Serial.println(code_PanTil*10+3);
          Serial.flush();
          delay(666);
        }
         til_last = til;
         pan_last = pan;
         tecla = teclado.getKey();
        if (tecla == '_'){ lcd.clear(); posFil = 0; posCol = 0; break; }
      }
        
///--------------------------------------------------------------------------------
    //////////////////////7--------------------------------------
    } else if (strcmp(DataCode, "00</") == 0){   ///--------------
      Serial.println(4);
      Serial.flush();
      for (int i=0; i<13; i++) {
            lcd.setCursor(i,0); lcd.print(".");
            delay(60); }
      unsigned long lastTime = 0;
      strcpy(DataCode, "");
      
      while (teclado.getKey() != '_'){
        if (millis() - lastTime >= 1618){
          lastTime = millis();
          Serial.println(4);
          Serial.flush();
        } 
       lcd.setCursor(0,0); lcd.print(" ROBOT   STOP  ");
       lcd.setCursor(0,1); lcd.print(" PAN-TIL STOP  ");
      } lcd.clear(); posFil = 0; posCol = 0;
    } else if(strcmp(DataCode, "20</") == 0){   ///----------------------------------------------------------------
      Serial.println(5);
      Serial.flush();
      for (int i=0; i<13; i++) {
            lcd.setCursor(i,0); lcd.print(".");
            delay(60); }
      strcpy(DataCode, "");
      while (teclado.getKey() != '_'){
        lcd.setCursor(0,0); lcd.print(" ROBOT   SEC-M ");
        lcd.setCursor(0,1); lcd.print(" PAN-TIL STOP  ");
      } lcd.clear(); posFil = 0; posCol = 0;
    } else if(strcmp(DataCode, "02</") == 0){   ///----------------------------------------------------------------
      Serial.println(6);
      Serial.flush();
      for (int i=0; i<13; i++) {
            lcd.setCursor(i,0); lcd.print(".");
            delay(60); }
     strcpy(DataCode, "");
      while (teclado.getKey() != '_'){ 
        lcd.setCursor(0,0); lcd.print(" ROBOT   STOP ");
        lcd.setCursor(0,1); lcd.print(" PAN-TIL SEC-M");
      } lcd.clear(); posFil = 0; posCol = 0;
    }
    ///////////////////////-------------------------------------------------
    jystckPressed = readJoystick();
    DataReturn = Data;
  } return DataReturn; }
///-------------------------------
void clearData(){
  while(data_count !=0){  Data[data_count--] = 0; }
  return;
}



/**___________________________________________________________________________________________________________________________________________________________________________________________________
 *   ######   ---   ---   ---   ---   ---   ---   ---   ---   ---   ---   ---   ---   ---   ---   ---   ---   ---   ---   ---   ---  PROGRAMA PRINCIPAL   ---   ---   ---     ---   ---   ---   ######
 */
void setup() {
  
  Serial.begin(115200); // Inicia la comunicación serial con el puerto USB
  //xbee.begin(); // Inicia la comunicación serial con el módulo Xbee
  pinMode(pinLed, OUTPUT);
  // Se inicializa el RF

  // Inicia el modulo RF
  //askTx.init();
  // Inicia el LCD:
  lcd.init(); lcd.backlight(); 
  //lcd.print(" - - - "); 
  delay(666);
    
    //mensajeInicio();
}
///
void loop() {
  uint8_t menu = 1;
  const String results[ARRAYSIZE] = {"", " - JOYSTICK    ", " - TECLADO     ",  "               "};
  lcd.setCursor(15,0); lcd.print(F("<"));
  while (jystckPressed != Joystick::Enter){
    jystckPressed = readJoystick();
    digitalWrite(pinLed, 1);
    if( jystckPressed == Joystick::Down){
      menu++; if (menu >= 2) {  menu = 2; }
      delay(300);
    } else if( jystckPressed == Joystick::Up){
      menu--; if (menu <= 1) {  menu = 1; }
      delay(300);
    }
    ///
    lcd.setCursor(0,0); lcd.print(results[menu]);
    lcd.setCursor(0,1); lcd.print(results[menu + 1]);
  }
  lcd.clear();
  delay(300);
  digitalWrite(pinLed, 0);
  switch (menu){
    case 1:
      menuJoystick();
      break;
    case 2:
      menuTeclado();
      break;  };
}








//// NOTAS ------------------------------------------------------------------------------------------------------------------------------------
