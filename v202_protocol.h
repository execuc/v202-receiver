/* v202_protocol.h -- Handle the v202 protocol.
 *
 * Copyright (C) 2016 execuc
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#ifndef V202_PROTOCOL_H_
#define V202_PROTOCOL_H_

#include "Arduino.h"
#include "nrf24l01p.h"


typedef struct __attribute__((__packed__)) {
  uint8_t throttle;
  int8_t yaw;
  int8_t pitch;
  int8_t roll;
  int8_t trim_yaw;
  int8_t trim_pitch;
  int8_t trim_roll;
  uint8_t flags;
  uint8_t crc;
} rx_values_t;


enum rxState
{
  NO_BIND = 0,
  WAIT_FIRST_SYNCHRO,
  BOUND,
  SIGNAL_LOST
};


enum rxReturn
{
  BOUND_NEW_VALUES = 0,   // Bound state, frame received with new TX values
  BOUND_NO_VALUES,        // Bound state, no new frame received
  NOT_BOUND,              // Not bound, initial state
  BIND_IN_PROGRESS,       // Bind in progress, first frame has been received with TX id, wait no bind frame
  ERROR_SIGNAL_LOST,      // Signal lost
  UNKNOWN                 // ???, not used for moment
};

class v202Protocol
{
public:
  v202Protocol();
  ~v202Protocol();

  void init(nrf24l01p *wireless);
  uint8_t run(rx_values_t *rx_value );
  
protected:
  void retrieveFrequency();
  bool checkCRC();
  inline bool checkTXaddr() { return ( mTxid[0] == mFrame[7] && mTxid[1] == mFrame[8] && mTxid[2] == mFrame[9]);}

  nrf24l01p *mWireless;
  uint8_t mTxid[3];
  uint8_t mRfChannels[16];
  uint8_t mRfChNum;
  uint8_t mFrame[16];
  uint8_t mState;
  uint8_t mErrorTimeoutCode;
  uint16_t mTimeout;
  unsigned long mLastSignalTime;
};
#endif /* V202_PROTOCOL_H_ */
