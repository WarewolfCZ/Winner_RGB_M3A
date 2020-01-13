# Winner_RGB_M3A MOD

This mod board reads lamp status and stores it in EEPROM. When the power is turned off and back on, it is restored to stored state.

## Serial port 

Serial port communication is configured to 19200 baud/s, 8 data bits, even parity


## Protocol

Mod board can be controlled via serial connection/bluetooth. Commands are separated by new line character <CR> or <LF>

### SET

Set lamp state - turn on/off, dim, change light temperature. If cold_level > 0 and warm_level > 0, the cold_level has to be equal to warm_level. 

  SET<COLD_LEVEL><WARM_LEVEL>

<COLD_LEVEL> = number 0 to 5
<WARM_LEVEL> = number 0 to 5
  

Example:

  SET11

Response:

  SET_OK:11


### PIN

Set bluetooth pin

  PIN<pin_code>

<pin_code> = 4 digit code

Example:

  PIN0000
  
Response:

  PIN_OK:0000
  

### STATUS

Get lamp status - light levels

Example:

  STATUS

Response:

  STATUS:<INPUT1>,<INPUT2>,<COLD_LEVEL>,<WARM_LEVEL>,<EEPROM_COLD_LEVEL>,<EEPROM_WARM_LEVEL>

<INPUT1> = value from A/D converter
<INPUT2> = value from A/D converter
<COLD_LEVEL> = number 0 to 5
<WARM_LEVEL> = number 0 to 5
<EEPROM_COLD_LEVEL> = number 0 to 5
<EEPROM_WARM_LEVEL> = number 0 to 5
  
Response example:

  STATUS:0,312,0,2,0,2
  
### DEBUG

Toggle debug mode

Example:

  DEBUG

Response:

  DEBUG_OK  