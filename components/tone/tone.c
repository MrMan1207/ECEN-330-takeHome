#include <stdlib.h>
#include <math.h> 
#include "tone.h"

#define BIAS 0x80
#define AMP 127
#define MAX_VAL 255
#define Q1 0.25
#define Q2 0.5
#define Q3 0.75


// global variables
uint8_t *tone_buffer = NULL;
uint32_t buffer_size, global_sample_hz;



// Initialize the tone driver. Must be called before using.
// May be called again to change sample rate.
// sample_hz: sample rate in Hz to playback tone.
// Return zero if successful, or non-zero otherwise.
int32_t tone_init(uint32_t sample_hz)
{
    // clear the old tone if exists
    if (tone_buffer != NULL) {
        tone_deinit();
    }

    // save the sample hz to a global to use later
    global_sample_hz = sample_hz;

    // return 1, fail, if sample rate is not at least twice the lowest Nyquist frequency.
    if(sample_hz < 2 * LOWEST_FREQ) return 1;

    sound_init(sample_hz);

    // allocate memory for the buffer
    // buffer size must be big enough to hold the lowest freq
    buffer_size = sample_hz  / LOWEST_FREQ;
    tone_buffer = malloc(buffer_size  * sizeof(uint8_t));


    // make sure the buffer is allocated 
    if (tone_buffer == NULL) return 1;
    return 0;


}

// Free resources used for tone generation (DAC, etc.).
// Return zero if successful, or non-zero otherwise.
int32_t tone_deinit(void)
{
    // if buffer is null it cannot deallocate
    if(tone_buffer == NULL) return 1;

    // clean the heap and reset variables
    free(tone_buffer);
    tone_buffer = NULL;
    buffer_size = 0;
    sound_deinit();

    return 0;
}

// Start playing the specified tone.
// tone: one of the enumerated tone types.
// freq: frequency of the tone in Hz.
void tone_start(tone_t tone, uint32_t freq)
{
    if(freq < LOWEST_FREQ) return;
    int32_t samples_per_period = global_sample_hz / freq;

    // switch to play sample the correct tone
    switch(tone)
    {
        case SINE_T:
            // loop to save sin wave in buffer
            for(uint32_t i = 0; i < samples_per_period; i++)
            {
                float angle = (2.0f * M_PI * (float)i) / (float)samples_per_period;
                tone_buffer[i] = BIAS + (AMP * sinf(angle));
            }
            break;

        case SQUARE_T:
            // loop to save upper wave in buffer
            for(uint32_t i = 0; i < samples_per_period / 2; i++)
            {
                tone_buffer[i] = MAX_VAL;
            } 
            // loop to save bottom of wave into buffer
            for(uint32_t i = samples_per_period / 2; i < samples_per_period; i++)
            {
                tone_buffer[i] = 0;
            }
            break;
        
        case TRIANGLE_T:
            // loop to sample into the buffer
            for (uint32_t i = 0; i < samples_per_period; i++)
            {
                // we use phase to get percent of way through. We need to divide by the quarter we are in
                float phase = (float)i / (float)samples_per_period;
                float val;
                
                // q1 rising to 255
                if (phase < Q1) {
                    val = BIAS + (AMP * (phase / Q1));
                }
                // q2-3 falling to 0 
                else if (phase < Q3) {
                    // multiply by 2 becasue this will range from q1 to q3
                    val = (BIAS + AMP) - (2.0f * AMP * ((phase - Q1) / Q2));
                }
                // q4 rising back to bias
                else {
                    val = (BIAS - AMP) + (AMP * ((phase - Q3) / Q1));
                }
                tone_buffer[i] = val;
            }
            break;
        
        case SAW_T:            
            // loop to sample into the buffer
            for (uint32_t i = 0; i < samples_per_period; i++)
            {
                // make sure to use float so we dont lose data
                float phase = (float)i / (float)samples_per_period; 
                
                // shift phase so it starts at bias
                float shifted_phase = phase + Q2;
                // make sure the phase stays between 0 and 1
                if (shifted_phase >= 1.0f) shifted_phase -= 1.0f;
                
                tone_buffer[i] = (BIAS - AMP) + (2.0f * AMP * shifted_phase);
            }
            break;

        case LAST_T:
            return;
            break;
    }

    // play the sound
    sound_cyclic(tone_buffer, samples_per_period);


}

