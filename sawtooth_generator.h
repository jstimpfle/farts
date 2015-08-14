#ifndef SAWTOOTH_GENERATOR_H_
#define SAWTOOTH_GENERATOR_H_

struct sawtooth_generator {
        short buf[48000][2];
        int amplitude;
        float phase;
        float speed;
};

void init_sawtooth_generator(struct sawtooth_generator *saw);
void exit_sawtooth_generator(struct sawtooth_generator *saw);

short *generate_sawtooth(struct sawtooth_generator *saw);

#endif
