#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <limits.h>
#include <unistd.h>
#include <string.h>

#define N 624
#define M 397
#define MATRIX_A 0x9908b0dfUL
#define UPPER_MASK 0x80000000UL
#define LOWER_MASK 0x7fffffffUL
#define VERSION 200
#define TESTIMES 1000000

static unsigned long mt[N];
static int mti=N+1;

void initGenrand(unsigned long s) {
    mt[0]= s & 0xffffffffUL;
    for (mti=1; mti<N; mti++) {
        mt[mti] = 
        (1812433253UL * (mt[mti-1] ^ (mt[mti-1] >> 30)) + mti); 
        mt[mti] &= 0xffffffffUL;
    }
}

unsigned long genrandMersenneTwister(void) {
    int i;
    unsigned long x;
    static unsigned long mag01[2]={0x0UL, MATRIX_A};
    if (mti >= N) {
        if (mti == N+1) initGenrand(5489UL);
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

unsigned long randNum(unsigned long num, unsigned long min, unsigned long max) {
    return num % (max - min + 1) + min;
}

unsigned int countDigits(unsigned long num) {
    int count = 0;
    while (num != 0) {
        num /= 10;
        ++count;
    }
    return count;
}

void padZero(char *val, int num, int len) {
    int i;
    for (i = 0; i < len; i++) {
        val[i] = '0';
    }
    val[i] = '\0';
    sprintf(val, "%d", num);
}

size_t writeData(void *buffer, size_t size, size_t nmemb, void **data) {
    size_t total_size = size * nmemb;
    char *temp = realloc(*data, total_size + 1);
    if (temp == NULL) {
        return 0;
    }
    *data = temp;
    memcpy(*data, buffer, total_size);
    ((char*)*data)[total_size] = '\0';
    return total_size;
}

short isLowercaseAlphanumeric(const char *str) {
    for (int i = 0; str[i]; i++) {
        if (!(str[i] >= '0' && str[i] <= '9') && !(str[i] >= 'a' && str[i] <= 'z')) {
            return 0;
        }
    }
    return 1;
}

unsigned long hash_djb2(unsigned char *str)
{
    unsigned long hash = 5381;
    int c;
    while ((c = *str++))
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    return hash;
}

void testEnv()
{
    unsigned int i;
    clock_t start, end;
    double cpu_time_used;
    time_t rawtime;
    unsigned long numHash = 0;
    time (&rawtime);
    initGenrand(rand());
    printf("=== NyarukoLuckDraw 2 by KagurazakaMiyabi v%d.%d.%d ===\nhttps://github.com/kagurazakayashi/NyarukoLuckDraw2\n", VERSION / 100, VERSION % 100 / 10, VERSION % 10);
    printf("    TIME = %lu(%lu)\n", rawtime, countDigits(rawtime));
    printf("    CHAR = %d(%lu) ~ %d(%lu) | %u(%lu)\n", CHAR_MIN, countDigits(CHAR_MIN), CHAR_MAX, countDigits(CHAR_MAX), UCHAR_MAX, countDigits(UCHAR_MAX));
    printf("   SHORT = %d(%lu) ~ %d(%lu) | %u(%lu)\n", SHRT_MIN, countDigits(SHRT_MIN), SHRT_MAX, countDigits(SHRT_MAX), USHRT_MAX, countDigits(USHRT_MAX));
    printf("     INT = %d(%lu) ~ %d(%lu) | %u(%lu)\n", INT_MIN, countDigits(INT_MIN), INT_MAX, countDigits(INT_MAX), UINT_MAX, countDigits(UINT_MAX));
    printf("    LONG = %ld(%lu) ~ %ld(%lu) | %lu(%lu)\n", LONG_MIN, countDigits(LONG_MIN), LONG_MAX, countDigits(LONG_MAX), ULONG_MAX, countDigits(ULONG_MAX));
    printf("LONGLONG = %lld(%lu) ~ %lld(%lu) | %llu(%lu)\n", LLONG_MIN, countDigits(LLONG_MIN), LLONG_MAX, countDigits(LLONG_MAX), ULLONG_MAX, countDigits(ULLONG_MAX));
    printf("RAND_MAX = %d(%lu)\n", RAND_MAX, countDigits(RAND_MAX));
    printf("N = %d(%lu)   M = %d(%lu)   MATRIX_A = %lu(%lu)\n", N, countDigits(N), M, countDigits(M), MATRIX_A, countDigits(MATRIX_A));
    printf("UPPER_MASK = %lu(%lu)   LOWER_MASK = %lu(%lu)\n", UPPER_MASK, countDigits(UPPER_MASK), LOWER_MASK, countDigits(LOWER_MASK));
    printf("BUILD = %s (%s %s)\n", __FILE__, __DATE__, __TIME__);
    fflush(stdout);
    start = clock();
    for (i = 0; i < TESTIMES; i++) {
        rawtime = time(NULL);
        time (&rawtime);
        srand(rawtime);
        initGenrand(rand()+rawtime);
        numHash = genrandMersenneTwister();
        if (i == 0) {
            printf("%020lu ... ", numHash);
        }
    }
    end = clock();
    cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
    printf("used %f seconds.\n", cpu_time_used);
}

int main(int argc, char *argv[]) {
    int i;
    char *latestHash = "\0";
    unsigned long numHash = 0;
    char hash[21] = "\0";
    time_t rawtime = time(NULL);
    struct tm * timeinfo;
    char *versionStr;
    time (&rawtime);
    srand(rawtime);
    for (i = 1; i < argc; i++) {
        if(strcmp(argv[i], "--about") == 0 && i + 1 < argc) {
            testEnv();
            return 0;
        }
    }
    initGenrand(rand()+rawtime);
    numHash = genrandMersenneTwister();
    sprintf(hash, "%020lu", numHash);
    printf("%ld %s %s\n", rawtime, hash, latestHash);
    return 0;
}