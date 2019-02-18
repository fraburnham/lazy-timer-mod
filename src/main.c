#include <Energia.h>

#define G2230

#ifdef G2553
// launchpad dev config
#define SWITCH P1_3
#define SIGOUT RED_LED
#endif

#ifdef G2230
// mod config
#define SWITCH P1_2
#define SIGOUT P1_5
#endif

#define DEBOUNCE_DELAY_MS 250

volatile long lastISRTime = 0;

void basket_lifted_isr() {
  if((millis() - lastISRTime) > DEBOUNCE_DELAY_MS) {
    digitalWrite(SIGOUT, HIGH);
    lastISRTime = millis();
  }
}

void setup() {
  pinMode(SIGOUT, OUTPUT);
  pinMode(SWITCH, INPUT_PULLUP);
  attachInterrupt(SWITCH, basket_lifted_isr, FALLING);
}

void loop() {
  LPM1;  // need more research into low power modes. This is a fairly high power choice...
  // the digital interrupt is supposed to wake the device from _any_ LPM... What did I do wrong?
  delay(100);
  digitalWrite(SIGOUT, LOW);
}
