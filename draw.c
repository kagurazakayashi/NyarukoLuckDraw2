#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <limits.h>
#include <unistd.h>

#define N 624
#define M 397
#define MATRIX_A 0x9908b0dfUL
#define UPPER_MASK 0x80000000UL
#define LOWER_MASK 0x7fffffffUL

static unsigned long mt[N];
static int mti=N+1;

void init_genrand(unsigned long s) {
    mt[0]= s & 0xffffffffUL;
    for (mti=1; mti<N; mti++) {
        mt[mti] = 
        (1812433253UL * (mt[mti-1] ^ (mt[mti-1] >> 30)) + mti); 
        mt[mti] &= 0xffffffffUL;
    }
}

// unsigned int genrand_int32(void) {
//     unsigned y;
//     static unsigned mag01[2]={0x0UL, MATRIX_A};
//     if (mti >= N) {
//         int kk;
//         for (kk=0;kk<N-M;kk++) {
//             y = (mt[kk]&UPPER_MASK)|(mt[kk+1]&LOWER_MASK);
//             mt[kk] = mt[kk+M] ^ (y >> 1) ^ mag01[y & 0x1UL];
//         }
//         for (;kk<N-1;kk++) {
//             y = (mt[kk]&UPPER_MASK)|(mt[kk+1]&LOWER_MASK);
//             mt[kk] = mt[kk+(M-N)] ^ (y >> 1) ^ mag01[y & 0x1UL];
//         }
//         y = (mt[N-1]&UPPER_MASK)|(mt[0]&LOWER_MASK);
//         mt[N-1] = mt[M-1] ^ (y >> 1) ^ mag01[y & 0x1UL];
//         mti = 0;
//     }
//     y = mt[mti++];
//     y ^= (y >> 11);
//     y ^= (y << 7) & 0x9d2c5680UL;
//     y ^= (y << 15) & 0xefc60000UL;
//     y ^= (y >> 18);
//     return y;
// }

unsigned long genrand_int64(void) {
    int i;
    unsigned long x;
    static unsigned long mag01[2]={0x0UL, MATRIX_A};
    if (mti >= N) {
        if (mti == N+1) init_genrand(5489UL);
        for (i=0;i<N-M;i++) {
            x = (mt[i] & UPPER_MASK)|(mt[i+1] & LOWER_MASK);
            mt[i] = mt[i+M] ^ (x >> 1) ^ mag01[x & 0x1UL];
        }
        for (;i<N-1;i++) {
            x = (mt[i] & UPPER_MASK)|(mt[i+1] & LOWER_MASK);
            mt[i] = mt[i+(M-N)] ^ (x >> 1) ^ mag01[x & 0x1UL];
        }
        x = (mt[N-1] & UPPER_MASK)|(mt[0] & LOWER_MASK);
        mt[N-1] = mt[M-1] ^ (x >> 1) ^ mag01[x & 0x1UL];
        mti = 0;
    }
    x = mt[mti++];
    x ^= (x >> 29) & 0x5555555555555555UL;
    x ^= (x << 17) & 0x71d67fffeda60000UL;
    x ^= (x << 37) & 0xfff7eee000000000UL;
    x ^= (x >> 43);
    return x;
}

// unsigned int rand32(unsigned min, unsigned max) {
//     return genrand_int32() % (max - min + 1) + min;
// }

unsigned long rand64(unsigned long min, unsigned long max) {
    return genrand_int64() % (max - min + 1) + min;
}

// unsigned int countDigits(unsigned long num) {
//     int count = 0;
//     while (num != 0) {
//         num /= 10;
//         ++count;
//     }
//     return count;
// }

void padZero(char *val, int num, int len) {
    int i;
    for (i = 0; i < len; i++) {
        val[i] = '0';
    }
    val[i] = '\0';
    sprintf(val, "%d", num);
}

int main(void) {
    // printf(" UINT_MAX LEN = %lu : %lu\n", countDigits(UINT_MAX), UINT_MAX);
    // printf("ULONG_MAX LEN = %lu : %lu\n", countDigits(ULONG_MAX), ULONG_MAX);
    int i;
    time_t rawtime = time(NULL);
    struct tm * timeinfo;
    time (&rawtime);
    unsigned long r = 0;
    char str[20]; // 32:10 digits, 64:20 digits
    while (rawtime < ULONG_MAX) {
        time (&rawtime);
        // timeinfo = gmtime(&rawtime);
        // rawtime = (rawtime / 60) * 60; // Round to minute
        init_genrand(rawtime);
        r = genrand_int64();
        sprintf(str, "%020lu", r); // Pad with zeros
        // printf("%ld %04d%02d%02d%02d%02d%02d %s\n",rawtime, timeinfo->tm_year + 1900, timeinfo->tm_mon + 1, timeinfo->tm_mday, timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec, str);
        printf("%ld %s\n",rawtime, str);
        fflush(stdout);
        sleep(1);
    }
    return 0;
}