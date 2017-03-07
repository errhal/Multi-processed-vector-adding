#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <zconf.h>
#include <sys/shm.h>
#include <string.h>

int MAX_BUF_SIZE = 1024;
int FILE_INPUT_MODE = 0;
int RANDOM_VECTOR_SIZE = 1000000;

int SIGUSR_FLAG = 0;

void setup_child_process(int, int, int, int);
int* init_vector(double*);
void catch_signal();

int main(int argc, char** argv) {

    if(argc != 2) {
        puts("Usage: ./program $1, where $1 is number of processes used in sum");
        return -1;
    }

    int n = atoi(argv[1]);
    int diff = RANDOM_VECTOR_SIZE/n;
    int fromaddr = shmget("FROM_ADDR", n * sizeof(int), 0666|IPC_CREAT), toaddr = shmget("TO_ADDR", n*sizeof(int), 0666|IPC_CREAT), result = shmget("RES_ADDR", n*sizeof(double), 0666|IPC_CREAT);

    int* f_addr = shmat(fromaddr, 0, 0);
    int* t_addr = shmat(toaddr, 0, 0);
    double* r_addr = shmat(result, 0, 0);

    int start = 0;
    int index = 0;
    while(start + diff < RANDOM_VECTOR_SIZE) {
        f_addr[index] = start;
        t_addr[index] = start + diff;
        start+=diff+1;
        index++;
    }
    f_addr[index] = start;
    t_addr[index] = RANDOM_VECTOR_SIZE -1;

    for(int i = 0; i < n; i++) {
        pid_t pid = fork();
        switch(pid) {
            case -1:
                puts("There is an error in creating new processes");
                return -1;
            case 0:
                setup_child_process(i, fromaddr, toaddr, result);
                break;
            default:
                puts("Parent process ...");

                double* vector;
                double* shared_vector;
                //for desperates ...
                if(FILE_INPUT_MODE) {
                    FILE *file = fopen("vector.dat", "w");
                    char* buffer = malloc(MAX_BUF_SIZE);
                    int vector_size = 0;
                    fgets(buffer, MAX_BUF_SIZE, file);
                    vector = malloc((vector_size=atoi(buffer)) * sizeof(double));
                    for(int x = 0; x<vector_size; x++) {
                        fgets(buffer, MAX_BUF_SIZE, file);
                        vector[x] = atof(buffer);
                    }

                    shared_vector = shmat(shmget("VECTOR", sizeof(double) * vector_size, 0666|IPC_CREAT), 0, 0);
                    memcpy(shared_vector, vector, vector_size*sizeof(double));
                    fclose(file);
                } else {
                    vector = init_vector(vector);
                    shared_vector = shmat(shmget("VECTOR", sizeof(double) * RANDOM_VECTOR_SIZE, 0666|IPC_CREAT), 0, 0);
                    memcpy(shared_vector, vector, RANDOM_VECTOR_SIZE*sizeof(double));
                }

                kill(pid, SIGUSR1);
                wait(20000);
                double sum = 0;
                for(int y = 0; y<n; y++) {
                    sum += r_addr[y];
                }
                printf("%f", sum);
                return 0;
        }
    }

}

void setup_child_process(int n, int from_addr, int to_addr, int result_addr) {
    printf("Spawned process number:  %d\n", n);

    sigset_t mask;
    struct sigaction usr1;
    sigemptyset(&mask);

    usr1.sa_flags = SA_SIGINFO;
    usr1.sa_mask = mask;
    usr1.sa_handler = &catch_signal;
    int flag = 0;
    sigaction(SIGUSR1, &usr1, NULL);
    pause();
    if(SIGUSR_FLAG) {
        puts("Caught sig!");

        double* shared_vector = shmat(shmget("VECTOR", NULL, 0666|IPC_CREAT), 0, 0);
        int* faddr = shmat(from_addr, 0 , 0);
        int* taddr = shmat(to_addr, 0, 0);
        double sum = 0;
        for(int x = faddr[n]; x < taddr[n]; x++) {
            sum += shared_vector[x];
        }

        double* result = shmat(result_addr, 0, 0);
        result[n] = sum;
        return;
    }
}

int* init_vector(double* vector) {
    vector = malloc(RANDOM_VECTOR_SIZE * sizeof(double));
    for(int i = 0; i< RANDOM_VECTOR_SIZE; i++) {
        vector[i] = 1;
    }
    return vector;
}

void catch_signal() {
    SIGUSR_FLAG = 1;
}