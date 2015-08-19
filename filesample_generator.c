#include "filesample_generator.h"
#include "read_file.h"
#include "sound_api.h"

short *filesample_generator_generate(struct filesample_generator *gen)
{
        int i;
        int n = num_samples_per_period();
        for (i = 0; i < n; i++) {
                if (gen->state >= gen->nsamples) {
                        gen->buf
                }
                else {
                }
        }
}

int filesample_generator_init(struct filesample_generator *gen,
                                const char *filepath)
{
        int r;
        size_t size;
        short (*buf)[2];

        r = determine_filesize(filepath, &size);
        if (r == -1) {
                fprintf(stderr, "failed to determine file size of %s\n",
                        filepath);
                return -1;
        }
        if ((size % (sizeof *buf)) != 0) {
                fprintf(stderr, "file size of %s is not a multiple of sample"
                        " size (%zd). Aborting read\n", filepath, sizeof *buf);
                return -1;
        }

        buf = malloc(size);
        if (buf == NULL) {
                fprintf(stderr, "OOM!\n");
                return -1;
        }

        if (read_file(buf, filepath, size) == -1) {
                fprintf(stderr, "failed to read file %s\n", filepath);
                free(buf);
                return -1;
        }

        gen->buf = buf;
        return 0;
}
