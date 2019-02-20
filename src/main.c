#include <msp430.h>
#include <msp430g2230.h>

#define HIGH 1
#define LOW 0
#define OUTPUT 1
#define INPUT 0
#define ENABLE 1
#define DISABLE 0

#define DISABLE_INTERRUPTS(port) port = 0
#define GPIO_PULLUP(pin_bit, enable) enable ? (P1REN |= pin_bit) : (P1REN &= ~pin_bit)
#define GPIO_DIRECTION(pin_bit, direction) direction ? (P1DIR |= pin_bit) : (P1DIR &= ~pin_bit)
#define GPIO_SET_INTERRUPT(pin_bit) P1IE |= pin_bit
#define GPIO_CLEAR_INTERRUPT(pin_bit) P1IFG &= ~pin_bit
#define GPIO_FALLING_EDGE_INTERRUPT(pin_bit) P1IES |= pin_bit
#define GPIO_WRITE(pin_bit, level) level ? (P1OUT |= pin_bit) : (P1OUT &= ~pin_bit)
#define GPIO_READ(pin_bit) (P1IN & pin_bit) ? 1 : 0
#define TIMER_SET_COMPARE(i) TACCR0 = i

#define SWITCH BIT2
#define SIGOUT BIT5
#define PULSE_LENGTH_CYCLES 200

#define TIMER_CONFIG TACTL = TASSEL_1 | ID_2 | MC_1
#define TIMER_ENABLE_INTERRUPT TACTL |= TAIE
#define TIMER_CLEAR_INTERRUPT TACTL &= ~TAIFG
#define TIMER_RESET_COUNTER TAR = 0
#define WATCHDOG_DISABLE WDTCTL = WDTPW | WDTHOLD
#define CLOCK_1MHZ BCSCTL1 |= CALBC1_1MHZ; DCOCTL |= CALDCO_1MHZ
#define CLOCK_LOW_FREQUENCY BCSCTL1 &= ~XTS
#define CLOCK_ACLK_VLO_SOURCE BCSCTL3 = LFXT1S_2

#pragma vector=TIMERA1_VECTOR
__interrupt void timer_isr() {
  if(TAIV == 0x0A) {
    TIMER_CLEAR_INTERRUPT;
    GPIO_WRITE(SIGOUT, LOW);  // Stop the pulse
    GPIO_SET_INTERRUPT(SWITCH);
  }
}

#pragma vector=PORT1_VECTOR
__interrupt void port1_isr() {
  if((P1IFG & SWITCH) && !GPIO_READ(SWITCH)) {
    GPIO_CLEAR_INTERRUPT(SWITCH);
    DISABLE_INTERRUPTS(P1IE);
    
    GPIO_WRITE(SIGOUT, HIGH);  // Start the pulse

    TIMER_ENABLE_INTERRUPT;
    TIMER_RESET_COUNTER;
    TACCR0 = PULSE_LENGTH_CYCLES;
  }
}

int main(void) {
  WATCHDOG_DISABLE;

  CLOCK_1MHZ;
  CLOCK_LOW_FREQUENCY;
  CLOCK_ACLK_VLO_SOURCE;
  
  GPIO_DIRECTION(SIGOUT, OUTPUT);
  GPIO_WRITE(SIGOUT, LOW);
    
  GPIO_DIRECTION(SWITCH, INPUT);
  GPIO_FALLING_EDGE_INTERRUPT(SWITCH);
  GPIO_CLEAR_INTERRUPT(SWITCH);
  GPIO_SET_INTERRUPT(SWITCH);

#ifdef DEV  // in dev enable the pullup so the button on the board works
  GPIO_PULLUP(SWITCH, ENABLE);
#else
  GPIO_PULLUP(SWITCH, DISABLE);
#endif

  TIMER_SET_COMPARE(0);
  TIMER_CONFIG;
  TIMER_CLEAR_INTERRUPT;

  _BIS_SR(LPM3_bits + GIE);
}
