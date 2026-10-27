#include "freertos/FreeRTOS.h" 
#include "hw.h"
#include "lcd.h"
#include "net.h"
#include "com.h"
#include "graphics.h"
#include "config.h"

#define DELAY_TIME_MS 4000


uint8_t sender[NET_ALEN];

// Initialize the communication channel.
// Return zero if successful, or non-zero otherwise.
int32_t com_init(void)
{
    /*
    Initialize the WiFi network with a call to net_init().
    Open group registration with a call to net_group_open(). Use a group id of 1 for compatibility with other students in the lab.
    Print a message to the LCD, "Waiting to join group...".
    Wait 4 seconds by calling the FreeRTOS function vTaskDelay().
    Close group registration with a call to net_group_close().
    */

    net_init();
    net_group_open(9);
    graphics_drawMessage("Waiting to join group...", CONFIG_WIFI_CLR, CONFIG_BACK_CLR);
    vTaskDelay(pdMS_TO_TICKS(DELAY_TIME_MS));
    return net_group_close();
}

// Free resources used for communication.
// Return zero if successful, or non-zero otherwise.
int32_t com_deinit(void)
{
    return net_deinit();
}

// Write data to the communication channel. Does not wait for data.
// *buf: pointer to data buffer
// size: size of data in bytes to write
// Return number of bytes written, or negative number if error.
int32_t com_write(const void *buf, uint32_t size)
{
    return net_send(NULL, buf, size, 0);
}

// Read data from the communication channel. Does not wait for data.
// *buf: pointer to data buffer
// size: size of data in bytes to read
// Return number of bytes read, or negative number if error.
int32_t com_read(void *buf, uint32_t size)
{
    return net_recv(sender, buf,  size, 0);
}
