#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// #define GETCMD "/root/bitcoin/bin/bitcoin-cli -datadir=/root/BitcoinData -rpcconnect=127.0.0.1 -rpcport=8332 -rpcuser=yashi -rpcpassword=****** getblockchaininfo"
#define GETCMD "cat test.json"
#define BASEDIR "."

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
        if (*q != ' ' && *q != '\n' && *q != ',') {
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

void chkjson(char **json) {
    char *line = strtok(*json, "\n");
    while (line != NULL) {
        line = strtok(NULL, "\n");
        if (line != NULL) {
            trim(line);
            removeChar(line, '"');
            char left[14] = "\0";
            char right[65] = "\0";
            split2(line, left, right);
            if (strlen(left) > 1 && strlen(right) > 1) {
                printf("%s = %s\n", left, right);
            }
        }
    }
}

int main() {
    char *json = NULL;
    char *timestamp = NULL;
    
    rungethashcmd(&json);
    chkjson(&json);

    free(json);
    return 0;
}