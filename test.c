#include <stdio.h>
#include <unistd.h>  /* sleep() */
#include "events.h"
#include "sound_api.h"

#define DEFAULT_SAMPLERATE (48000)
#define DEFAULT_SAMPLES (512)
#define DEFAULT_PERIODS (4)

static int run_program(void)
{
        int r;

        sound_api_init(DEFAULT_SAMPLERATE, DEFAULT_SAMPLES, DEFAULT_PERIODS);
        r = events_init();
        if (r == -1) {
                fprintf(stderr, "failed to init events creator module\n");
                sound_api_exit();
                return -1;
        }

        sound_api_start_playing();
        events_start_producing();

        while (!user_wants_to_quit())
                sleep(1);

        events_stop_producing();
        sound_api_stop_playing();

        events_exit();
        sound_api_exit();

        return 0;
}

int main(void)
{
        int r;

        r = run_program();
        if (r == -1)
                return 1;

        return 0;
}
