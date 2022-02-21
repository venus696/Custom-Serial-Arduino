/*
 * CustomSerial0.cpp
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

#if defined(HAVE_C_SERIAL0)

#if defined(USART_RX_vect)
ISR(USART_RX_vect)
#elif defined(USART0_RX_vect)
ISR(USART0_RX_vect)
#elif defined(USART_RXC_vect)
ISR(USART_RXC_vect) // ATmega8
#else
  #error "Don't know what the Data Received vector is called for Serial"
#endif
{
  C_Serial._rx_complete_irq();
}
  
#if defined(UBRRH) && defined(UBRRL)
  CustomSerial C_Serial(&UBRRH, &UBRRL, &UCSRA, &UCSRB, &UCSRC, &UDR);
#else
  CustomSerial C_Serial(&UBRR0H, &UBRR0L, &UCSR0A, &UCSR0B, &UCSR0C, &UDR0);
#endif

// Function that can be weakly referenced by serialEventRun to prevent
// pulling in this file if it's not otherwise used.
bool C_Serial0_available() {
  return C_Serial.available();
}

#endif // HAVE_C_SERIAL0
