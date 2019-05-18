
int buttonState = 0;
// the setup function runs once when you press reset or power the board
void setup() {
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(0, OUTPUT);
  Serial.begin(115200);
}

// the loop function runs over and over again forever
void loop() {
  buttonState = analogRead(A0);
  Serial.println(buttonState);
  if (buttonState > 250) {
    digitalWrite(0, HIGH);
  } else  {
    digitalWrite(0, LOW);
  }
  delay(1000);
}
