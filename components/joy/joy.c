#include "esp_adc/adc_oneshot.h"
#include "joy.h"
#include "hw.h"

#define AVG_FACTOR 3

// global variables
adc_oneshot_unit_handle_t adc1_handle;
adc_channel_t CHAN6;
adc_channel_t CHAN7;
adc_unit_t unit[2];

int_fast32_t averagex;
int_fast32_t averagey;


// Initialize the joystick driver. Must be called before use.
// Return zero if successful, or non-zero otherwise.
int32_t joy_init(void)
{
    // check if already initailized 
    if(adc1_handle != NULL) return 0;

    adc_oneshot_io_to_channel(HW_JOY_X, &unit[0], &CHAN6);
    adc_oneshot_io_to_channel(HW_JOY_Y, &unit[1], &CHAN7);

    // Create an ADC Unit Handle Under Normal Oneshot Mode
    adc_oneshot_unit_init_cfg_t init_config1 = {
    .unit_id = ADC_UNIT_1,
    .ulp_mode = ADC_ULP_MODE_DISABLE,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config1, &adc1_handle));

    // configures the adc with default bitwidth and Attenuates by 12dB
    adc_oneshot_chan_cfg_t config = {
    .bitwidth = ADC_BITWIDTH_DEFAULT,
    .atten = ADC_ATTEN_DB_12,
    };
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, CHAN6, &config));
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, CHAN7, &config));


    // declare variables to get samples to calibrate the center pos
    int_fast32_t xval1, xval2, xval3, yval1, yval2, yval3;

    ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, CHAN6, &xval1));
    ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, CHAN7, &yval1));
    ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, CHAN6, &xval2));
    ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, CHAN7, &yval2));
    ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, CHAN6, &xval3));
    ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, CHAN7, &yval3));

    // stores data in global variable
    averagex = (xval1 + xval2 + xval3) / AVG_FACTOR;
    averagey = (yval1 + yval2 + yval3) / AVG_FACTOR;

    if(adc1_handle == NULL) return 1;
    return 0;
}


// Get the joystick displacement from center position.
void joy_get_displacement(int32_t *dcx, int32_t *dcy)
{
    // declare variables to get samples
    int_fast32_t xval, yval;

    ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, CHAN6, &xval));
    ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, CHAN7, &yval));

    // calculate in relation to average
    xval -= averagex;
    yval -= averagey;

    *dcx = xval;
    *dcy = yval;
}


// Free resources used by the joystick 
int32_t joy_deinit(void)
{
    if(adc1_handle != NULL) 
    {
        ESP_ERROR_CHECK(adc_oneshot_del_unit(adc1_handle));
        return 0;
    }

    return 1;
}
