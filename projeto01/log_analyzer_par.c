#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <stdbool.h>
#include <time.h>

#define MAX_LINES 1000000
#define MAX_LINE_LENGTH 1024

// Struct para os contadores
typedef struct {
    long long count404;
    long long count200Bytes;
} Contadores;

// Variáveis globais
char *lines[MAX_LINES];
int chunk_size;
int num_threads;
pthread_mutex_t mutex;
Contadores contadores = {0, 0}; // Inicializa a struct com zeros

void* thread_func(void* arg) {
    int thread_id = *(int*)arg;
    int start = thread_id * chunk_size;
    int end = (thread_id == num_threads - 1) ? MAX_LINES : start + chunk_size;

    for (int i = start; i < end && lines[i] != NULL; i++) {
        char *line = lines[i];
        char *token;
        int i = 0;
        bool is200 = false;
        token = strtok(line, " ");
        while (token != NULL) {
            if (i == 8 && strcmp(token, "404") == 0) {
                pthread_mutex_lock(&mutex);
                contadores.count404++;
                pthread_mutex_unlock(&mutex);
            } else if (is200) {
                pthread_mutex_lock(&mutex);
                contadores.count200Bytes += atoll(token);

                is200 = false;
                pthread_mutex_unlock(&mutex);
            } else if (strcmp(token, "200") == 0) {
                pthread_mutex_lock(&mutex);
                is200 = true;
                pthread_mutex_unlock(&mutex);
            }
            token = strtok(NULL, " ");
            i++;
        }
    }

    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Uso: %s <num_threads>\n", argv[0]);
        return 1;
    }

    num_threads = atoi(argv[1]);

    FILE *arquivo = fopen("access_log_large.txt", "r");
    if (!arquivo) {
        perror("Erro ao abrir o arquivo");
        return 1;
    }

    char buffer[MAX_LINE_LENGTH];
    int total_lines = 0;

    while (fgets(buffer, sizeof(buffer), arquivo) && total_lines < MAX_LINES) {
        lines[total_lines++] = strdup(buffer);
    }
    fclose(arquivo);

    chunk_size = total_lines / num_threads;

    pthread_t threads[num_threads];
    int thread_ids[num_threads];
    pthread_mutex_init(&mutex, NULL);

    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    for (int i = 0; i < num_threads; i++) {
        thread_ids[i] = i;
        pthread_create(&threads[i], NULL, thread_func, &thread_ids[i]);
    }

    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    clock_gettime(CLOCK_MONOTONIC, &end);
    double tempo_execucao = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;

    printf("404: %lld\n", contadores.count404);
    printf("200 Bytes: %lld\n", contadores.count200Bytes);
    printf("Tempo de execução: %.6f segundos\n", tempo_execucao);

    for (int i = 0; i < total_lines; i++) {
        free(lines[i]);
    }

    pthread_mutex_destroy(&mutex);
    return 0;
}

