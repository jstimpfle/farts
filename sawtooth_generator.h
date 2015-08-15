#ifndef SAWTOOTH_GENERATOR_H_
#define SAWTOOTH_GENERATOR_H_

struct sawtooth_generator {
        short buf[48000][2];
        int amplitude;
        float phase;
        float speed;
};

void sawtooth_generator_init(struct sawtooth_generator *saw);
void sawtooth_generator_exit(struct sawtooth_generator *saw);

short *sawtooth_generator_generate(struct sawtooth_generator *saw);

#endif
