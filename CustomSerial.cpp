/*
 * CustomSerial.cpp
 *
 * Created on: Nov 9, 2020
 * Author: Lynkit-Vinas
 */
 
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <util/atomic.h>
#include "Arduino.h"
#include "wiring_private.h"
#include "CustomSerial.h"

// this next line disables the entire CustomSerial.cpp, 
// this is so I can support Attiny series and any other chip without a uart
#if defined(HAVE_C_SERIAL0) || defined(HAVE_C_SERIAL1) || defined(HAVE_C_SERIAL2) || defined(HAVE_C_SERIAL3)

// C_SerialEvent functions are weak, so when the user doesn't define them,
// the linker just sets their address to 0 (which is checked below).
// The C_Serialx_available is just a wrapper around C_Serialx.available(),
// but we can refer to it weakly so we don't pull in the entire
// CustomSerial instance if the user doesn't also refer to it.
#if defined(HAVE_C_SERIAL0)
  void C_SerialEvent() __attribute__((weak));
  bool C_Serial0_available() __attribute__((weak));
#endif

#if defined(HAVE_C_SERIAL1)
  void C_SerialEvent1() __attribute__((weak));
  bool C_Serial1_available() __attribute__((weak));
#endif

#if defined(HAVE_C_SERIAL2)
  void C_SerialEvent2() __attribute__((weak));
  bool C_Serial2_available() __attribute__((weak));
#endif

#if defined(HAVE_C_SERIAL3)
  void C_SerialEvent3() __attribute__((weak));
  bool C_Serial3_available() __attribute__((weak));
#endif

void C_SerialEventRun(void)
{
#if defined(HAVE_C_SERIAL0)
  if (C_Serial0_available && C_SerialEvent && C_Serial0_available()) C_SerialEvent();
#endif
#if defined(HAVE_C_SERIAL1)
  if (C_Serial1_available && C_SerialEvent1 && C_Serial1_available()) C_SerialEvent1();
#endif
#if defined(HAVE_C_SERIAL2)
  if (C_Serial2_available && C_SerialEvent2 && C_Serial2_available()) C_SerialEvent2();
#endif
#if defined(HAVE_C_SERIAL3)
  if (C_Serial3_available && C_SerialEvent3 && C_Serial3_available()) C_SerialEvent3();
#endif
}

// macro to guard critical sections when needed for large TX buffer sizes
#if (CUSTOM_TX_BUFFER_SIZE>256)
#define TX_BUFFER_ATOMIC ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
#else
#define TX_BUFFER_ATOMIC
#endif


// Public Methods //////////////////////////////////////////////////////////////

void CustomSerial::begin(unsigned long baud, uint8_t config)
{
  // Try u2x mode first
  uint16_t baud_setting = (F_CPU / 4 / baud - 1) / 2;
  *_ucsra = 1 << U2X0;

  // hardcoded exception for 57600 for compatibility with the bootloader
  // shipped with the Duemilanove and previous boards and the firmware
  // on the 8U2 on the Uno and Mega 2560. Also, The baud_setting cannot
  // be > 4095, so switch back to non-u2x mode if the baud rate is too
  // low.
  if (((F_CPU == 16000000UL) && (baud == 57600)) || (baud_setting >4095))
  {
    *_ucsra = 0;
    baud_setting = (F_CPU / 8 / baud - 1) / 2;
  }

  // assign the baud_setting, a.k.a. ubrr (USART Baud Rate Register)
  *_ubrrh = baud_setting >> 8;
  *_ubrrl = baud_setting;

  _written = false;

  //set the data bits, parity, and stop bits
#if defined(__AVR_ATmega8__)
  config |= 0x80; // select UCSRC register (shared with UBRRH)
#endif
  *_ucsrc = config;
  
  sbi(*_ucsrb, RXEN0);
  sbi(*_ucsrb, TXEN0);
  sbi(*_ucsrb, RXCIE0);
  cbi(*_ucsrb, UDRIE0);
}

void CustomSerial::end()
{
  // wait for transmission of outgoing data
  //flush();

  cbi(*_ucsrb, RXEN0);
  cbi(*_ucsrb, TXEN0);
  cbi(*_ucsrb, RXCIE0);
  cbi(*_ucsrb, UDRIE0);
  
  // clear any received data
  _rx_buffer_count = 0;
}

int CustomSerial::available(void)
{
  return (unsigned int) _rx_buffer_count;
}

int CustomSerial::peek(void)
{

}

void CustomSerial::clearSerial(void)
{
  // clear any received data
  _rx_buffer_count = 0;
}

int CustomSerial::read(void)
{
  return _rx_buffer[0];
}

int CustomSerial::availableForWrite(void)
{
  return _tx_buffer_count;
}


size_t CustomSerial::write(uint8_t c)
{
  _written = true;
  /* Wait for empty transmit buffer */
  while ( !( *_ucsra & (1<<UDRE0)) );
  // If the buffer and the data register is empty, just write the byte
  // to the data register and be done. This shortcut helps
  // significantly improve the effective datarate at high (>
  // 500kbit/s) bitrates, where interrupt overhead becomes a slowdown.
  if (bit_is_set(*_ucsra, UDRE0))
  {
    // If TXC is cleared before writing UDR and the previous byte
    // completes before writing to UDR, TXC will be set but a byte
    // is still being transmitted causing flush() to return too soon.
    // So writing UDR must happen first.
    // Writing UDR and clearing TC must be done atomically, otherwise
    // interrupts might delay the TXC clear so the byte written to UDR
    // is transmitted (setting TXC) before clearing TXC. Then TXC will
    // be cleared when no bytes are left, causing flush() to hang
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) 
	{
      *_udr = c;
#ifdef MPCM0
      *_ucsra = ((*_ucsra) & ((1 << U2X0) | (1 << MPCM0))) | (1 << TXC0);
#else
      *_ucsra = ((*_ucsra) & ((1 << U2X0) | (1 << TXC0)));
#endif
    }
    return 1;
  }
}

#endif // whole file
