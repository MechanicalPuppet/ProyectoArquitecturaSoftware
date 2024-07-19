#define LED_PIN 23 
#define BUTTON_PIN 4 

int buttonState = 0; 
int lastButtonState = 0; 
bool ledState = false; 

void setup() {
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP); 
}

void loop() {

  buttonState = digitalRead(BUTTON_PIN);


  if (buttonState != lastButtonState) {

    if (buttonState == LOW) {
      ledState = !ledState; 
      digitalWrite(LED_PIN, ledState); 
    }
    delay(50); 
  }

  lastButtonState = buttonState; 
}
