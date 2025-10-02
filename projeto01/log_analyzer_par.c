//Francisco Losada Totaro - 103646673
//Pedro Henrique L. Morrerias - 10441998
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <stdbool.h>
#include <time.h>

#define MAX_LINES 1000000
#define MAX_LINE_LENGTH 4096

typedef struct {
    long long errors404;
    long long total_bytes;
} Stats;

char *lines[MAX_LINES];
int total_lines = 0;
int num_threads = 0;
Stats global_stats = {0, 0};
pthread_mutex_t mutex;

void* thread_func(void* arg) {
    int thread_id = *(int*)arg;
    int base = total_lines / num_threads;
    int rem = total_lines % num_threads;
    int start = thread_id * base + (thread_id < rem ? thread_id : rem);
    int end = start + base + (thread_id < rem ? 1 : 0);

    long long local404 = 0;
    long long localBytes = 0;

    for (int idx = start; idx < end; idx++) {
        if (!lines[idx]) continue;
        char local[MAX_LINE_LENGTH];
        strncpy(local, lines[idx], MAX_LINE_LENGTH-1);
        local[MAX_LINE_LENGTH-1] = '\0';

        char *saveptr = NULL;
        char *token = strtok_r(local, " ", &saveptr);
        int field = 0;
        char *status = NULL;
        char *bytes = NULL;
        while (token && field <= 8) {
            if (field == 8) status = token;
            token = strtok_r(NULL, " ", &saveptr);
            field++;
        }
        if (status && token) bytes = token;

        if (status && strcmp(status, "404") == 0) {
            local404++;
        } else if (status && strcmp(status, "200") == 0 && bytes) {
            localBytes += atoll(bytes);
        }
    }

    pthread_mutex_lock(&mutex);
    global_stats.errors404 += local404;
    global_stats.total_bytes += localBytes;
    pthread_mutex_unlock(&mutex);
    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Uso: %s <num_threads>\n", argv[0]);
        return 1;
    }
    num_threads = atoi(argv[1]);
    if (num_threads <= 0) {
        printf("Número de threads deve ser maior que 0\n");
        return 1;
    }

    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    FILE *fp = fopen("access_log_large.txt", "r");
    if (!fp) {
        perror("Erro ao abrir o arquivo");
        exit(EXIT_FAILURE);
    }
    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    while ((read = getline(&line, &len, fp)) != -1 && total_lines < MAX_LINES) {
        lines[total_lines++] = strdup(line);
    }
    fclose(fp);
    if (line) {
        free(line);
    }

    pthread_t threads[num_threads];
    int thread_ids[num_threads];
    pthread_mutex_init(&mutex, NULL);
    for (int i = 0; i < num_threads; i++) {
        thread_ids[i] = i;
        pthread_create(&threads[i], NULL, thread_func, &thread_ids[i]);
    }
    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    clock_gettime(CLOCK_MONOTONIC, &end);
    double time_taken = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;

    printf("404: %lld\n", global_stats.errors404);
    printf("200 Bytes: %lld\n", global_stats.total_bytes);
    printf("Tempo de execucao: %f segundos\n", time_taken);

    for (int i = 0; i < total_lines; i++) {
        free(lines[i]);
    }
    pthread_mutex_destroy(&mutex);
    return 0;
}