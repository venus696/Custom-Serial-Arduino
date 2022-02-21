/*
 * Uart 3 transmit a integer value "1" to Uart 2 ervery 200 milli seconds
 * Uart 1 serial print the total count of received integers from the start of the main loop.
 * 
 * Note: Before running this example on your mega board 
 * please ensure to short uart 2 with uart 3
*/
#include <CustomSerial.h>
#include <CustomSerial_Private.h>


uint8_t C_Serial1_rx_buffer[256] = {0};
bool pktReceived = false;


void C_Serial_Interrupt_Handler(uint8_t rx)
{
  ;
}


void C_Serial1_Interrupt_Handler(uint8_t rx)
{
    pktReceived = true;
    C_Serial1._rx_buffer[C_Serial1._rx_buffer_count] = rx;
    C_Serial1._rx_buffer_count = (C_Serial1._rx_buffer_count + 1)%C_Serial1._rx_buffer_size;
}


void C_Serial2_Interrupt_Handler(uint8_t rx)
{
  ;
}


void setup()
{
  /********Custom Serial 0************/
  C_Serial.begin(115200, C_Serial_Interrupt_Handler);
  while(!C_Serial);
  C_Serial.println("setup...in");

  /********Custom Serial 1************/
  C_Serial1.begin(115200, C_Serial1_Interrupt_Handler, C_Serial1_rx_buffer, (uint16_t)sizeof(C_Serial1_rx_buffer));    //receiver
  C_Serial1.clearSerial();

  /********Custom Serial 2************/
  C_Serial2.begin(115200, C_Serial2_Interrupt_Handler);                                                                //transmitter

  C_Serial.println("setup...exit");
}

uint16_t temp = 0;
void loop()
{
  delay(100);
  C_Serial2.write(temp);
  delay(100);
  if(pktReceived)
  {
    C_Serial.println(C_Serial1._rx_buffer_count);
    temp = C_Serial1._rx_buffer_count;
    pktReceived = false;
  }
}
