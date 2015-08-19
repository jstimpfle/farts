#include <stdlib.h>
#include <math.h>
#include "lohi_generator.h"
#include "sound_api.h"

void lohi_generator_generate(struct lohi_generator *lohi)
{
        int i;
        int n = num_samples_per_period();

        for (i = 0; i < n; i++) {
                lohi->buf[i][0] = lohi->amplitude * sinf(lohi->phase);
                lohi->buf[i][1] = lohi->amplitude * sinf(lohi->phase);
                lohi->phase += lohi->speed;
                lohi->speed += lohi->speedup;
                if (lohi->speed > lohi->high)
                        lohi->speed -= lohi->interval;
        }
}

void lohi_generator_init(struct lohi_generator *lohi)
{
        lohi->amplitude = 20000;
        lohi->phase = 0.0f;
        lohi->speed = 0.1f;
        lohi->speedup = 0.00001f;
        lohi->high = 0.1f;
        lohi->interval = 0.05f;
}

void lohi_generator_exit(struct lohi_generator *lohi)
{
}
