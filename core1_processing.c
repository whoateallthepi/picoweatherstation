#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "hardware/irq.h"
#include "hardware/gpio.h"
#include "constants.h"
#include "core1_processing.h"
#include "hardware/adc.h"
//#include "bme280.h"  
#include "types.h"

//#define DEBUG
//#define TRACE
#undef DEBUG
#undef TRACE


volatile int windClicks = 0;

uint64_t lastSecond = 0;
uint64_t lastWindCheck = 0;
uint64_t lastWindIRQ = 0; // Used for primitive debouncing
uint64_t lastRainIRQ = 0; // Used for primitive debouncing

char minutes = 0;  // index for minute array rain data (used in IRQs)

/* ================= Note on timers ================================
 * Getting the processor time the usual was time_us_64() during the 
 * interrupts was causing UART errors on the other core. (and occasional SPI 
 * errors on this core). I'm not completely sure *why*, but I suspect it 
 * is because reading the lower register, according to the docs, 'latches' 
 * the upper register to allow a reliable time to be grabbed. I suspect 
 * this latching stuffs up the UART timing. 
 * 
 * To remedy this I'm accessing the raw timer registers timer_hw->timerawl and
 * timer_hw->timerawh. This is via an inline function raw_time_64(). Again this
 * is to speed the interrupt processing as much as possible.
 * .
 * This isn't ideal as there is a chance of the upper register incrementing 
 * after the lower register is read (and before the upper is read).
 * I'm prepared to wear this as it is a *relatively* remote happening and
 * the consequences are minor (a failure to debounce, maybe, or an
 * instantaneous mis-read of wind speed that will fix in next second.
 *
 * The 32-bit lower register covers 4294 seconds (about two hours). I'll
 * wear that for now and run some careful tests to see if we need to code
 * for this.
 */ 

void core1_process (void) {
	//printf("hello from core1_process()\n");
    
	uint64_t now;
	char seconds_2m = 0; // These two are indexes to the arrays
    char seconds = 0; 
	
	char minutes_10m = 0;
    
    //#if defined TRACE || defined DEBUG
    //  stdio_init_all();
    //  sleep_ms(10000); // give time to start serial terminal
    //  printf("> core1_processing()\n"); 
    //#endif  
	
	// Initialise pins and GPIO
	gpio_pull_up(WIND_IRQ);
    gpio_pull_up(RAIN_IRQ);
    
    gpio_set_irq_enabled(WIND_IRQ,GPIO_IRQ_EDGE_FALL, true);
    gpio_set_irq_enabled(RAIN_IRQ,GPIO_IRQ_EDGE_FALL, true);
    
    // The following actually sets for ALL GPIO IRQs - review with 
    // developments in the SDK
    
    gpio_set_irq_enabled_with_callback(WIND_IRQ, 
                                       GPIO_IRQ_EDGE_FALL, 
                                       true, 
                                       &gpio_callback); 
	// The ADC for wind direction
    adc_init();
    adc_gpio_init(WIND_DIR_GP); // makes sure no pullups etc
    adc_select_input(WIND_DIR);                                   
                                       
    // The BME280 pressure/humity/temperature sensor
    
    /*if (!bme280_initialise()) {
        printf("Error: failed to initialise BME280\n");
        assert(true); 
    } */
    
    /* =========================== main process loop ==================
     */
     
    while(1) {
		
		if (time_us_64() - lastSecond >= 1000000) {
            
            #ifdef DEBUG
              printf("...second processing\n");
            #endif            
            
            lastSecond += 1000000; // microseconds = 1s
            
            // wind - keep 120 readings each second for 2 mins 
            
            // Increment the array counters, roll back to zero if needed
            if (++seconds_2m > 119) seconds_2m = 0;
            
            get_wind_readings();
            
            windspeed[seconds_2m].speed = current_wind.speed;
            windspeed[seconds_2m].direction = current_wind.direction;
            
            // check the gust for the minute
            if (current_wind.speed > gust10m[minutes_10m].speed) {
                gust10m[minutes_10m].speed = current_wind.speed;
                gust10m[minutes_10m].direction = current_wind.direction;
            }
            // check if this is the fastest gust for the day
            if (current_wind.speed > max_gust.speed) {
                max_gust.speed = current_wind.speed;
                max_gust.direction = current_wind.direction;
            }
            
            // Read the BME280 every 10 seconds 
            
            /*if ((seconds % 10) == 0) {
				bme280_fetch (&humidity, &pressure, &temperature);  
				
				#ifdef DEBUG
                  printf("Humidity = %.2f%%\n", humidity / 1024.0);
                  printf("Pressure = %dPa\n", pressure);
                  printf("Temp. = %.2fC\n", temperature / 100.0);
                #endif
			}*/
            
            /* Rain: Minute totals for an hour
             * Wind Gusts - 10 minutes worth of max gust in minute
             */
            
            if (++seconds > 59) {
                seconds = 0;
                if (++minutes > 59) minutes = 0;
                if (++minutes_10m > 9) minutes_10m = 0;
                
                // This is a new minute - reset the rain click total
                rainHour[minutes] = 0;
                
                gust10m[minutes_10m].speed = 0.0; // and wind gusts
                gust10m[minutes_10m].direction = 0; //
            }
		} // End every second processing
	} // End main loop 
}
void gpio_callback(uint gpio, uint32_t events) {
    // handles ALL the GPIO interrupts
     
    #ifdef TRACE
      printf("> gpio_callback\n");
    #endif  
    
    #ifdef DEBUG
      printf("...pin detected: %d\n", gpio);
    #endif  
    
    
    if (gpio == WIND_IRQ) {
        uint64_t now = raw_time_64();
        if ((now - lastWindIRQ) > DEBOUNCE) {
            // We have a valid interrupt - process 
            lastWindIRQ = now; 
            windClicks ++;
        }
        else {
            #ifdef DEBUG
              printf("...debounced wind\n");
           #endif 
       }
       return;
   }
            
    if (gpio == RAIN_IRQ) {
        uint64_t now = raw_time_64();
        if ((now - lastRainIRQ) > DEBOUNCE) {
            //printf("...interrupt: %lld\n");
            // valid interrupt 
            lastRainIRQ = now;
            rainToday += RAIN_CLICK;
            rainSinceLast += RAIN_CLICK;
            rainHour[minutes] += RAIN_CLICK;
        }
         else {
             #ifdef DEBUG
              printf("...debounced rain\n");
             #endif 
       }
        return;
    }
    
    // unrecognised interrupt 
    assert(true);
}
void get_wind_readings () {
    #ifdef TRACE
      printf("> get_wind_readings(core1)\n");
    #endif 
    
    float deltaTime;
    uint64_t deltaTime_micros;
    uint64_t now;
    
    now = time_us_64();
    
    deltaTime_micros = now - lastWindCheck;
    
    #ifdef DEBUG
      printf("...deltaTime_micros: %lld\n", deltaTime_micros);
    #endif  
    
    deltaTime = deltaTime_micros / 1000000.0; // in seconds 
    
    // WINDCLICK converts clicks per second to speed
    
    #ifdef DEBUG
      printf("...windClicks: %d\n", windClicks);
      printf("... deltaTime: %.2f\n", deltaTime);
    #endif  
    
    float windSpeed = ((float)windClicks / deltaTime) * WINDCLICK;
    current_wind.speed = windSpeed;
    
    windClicks = 0; // Reset ready for next reading
    lastWindCheck = time_us_64();
    
    // Direction
    current_wind.direction = get_wind_direction();
    
    #ifdef DEBUG
      printf("... latest windspeed: %.2f\n", current_wind.speed);
      printf("... latest direction: %.2f\n", current_wind.direction);
    #endif  
     
    #ifdef TRACE
      printf("< get_wind_readings(core1)\n");
    #endif 
}  
  
  
float get_wind_direction () {
    #ifdef TRACE
      printf("> get_wind_direction\n");
    #endif
      
    uint16_t adc = adc_read();
    //#ifdef DEBUG 
    //  printf("... ADC raw value: 0x%03x\n", adc);
    //#endif
    int sector = get_sector(adc);
    
    float direction = (PI/8) * sector;
    
    //#ifdef DEBUG
    //printf("...wind direction in Radians: %.2f", direction);
    //#endif  
    
    #ifdef TRACE
      printf("< get_wind_direction\n");
    #endif
    
    return direction;
}

int get_sector (uint16_t adc) {
    #ifdef TRACE
      printf("> get_sector\n");
    #endif
    
    if (adc < 0x12B) return 5; 
    if (adc < 0x161) return 3; 
    if (adc < 0x1B7) return 4; 
    if (adc < 0x26E) return 7; 
    if (adc < 0x35A) return 6; 
    if (adc < 0x427) return 9; 
    if (adc < 0x56A) return 8; 
    if (adc < 0x6C6) return 1;
    if (adc < 0x849) return 2; 
    if (adc < 0x99A) return 11; 
    if (adc < 0xA69) return 10; 
    if (adc < 0xBA0) return 15; 
    if (adc < 0xC99) return 0; 
    if (adc < 0xD64) return 13; 
    if (adc < 0xE50) return 14;
    if (adc < 0xF61) return 12; 
    return -1; //error
    
    #ifdef TRACE
      printf("< get_sector\n");
    #endif
}

static inline uint64_t raw_time_64(void) {
	uint32_t lo = timer_hw->timerawl;
    uint32_t hi = timer_hw->timerawh;
    return ((uint64_t) hi << 32u) | lo;
}

    
	
