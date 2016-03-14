/* nrf24l01p.cpp -- Handle communication with the nrf24l01+ chip.
 *
 * Copyright (C) 2014 execuc
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#include "nrf24l01p.h"

nrf24l01p::nrf24l01p()
{
  /* EMPTY */
}

nrf24l01p::~nrf24l01p()
{
  /* EMPTY */
}

void nrf24l01p::setPins(uint8_t cePin, uint8_t csPin)
{
  mCePin = cePin;
  mCsPin = csPin;
  pinMode(mCePin,OUTPUT);
  pinMode(mCsPin,OUTPUT);
  setCeLow();
  setCsHigh();
}
  
void nrf24l01p::init(uint8_t payloadSize)
{
  // Initialize SPI
  SPI.begin();
  mPayloadSize = payloadSize;
  SPI.setDataMode(SPI_MODE0);
  SPI.setClockDivider(SPI_CLOCK_DIV2);
}


uint8_t nrf24l01p::rxMode(uint8_t initialFreq)
{
  setCeLow();
  writeRegister(CONFIG, _BV(EN_CRC) | _BV(CRCO));	// Enable CRC (2bytes)
  delayMicroseconds(100);
  writeRegister(EN_AA, 0x00);		// Disable auto acknowledgment
  writeRegister(EN_RXADDR, 0x01);	// Enable all data pipe (PO to P5)
  writeRegister(SETUP_AW, 0x03);	// 5 bytes address
  writeRegister(SETUP_RETR, 0xFF);	// 15 retransmit, 4000us pause
  writeRegister(RF_CH, initialFreq);	// channel 8
  writeRegister(RF_SETUP, 0x05); 	// 1Mbps, -6dBm power (0x06 => 1Mbps, 0dBm)
  writeRegister(STATUS, 0x70);		// Clear status register
  writeRegister(RX_PW_P0, 0x10);	// RX payload of 16 bytes
  writeRegister(FIFO_STATUS, 0x00);	// Nothing useful for write command
  const uint8_t* rx_tx_addr = reinterpret_cast<const uint8_t *>("\x66\x88\x68\x68\x68");
  writeRegister(RX_ADDR_P0, rx_tx_addr, 5);
  delay(50);
  flushTx();
  flushRx();
  delayMicroseconds(100);
  writeRegister(CONFIG, _BV(EN_CRC) | _BV(CRCO) | _BV(PWR_UP)  );
  delayMicroseconds(100);
  writeRegister(CONFIG, _BV(EN_CRC) | _BV(CRCO) | _BV(PWR_UP) | _BV(PRIM_RX) );
  delayMicroseconds(100);
  setCeHigh();
  delayMicroseconds(100);
}

uint8_t nrf24l01p::readRegister(uint8_t reg)
{
  setCsLow();
  SPI.transfer( R_REGISTER | ( REGISTER_MASK & reg ) );
  uint8_t result = SPI.transfer(0xff);

  setCsHigh();
  return result;
}

uint8_t nrf24l01p::writeRegister(uint8_t reg, const uint8_t* buf, uint8_t len)
{
  setCsLow();
  uint8_t result = SPI.transfer( W_REGISTER | ( REGISTER_MASK & reg ) );
  while ( len-- )
    SPI.transfer(*buf++);

  setCsHigh();

  return result;
}

uint8_t nrf24l01p::writeRegister(uint8_t reg, uint8_t value)
{
  setCsLow();
  uint8_t result = SPI.transfer( W_REGISTER | ( REGISTER_MASK & reg ) );
  SPI.transfer(value);
  setCsHigh();

  return result;
}

uint8_t nrf24l01p::readPayload(void* buf, uint8_t len)
{
  uint8_t result;
  uint8_t* current = reinterpret_cast<uint8_t*>(buf);

  uint8_t data_len = min(len,mPayloadSize);
  uint8_t blank_len = mPayloadSize - data_len;
  
  setCsLow();
  result = SPI.transfer( R_RX_PAYLOAD );
  while ( data_len-- )
    *current++ = SPI.transfer(0xff);
  while ( blank_len-- )
    SPI.transfer(0xff);
  setCsHigh();

  return result;
}

uint8_t nrf24l01p::flushRx(void)
{
  setCsLow();
  uint8_t result = SPI.transfer( FLUSH_RX );
  setCsHigh();

  return result;
}

uint8_t nrf24l01p::flushTx(void)
{
  setCsLow();
  uint8_t result = SPI.transfer( FLUSH_TX );
  setCsHigh();

  return result;
}
