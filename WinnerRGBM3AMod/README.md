# Winner_RGB_M3A MOD

## Serial port 

Serial port communication is configured to 19200 baud/s, 8 data bits, even parity


## Protocol

Mod board can be controlled via serial connection/bluetooth. Commands are separated by new line character <CR> or <LF>

### SET

Set lamp state - turn on/off, dim, change light temperature. If cold_level > 0 and warm_level > 0, the cold_level has to be equal to warm_level. 

  SET:<COLD_LEVEL><WARM_LEVEL>

<COLD_LEVEL> = number 0 to 5
<WARM_LEVEL> = number 0 to 5
  

Example:

  SET:11

Response:

  SET_OK:11


### PIN

Set bluetooth pin

  PIN:<pin_code>

<pin_code> = 4 digit code

Example:

  PIN:0000
  
Response:

  PIN_OK:0000
  

### STATUS

Get lamp status - light levels

Example:

  STATUS

Response:

  STATUS:<COLD_LEVEL><WARM_LEVEL>

<COLD_LEVEL> = number 0 to 5
<WARM_LEVEL> = number 0 to 5
  
Response example:

  STATUS:44