struct Sample {
  uint32_t delta;  // 4 bytes
  uint8_t pwm;     // 1 byte
  uint16_t rpm;    // 2 bytes
};

const int maxSamples = 9371;
Sample buffer[maxSamples];
int bufferIndex = 0;

unsigned long startTime;
bool sentCSV = false;

void setup() {
  Serial.begin(115200);
  startTime = millis();
}

void loop() {
  // Guardar datos si aún hay espacio
  if (bufferIndex < maxSamples) {
    Sample s;
    s.delta = millis() - startTime;
    s.pwm = 50;         // Simulación de PWM
    s.rpm = 1500;     // Simulación de RPM

    buffer[bufferIndex++] = s;
    delay(5); // Simular toma de datos
  }
  // Cuando el buffer esté lleno, mandar los datos si aún no se han enviado
  else if (!sentCSV) {
    Serial.println("delta;pwm;rpm"); // Encabezado CSV

    for (int i = 0; i < maxSamples; i++) {
      Serial.print(buffer[i].delta);
      Serial.print(";");
      Serial.print(buffer[i].pwm);
      Serial.print(";");
      Serial.println(buffer[i].rpm);
    }

    sentCSV = true;
    Serial.println("✅ CSV enviado.");
  }
}
