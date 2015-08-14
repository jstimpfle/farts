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

void generate_lohi(struct lohi_generator *lohi);
void init_lohi_generator(struct lohi_generator *lohi);
void exit_lohi_generator(struct lohi_generator *lohi);

#endif
