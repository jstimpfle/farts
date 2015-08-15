#ifndef X_GENERATOR_H_
#define X_GENERATOR_H_

struct x_generator {
        short buf[48000][2];
        float phase;
        float speed;
        float amplitude;
};

void x_generator_init(struct x_generator *gen);
short *x_generator_generate(struct x_generator *gen);

#endif
