// Pines del L298
const int in1Pin = 2;
const int in2Pin = 3;
const int enAPin = 4;

// Encoder
const int encoderPin = 5;
const int pulsesPerRevolution = 20;

// Estructura para almacenamiento de datos
struct Sample {
  uint32_t delta;  // Tiempo desde inicio (ms)
  uint8_t pwm;     // PWM aplicado
  uint16_t rpm;    // Velocidad medida
};

const int maxSamples = 7000;
Sample buffer[maxSamples];
int bufferIndex = 0;

volatile unsigned int pulseCount = 0;

// Temporizadores
unsigned long lastSampleTime = 0;
unsigned long lastStepTime = 0;
unsigned long lastPrintTime = 0;
unsigned long startTime;

// Intervals
const unsigned long sampleInterval = 4;       // 250 Hz
const unsigned long stepInterval = 2000;      // 2 s
const unsigned long printInterval = 500;      // 2 Hz

// Estado del sistema
enum State { IDLE, MANUAL_PWM, CAPTURING, SENDING };
State currentState = IDLE;

int currentPWM = 0;
int pwmStep = 20;
bool descending = false;

void countPulse() {
  pulseCount++;
}

void setup() {
  Serial.begin(115200);
  pinMode(in1Pin, OUTPUT);
  pinMode(in2Pin, OUTPUT);
  pinMode(enAPin, OUTPUT);
  pinMode(encoderPin, INPUT_PULLUP);

  digitalWrite(in1Pin, HIGH);  // Dirección fija
  digitalWrite(in2Pin, LOW);

  analogWriteFreq(1000);  // PWM de 1kHz
  analogWrite(enAPin, 0);

  attachInterrupt(digitalPinToInterrupt(encoderPin), countPulse, RISING);
}

void loop() {
  static String input = "";

  // Leer comandos seriales
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

  // Muestreo de RPM
  if (currentTime - lastSampleTime >= sampleInterval) {
    lastSampleTime = currentTime;

    noInterrupts();
    unsigned int count = pulseCount;
    pulseCount = 0;
    interrupts();

    float rpm = (count * 60.0) / pulsesPerRevolution;

    if (currentState == CAPTURING && bufferIndex < maxSamples) {
      Sample s;
      s.delta = currentTime - startTime;
      s.pwm = currentPWM;
      s.rpm = rpm;
      buffer[bufferIndex++] = s;
    }

    if (currentState == MANUAL_PWM && currentTime - lastPrintTime >= printInterval) {
      lastPrintTime = currentTime;
      Serial.print("PWM: ");
      Serial.print(currentPWM);
      Serial.print(" | RPM: ");
      Serial.println(rpm);
    }
  }

  // Escalones durante captura
  if (currentState == CAPTURING && millis() - lastStepTime >= stepInterval) {
    lastStepTime = millis();

    if (!descending) {
      currentPWM += pwmStep;
      if (currentPWM > 100) {
        currentPWM -= pwmStep;  // retrocede uno
        descending = true;
      }
    } else {
      currentPWM -= pwmStep;
      if (currentPWM < 0) {
        currentPWM = 0;
        analogWrite(enAPin, 0);
        currentState = SENDING;
        sendCSV();
        return;
      }
    }

    analogWrite(enAPin, map(currentPWM, 0, 100, 0, 255));
  }
}

// Procesar comandos
void processCommand(String cmd) {
  cmd.trim();
  cmd.toUpperCase();

  if (cmd.startsWith("START")) {
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
      Serial.println("📊 Iniciando captura...");
    }
  } else if (cmd.startsWith("PWM")) {
    int spaceIndex = cmd.indexOf(' ');
    if (spaceIndex != -1) {
      int pwmVal = cmd.substring(spaceIndex + 1).toInt();
      pwmVal = constrain(pwmVal, 0, 100);
      currentPWM = pwmVal;
      analogWrite(enAPin, map(pwmVal, 0, 100, 0, 255));
      currentState = MANUAL_PWM;
      Serial.println("🕹️ PWM manual activo.");
    }
  }
}

// Enviar CSV al final de captura
void sendCSV() {
  Serial.println("delta;pwm;rpm");
  for (int i = 0; i < bufferIndex; i++) {
    Serial.print(buffer[i].delta);
    Serial.print(";");
    Serial.print(buffer[i].pwm);
    Serial.print(";");
    Serial.println(buffer[i].rpm);
  }
  Serial.println("✅ CSV enviado.");
  currentState = IDLE;
}
