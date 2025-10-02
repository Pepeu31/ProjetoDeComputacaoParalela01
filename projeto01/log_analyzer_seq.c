#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>

int main () {
    char *token;
    const char delimiter[] = " ";
    int count404 = 0;
    bool is200 = false;
    long long count200Bytes = 0;
    bool is404 = false;
    int i = 0;
    struct timespec start, end;

    clock_gettime(CLOCK_MONOTONIC, &start);
    FILE *fp = fopen("access_log_large.txt", "r");
    if (fp == NULL) {
        perror("Erro ao abrir o arquivo");
        exit(EXIT_FAILURE);
    }

    char *line = NULL;
    size_t len = 0;
    ssize_t read;

    while((read = getline(&line, &len, fp)) != -1) {
        token = strtok(line, delimiter);
        i = 0;
        is200 = false;  // Reset is200 para cada linha (processamento independente)
        while (token != NULL) {
            if (i == 8 && strcmp(token, "404") == 0) {
                count404++;
            }
            else if(is200 == true) {
                count200Bytes += atoi(token);
                is200 = false;
            }
            else if(strcmp(token, "200") == 0) {
                is200 = true;
            }
            i++;
            token = strtok(NULL, delimiter);

        }
    }
    printf("404: %d\n200 Bytes: %lld\n", count404, count200Bytes);
    fclose(fp);
    if (line) {
        free(line);
    }
    clock_gettime(CLOCK_MONOTONIC, &end);
    double time_taken = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
    printf("Tempo de execucao: %f segundos\n", time_taken);
    return 0;
}