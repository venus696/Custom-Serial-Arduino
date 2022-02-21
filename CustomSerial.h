/*
 * CustomSerial.h
 *
 * Created on: Nov 9, 2020
 * Author: Lynkit-Vinas
 */
#ifndef CustomSerial_h
#define CustomSerial_h


#include <inttypes.h>
#include "Stream.h"

// Define constants and variables for buffering incoming serial data.  We're
// using a ring buffer (I think), in which head is the index of the location
// to which to write the next incoming character and tail is the index of the
// location from which to read.
// NOTE: a "power of 2" buffer size is reccomended to dramatically
//       optimize all the modulo operations for ring buffers.
// WARNING: When buffer sizes are increased to > 256, the buffer index
// variables are automatically increased in size, but the extra
// atomicity guards needed for that are not implemented. This will
// often work, but occasionally a race condition can occur that makes
// Serial behave erratically. See https://github.com/arduino/Arduino/issues/2405


#define HAVE_C_SERIAL0
//#define HAVE_C_SERIAL1
//#define HAVE_C_SERIAL2
//#define HAVE_C_SERIAL3

#if !defined(CUSTOM_TX_BUFFER_SIZE)
#define CUSTOM_TX_BUFFER_SIZE 255
#endif
#if !defined(CUSTOM_RX_BUFFER_SIZE)
#define CUSTOM_RX_BUFFER_SIZE 255
#endif

// Define config for Serial.begin(baud, config);
#define SERIAL_5N1 0x00
#define SERIAL_6N1 0x02
#define SERIAL_7N1 0x04
#define SERIAL_8N1 0x06
#define SERIAL_5N2 0x08
#define SERIAL_6N2 0x0A
#define SERIAL_7N2 0x0C
#define SERIAL_8N2 0x0E
#define SERIAL_5E1 0x20
#define SERIAL_6E1 0x22
#define SERIAL_7E1 0x24
#define SERIAL_8E1 0x26
#define SERIAL_5E2 0x28
#define SERIAL_6E2 0x2A
#define SERIAL_7E2 0x2C
#define SERIAL_8E2 0x2E
#define SERIAL_5O1 0x30
#define SERIAL_6O1 0x32
#define SERIAL_7O1 0x34
#define SERIAL_8O1 0x36
#define SERIAL_5O2 0x38
#define SERIAL_6O2 0x3A
#define SERIAL_7O2 0x3C
#define SERIAL_8O2 0x3E

typedef void (*EventHandler)(uint8_t v);

class CustomSerial : public Stream
{
  protected:
    volatile uint8_t * const _ubrrh;
    volatile uint8_t * const _ubrrl;
    volatile uint8_t * const _ucsra;
    volatile uint8_t * const _ucsrb;
    volatile uint8_t * const _ucsrc;
    volatile uint8_t * const _udr;
    // Has any byte been written to the UART since begin()
    bool _written;
    
  public:
    inline CustomSerial(
      volatile uint8_t *ubrrh, volatile uint8_t *ubrrl,
      volatile uint8_t *ucsra, volatile uint8_t *ucsrb,
      volatile uint8_t *ucsrc, volatile uint8_t *udr);  
	uint8_t  _temp_buffer;
	uint8_t* _rx_buffer;
    uint8_t* _tx_buffer;
	uint16_t _rx_buffer_size;
	uint16_t _tx_buffer_size; 
	volatile uint16_t _rx_buffer_count;
    volatile uint16_t _tx_buffer_count;
	void begin(unsigned long baud, EventHandler handler) 
	{ 
		begin(baud, SERIAL_8N1);
		_rx_complete_irq_fun_poin = handler;
		_rx_buffer = &_temp_buffer;
		_rx_buffer_size = 1;
	}
	void begin(unsigned long baud, EventHandler handler, uint8_t* rx_buffer, uint16_t rx_buffer_size) 
	{ 
		begin(baud, SERIAL_8N1);
		_rx_complete_irq_fun_poin = handler;
		_rx_buffer = rx_buffer;
		_rx_buffer_size = rx_buffer_size;
	}
	void begin(unsigned long, uint8_t);
    void end();
    virtual int available(void);
    virtual int peek(void);
    virtual int read(void);
    virtual int availableForWrite(void);
	virtual void clearSerial(void);
    virtual size_t write(uint8_t);
    inline size_t write(unsigned long n) { return write((uint8_t)n); }
    inline size_t write(long n) { return write((uint8_t)n); }
    inline size_t write(unsigned int n) { return write((uint8_t)n); }
    inline size_t write(int n) { return write((uint8_t)n); }
    using Print::write; // pull in write(str) and write(buf, size) from Print
    operator bool() { return true; }

   // Interrupt handlers - Not intended to be called externally 
   inline void _rx_complete_irq(void);
   void (*_rx_complete_irq_fun_poin)(uint8_t rx);	
};

#if (defined(UBRRH) || defined(UBRR0H)) && defined(HAVE_C_SERIAL0)
  extern CustomSerial C_Serial;
#endif
#if defined(UBRR1H) && defined(HAVE_C_SERIAL1)
  extern CustomSerial C_Serial1;
#endif
#if defined(UBRR2H) && defined(HAVE_C_SERIAL2)
  extern CustomSerial C_Serial2;
#endif
#if defined(UBRR3H) && defined(HAVE_C_SERIAL3)
  extern CustomSerial C_Serial3;
#endif

extern void serialEventRun(void) __attribute__((weak));

#endif
