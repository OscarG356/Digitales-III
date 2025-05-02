volatile unsigned int pulseCount = 0;
unsigned long lastTime = 0;
float rpm = 0;
const int encoderPin = 2;  // Ajusta según tu conexión
const int pulsesPerRevolution = 20;

void countPulse() {
  pulseCount++;
}

void setup() {
  Serial.begin(115200);
  pinMode(encoderPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(encoderPin), countPulse, RISING);
  lastTime = millis();
}

void loop() {
  unsigned long currentTime = millis();
  
  if (currentTime - lastTime >= 1000) {
    noInterrupts();
    unsigned int count = pulseCount;
    pulseCount = 0;
    interrupts();

    rpm = (count * 60.0) / pulsesPerRevolution;
    Serial.print("RPM: ");
    Serial.println(rpm);

    lastTime = currentTime;
  }
}
