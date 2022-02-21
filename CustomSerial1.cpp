/*
 * CustomSerial1.cpp
 *
 * Created on: Nov 9, 2020
 * Author: Lynkit-Vinas
 */
 
#include "Arduino.h"
#include "CustomSerial.h"
#include "CustomSerial_Private.h"

// Each HardwareSerial is defined in its own file, since the linker pulls
// in the entire file when any element inside is used. --gc-sections can
// additionally cause unused symbols to be dropped, but ISRs have the
// "used" attribute so are never dropped and they keep the
// HardwareSerial instance in as well. Putting each instance in its own
// file prevents the linker from pulling in any unused instances in the
// first place.

#if defined(HAVE_C_SERIAL1)

#if defined(UART1_RX_vect)
ISR(UART1_RX_vect)
#elif defined(USART1_RX_vect)
ISR(USART1_RX_vect)
#else
#error "Don't know what the Data Register Empty vector is called for Serial1"
#endif
{
  C_Serial1._rx_complete_irq();
}

CustomSerial C_Serial1(&UBRR1H, &UBRR1L, &UCSR1A, &UCSR1B, &UCSR1C, &UDR1);

// Function that can be weakly referenced by serialEventRun to prevent
// pulling in this file if it's not otherwise used.
bool C_Serial1_available() {
  return C_Serial1.available();
}

#endif // HAVE_C_SERIAL1