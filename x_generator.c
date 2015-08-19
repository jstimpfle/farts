#include <math.h>
#include "sound_api.h"
#include "x_generator.h"

short *x_generator_generate(struct x_generator *x)
{
        int i;
        int n = num_samples_per_period();

        for (i = 0; i < n; i++) {
                x->buf[i][0] = 0.0f;
                x->buf[i][0] += x->amplitude * 0.3  * sinf(1*x->phase);
                x->buf[i][0] += x->amplitude * 0.3  * sinf(2*x->phase);
                x->buf[i][0] += x->amplitude * 0.5  * sinf(3*x->phase);
                x->buf[i][0] += x->amplitude * 0.1  * sinf(4*x->phase);
                x->buf[i][0] += x->amplitude * 0.75 * sinf(5*x->phase);
                x->buf[i][0] += x->amplitude * 0.05 * sinf(6*x->phase);
                x->buf[i][0] += x->amplitude * 0.05 * sinf(7*x->phase);
                x->buf[i][0] += x->amplitude * 0.05 * sinf(8*x->phase);
                x->buf[i][1] = x->buf[i][0];
                x->phase += x->speed;
                if (x->phase > M_PI)
                        x->phase -= M_PI;
        }

        return (short *) x->buf;
}

void x_generator_init(struct x_generator *x)
{
        x->phase = 0.0f;
        x->speed = 0.2f;
        x->amplitude = 16*1024;
}
