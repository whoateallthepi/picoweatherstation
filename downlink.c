#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"

#include "hardware/rtc.h"

#include "constants.h"
#include "types.h"
#include "downlink.h"
#include "utilities.h"
#include "ds3231.h"

#define DS3231 // Include code to read the external clock

extern stationData stationdata;

void set_timezone(incomingMessage *data)
{

    /* Message will look like: 
     * fffa\r\n
     * ^  ^            
     * timezone seconds correction
     * (here -1) (here -6)
     * 
     * timezone is currently hours only - review
     * 
     */
  stationdata.timezone = hex2int2sComplement(data->incomingdata.timemessage.timezone);

  /* Deal with the seconds correction 
   *
   */ 
  int seconds_offset = hex2int2sComplement(data->incomingdata.timemessage.seconds_offset);
  if ( seconds_offset != 0)
  {
    datetime_t t0, t1;
    time_t t0_s, t1_s;

    t0 = ds3231ReadTime();
    t0_s = rtc_to_epoch(t0, 0); // rtc now always runs UTC
    t1_s = t0_s + seconds_offset;

    epoch_to_rtc(t1_s, &t1);

    ds3231SetTime(t1);

  }

  //if an external clock is connected - update the pico board clock

  #ifdef DS3231
  datetime_t t = ds3231ReadTime();
  rtc_set_datetime(&t);
  #endif
  
}

void set_station_data(incomingMessage *data)
{

  int32_t latitude, longitude;
  int16_t altitude;

  latitude = hex2int32(data->incomingdata.stationdatamessage.latitude);
  longitude = hex2int32(data->incomingdata.stationdatamessage.longitude);
  altitude = hex2int16(data->incomingdata.stationdatamessage.altitude);

  stationdata.longitude = (float)longitude / 100000; // five implied decimals
  stationdata.latitude = (float)latitude / 100000;
  stationdata.altitude = altitude;

  // Double flash CORE0_LED to confirm update
  led_double_flash(CORE0_LED);

#ifdef TRACE
  printf("< set_station_data\n");
#endif
}
