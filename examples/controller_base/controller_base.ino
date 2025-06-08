/*
  ============================================================
   PROYECTO BALAM 2025 – Todo Terreno STEM - controller_base
   Universidad Galileo | Grupo Intelecto
  ============================================================

   Descripción:
     Control básico de un robot de cuatro motores DC usando un gamepad Bluetooth.
     Demuestra:
      - Movimiento adelante, atrás

   Objetivo:
     Enseñar a principiantes:
      1. Configurar pines y librerías.
      2. Conectar y gestionar controles Bluetooth.
      3. Controlar motores con PWM y digital.

   Librerías:
    - TB6612_ESP32: Control de drivers de motor TB6612.
    - Bluepad32: Comunicación con gamepads Bluetooth.
      https://github.com/ricardoquesada/bluepad32/

   Autor(es):
     Gabriela Subuyú – Tesla Lab
     Oscar Rodas - Tesla Lab
     Equipo BALAM / Grupo Intelecto

   Fecha: Junio 2025
   Versión: 1.0
*/

// ------------------------- Importar Librerías -------------------------
#include <TB6612_ESP32.h>   // Manejo de drivers de motor TB6612
#include <Bluepad32.h>      // Soporte para gamepads Bluetooth

// ------------------------ Definición de Pines ------------------------
// Cada motor utiliza dos pines digitales para dirección y uno PWM para velocidad.

// Motor 1 – Atrás derecha
#define PWM1 15  // Pin PWM velocidad
#define M1_1 18  // Dirección A
#define M1_2 5   // Dirección B

// Motor 2 – Adelante derecha
#define PWM2 2   // Pin PWM velocidad
#define M2_1 14  // Dirección A
#define M2_2 27  // Dirección B

// Motor 3 – Atrás izquierda
#define PWM3 13  // Pin PWM velocidad
#define M3_1 26  // Dirección A
#define M3_2 25  // Dirección B

// Motor 4 – Adelante izquierda
#define PWM4 12  // Pin PWM velocidad
#define M4_1 33  // Dirección A
#define M4_2 32  // Dirección B

#define STBY   0  // Pin de stand-by para motores 1 y 3
#define STBY1 22  // Pin de stand-by para motores 2 y 4

// ---------------------- Parámetros de Motor ----------------------
// Ajuste de orientación de cada motor (1 = normal, -1 = invertido).
const int offsetA  =  1;
const int offsetB  =  1;
const int offsetA1 =  1;
const int offsetB1 =  1;

// Dirección Bluetooth permitida (6 bytes): solo este control podrá conectarse.
const uint8_t allowedAddress[6] = {0x41, 0x42, 0x00, 0x00, 0x68, 0x13};

// ------------------- Inicializar Objetos Motor -------------------
Motor motor1 = Motor(M1_1, M1_2, PWM1, offsetA,  STBY,  5000, 8, 1);
Motor motor2 = Motor(M3_1, M3_2, PWM3, offsetB,  STBY,  5000, 8, 2);
Motor motor11= Motor(M2_1, M2_2, PWM2, offsetA1, STBY1, 5000, 8, 4);
Motor motor22= Motor(M4_1, M4_2, PWM4, offsetB1, STBY1, 5000, 8, 5);

// ------------------- Variables de Joystick -------------------
// Almacenan posición de ejes analógicos (inicialmente en 0).
int LStickY = 0, LStickX = 0;
int RStickY = 0, RStickX = 0;

// ------------- Velocidad Máxima y Deadzone -------------
// limitan velocidad y evitan lecturas accidentales cerca del centro.
int valorMaxMotorR = 255;
int valorMaxMotorL = 255;
const int deadMin = -5, deadMax = 5;

// ------------ Control de Varios Controles ------------
// Array para guardar hasta 4 controles simultáneos.
ControllerPtr myControllers[BP32_MAX_GAMEPADS];

// ------------ Callback: Control Conectado ------------
void onConnectedController(ControllerPtr ctl) {
  // Obtener propiedades y dirección Bluetooth
  const uint8_t* addr = ctl->getProperties().btaddr;

  // Verificar lista de direcciones permitidas
  bool isAllowed = true;
  for (int i = 0; i < 6; i++) {
    if (addr[i] != allowedAddress[i]) { isAllowed = false; break; }
  }
  if (!isAllowed) {
    Serial.println("Control rechazado: dirección no permitida.");
    return;
  }

  // Guardar en la primera ranura libre
  for (int i = 0; i < BP32_MAX_GAMEPADS; i++) {
    if (myControllers[i] == nullptr) {
      myControllers[i] = ctl;
      Serial.printf("Control conectado en ranura %d\n", i);
      return;
    }
  }
  Serial.println("No hay ranuras libres para nuevo control.");
}

// ------------ Callback: Control Desconectado ------------
void onDisconnectedController(ControllerPtr ctl) {
  // Buscar y liberar la ranura correspondiente
  for (int i = 0; i < BP32_MAX_GAMEPADS; i++) {
    if (myControllers[i] == ctl) {
      myControllers[i] = nullptr;
      Serial.printf("Control desconectado de ranura %d\n", i);
      return;
    }
  }
  Serial.println("Control desconectado no registrado.");
}

// ------------ Debug: Volcar Datos de Control ------------
void dumpGamepad(ControllerPtr ctl) {
  // Imprime ejes, botones y sensores en monitor serial
  Serial.printf(
    "idx=%d, buttons=0x%04x, LX=%4d, LY=%4d, RX=%4d, RY=%4d\n",
    ctl->index(), ctl->buttons(),
    ctl->axisX(), ctl->axisY(), ctl->axisRX(), ctl->axisRY()
  );
}

// ------------ Procesar Entrada de Control ------------
void processGamepad(ControllerPtr ctl) {
  // Cambiar velocidad máxima con un botón específico (0x0080)
  if (ctl->buttons() == 0x0080) {
    valorMaxMotorL = (valorMaxMotorL == 255) ? 200 : 255;
    valorMaxMotorR = valorMaxMotorL;
    Serial.printf("Velocidad máxima: %d\n", valorMaxMotorL);
  }

  // Lectura de joystick derecho para movimiento
  int ejeX = ctl->axisRX();
  int ejeY = ctl->axisRY();
  // Aplicar zona muerta
  ejeX = (abs(ejeX) < deadMax) ? 0 : ejeX;
  ejeY = (abs(ejeY) < deadMax) ? 0 : ejeY;

  // Mapear valores a velocidad de motor
  int velocidad = map(ejeY, 512, -508, -valorMaxMotorL, valorMaxMotorL);

  // Enviar velocidad a cada motor
  motor2.drive(velocidad, 5);
  motor22.drive(velocidad, 5);
  motor1.drive(velocidad, 5);
  motor11.drive(velocidad, 5);

  // Frenar si no hay movimiento
  if (velocidad == 0) {
    brake(motor1, motor11);
    brake(motor2, motor22);
  }
}

// ------------ Procesar Todos los Controles ------------
void processControllers() {
  for (auto ctl : myControllers) {
    if (ctl && ctl->isConnected() && ctl->hasData()) {
      processGamepad(ctl);
    }
  }
}

// ---------------------- Setup de Arduino ----------------------
void setup() {
  Serial.begin(115200);  // Iniciar monitor serial

  // Frenar todos los motores al iniciar
  brake(motor1, motor2);
  brake(motor11, motor22);

  // Mostrar versión de firmware Bluetooth
  Serial.printf("Firmware Bluepad32: %s\n", BP32.firmwareVersion());

  // Configurar callbacks de conexión/desconexión
  BP32.setup(&onConnectedController, &onDisconnectedController);

  // Desactivar dispositivo virtual (mouse) de controles compatibles
  BP32.enableVirtualDevice(false);
}

// ---------------------- Loop Principal ----------------------
void loop() {
  // Leer datos de controles
  if (BP32.update()) processControllers();

  // Pequeña pausa para evitar watchdog
  delay(150);
}
