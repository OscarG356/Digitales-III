/**
 * @file Completo.ino
 * @brief Sistema de caracterización de motores DC con interfaz serial
 * @author Imar Jimenez y Oscar Gutierrez
 * @date 5/05/2025
 * 
 * Este sistema caracteriza el rendimiento de motores DC midiendo la respuesta en RPM
 * ante diferentes valores de PWM. Ofrece dos modos de operación:
 * 1. Control manual de PWM con visualización en tiempo real de las RPM
 * 2. Prueba automática con escalones de PWM y captura de datos
 */

// ------------------ CONFIGURACIÓN DE PINES ------------------
const int in1Pin = 2;   ///< Pin de dirección 1 del puente H (L298N)
const int in2Pin = 3;   ///< Pin de dirección 2 del puente H
const int enAPin = 4;   ///< Pin PWM para control de velocidad

// ------------------ CONFIGURACIÓN DEL ENCODER ------------------
const int encoderPin = 5;           ///< Pin de entrada del encoder óptico
const int pulsesPerRevolution = 20; ///< Pulsos por revolución del encoder

/**
 * @struct Sample
 * @brief Estructura para almacenar muestras de caracterización
 */
struct Sample {
  uint32_t delta;  ///< Tiempo transcurrido desde inicio (ms)
  uint8_t pwm;     ///< Valor de PWM aplicado (0-100%)
  uint16_t rpm;    ///< RPM medidos
};

// ------------------ PARÁMETROS DEL SISTEMA ------------------
const int maxSamples = 7000;       ///< Máximo número de muestras a almacenar
Sample buffer[maxSamples];         ///< Buffer para almacenamiento de datos
int bufferIndex = 0;               ///< Índice actual del buffer

volatile unsigned int pulseCount = 0; ///< Contador de pulsos del encoder

// ------------------ VARIABLES DE TEMPORIZACIÓN ------------------
unsigned long lastSampleTime = 0;  ///< Último tiempo de muestreo
unsigned long lastStepTime = 0;    ///< Último cambio de paso PWM
unsigned long lastPrintTime = 0;   ///< Último tiempo de impresión serial
unsigned long startTime;           ///< Tiempo de inicio de prueba

// Intervalos de tiempo (ms)
const unsigned long sampleInterval = 4;   ///< Intervalo de muestreo de RPM (250Hz)
const unsigned long stepInterval = 2000;  ///< Intervalo entre pasos PWM
const unsigned long printInterval = 500;  ///< Intervalo de reporte serial (2Hz)

// ------------------ ESTADOS DEL SISTEMA ------------------
/**
 * @enum State
 * @brief Estados de operación del sistema
 */
enum State { 
  IDLE,       ///< Sistema en reposo
  MANUAL_PWM, ///< Modo control manual de PWM
  CAPTURING,  ///< Modo captura de datos
  SENDING     ///< Enviando datos capturados
};

State currentState = IDLE; ///< Estado actual del sistema
int currentPWM = 0;        ///< Valor actual de PWM (0-100%)
int pwmStep = 20;          ///< Tamaño de paso para pruebas automáticas
bool descending = false;   ///< Indicador de secuencia descendente

// ------------------ FUNCIONES ------------------

/**
 * @brief Rutina de interrupción para el encoder
 * 
 * Incrementa el contador de pulsos en cada flanco ascendente
 */
void countPulse() {
  pulseCount++;
}

/**
 * @brief Configuración inicial del sistema
 * 
 * Inicializa pines, comunicación serial e interrupciones
 */
void setup() {
  Serial.begin(115200);
  
  // Configurar pines del motor
  pinMode(in1Pin, OUTPUT);
  pinMode(in2Pin, OUTPUT);
  pinMode(enAPin, OUTPUT);
  
  // Establecer dirección fija del motor
  digitalWrite(in1Pin, HIGH);
  digitalWrite(in2Pin, LOW);
  
  // Configurar PWM
  analogWriteFreq(1000);  // Frecuencia PWM de 1kHz
  analogWrite(enAPin, 0); // Iniciar con motor detenido
  
  // Configurar encoder
  pinMode(encoderPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(encoderPin), countPulse, RISING);
}

/**
 * @brief Bucle principal del programa
 * 
 * Gestiona la máquina de estados principal y las tareas periódicas
 */
void loop() {
  static String input = ""; // Buffer para entrada serial

  // Procesar comandos seriales entrantes
  while (Serial.available()) {
    char c = Serial.read();
    if (c == '\n') {
      processCommand(input);
      input = "";
    } else {
      input += c;
    }
  }

  unsigned long currentTime = millis();

  // Muestreo periódico de RPM
  if (currentTime - lastSampleTime >= sampleInterval) {
    lastSampleTime = currentTime;

    // Leer contador de pulsos de forma segura
    noInterrupts();
    unsigned int count = pulseCount;
    pulseCount = 0;
    interrupts();

    // Calcular RPM
    float rpm = (count * 60.0) / pulsesPerRevolution;

    // Almacenar datos si estamos en modo captura
    if (currentState == CAPTURING && bufferIndex < maxSamples) {
      Sample s;
      s.delta = currentTime - startTime;
      s.pwm = currentPWM;
      s.rpm = rpm;
      buffer[bufferIndex++] = s;
    }

    // Reportar estado en modo manual
    if (currentState == MANUAL_PWM && currentTime - lastPrintTime >= printInterval) {
      lastPrintTime = currentTime;
      Serial.print("PWM: ");
      Serial.print(currentPWM);
      Serial.print("% | RPM: ");
      Serial.println(rpm);
    }
  }

  // Secuencia de pasos durante captura
  if (currentState == CAPTURING && millis() - lastStepTime >= stepInterval) {
    lastStepTime = millis();

    if (!descending) {
      // Fase ascendente
      currentPWM += pwmStep;
      if (currentPWM > 100) {
        currentPWM -= pwmStep;
        descending = true;
      }
    } else {
      // Fase descendente
      currentPWM -= pwmStep;
      if (currentPWM < 0) {
        currentPWM = 0;
        analogWrite(enAPin, 0);
        currentState = SENDING;
        sendCSV();
        return;
      }
    }

    // Aplicar nuevo valor PWM
    analogWrite(enAPin, map(currentPWM, 0, 100, 0, 255));
  }
}

/**
 * @brief Procesa comandos recibidos por el puerto serial
 * @param cmd Comando recibido (String)
 */
void processCommand(String cmd) {
  cmd.trim();
  cmd.toUpperCase();

  if (cmd.startsWith("START")) {
    // Iniciar secuencia de prueba automática
    int spaceIndex = cmd.indexOf(' ');
    if (spaceIndex != -1) {
      pwmStep = cmd.substring(spaceIndex + 1).toInt();
      pwmStep = constrain(pwmStep, 1, 100);
      bufferIndex = 0;
      currentPWM = 0;
      descending = false;
      analogWrite(enAPin, 0);
      startTime = millis();
      lastStepTime = millis();
      currentState = CAPTURING;
      Serial.println("📊 Iniciando captura de datos...");
    }
  } else if (cmd.startsWith("PWM")) {
    // Configurar PWM manualmente
    int spaceIndex = cmd.indexOf(' ');
    if (spaceIndex != -1) {
      int pwmVal = cmd.substring(spaceIndex + 1).toInt();
      pwmVal = constrain(pwmVal, 0, 100);
      currentPWM = pwmVal;
      analogWrite(enAPin, map(pwmVal, 0, 100, 0, 255));
      currentState = MANUAL_PWM;
      Serial.println("🕹️ Modo manual activado");
    }
  }
}

/**
 * @brief Envía los datos capturados en formato CSV
 * 
 * Los datos se envían por el puerto serial con el formato:
 * tiempo_ms;PWM;RPM
 */
void sendCSV() {
  Serial.println("delta;pwm;rpm");
  for (int i = 0; i < bufferIndex; i++) {
    Serial.print(buffer[i].delta);
    Serial.print(";");
    Serial.print(buffer[i].pwm);
    Serial.print(";");
    Serial.println(buffer[i].rpm);
  }
  Serial.println("✅ Datos enviados correctamente");
  currentState = IDLE;
}