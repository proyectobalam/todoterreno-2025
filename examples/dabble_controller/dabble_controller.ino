/*
  ============================================================
   PROYECTO BALAM 2025 – Todo Terreno STEM - DabbleControl
   Universidad Galileo | Grupo Intelecto
  ============================================================

   Descripción:
     Control de movimiento de robot con cuatro motores DC
     mediante la app Dabble vía Bluetooth. Fase: COMPITE.

   Objetivo:
     Controlar el robot en direcciones básicas (adelante, atrás,
     izquierda, derecha y detener) usando la cruz del Dabble GamePad.

   Librerías:
     - DabbleESP32.h → interfaz con app Dabble

   Autor(es):
     Gabriela Subuyú – Tesla Lab
     Equipo BALAM / Grupo Intelecto

   Fecha: Mayo 2025
   Versión: 1.5

   Organización: https://github.com/proyectobalam
   Repositorio: https://github.com/proyectobalam/retostem-2025
  ============================================================
*/

// ==============================================================
// Inclusión de librerías y definición del módulo Dabble
// ==============================================================

#include <TB6612_ESP32.h>
#include <DabbleESP32.h>


#define CUSTOM_SETTINGS
#define INCLUDE_GAMEPAD_MODULE  // Activar módulo de GamePad de Dabble

// ==============================================================
// Definición de pines para motores (verificar con la tarjeta)
// ==============================================================

// M1 – Atrás derecha
#define PWM1 15
#define M1_1 5
#define M1_2 18

// M2 – Adelante derecha
#define PWM2 2
#define M2_1 14
#define M2_2 27

// M3 – Atrás izquierda
#define PWM3 13
#define M3_1 26
#define M3_2 25

// M4 – Adelante izquierda
#define PWM4 12
#define M4_1 33
#define M4_2 32

// MS (Mecanismo servomotores) 
//TODO: Pines de mecanismo
#define PWMS 0


// ==============================================================
// Configuración inicial del sistema
// ==============================================================

void setup() {
  // Configurar pines como salida
  pinMode(PWM1, OUTPUT);
  pinMode(M1_1, OUTPUT); pinMode(M1_2, OUTPUT);

  pinMode(PWM2, OUTPUT);
  pinMode(M2_1, OUTPUT); pinMode(M2_2, OUTPUT);

  pinMode(PWM3, OUTPUT);
  pinMode(M3_1, OUTPUT); pinMode(M3_2, OUTPUT);

  pinMode(PWM4, OUTPUT);
  pinMode(M4_1, OUTPUT); pinMode(M4_2, OUTPUT);

  //TODO: Pines mecanismo

  Serial.begin(115200);     // Comunicación serial
  Dabble.begin("BALAM_2025");       //Cambiar nombre visible en la app
}

// ==============================================================
// Loop principal – Lectura de comandos Dabble
// ==============================================================

void loop() {
  // Activar PWM
  digitalWrite(PWM1, HIGH);
  digitalWrite(PWM2, HIGH);
  digitalWrite(PWM3, HIGH);
  digitalWrite(PWM4, HIGH);

  Dabble.processInput();  // Procesar entrada desde app Dabble

  // Dirección del movimiento según botón presionado
  if (GamePad.isUpPressed()) {
    Serial.println("Forward");
    forward();
  } 
  if (GamePad.isDownPressed()) {
    Serial.println("Back");
    back();
  } 
  if (GamePad.isLeftPressed()) {
    Serial.println("Left");
    left();
  } 
  if (GamePad.isRightPressed()) {
    Serial.println("Right");
    right();
  } 
  
}

// ==============================================================
// Funciones de control de dirección
// ==============================================================

void forward() {
  // Motores giran hacia adelante
  // TODO: Revisar mejorar ajuste para que vaya recto
  analogWrite(M1_1, 200); analogWrite(M1_2, 0);
  analogWrite(M2_1, 200); analogWrite(M2_2, 0);
  analogWrite(M3_1, 250); analogWrite(M3_2, 0);
  analogWrite(M4_1, 250); analogWrite(M4_2, 0);
}

void back() {
  // Motores giran hacia atrás
  analogWrite(M1_1, 0); analogWrite(M1_2, 250);
  analogWrite(M2_1, 0); analogWrite(M2_2, 250);
  analogWrite(M3_1, 0); analogWrite(M3_2, 250);
  analogWrite(M4_1, 0); analogWrite(M4_2, 250);
}

void left() {
  // Giro a la izquierda
  analogWrite(M1_1, 250); analogWrite(M1_2, 0);
  analogWrite(M2_1, 250); analogWrite(M2_2, 0);
  analogWrite(M3_1, 0); analogWrite(M3_2, 250);
  analogWrite(M4_1, 0); analogWrite(M4_2, 250);
}

void right() {
  // Giro a la derecha
  analogWrite(M1_1, 0); analogWrite(M1_2, 250);
  analogWrite(M2_1, 0); analogWrite(M2_2, 250);
  analogWrite(M3_1, 250); analogWrite(M3_2, 0);
  analogWrite(M4_1, 250); analogWrite(M4_2, 0);
}

void stop() {
  // Detener todos los motores
  analogWrite(M1_1, 0); analogWrite(M1_2, 0);
  analogWrite(M2_1, 0); analogWrite(M2_2, 0);
  analogWrite(M3_1, 0); analogWrite(M3_2, 0);
  analogWrite(M4_1, 0); analogWrite(M4_2, 0);
}


