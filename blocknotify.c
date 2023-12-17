#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <sys/stat.h>
#include <dirent.h>

// 測試環境配置 Development
#define GETCMD "cat test.json"
#define BASEDIR "/d/c/NyarukoLuckDraw2"

// 生產環境配置 Production
// #define GETCMD "/root/bitcoin/bin/bitcoin-cli -datadir=/root/BitcoinData -rpcconnect=127.0.0.1 -rpcport=8332 -rpcuser=yashi -rpcpassword=****** getblockchaininfo"
// #define BASEDIR "/root/BitcoinData/hashlog"

/**
 * 執行 gethash 命令，將結果存儲在指定的結果指針中。
 * 
 * @param result 存儲結果的指針
 */
void rungethashcmd(char **result) {
    char buffer[128];
    FILE *pipe;
    size_t result_len = 0;
    pipe = popen(GETCMD, "r");
    if (pipe == NULL) {
        perror("popen");
        return;
    }
    while (fgets(buffer, sizeof(buffer), pipe) != NULL) {
        size_t new_len = result_len + strlen(buffer) + 1;
        *result = realloc(*result, new_len);
        if (*result == NULL) {
            perror("realloc");
            return;
        }
        strcpy(*result + result_len, buffer);
        result_len = new_len - 1;
    }
    pclose(pipe);
}

/**
 * @brief 去除字串中的空格、換行符號和逗號。
 * 
 * @param str 要處理的字串。
 */
void trim(char *str) {
    char *p = str;
    char *q = str;
    while (*q != '\0') {
        if (*q != ' ' && *q != '\r' && *q != '\n' && *q != ',') {
            *p++ = *q++;
        } else {
            ++q;
        }
    }
    *p = '\0';
}

/**
 * 從字串中移除指定的字符。
 *
 * @param str 要處理的字串
 * @param garbage 要移除的字符
 */
void removeChar(char *str, char garbage) {
    char *src, *dst;
    for (src = dst = str; *src != '\0'; src++) {
        *dst = *src;
        if (*dst != garbage) {
            dst++;
        }
    }
    *dst = '\0';
}

/**
 * 將字串以 : 分割成兩個部分。
 * 
 * @param str 要分割的字串
 * @param left 存放分割後的左半部分
 * @param right 存放分割後的右半部分
 */
void split2(char *str, char *left, char *right) {
    int i = 0;
    int j = 0;
    int len = strlen(str);
    while (str[i] != ':') {
        left[i] = str[i];
        i++;
    }
    left[i] = '\0';
    i++;
    while (i < len) {
        right[j] = str[i];
        i++;
        j++;
    }
    right[j] = '\0';
}

/**
 * 檢查 json 字串，並將結果存儲在指定的結果指針中。
 * 
 * @param json 要檢查的 json 字串
 * @param timestamp 存儲 timestamp 的指針
 * @param bestblockhash 存儲 bestblockhash 的指針
 * @param blocks 存儲 blocks 的指針
 */
void chkjson(char *json, char **timestamp, char **bestblockhash, char **blocks) {
    char *line = strtok(json, "\n");
    while (line != NULL) {
        line = strtok(NULL, "\n");
        if (line != NULL && strlen(line) >= 12) {
            trim(line);
            removeChar(line, '"');
            char left[14] = "\0";
            char right[65] = "\0";
            split2(line, left, right);
            if (strlen(left) > 1 && left[13] == '\0' && strlen(right) > 1 && right[64] == '\0') {
                printf("%s = %s\n", left, right);
                if (strcmp(left, "time") == 0) {
                    *timestamp = strdup(right);
                } else if (strcmp(left, "bestblockhash") == 0) {
                    *bestblockhash = strdup(right);
                } else if (strcmp(left, "blocks") == 0) {
                    *blocks = strdup(right);
                }
            }
        }
    }
}

/**
 * 檢查目錄是否存在，不存在則創建。
 * 
 * @param path 要檢查的目錄
 */
void checkdir(char *path) {
    struct stat st;
    char *token = strtok(path, "/");
    char dir[256] = "";
    char tmp[256] = "";
    int mkdirok = 0;
    char start = '\0';
    if (path[0] == '/') {
        start = '/';
    }
    while (token != NULL) {
        strcat(dir, token);
        strcat(dir, "/");
        if (start != '\0') {
            sprintf(tmp, "%c%s", start, dir);
        }
        if (stat(tmp, &st) == -1) {
            printf("mkdir : %s\n", tmp);
            # ifdef _WIN32
            mkdirok = mkdir(tmp);
            // system("mkdir %s", tmp);
            # else
            mkdirok = mkdir(tmp, 0700);
            // system("mkdir -p %s", tmp);
            # endif
            if (mkdirok != 0) {
                printf("Error: Failed to create dir : %s\n", tmp);
                return;
            }
        }
        token = strtok(NULL, "/");
    }
}

/**
 * 移除字串中的點號及其後面的內容。
 * 
 * @param txt 要處理的字串
 */
void removeDotAfter(char *txt) {
    char *dot = strchr(txt, '.');
    if (dot != NULL) {
        *dot = '\0';
    }
}

/**
 * 將檔案名稱寫入索引檔。
 * 
 * @param txt 檔案名稱
 * @param dir 索引檔所在目錄
 */
void indexfileadd(char *txt, char *dir) {
    char *filename = NULL;
    char tmp[256] = "\0";
    sprintf(tmp, "%s/index.txt", dir);
    filename = strdup(tmp);
    FILE *fp = fopen(filename, "a+");
    if (fp != NULL) {
        fputs(txt, fp);
        fputs("\n", fp);
        fclose(fp);
    } else {
        printf("Error: Failed to open file %s\n", filename);
    }
}

/**
 * 主函式。
 * 
 * @param argc 參數個數
 * @param argv 參數列表
 * @return 程式執行結果
 */
int main(int argc, char *argv[]) {
    char *json = NULL;
    char *blocks = NULL;
    char *bestblockhash = NULL;
    char *timestamp = NULL;
    time_t timestampt = 0;
    struct tm *times = NULL;
    char gz[256] = "\0";
    char dir[128] = "\0";
    char filename[128] = "\0";
    char path[256] = "\0";
    
    rungethashcmd(&json);
    chkjson(strdup(json), &timestamp, &bestblockhash, &blocks);
    if (timestamp == NULL || bestblockhash == NULL || blocks == NULL) {
        printf("Error: Invalid json : %s\n", json);
        return 1;
    }
    
    // bitcoind -blocknotify=/root/blocknotify %s
    if (argc > 1) {
        char *firstArg = argv[1];
        if (strcmp(firstArg, bestblockhash) != 0) {
            printf("Error: Invalid hash arg : %s\n", firstArg);
            return 1;
        }
    }

    timestampt = atoi(timestamp);
    times = gmtime(&timestampt); // localtime(&timestampt);
    sprintf(dir, "%s/%d/%02d/%02d", BASEDIR, times->tm_year + 1900, times->tm_mon + 1, times->tm_mday);
    sprintf(filename, "%d%02d%02d_%02d%02d%02d_%s_%s_%s.json", times->tm_year + 1900, times->tm_mon + 1, times->tm_mday, times->tm_hour, times->tm_min, times->tm_sec, timestamp, blocks, bestblockhash);
    sprintf(path, "%s/%s", dir, filename);
    printf("saveto = %s\n", path);
    checkdir(strdup(dir));
    if (strlen(path) < 1 || strlen(path) > 254) {
        printf("Error: Invalid path : %s\n", path);
        return 1;
    }
    FILE *fp = fopen(path, "w");
    if (fp != NULL) {
        removeChar(json, ' ');
        removeChar(json, '\r');
        removeChar(json, '\n');
        fputs(json, fp);
        fclose(fp);
    } else {
        printf("Error: Failed to open file : %s\n", path);
        return 1;
    }
    sprintf(gz, "gzip -f -9 %s", path);
    system(gz);
    removeDotAfter(filename);
    indexfileadd(filename, dir);
    printf("OK\n");
    return 0;
}
