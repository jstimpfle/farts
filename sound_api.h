#ifndef SOUND_API_H_
#define SOUND_API_H_

void sound_api_init(int samples_per_second,
                    int samples_per_period,
                    int periods_in_buffer);

void sound_api_exit(void);

void sound_api_start_playing(void);
void sound_api_stop_playing(void);

int num_samples_per_second(void);
int num_samples_per_period(void);
int num_periods_in_buffer(void);

#endif
