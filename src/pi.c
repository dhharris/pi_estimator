/**
 * Multithreaded program that estimates pi
 * using a Monte Carlo simulation
 *
 * Random number generation is done by random_r.
 * See 'man random_r' for details.
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <float.h>
#include <limits.h>
#include <time.h>
#include <pthread.h>
#include <unistd.h>

#define SEED time(NULL)
#define STATE_SIZE 128 /* Chosen from "optimal" values 8, 32, 64, 128, 256
                        * based on tests I performed */


/* This function lets me create different seeds even when
 * called multiple times per second. This works better than say,
 * time(NULL) because threads won't all get the same seed.
 *
 * Sourced from
 * http://www.concentric.net/~Ttwang/tech/inthash.htm
 */
unsigned long mix(unsigned long a, unsigned long b, unsigned long c)
{
    a=a-b;  a=a-c;  a=a^(c >> 13);
    b=b-c;  b=b-a;  b=b^(a << 8);
    c=c-a;  c=c-b;  c=c^(b >> 13);
    a=a-b;  a=a-c;  a=a^(c >> 12);
    b=b-c;  b=b-a;  b=b^(a << 16);
    c=c-a;  c=c-b;  c=c^(b >> 5);
    a=a-b;  a=a-c;  a=a^(c >> 3);
    b=b-c;  b=b-a;  b=b^(a << 10);
    c=c-a;  c=c-b;  c=c^(b >> 15);
    return c;
}

void *run(void *ptr)
{
    uint64_t max_iter = (uintptr_t)ptr;
    double x, y, z;
    uint64_t i, count = 0;
    char state[STATE_SIZE];
    struct random_data buf;
    unsigned long seed = mix(clock(), time(NULL), getpid());

    /* Seed random numbers */
    initstate_r(seed, state, STATE_SIZE, &buf);

    for (i = 0; i < max_iter; ++i) {
        int32_t rand1, rand2;
        random_r(&buf, &rand1);
        random_r(&buf, &rand2);

        x = (double)rand1 / RAND_MAX;
        y = (double)rand2 / RAND_MAX;
        z = x * x + y * y;

        if (z <= 1)
            ++count;
    }

    return (void*)(uintptr_t)count;
}

int main(int argc, char **argv)
{
    uint64_t max_iter, count = 0;
    int num_threads, i;
    pthread_t *threads;
    double pi;

    if (argc != 3) {
        fprintf(stderr, "%s <max sample size> <number of threads>\n", *argv);
        return 1;
    }
    if ((max_iter = strtoll(argv[1], NULL, 10)) <= 1) {
        fprintf(stderr, "sample size must be > 1\n");
        return 1;
    }

    num_threads = atoi(argv[2]);
    if ((uint64_t)num_threads > max_iter)
        num_threads = 1;

    threads = malloc(sizeof(pthread_t) * num_threads);

    /* Create threads and divide work evenly */
    for (i = 0; i < num_threads; ++i) {
        uint64_t num_iter = max_iter / num_threads;

        /* On the last thread, account for uneven distribution of work
         * so we are certain we do max_iter iterations
         */
        if (i == num_threads - 1)
            num_iter += max_iter % num_threads; 

        /* Using the uintptr_t type prevents unnecessary heap allocations */
        pthread_create(&threads[i], NULL, run, (void*)(uintptr_t)num_iter);
    }

    /* Wait for threads to finish and add up counts */
    for (i = 0; i < num_threads; ++i) {
        void *ret;
        pthread_join(threads[i], &ret);
        count += (uintptr_t)ret;
    }

    pi = count / (double)max_iter * 4;
    printf("Number of trials: %lu\nNumber of threads: %d\nEstimation of pi is %.10f\n", max_iter, num_threads, pi);

    free(threads);

    return 0;
}
