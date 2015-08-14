#include <math.h>
#include "sine_generator.h"
#include "sound_api.h"

static float speed = 0.2f;
static float phase;
static int amplitude = 20000;

short *generate_sine(void)
{
        int i;
        static short buf[48000][2];
        for (i = 0; i < num_samples_per_period(); i++) {
                phase += speed;
                if (phase > 2*M_PI)
                        phase -= 2*M_PI;
                buf[i][0] = buf[i][1] = amplitude * sinf(phase);
        }
        return (short *)buf;
}
