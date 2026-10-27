#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <watch.h>
#include <driver/gptimer.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_timer.h"

#include "hw.h"  //defines I/O pins associated with buttons
#include "lcd.h"
#include "pin.h" // functions from lab 2 to control pin setup

static const char *TAG = "lab03";

#define CLOCK_HZ  1 * 1000 * 1000  // 1 MHz
#define ALARM_COUNT 10000  // 10 ms
#define ISR_CNT_TIMER 500 // 1 s

// global variables
volatile bool running = false;
volatile uint32_t timer_ticks = 0;

volatile int64_t isr_max; // Maximum ISR execution time (us)
volatile int32_t isr_cnt; // Count of ISR invocations


// timer call back function
static bool timer_cb(gptimer_handle_t timer, const gptimer_alarm_event_data_t *edata, void *user_ctx)
{
    // timer to test ISR run
    int64_t start, finish;
    start = esp_timer_get_time();
    isr_cnt++;

    // increase tick counter if running
    if(running) timer_ticks++;
    
    // starts the timer on button A press
    if (!(pin_get_level(HW_BTN_A)))
    {
        running = true;
    }

    // stops the timer on button B press
    if (!(pin_get_level(HW_BTN_B)))
    {
        running = false;
    }

    // stops and resets timer on button START press
    if (!(pin_get_level(HW_BTN_START)))
    {
        running = false;
        timer_ticks = 0;
    }

    // stop the test timer and save the value if it is the largest
    finish = esp_timer_get_time();
    if((finish - start) > isr_max) isr_max = (finish - start);

    return false;
}


// Main application.
void app_main(void)
{
    // will hold count times for testing
    int64_t start, finish;

    start = esp_timer_get_time();

    // button setup
    pin_reset(HW_BTN_A);
    pin_reset(HW_BTN_B);
    pin_reset(HW_BTN_START);

    pin_input(HW_BTN_A, true);
    pin_input(HW_BTN_B, true);
    pin_input(HW_BTN_START, true);

    finish = esp_timer_get_time();
    printf("Pin setup time: %lld microseconds\n", finish-start);

    // timer to test gsp timer setup
    start = esp_timer_get_time();

    // timer setup
    gptimer_handle_t gptimer = NULL;
    gptimer_config_t timer_config = 
    {
        .clk_src = GPTIMER_CLK_SRC_DEFAULT, // Select the default clock source
        .direction = GPTIMER_COUNT_UP,      // Counting direction is up
        .resolution_hz = CLOCK_HZ,   // Resolution is 1 MHz, i.e., 1 tick equals 1 microsecond
    };

    gptimer_alarm_config_t alarm_config = 
    {
        .reload_count = 0,      // When the alarm event occurs, the timer will automatically reload to 0
        .alarm_count = ALARM_COUNT, // Set the actual alarm period, since the resolution is 1us, 1000000 represents 1s
        .flags.auto_reload_on_alarm = true, // Enable auto-reload function
    };

    // this will call the callback funtion automatically when the timer hits alarm
    gptimer_event_callbacks_t cbs = {
        .on_alarm = timer_cb, // Call the user callback function when the alarm event occurs
    };

    // Create a timer instance
    ESP_ERROR_CHECK(gptimer_new_timer(&timer_config, &gptimer));
    // Set the timer's alarm action
    ESP_ERROR_CHECK(gptimer_set_alarm_action(gptimer, &alarm_config));
    // Register timer event callback functions, allowing user context to be carried
    ESP_ERROR_CHECK(gptimer_register_event_callbacks(gptimer, &cbs, NULL));
    // Enable the timer
    ESP_ERROR_CHECK(gptimer_enable(gptimer));
    // Start the timer
    ESP_ERROR_CHECK(gptimer_start(gptimer));

    finish = esp_timer_get_time();
    printf("Timer setup time: %lld microseconds\n", finish-start);

    // timer to test esp_logi setup
    start = esp_timer_get_time();

    // tags the timer
    ESP_LOGI(TAG, "Stopwatch update");

    finish = esp_timer_get_time();
    printf("ESP_LOGI setup time: %lld microseconds\n", finish-start);

    lcd_init(); // Initialize LCD display
    watch_init(); // Initialize stopwatch face
    for (;;) { // forever update loop
        watch_update(timer_ticks);

        // print the max ISR runtime every 5 seconds
        if(isr_cnt == ISR_CNT_TIMER)
        {
            printf("ISR runtime: %lld microseconds\n", isr_max);
            isr_cnt = 0;
            isr_max = 0;
        }
    }

}   