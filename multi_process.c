#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <zconf.h>
#include <sys/shm.h>
#include <string.h>

int MAX_BUF_SIZE = 1024;
int FILE_INPUT_MODE = 0;
int RANDOM_VECTOR_SIZE = 20001;

int SIGUSR_FLAG = 0;

void setup_child_process(int, int, int, int);
int* init_vector(double*);
void catch_signal(int);

int main(int argc, char** argv) {

    if(argc != 2) {
        puts("Usage: ./program $1, where $1 is number of processes used in sum");
        return -1;
    }

    int n = atoi(argv[1]);
    int fromaddr = shmget("FROM_ADDR", (n+1) * sizeof(int), 0666|IPC_CREAT);
    int toaddr = shmget("TO_ADDR", (n+1) * sizeof(int), 0666|IPC_CREAT);
    int resaddr = shmget("RES_ADDR", (n+1) * sizeof(double), 0666|IPC_CREAT);
    int* subprocesses = (int*)malloc(1024 * sizeof(int));
    double* vector;
    int vector_size = 0;

    if(FILE_INPUT_MODE) {
        FILE *file = fopen("vector.dat", "r");
        char* buffer = malloc(MAX_BUF_SIZE);
        vector_size = 0;
        fgets(buffer, MAX_BUF_SIZE, file);
        vector = malloc((vector_size=atoi(buffer)) * sizeof(double));
        for(int x = 0; x<vector_size; x++) {
            fgets(buffer, MAX_BUF_SIZE, file);
            vector[x] = atof(buffer);
        }
        fclose(file);
    } else {
        vector_size = RANDOM_VECTOR_SIZE;
        vector = init_vector(vector);
    }

    int vecaddr = shmget("VEC_ADDR", vector_size * sizeof(double), 0666|IPC_CREAT);

    double* shared_vector = shmat(vecaddr, 0, 0);
    memcpy(shared_vector, vector, vector_size*sizeof(double));

    int* f_addr = shmat(fromaddr, 0, 0);
    int* t_addr = shmat(toaddr, 0, 0);
    double* r_addr = shmat(resaddr, 0, 0);


    int diff = vector_size/n;
    //printf("DIFF %d", diff);
    int start = 0;
    int index = 0;
    for(int z = 0; z < n+1; z++) {
        f_addr[z] = start;
        t_addr[z] = start + diff > RANDOM_VECTOR_SIZE-1 ? RANDOM_VECTOR_SIZE-1 : start + diff;
        start+=diff+1;
    }


    for(int i = 0; i < n+1; i++) {
        pid_t pid = fork();
        switch(pid) {
            case -1:
                puts("There is an error in creating new processes");
                return -1;
            case 0:
                //printf("STARTED : %d", i);
                setup_child_process(i, fromaddr, toaddr, resaddr);
                return 0;
            default:
                subprocesses[i] = pid;
                continue;
        }
    }

    sleep(1);
    int status = 0;
    //printf("SUBPROCESSES STATUS : %d\n", status);

    puts("Parent process ...");

    for(int i = 0; i<n+1; i++) {
        kill(subprocesses[i], SIGUSR1);
    }
    for(int i = 0; i<n+1; i++) {
        waitpid(subprocesses[i], &status, 0);
    }

    double sum = 0;
    for(int y = 0; y<n+1; y++) {
        sum += r_addr[y];
    }
    printf("%lf", sum);

    shmdt(f_addr);
    shmdt(t_addr);
    shmdt(r_addr);
    shmdt(shared_vector);
    shmctl(fromaddr, IPC_RMID, 0);
    shmctl(toaddr, IPC_RMID, 0);
    shmctl(resaddr, IPC_RMID, 0);
    shmctl(vecaddr, IPC_RMID, 0);
    return 0;



}

void setup_child_process(int n, int from_addr, int to_addr, int result_addr) {
    //printf("Spawned process number:  %d %d %d\n", n, from_addr, to_addr);

    sigset_t mask;
    struct sigaction usr1;
    sigemptyset(&mask);

    usr1.sa_flags = SA_SIGINFO;
    usr1.sa_mask = mask;
    usr1.sa_handler = &catch_signal;

    sigaction(SIGUSR1, &usr1, NULL);
    pause();
    if(SIGUSR_FLAG) {
        //puts("Caught sig!");

        double* shared_vector = shmat(shmget("VEC_ADDR", NULL, 0666|IPC_CREAT), 0, 0);
        int* faddr = shmat(from_addr, 0 , 0);
        int* taddr = shmat(to_addr, 0, 0);
        double sum = 0;

        for(int x = faddr[n]; x <= taddr[n]; x++) {
            sum += shared_vector[x];
        }

        double* result = shmat(result_addr, 0, 0);
        result[n] = sum;
        shmdt(shared_vector);
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

void catch_signal(int sig_info) {
    SIGUSR_FLAG = 1;
}