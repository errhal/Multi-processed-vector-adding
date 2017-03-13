import java.io.File;
import java.io.FileNotFoundException;
import java.util.Scanner;

public class Main {
    public static void main(String[] args) {

        int row, col;
        double matrixA[][] = read("A.txt");
        double matrixB[][] = read("B.txt");
        double matrixC[][] = new double[matrixA.length][matrixB[0].length];
        int threadcount = 0;
        int num_of_thread = (matrixA.length * matrixB[0].length);
        Thread[] thrd = new Thread[num_of_thread];
        try {
            for (row = 0; row < matrixA.length; row++) {
                for (col = 0; col < matrixB[0].length; col++) {
                    thrd[threadcount] = new Thread(new MyThread(matrixA, matrixB, matrixC, row, col));
                    thrd[threadcount].start();
                    threadcount++;
                }
            }
            for(int j = 0; j < threadcount; j++) {
                thrd[j].join();
            }
        } catch (InterruptedException e) {
        }
        display(matrixC);
    }

    public static double[][] read(String fileName) {

        File file = new File(fileName);
        Scanner scan = null;
        try {
            scan = new Scanner(file);
        } catch (FileNotFoundException e) {
            System.out.println("Nie znaleziono takiego pliku");
            e.printStackTrace();
        } finally {
            int rows = scan.nextInt();
            int cols = scan.nextInt();
            double matrix[][] = new double[rows][cols];

            for (int i = 0; i < rows; i++) {
                for (int j = 0; j < cols; j++) {
                    matrix[i][j] = scan.nextDouble();
                }
            }
            return matrix;
        }
    }

    public static void display(double[][] matrix) {
        for (int i = 0; i < matrix.length; i++) {
            for (int j = 0; j < matrix[0].length; j++) {
                System.out.print(matrix[i][j] + "\t");
            }
            System.out.println();
        }
        System.out.println("Suma elementów macierzy: " + sumOfElements(matrix));
        System.out.println("Norma Forbeniusa: " + forbeniusNorm(matrix));
    }

    public static int sumOfElements(double[][] matrix) {
        int sum = 0;
        for (int i = 0; i < matrix.length; i++) {
            for (int j = 0; j < matrix[0].length; j++) {
                sum += matrix[i][j];
            }
        }
        return sum;
    }

    public static double forbeniusNorm(double[][] matrix) {
        double norm = 0;
        double sumOfSquare = 0;
        for (int i = 0; i < matrix.length; i++) {
            for (int j = 0; j < matrix[0].length; j++) {
                sumOfSquare += Math.pow(matrix[i][j], 2);
            }
        }
        norm = Math.sqrt(sumOfSquare);
        return norm;
    }

    public static class MyThread implements Runnable {

        double[][] matrixA, matrixB, matrixC;
        int row;
        int col;

        public MyThread(double matrixA[][], double matrixB[][], double matrixC[][], int rows, int cols) {
            this.matrixA = matrixA;
            this.matrixB = matrixB;
            this.matrixC = matrixC;
            this.row = rows;
            this.col = cols;
        }

        public void run() {
            int sum = 0;
            for (int i = 0; i < matrixB.length; i++)
                sum += (matrixA[row][i] * matrixB[i][col]);
            matrixC[row][col] = sum;
        }
    }
}
