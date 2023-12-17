#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <limits.h>
#include <unistd.h>
#include <curl/curl.h>
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

/**
 * 初始化隨機數生成器
 * @param s 初始種子值
 */
void initGenRand(unsigned long s) {
    mt[0]= s & 0xffffffffUL;
    for (mti=1; mti<N; mti++) {
        mt[mti] = 
        (1812433253UL * (mt[mti-1] ^ (mt[mti-1] >> 30)) + mti); 
        mt[mti] &= 0xffffffffUL;
    }
}

/**
 * 產生 Mersenne Twister 亂數。
 * @return 一個 64 位元的無符號整數。
 */
unsigned long genrandMersenneTwister(void) {
    // 函式內部變數宣告
    int i;
    unsigned long x;
    static unsigned long mag01[2]={0x0UL, MATRIX_A};

    // 檢查是否需要重新初始化
    if (mti >= N) {
        if (mti == N+1) initGenRand(5489UL);
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

    // 進行亂數運算
    x = mt[mti++];
    x ^= (x >> 29) & 0x5555555555555555UL;
    x ^= (x << 17) & 0x71d67fffeda60000UL;
    x ^= (x << 37) & 0xfff7eee000000000UL;
    x ^= (x >> 43);

    // 回傳亂數結果
    return x;
}

/**
 * 使用djb2算法計算字串的哈希值。
 * 
 * @param str 要計算哈希值的字串
 * @return 計算得到的哈希值
 */
unsigned long hash_djb2(unsigned char *str)
{
    unsigned long hash = 5381;
    int c;
    while ((c = *str++))
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    return hash;
}

/**
 * 生成指定範圍內的隨機數。
 *
 * @param num 隨機數種子
 * @param min 最小值
 * @param max 最大值
 * @return 生成的隨機數
 */
unsigned long randNum(unsigned long num, unsigned long min, unsigned long max) {
    return num % (max - min + 1) + min;
}

/**
 * 計算一個無符號長整數的位數。
 * 
 * @param num 要計算位數的數字
 * @return 數字的位數
 */
unsigned int countDigits(unsigned long num) {
    int count = 0;
    while (num != 0) {
        num /= 10;
        ++count;
    }
    return count;
}

/**
 * 將數字填充為指定長度的字串，不足的部分用 '0' 填充。
 * 
 * @param val 要填充的字串
 * @param num 要填充的數字
 * @param len 指定的長度
 */
void padZero(char *val, int num, int len) {
    int i;
    for (i = 0; i < len; i++) {
        val[i] = '0';
    }
    val[i] = '\0';
    sprintf(val, "%d", num);
}

/**
 * @brief 寫入資料的函式。
 * 
 * 此函式將資料寫入指定的緩衝區，並返回寫入的字節數。
 * 
 * @param buffer 要寫入的資料的指標。
 * @param size 每個元素的大小（以字節為單位）。
 * @param nmemb 要寫入的元素數量。
 * @param data 指向指標的指標，用於存儲寫入的資料。
 * @return 寫入的字節數。
 */
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

/**
 * @brief 從指定的URL獲取資訊。
 * 
 * @param url 要獲取資訊的URL。
 * @return char* 獲取的資訊，如果獲取失敗則返回NULL。
 */
char *getInfo(char *url) {
    char *data = NULL;
    CURL *curl = curl_easy_init();
    if(curl) {
        CURLcode res;
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeData);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &data);
        res = curl_easy_perform(curl);
        if(res != CURLE_OK) {
            free(data);
            data = NULL;
        }
        curl_easy_cleanup(curl);
    }
    return data;
}

/**
 * 判斷字串是否僅包含小寫字母和數字。
 * 
 * @param str 要判斷的字串
 * @return 如果字串僅包含小寫字母和數字，則返回 1；否則返回 0。
 */
short isLowercaseAlphanumeric(const char *str) {
    for (int i = 0; str[i]; i++) {
        if (!(str[i] >= '0' && str[i] <= '9') && !(str[i] >= 'a' && str[i] <= 'z')) {
            return 0;
        }
    }
    return 1;
}

/**
 * 測試環境。
 */
void testEnv()
{
    unsigned int i;
    clock_t start, end;
    double cpu_time_used;
    time_t rawtime;
    unsigned long numHash = 0;
    time (&rawtime);
    initGenRand(rand());
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
    printf("cURL = %s\nTest genrand %u times ... ", curl_version(), TESTIMES);
    fflush(stdout);
    start = clock();
    for (i = 0; i < TESTIMES; i++) {
        rawtime = time(NULL);
        time (&rawtime);
        srand(rawtime);
        initGenRand(rand()+rawtime);
        numHash = genrandMersenneTwister();
        if (i == 0) {
            printf("%020lu ... ", numHash);
        }
    }
    end = clock();
    cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
    printf("used %f seconds.\n", cpu_time_used);
}

/**
 * 主函式。
 * 
 * @param argc 參數個數
 * @param argv 參數列表
 * @return 程式執行結果
 */
int main(int argc, char *argv[]) {
    int i;
    char *argURL = NULL;
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
        if(strcmp(argv[i], "--hash") == 0 && i + 1 < argc) {
            latestHash = argv[i + 1];
        }
        if(strcmp(argv[i], "--url") == 0 && i + 1 < argc) {
            argURL = argv[i + 1];
        }
    }
    if (latestHash != NULL && strlen(latestHash) == 64 && isLowercaseAlphanumeric(latestHash) != 0) {
        numHash = hash_djb2(latestHash);
    } else if (argURL != NULL && strlen(argURL) > 0) {
        latestHash = getInfo(argURL);
        if(latestHash == NULL) {
            printf("Error: Failed to get latesthash from %s\n", argURL);
            return 1;
        }
        if (strlen(latestHash) != 64 || isLowercaseAlphanumeric(latestHash) == 0) {
            printf("Error: Invalid latesthash : %s\n", latestHash);
            return 1;
        }
        numHash = hash_djb2(latestHash);
    } else {
        initGenRand(rand()+rawtime);
        numHash = genrandMersenneTwister();
    }
    sprintf(hash, "%020lu", numHash);
    printf("%ld %s %s\n", rawtime, hash, latestHash);
    return 0;
}
