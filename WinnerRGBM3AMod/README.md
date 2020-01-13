# Winner_RGB_M3A MOD

This mod board reads lamp status and stores it in EEPROM. When the power is turned off and back on, it is restored to stored state.

## Serial port 

Serial port communication is configured to 19200 baud/s, 8 data bits, even parity


## Protocol

Mod board can be controlled via serial connection/bluetooth. Commands are separated by new line character &lt;CR&gt; or &lt;LF&gt;

### SET

Set lamp state - turn on/off, dim, change light temperature. If cold_level &gt; 0 and warm_level &gt; 0, the cold_level has to be equal to warm_level. 

  SET&lt;COLD_LEVEL&gt;&lt;WARM_LEVEL&gt;

&lt;COLD_LEVEL&gt; = number 0 to 5
&lt;WARM_LEVEL&gt; = number 0 to 5
  

Example:

  SET11

Response:

  SET_OK:11


### PIN

Set bluetooth pin

  PIN&lt;pin_code&gt;

&lt;pin_code&gt; = 4 digit code

Example:

  PIN0000
  
Response:

  PIN_OK:0000
  

### STATUS

Get lamp status - light levels

Example:

  STATUS

Response:

  STATUS:&lt;INPUT1&gt;,&lt;INPUT2&gt;,&lt;COLD_LEVEL&gt;,&lt;WARM_LEVEL&gt;,&lt;EEPROM_COLD_LEVEL&gt;,&lt;EEPROM_WARM_LEVEL&gt;

&lt;INPUT1&gt; = value from A/D converter
&lt;INPUT2&gt; = value from A/D converter
&lt;COLD_LEVEL&gt; = number 0 to 5
&lt;WARM_LEVEL&gt; = number 0 to 5
&lt;EEPROM_COLD_LEVEL&gt; = number 0 to 5
&lt;EEPROM_WARM_LEVEL&gt; = number 0 to 5
  
Response example:

  STATUS:0,312,0,2,0,2
  
### DEBUG

Toggle debug mode

Example:

  DEBUG

Response:

  DEBUG_OK  