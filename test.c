#include <unistd.h>  /* sleep() */
#include "sound_api.h"

#define DEFAULT_SAMPLERATE (48000)
#define DEFAULT_SAMPLES (512)
#define DEFAULT_PERIODS (4)

int main(void)
{
        init_sound_api(DEFAULT_SAMPLERATE, DEFAULT_SAMPLES, DEFAULT_PERIODS);

        /*for (;;)
         */
                sleep(5);

        exit_sound_api();

        return 0;
}
