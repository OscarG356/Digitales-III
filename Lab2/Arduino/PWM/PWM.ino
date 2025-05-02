const int in1Pin = 2;     // IN1 del L298
const int in2Pin = 3;     // IN2 del L298
const int enAPin = 4;     // ENA del L298 (PWM)

int duty = 0;

void setup() {
  Serial.begin(115200);
  
  pinMode(in1Pin, OUTPUT);
  pinMode(in2Pin, OUTPUT);
  pinMode(enAPin, OUTPUT);

  // Fijamos la dirección del motor hacia adelante
  digitalWrite(in1Pin, HIGH);
  digitalWrite(in2Pin, LOW);

  // Frecuencia PWM opcional (para que no "chille")
  analogWriteFreq(1000);
}

void loop() {
  analogWrite(enAPin, duty);

  Serial.print("PWM %: ");
  Serial.print((duty * 100) / 250);  // Esto muestra el porcentaje real
  Serial.println("%");

  delay(1000);  // Espera 1 segundo entre escalones

  duty += 25;  // Escalones de 10%

  if (duty > 250) {
    duty = 0;  // Reinicia desde 0%
  }
}
