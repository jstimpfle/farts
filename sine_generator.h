#ifndef SINE_GENERATOR_H_
#define SINE_GENERATOR_H_

struct sine_generator {
        short buf[48000][2];
        float speed;
        float phase;
        float amplitude;
};

void sine_generator_init(struct sine_generator *gen);
void sine_generator_exit(struct sine_generator *gen);

short *sine_generator_generate(struct sine_generator *gen);

#endif
