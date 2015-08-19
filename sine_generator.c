#include <math.h>
#include "sine_generator.h"
#include "sound_api.h"

short *sine_generator_generate(struct sine_generator *gen)
{
        int i;
        int n = num_samples_per_period();

        for (i = 0; i < n; i++) {
                gen->phase += gen->speed;
                if (gen->phase > 2*M_PI)
                        gen->phase -= 2*M_PI;
                gen->buf[i][0] = gen->buf[i][1] = gen->amplitude
                        * sinf(gen->phase);
        }
        return (short *)gen->buf;
}

void sine_generator_init(struct sine_generator *gen)
{
        gen->phase = 0.0f;
        gen->speed = 0.2f;
        gen->amplitude = 32*1024;
}

void sine_generator_exit(struct sine_generator *gen)
{
}
