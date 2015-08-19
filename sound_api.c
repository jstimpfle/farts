#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <alsa/asoundlib.h>
#include <pthread.h>
#include "events.h"
#include "sound_api.h"
#include "sine_generator.h"
#include "sawtooth_generator.h"
#include "x_generator.h"
#include "lohi_generator.h"
#include "print_time.h"

static int g_samples_per_second;
static int g_samples_per_period;
static int g_periods_in_buffer;

static snd_pcm_t *pcm_handle;
static int alsa_err;

static struct pollfd *poll_fds;
static int num_poll_fds;

static pthread_t pthread_var;

static struct sawtooth_generator saw;
static struct sine_generator sine;
static struct lohi_generator lohi;
static struct x_generator xgen;

static volatile int write_thread_exit_requested;

int num_samples_per_second(void)
{
        return g_samples_per_second;
}

int num_samples_per_period(void)
{
        return g_samples_per_period;
}

int num_periods_in_buffer(void)
{
        return g_periods_in_buffer;
}

static void fail_if_alsa_error(const char *when)
{
        if (alsa_err < 0) {
                fprintf(stderr, "ALSA error when %s: %s\n",
                        when, snd_strerror(alsa_err));
                exit(1);
        }
}

static const char *alsa_state_to_string(snd_pcm_state_t state)
{
        switch(state) {
        case SND_PCM_STATE_DISCONNECTED: return "DISCONNECTED";
        case SND_PCM_STATE_DRAINING: return "DRAINING";
        case SND_PCM_STATE_OPEN: return "OPEN";
        case SND_PCM_STATE_PAUSED: return "PAUSED";
        case SND_PCM_STATE_PREPARED: return "PREPARED";
        case SND_PCM_STATE_RUNNING: return "RUNNING";
        case SND_PCM_STATE_SETUP: return "SETUP";
        case SND_PCM_STATE_SUSPENDED: return "SUSPENDED";
        case SND_PCM_STATE_XRUN: return "XRUN";
        default: return "(unknown)";
        }
}

static void __attribute__((unused)) print_alsa_state(snd_pcm_state_t state)
{
        fprintf(stderr, "ALSA state is %s\n", alsa_state_to_string(state));
}

static void __attribute__((unused)) print_alsa_state_if_not_running(
                                                        snd_pcm_state_t state)
{
        if (state != SND_PCM_STATE_RUNNING)
                fprintf(stderr, "ALSA state is %s\n",
                        alsa_state_to_string(state));
}

static void set_hw_params(void)
{
        snd_pcm_hw_params_t *hw_params;

        alsa_err = snd_pcm_hw_params_malloc(&hw_params);
        fail_if_alsa_error("allocating hw_params structure");

        alsa_err = snd_pcm_hw_params_any(pcm_handle, hw_params);
        fail_if_alsa_error("snd_pcm_hw_params_any()");

        alsa_err = snd_pcm_hw_params_set_access(pcm_handle, hw_params,
                                                SND_PCM_ACCESS_RW_INTERLEAVED);
        fail_if_alsa_error("setting hardware access mode");

        alsa_err = snd_pcm_hw_params_set_format(pcm_handle, hw_params,
                                                SND_PCM_FORMAT_S16_LE);
        fail_if_alsa_error("choosing signed 16bit litte-endian samples");

        alsa_err = snd_pcm_hw_params_set_rate(pcm_handle, hw_params,
                                              num_samples_per_second(), 0);
        fail_if_alsa_error("setting hardware sample rate");

        alsa_err = snd_pcm_hw_params_set_channels(pcm_handle, hw_params, 2);
        fail_if_alsa_error("setting hardware to 2-channel mode");

        alsa_err = snd_pcm_hw_params_set_period_size(pcm_handle, hw_params,
                                             num_samples_per_period(), 0);
        fail_if_alsa_error("setting hardware period size");

        alsa_err = snd_pcm_hw_params_set_buffer_size(pcm_handle, hw_params,
                     num_periods_in_buffer() * num_samples_per_period());
        fail_if_alsa_error("setting hardware buffer size");

        alsa_err = snd_pcm_hw_params(pcm_handle, hw_params);
        fail_if_alsa_error("applying hardware configuration");

        snd_pcm_hw_params_free(hw_params);
}

static void set_sw_params(void)
{
        snd_pcm_sw_params_t *sw_params;

        alsa_err = snd_pcm_sw_params_malloc(&sw_params);
        fail_if_alsa_error("allocating sw_params structure");

        alsa_err = snd_pcm_sw_params_current(pcm_handle, sw_params);
        fail_if_alsa_error("fill sw_params structures with current values");

        alsa_err = snd_pcm_sw_params_set_start_threshold(pcm_handle,
                                         sw_params, num_samples_per_period());
        fail_if_alsa_error("set software start threshold to period size");

        alsa_err = snd_pcm_sw_params(pcm_handle, sw_params);
        fail_if_alsa_error("applying software configuration");

        snd_pcm_sw_params_free(sw_params);
}

static void init_poll_fds(void)
{
        alsa_err = num_poll_fds = snd_pcm_poll_descriptors_count(pcm_handle);
        fail_if_alsa_error("requesting number of poll descriptors");

        poll_fds = malloc(num_poll_fds * sizeof *poll_fds);
        if (poll_fds == NULL) {
                fprintf(stderr, "OOM!\n");
                exit(1);
        }

        alsa_err = snd_pcm_poll_descriptors(pcm_handle,
                                            poll_fds, num_poll_fds);
        fail_if_alsa_error("requesting poll descriptors");
}

static void exit_poll_fds(void)
{
        free(poll_fds);
}

static void poll_until_writeable(void)
{
        unsigned short revents;

        for (;;) {
                poll(poll_fds, num_poll_fds, -1);
                snd_pcm_poll_descriptors_revents(
                                pcm_handle, poll_fds, num_poll_fds, &revents);
                if (revents & POLLERR) {
                        fprintf(stderr, "POLLERR!\n");
                        exit(1);
                }
                if (revents & POLLOUT)
                        return;
        }
}

static void write_once(void)
{
        snd_pcm_state_t state;
        snd_pcm_sframes_t avail;
        
        state = snd_pcm_state(pcm_handle);
        if (state == SND_PCM_STATE_RUNNING) {
                poll_until_writeable();
                state = snd_pcm_state(pcm_handle);
                if (state == SND_PCM_STATE_RUNNING) {
                        alsa_err = avail = snd_pcm_avail(pcm_handle);
                        state = snd_pcm_state(pcm_handle);
                        if (alsa_err < 0) {
                        } else {
                                /*fprintf(stderr, "avail: %d\n", (int) avail);
                                 */
                        }
                }
        }

        if (state != SND_PCM_STATE_RUNNING)
                print_alsa_state(state);

        struct event ev;
        static float mousex = 0.5f;
        static float mousey = 0.5f;
        static enum {
                SOUND_SAW, SOUND_SINE, SOUND_LOHI,
                SOUND_XGEN, SOUND_LAST,
        } sound = SOUND_SAW;

        if (events_dequeue_if_avail(&ev) != -1) {
                while (events_dequeue_if_avail(&ev) != -1)
                        continue;
                switch (ev.evtp) {
                case EVENT_MOUSEMOVE:
                        mousex = ev.mouse_event.ratiox;
                        mousey = ev.mouse_event.ratioy;
                        break;
                case EVENT_KEYPRESS:
                        switch (ev.key_press_event.key) {
                        case KEY_SPACE:
                                sound = (sound + 1) % SOUND_LAST;
                                break;
                        default:
                                break;
                        }
                        break;
                default:
                        break;
                }
        }

        short *buf;

        switch (sound) {
        case SOUND_SAW:
                saw.speed = mousex / 20;
                saw.amplitude = mousey * 32*1024;
                buf = sawtooth_generator_generate(&saw);
                break;
        case SOUND_SINE:
                sine.speed = mousex / 5;
                sine.amplitude = mousey * 32 * 1024;
                buf = sine_generator_generate(&sine);
                break;
        case SOUND_LOHI:
                lohi.speed += mousex - lohi.high;
                lohi.high = mousex;
                lohi.amplitude = mousey * 32*1024;
                lohi_generator_generate(&lohi);
                buf = (short *)lohi.buf;
                break;
        case SOUND_XGEN:
                xgen.speed = mousex / 20;
                xgen.amplitude = mousey * 32*1024;
                buf = x_generator_generate(&xgen);
                break;
        default:
                assert(0);
                break;
        }

        alsa_err = snd_pcm_writei(pcm_handle, buf, num_samples_per_period());
        if (alsa_err < 0) {
                fprintf(stderr, "Trying to handle error after writei\n");
                assert(alsa_err != -EBADFD);
                if (alsa_err == -EPIPE || alsa_err == -ESTRPIPE) {
                        alsa_err = snd_pcm_recover(pcm_handle, alsa_err, 0);
                        fail_if_alsa_error("recovering from error");
                        fprintf(stderr, "state is now: %s\n",
                          alsa_state_to_string(snd_pcm_state(pcm_handle)));
                }
                else {
                        fprintf(stderr, "Unknown alsa error after writei");
                        exit(1);
                }
        }
}

static void init_write_thread(void)
{
        sawtooth_generator_init(&saw);
        sine_generator_init(&sine);
        lohi_generator_init(&lohi);
        x_generator_init(&xgen);
}

static void exit_write_thread(void)
{
        sawtooth_generator_exit(&saw);
        sine_generator_exit(&sine);
        lohi_generator_exit(&lohi);
        x_generator_exit(&xgen);
}

static void *write_thread(void *dummy)
{
        init_write_thread();
        while (!write_thread_exit_requested)
                write_once();
        exit_write_thread();

        return NULL;
}

void sound_api_start_playing(void)
{
        write_thread_exit_requested = 0;
        pthread_create(&pthread_var, NULL, write_thread, NULL);
        fprintf(stderr, "Press space to change sounds\n");
}

void sound_api_stop_playing(void)
{
        write_thread_exit_requested = 1;

        if (pthread_join(pthread_var, NULL)) {
                fprintf(stderr, "ERROR: pthread_join failed\n");
                exit(1);
        }
}

void sound_api_init(int samples_per_second,
                    int samples_per_period,
                    int periods_in_buffer)
{
        g_samples_per_second = samples_per_second;
        g_samples_per_period = samples_per_period;
        g_periods_in_buffer = periods_in_buffer;

        alsa_err = snd_pcm_open(&pcm_handle, "default",
                                SND_PCM_STREAM_PLAYBACK, 0);
        fail_if_alsa_error("opening default device");

        set_hw_params();
        set_sw_params();

        alsa_err = snd_pcm_prepare(pcm_handle);
        fail_if_alsa_error("preparing pcm");

        init_poll_fds();

        static short buf[48000][2];
        memset(buf, 0, sizeof buf);
        snd_pcm_writei(pcm_handle, buf, num_samples_per_period());
        /*alsa_err = snd_pcm_start(pcm_handle);
        fail_if_alsa_error("starting pcm");
        */
}

void sound_api_exit(void)
{
        exit_poll_fds();

        alsa_err = snd_pcm_close(pcm_handle);
        fail_if_alsa_error("closing pcm");

        /* Documented in file MEMORY LEAK in alsa-lib package */
        snd_config_update_free_global();
}
