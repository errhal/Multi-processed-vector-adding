#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

struct thread_info {
    double **A_matrix_pointer;
    double **B_matrix_pointer;
    double **C_matrix_pointer;

    int i, j, b;
};

void *subsum(void *arg) {
    double s = 0;
    struct thread_info t1 = *(struct thread_info *) (arg);
    for (int k = 0; k < t1.b; k++) {
        s += t1.A_matrix_pointer[t1.i][k] * t1.B_matrix_pointer[k][t1.j];
    }
    t1.C_matrix_pointer[t1.i][t1.j] = s;
    //printf("%d %d %d\n", t1.i, t1.j ,t1.b);
}

void mnoz(double **A, int a, int b, double **B, int c, int d, double **C) {
    int *status = malloc(a * d * sizeof(int));
    pthread_t *pthread_id = malloc(a * d * sizeof(pthread_id));
    struct thread_info* t1 = malloc(d * sizeof(struct thread_info));
    int thread_count = 0;

    int i, j;
    for (i = 0; i < a; i++) {
        for (j = 0; j < d; j++) {

            t1[thread_count].A_matrix_pointer = A;
            t1[thread_count].B_matrix_pointer = B;
            t1[thread_count].C_matrix_pointer = C;
            t1[thread_count].i = i;
            t1[thread_count].j = j;
            t1[thread_count].b = b;

            pthread_create(&pthread_id[thread_count], NULL, &subsum, &t1[thread_count]);
            thread_count++;
        }

    }
    for (i = 0; i < thread_count; i++) {
        pthread_join(pthread_id[i], (void **)status[i]);
        printf("Thread %d status: %d\n", i, status[i]);
    }

}

print_matrix(double **A, int m, int n) {
    int i, j;
    printf("[");
    for (i = 0; i < m; i++) {
        for (j = 0; j < n; j++) {
            printf("%f ", A[i][j]);
        }
        printf("\n");
    }
    printf("]\n");
}

print_sum(double **A, int m, int n) {
    double sum = 0, frobenius = 0;
    for(int i = 0; i<m; i++) {
        for(int j = 0; j< n;j++) {
            sum += A[i][j];
            frobenius += A[i][j] * A[i][j];
        }
    }
    printf("Suma elementów: %lf\n", sum);
    printf("Miara Frobeniusa: %lf\n", sqrt(frobenius));
}

int main(int argc, char **argv) {
    FILE *fpa;
    FILE *fpb;
    double **A;
    double **B;
    double **C;
    int ma, mb, na, nb;
    int i, j;
    double x;

    fpa = fopen("A.txt", "r");
    fpb = fopen("B.txt", "r");
    if (fpa == NULL || fpb == NULL) {
        perror("Błąd otwarcia pliku");
        exit(-10);
    }

    fscanf(fpa, "%d", &ma);
    fscanf(fpa, "%d", &na);

    fscanf(fpb, "%d", &mb);
    fscanf(fpb, "%d", &nb);

    printf("Pierwsza macierz ma wymiar %d x %d, a druga %d x %d\n", ma, na, mb, nb);

    if (na != mb) {
        printf("Złe wymiary macierzy!\n");
        return EXIT_FAILURE;
    }

    A = malloc(ma * sizeof(double));
    for (i = 0; i < ma; i++) {
        A[i] = malloc(na * sizeof(double));
    }

    B = malloc(mb * sizeof(double));
    for (i = 0; i < mb; i++) {
        B[i] = malloc(nb * sizeof(double));
    }

    C = malloc(ma * sizeof(double));
    for (i = 0; i < ma; i++) {
        C[i] = malloc(nb * sizeof(double));
    }

    printf("Rozmiar C: %dx%d\n", ma, nb);

    for (i = 0; i < ma; i++) {
        for (j = 0; j < na; j++) {
            fscanf(fpa, "%lf", &x);
            A[i][j] = x;
        }
    }

    //printf("A:\n");
    //print_matrix(A, ma, mb);

    for (i = 0; i < mb; i++) {
        for (j = 0; j < nb; j++) {
            fscanf(fpb, "%lf", &x);
            B[i][j] = x;
        }
    }

    //printf("B:\n");
    //print_matrix(B, mb, nb);

    mnoz(A, ma, na, B, mb, nb, C);

    printf("C:\n");
    print_matrix(C, ma, nb);
    print_sum(C, ma, nb);

    for (i = 0; i < na; i++) {
        free(A[i]);
    }
    free(A);

    for (i = 0; i < nb; i++) {
        free(B[i]);
    }
    free(B);

    for (i = 0; i < nb; i++) {
        free(C[i]);
    }
    free(C);

    fclose(fpa);
    fclose(fpb);

    return 0;
}