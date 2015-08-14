#include <stdio.h>
#include <stdlib.h>
#include "sawtooth_generator.h"
#include "sound_api.h"

void init_sawtooth_generator(struct sawtooth_generator *saw)
{
        saw->amplitude = 20000;
        saw->phase = 0.0f;
        saw->speed = 0.001f;
}

void exit_sawtooth_generator(struct sawtooth_generator *saw)
{
        (void) saw;
}

short *generate_sawtooth(struct sawtooth_generator *saw)
{
        int i;
        for (i = 0; i < num_samples_per_period(); i++) {
                saw->buf[i][0] = saw->buf[i][1] = saw->amplitude * saw->phase;
                saw->phase += saw->speed;
                if (saw->phase > 1.0f)
                        saw->phase -= 2.0f;
        }
        return (short *) saw->buf;
}
