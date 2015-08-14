#ifndef SOUND_API_H_
#define SOUND_API_H_

void init_sound_api(int samples_per_second,
                    int samples_per_period,
                    int periods_in_buffer);

void exit_sound_api(void);

int num_samples_per_second(void);
int num_samples_per_period(void);
int num_periods_in_buffer(void);

#endif
