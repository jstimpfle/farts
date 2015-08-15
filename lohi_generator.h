#ifndef LOHI_GENERATOR_H_
#define LOHI_GENERATOR_H_

struct lohi_generator {
        short buf[48000][2];
        float speed;
        float phase;
        float speedup;
        float amplitude;
        float high;
        float interval;
};

void lohi_generator_generate(struct lohi_generator *lohi);
void lohi_generator_init(struct lohi_generator *lohi);
void lohi_generator_exit(struct lohi_generator *lohi);

#endif
