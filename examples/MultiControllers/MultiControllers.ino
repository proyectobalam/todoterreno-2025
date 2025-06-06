/*
  ============================================================
   PROYECTO BALAM 2025 – Reto STEM - GenericController
   Universidad Galileo | Grupo Intelecto
  ============================================================

   Descripción:
     Fase: COMPITE.
     Control de movimiento de robot con cuatro motores DC
     mediante diferentes controles genéricos. Controles probados:
      - Control Marvo para PS4
      - Control GameSir Nova Lite Control Inalámbrico

   Objetivo:
     Controlar el robot en direcciones básicas (adelante, atrás,
     izquierda, derecha y detener) utilizando el joystick derecho.
     Igualmente la activación del mecanismo por medio del joystick
     izquierdo.

   Librerías:
    - ESP32Servo
    - Bluepad32
      https://github.com/ricardoquesada/bluepad32/

   Autor(es):
     Gabriela Subuyú – Tesla Lab
     Equipo BALAM / Grupo Intelecto

   Fecha: Mayo 2025
   Versión: 1.0
   Estado: 🧪 En pruebas (FASE COMPITE)

   Organización: https://github.com/proyectobalam
   Repositorio: https://github.com/proyectobalam/balam-2025-completo
   Tutorial/Referencia: https://racheldebarros.com/esp32-projects/connect-your-game-controller-to-an-esp32/
  ============================================================
*/

// ==============================================================
// Importación de librerías
// ==============================================================
// TODO: Eliminar librería
#include <TB6612_ESP32.h>
#include <Bluepad32.h>


// M1 – Atrás derecha
#define PWM1 15
#define M1_1 18
#define M1_2 5

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


#define STBY 0
#define STBY1 22


// estas constantes se utilizan para permitirle realizar la configuración del motor
// alinearse con nombres de funciones como forward. El valor puede ser 1 o -1
const int offsetA = 1;
const int offsetB = 1;

const int offsetA1 = 1;
const int offsetB1 = 1;
// MARVO: 41:42: 0: 0:68:13
const uint8_t allowedAddress[6] = {0x41, 0x42, 0x0, 0x0, 0x68, 0x13};



// Inicializando motores. La biblioteca le permitirá los motores.
// Si está utilizando funciones como reenviar que toman 2 motores como argumentos puedes escribir nuevas funciones o
// llama a la función más de una vez.

Motor motor1 = Motor(M1_1, M1_2, PWM1, offsetA, STBY, 5000, 8, 1);
Motor motor2 = Motor(M3_1, M3_2, PWM3, offsetB, STBY, 5000, 8, 2);

Motor motor11 = Motor(M2_1, M2_2, PWM2, offsetA1, STBY1, 5000, 8, 4);
Motor motor22 = Motor(M4_1, M4_2, PWM4, offsetB1, STBY1, 5000, 8, 5);

// Valores Iniciales Joystick
int LStickY = 0;
int LStickX = 0;
int RStickY = 0;
int RStickX = 0;


// Valores máximos de motores
int valorMaxMotorR = 255;
int valorMaxMotorL = 255;

// Valores de deadzone
int valorDeadZoneLXMin = -5;
int valorDeadZoneLXMax = 5;
int valorDeadZoneLYMin = -5;
int valorDeadZoneLYMax = 5;
int valorDeadZoneRXMin = -5;
int valorDeadZoneRXMax = 5;
int valorDeadZoneRYMin = -5;
int valorDeadZoneRYMax = 5;




ControllerPtr myControllers[BP32_MAX_GAMEPADS];

// This callback gets called any time a new gamepad is connected.
// Up to 4 gamepads can be connected at the same time.
void onConnectedController(ControllerPtr ctl) {
  ControllerProperties properties = ctl->getProperties();
  const uint8_t* addr = properties.btaddr;

  // Verificar si la dirección coincide con la permitida
  bool isAllowed = true;
  for (int i = 0; i < 6; i++) {
    if (addr[i] != allowedAddress[i]) {
      isAllowed = false;
      break;
    }
  }

  if (!isAllowed) {
    Serial.println("CALLBACK: Controller rechazado (address no permitida).");
    return;  // Salir sin hacer nada
  }
  bool foundEmptySlot = false;
  for (int i = 0; i < BP32_MAX_GAMEPADS; i++) {
    Serial.println("Controller");

    if (myControllers[i] == nullptr) {
      Serial.printf("CALLBACK: Controller is connected, index=%d\n", i);
      // Additionally, you can get certain gamepad properties like:
      // Model, VID, PID, BTAddr, flags, etc.
      ControllerProperties properties = ctl->getProperties();
      Serial.printf("Controller model: %s, VID=0x%04x, PID=0x%04x\n", ctl->getModelName().c_str(), properties.vendor_id, properties.product_id);
      const uint8_t* addr = properties.btaddr;
      Serial.printf("Address Controler: %2X:%2X:%2X:%2X:%2X:%2X\n", addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]);
      // MARVO: 41:42: 0: 0:68:13
      // Play: 0: 1:6C:9F:95:8
      // GAMESIR:  A9:D7: D:A0:BD:A3
      myControllers[i] = ctl;
      foundEmptySlot = true;
      break;
    }
  }

  if (!foundEmptySlot) {
    Serial.println("CALLBACK: Controller connected, but could not found empty slot");
  }
}

void onDisconnectedController(ControllerPtr ctl) {
  bool foundController = false;

  for (int i = 0; i < BP32_MAX_GAMEPADS; i++) {
    if (myControllers[i] == ctl) {
      Serial.printf("CALLBACK: Controller disconnected from index=%d\n", i);
      myControllers[i] = nullptr;
      foundController = true;
      break;
    }
  }

  if (!foundController) {
    Serial.println("CALLBACK: Controller disconnected, but not found in myControllers");
  }
}

// ========= SEE CONTROLLER VALUES IN SERIAL MONITOR ========= //

void dumpGamepad(ControllerPtr ctl) {
  Serial.printf(
    "idx=%d, dpad: 0x%02x, buttons: 0x%04x, axis L: %4d, %4d, axis R: %4d, %4d, brake: %4d, throttle: %4d, "
    "misc: 0x%02x, gyro x:%6d y:%6d z:%6d, accel x:%6d y:%6d z:%6d\n",
    ctl->index(),        // Controller Index
    ctl->dpad(),         // D-pad
    ctl->buttons(),      // bitmask of pressed buttons
    ctl->axisX(),        // (-511 - 512) left X Axis
    ctl->axisY(),        // (-511 - 512) left Y axis
    ctl->axisRX(),       // (-511 - 512) right X axis
    ctl->axisRY(),       // (-511 - 512) right Y axis
    ctl->brake(),        // (0 - 1023): brake button
    ctl->throttle(),     // (0 - 1023): throttle (AKA gas) button
    ctl->miscButtons(),  // bitmask of pressed "misc" buttons
    ctl->gyroX(),        // Gyro X
    ctl->gyroY(),        // Gyro Y
    ctl->gyroZ(),        // Gyro Z
    ctl->accelX(),       // Accelerometer X
    ctl->accelY(),       // Accelerometer Y
    ctl->accelZ()        // Accelerometer Z
  );
}

// ========= GAME CONTROLLER ACTIONS SECTION ========= //

void processGamepad(ControllerPtr ctl) {
  // ====== BOTONES (se mantiene igual) ======
  if (ctl->buttons() == 0x0080) {
    if (valorMaxMotorL == 255) {
      valorMaxMotorL = 200;
      valorMaxMotorR = 200;
    } else {
      valorMaxMotorL = 255;
      valorMaxMotorR = 255;
    }
  }
  //== PS4 X button = 0x0001 ==//
  if (ctl->buttons() == 0x0001) {
    // code for when X button is pushed
    Serial.println("Accionando el mecanismo");
    delay(200);
  }

  // Obtener valores del joystick izquierdo
  int ejeX = ctl->axisRX();
  int ejeY = ctl->axisRY();

  if (abs(ejeX) < 10) ejeX = 0;
  if (abs(ejeY) < 10) ejeY = 0;

  int velocidad = map(ejeY, 512, -508, -valorMaxMotorL, valorMaxMotorL);


  motor2.drive(velocidad, 5);
  motor22.drive(velocidad, 5);
  motor1.drive(velocidad, 5);
  motor11.drive(velocidad, 5);

  if (velocidad == 0) {
    brake(motor1, motor11);
    brake(motor2, motor22);
  }
}


void processControllers() {
  for (auto myController : myControllers) {
    if (myController && myController->isConnected() && myController->hasData()) {
      if (myController->isGamepad()) {
        processGamepad(myController);
      } else {
        Serial.println("Unsupported controller");
      }
    }
  }
}

// ==============================================================
// Funciones de control de dirección
// ==============================================================




// Arduino setup function. Runs in CPU 1

void setup() {
  Serial.begin(115200);
  brake(motor1, motor2);
  brake(motor11, motor22);
  Serial.printf("Firmware: %s\n", BP32.firmwareVersion());
  const uint8_t* addr = BP32.localBdAddress();
  Serial.printf("BD Addr: %2X:%2X:%2X:%2X:%2X:%2X\n", addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]);

  // Setup the Bluepad32 callbacks
  BP32.setup(&onConnectedController, &onDisconnectedController);

  // "forgetBluetoothKeys()" should be called when the user performs
  // a "device factory reset", or similar.
  // Calling "forgetBluetoothKeys" in setup() just as an example.
  // Forgetting Bluetooth keys prevents "paired" gamepads to reconnect.
  // But it might also fix some connection / re-connection issues.
  //BP32.forgetBluetoothKeys();

  // Enables mouse / touchpad support for gamepads that support them.
  // When enabled, controllers like DualSense and DualShock4 generate two connected devices:
  // - First one: the gamepad
  // - Second one, which is a "virtual device", is a mouse.
  // By default, it is disabled.
  BP32.enableVirtualDevice(false);
}

// Arduino loop function. Runs in CPU 1.

void loop() {
  // This call fetches all the controllers' data.
  // Call this function in your main loop.
  bool dataUpdated = BP32.update();
  if (dataUpdated)
    processControllers();

  // The main loop must have some kind of "yield to lower priority task" event.
  // Otherwise, the watchdog will get triggered.
  // If your main loop doesn't have one, just add a simple `vTaskDelay(1)`.
  // Detailed info here:
  // https://stackoverflow.com/questions/66278271/task-watchdog-got-triggered-the-tasks-did-not-reset-the-watchdog-in-time

  // vTaskDelay(1);
  delay(150);
}