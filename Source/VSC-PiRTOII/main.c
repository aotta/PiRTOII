/*
//                       PiRTO II Flash MultiCART by Andrea Ottaviani 2024
//
//  Intellivision flash multicart based on Raspberry Pico board -
//
//  More info on https://github.com/aotta/ 
//
//   parts of code are directly from the A8PicoCart project by Robin Edwards 2023
//  
//   Needs to be a release NOT debug build for the cartridge emulation to work
// 
//   Edit myboard.h depending on the type of flash memory on the pico clone//
//
//   v. 1.0 2024-05-04 : Initial version for Pi Pico 
//
*/



#include <stdio.h>
#include <string.h>

#include "pico/stdlib.h"
#include "pico/time.h"
#include "tusb.h"

#include "inty_cart.h"
#include "fatfs_disk.h"

//void cdc_task(void);


//--------------------------------------------------------------------+
// USB CDC
//--------------------------------------------------------------------+
void cdc_task(void)
{
  // connected() check for DTR bit
  // Most but not all terminal client set this when making connection
  // if ( tud_cdc_connected() )
  {
    // connected and there are data available
    if ( tud_cdc_available() )
    {
      // read data
      char buf[64];
      uint32_t count = tud_cdc_read(buf, sizeof(buf));
      (void) count;

      // Echo back
      // Note: Skip echo by commenting out write() and write_flush()
      // for throughput test e.g
      //    $ dd if=/dev/zero of=/dev/ttyACM0 count=10000
      tud_cdc_write(buf, count);
      tud_cdc_write_flush();
    }
  }
}

// Invoked when cdc when line state changed e.g connected/disconnected
void tud_cdc_line_state_cb(uint8_t itf, bool dtr, bool rts)
{
  (void) itf;
  (void) rts;

  // TODO set some indicator
  if ( dtr )
  {
    // Terminal connected
  }else
  {
    // Terminal disconnected
  }
}

// Invoked when CDC interface received data from host
void tud_cdc_rx_cb(uint8_t itf)
{
  (void) itf;
}


int main(void)
{
   tusb_init();
   stdio_init_all();   // for serial output, via printf()
   
    tud_init(BOARD_TUD_RHPORT);
    gpio_init(MSYNC_PIN);
    gpio_set_dir(MSYNC_PIN,false);
    gpio_init(RST_PIN);
    gpio_set_dir( RST_PIN,true);
    //gpio_put(RST_PIN,true);
    sleep_ms(2);
   
    while(1) {          
    
   // since v. 1.04 adopted gtortone (Minty) startup routine since it seems more compatible with different Inty'sversions     
   // reset interval in ms
   int t = 100;

   while (gpio_get(MSYNC_PIN) == 0 && to_ms_since_boot(get_absolute_time()) < 2000) {   // wait for Inty powerup
      if (to_ms_since_boot(get_absolute_time()) > t) {
         t += 100;
         gpio_put(RST_PIN, false);
         sleep_ms(5);
         gpio_put(RST_PIN, true);
      }
   }

   // check why loop is ended...
   if (gpio_get(MSYNC_PIN) == 1)
      Inty_cart_main();

/*
    while (to_ms_since_boot(get_absolute_time()) < 150)  // was 200, to test
    {
      if ((gpio_get(MSYNC_PIN)==1))
   //if ((gpio_get(1)==1)) // alternate_boot
   //      gpio_put(RST_PIN,false);
        Inty_cart_main();
    }

     
       // init device stack on configured roothub port
       // we are presumably powered from USB
       // enter USB mass storage mode
     //  tud_init(BOARD_TUD_RHPORT);
     //  stdio_init_all();   // for serial output, via printf()
     //  printf("Start\n");
*/
       while (1) {
          tud_task(); // tinyusb device task
         
          cdc_task();
       }
    
    }
   return 0;
}

//--------------------------------------------------------------------+
// Device callbacks
//--------------------------------------------------------------------+

// Invoked when device is mounted
void tud_mount_cb(void)
{
  printf("Device mounted\n"); 
  if (!mount_fatfs_disk())
    create_fatfs_disk();
}

// Invoked when device is unmounted
void tud_umount_cb(void)
{
  printf("Device unmounted\n");  
}

// Invoked when usb bus is suspended
// remote_wakeup_en : if host allow us  to perform remote wakeup
// Within 7ms, device must draw an average of current less than 2.5 mA from bus
void tud_suspend_cb(bool remote_wakeup_en)
{
  (void) remote_wakeup_en;
//  blink_interval_ms = BLINK_SUSPENDED;
}

// Invoked when usb bus is resumed
void tud_resume_cb(void)
{
//  blink_interval_ms = BLINK_MOUNTED;
}

