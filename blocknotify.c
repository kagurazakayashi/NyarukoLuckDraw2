#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <sys/stat.h>
#include <dirent.h>

// Development
#define GETCMD "cat test.json"
#define BASEDIR "/d/c/NyarukoLuckDraw2"

// Production
// #define GETCMD "/root/bitcoin/bin/bitcoin-cli -datadir=/root/BitcoinData -rpcconnect=127.0.0.1 -rpcport=8332 -rpcuser=yashi -rpcpassword=****** getblockchaininfo"
// #define BASEDIR "/root/BitcoinData/hashlog"

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

int main(int argc, char *argv[]) {
    char *json = NULL;
    char *blocks = NULL;
    char *bestblockhash = NULL;
    char *timestamp = NULL;
    time_t timestampt = 0;
    struct tm *times = NULL;
    char gz[212] = "\0";
    char dir[128] = "\0";
    char path[200] = "\0";
    
    rungethashcmd(&json);
    chkjson(strdup(json), &timestamp, &bestblockhash, &blocks);
    if (timestamp == NULL || bestblockhash == NULL || blocks == NULL) {
        printf("Error: Invalid json : %s\n", json);
        return 1;
    }

    timestampt = atoi(timestamp);
    times = gmtime(&timestampt); // localtime(&timestampt);
    sprintf(dir, "%s/%d/%02d/%02d/%02d", BASEDIR, times->tm_year + 1900, times->tm_mon + 1, times->tm_mday, times->tm_hour);
    sprintf(path, "%s/%d%02d%02d_%02d%02d%02d_%s_%s_%s.json", dir, times->tm_year + 1900, times->tm_mon + 1, times->tm_mday, times->tm_hour, times->tm_min, times->tm_sec, timestamp, blocks, bestblockhash);
    printf("saveto = %s\n", dir);
    checkdir(strdup(dir));
    if (strlen(path) < 1 || strlen(path) > 254) {
        printf("Error: Invalid path : %s\n", path);
        return 1;
    }
    FILE *fp = fopen(path, "w");
    printf("file = %s\n", path);
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
    printf("OK\n");
    return 0;
}