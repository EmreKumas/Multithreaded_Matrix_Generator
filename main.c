#pragma GCC diagnostic ignored "-Wpointer-to-int-cast"
#pragma GCC diagnostic ignored "-Wint-to-pointer-cast"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>
#include <math.h>
#include <stdbool.h>

//Struct declarations.
typedef struct{

    int matrix_index;
    int matrix[5][5];
    int global_row_index;
    int global_column_index;

}Matrix;

//Global variables.
int **main_matrix = NULL;
int total_sum = 0;
int matrix_size;

int index_of_the_matrix_to_generate = 0;
int submatrix_needed;
int extra_matrix_needed;
int extra_log_needed;
int extra_mod_needed;
int extra_add_needed;

Matrix **first_queue = NULL;
Matrix **second_queue = NULL;

int first_queue_push_index = 0;
int second_queue_push_index = 0;
int queue_log_index = 0;
int queue_mod_index = 0;
int queue_add_index = 0;

//Mutex and Semaphores
pthread_mutex_t generate_mutex;
pthread_mutex_t log_mutex;
pthread_mutex_t mod_mutex;
pthread_mutex_t add_mutex;
pthread_mutex_t write_to_screen_mutex;

sem_t log_semaphore;
sem_t mod_semaphore;
sem_t add_semaphore;

//Function declarations.
void *Generate_Thread(void *);
void *Log_Thread(void *);
void *Mod_Thread(void *);
void *Add_Thread(void *);

void enqueue(Matrix **, int *, Matrix *);
void freeQueue(Matrix **, int);

Matrix **createQueue();
Matrix *getFromQueue(Matrix **, int *);

int main(int argc, char *argv[]){

    if(argc != 8){
        fprintf(stderr, "Incorrect usage of %s, Usage: %s -d Matrix_size -n Generate_thread_count Log_thread_count Mod_thread_count Add_thread_count\n", argv[0], argv[0]);
        exit(-1);
    }

    //Random number generator.
    srand((unsigned int) time(NULL)); // NOLINT(cert-msc32-c,cert-msc51-cpp)

    matrix_size = (int) strtol(argv[2], NULL, 10);

    //Local variables.
    int generate_thread_count = (int) strtol(argv[4], NULL, 10);
    int log_thread_count = (int) strtol(argv[5], NULL, 10);
    int mod_thread_count = (int) strtol(argv[6], NULL, 10);
    int add_thread_count = (int) strtol(argv[7], NULL, 10);

    int thread_control, loop_index;

    //If any thread count entered is 0, we won't execute the program.
    if(generate_thread_count == 0 | log_thread_count == 0 | mod_thread_count == 0 | add_thread_count == 0){
        fprintf(stderr, "Incorrect usage, no thread count can be 0, at least 1 is expected!!!\n");
        exit(-1);
    }

    //Deciding how many submatrix we're gonna need.
    submatrix_needed = (int) pow(matrix_size / 5, 2); // NOLINT(bugprone-integer-division)

    /**
     * After deciding the number of submatrix needed, we need to check if the generate thread count is enough to create these matrices. If not, we need to generate the rest
     * by the same generate threads.
     */

    if(submatrix_needed > generate_thread_count)
        extra_matrix_needed = submatrix_needed - generate_thread_count;

    if(submatrix_needed > log_thread_count)
        extra_log_needed = submatrix_needed - log_thread_count;

    if(submatrix_needed > mod_thread_count)
        extra_mod_needed = submatrix_needed - mod_thread_count;

    if(submatrix_needed > add_thread_count)
        extra_add_needed = submatrix_needed - add_thread_count;

    pthread_t generate_thread_ids[generate_thread_count];
    pthread_t log_thread_ids[log_thread_count];
    pthread_t mod_thread_ids[mod_thread_count];
    pthread_t add_thread_ids[add_thread_count];

    //Initializing mutex and semaphores.
    pthread_mutex_init(&generate_mutex, NULL);
    pthread_mutex_init(&log_mutex, NULL);
    pthread_mutex_init(&mod_mutex, NULL);
    pthread_mutex_init(&add_mutex, NULL);
    pthread_mutex_init(&write_to_screen_mutex, NULL);

    sem_init(&log_semaphore, 0, 0);
    sem_init(&mod_semaphore, 0, 0);
    sem_init(&add_semaphore, 0, 0);

    //Creating generate threads. If the thread count is greater than the submatrix needed, we won't run them since we don't need them.
    for(loop_index = 0; loop_index < generate_thread_count && loop_index < submatrix_needed; loop_index++){

        thread_control = pthread_create(&generate_thread_ids[loop_index], NULL, &Generate_Thread, (void *) loop_index);

        if(thread_control){
            fprintf(stderr, "Error: return code from creating generate thread is %d\n", thread_control);
            exit(-1);
        }
    }

    //Creating log threads.
    for(loop_index = 0; loop_index < log_thread_count && loop_index < submatrix_needed; loop_index++){

        thread_control = pthread_create(&log_thread_ids[loop_index], NULL, &Log_Thread, (void *) loop_index);

        if(thread_control){
            fprintf(stderr, "Error: return code from creating log thread is %d\n", thread_control);
            exit(-1);
        }
    }

    //Creating mod threads.
    for(loop_index = 0; loop_index < mod_thread_count && loop_index < submatrix_needed; loop_index++){

        thread_control = pthread_create(&mod_thread_ids[loop_index], NULL, &Mod_Thread, (void *) loop_index);

        if(thread_control){
            fprintf(stderr, "Error: return code from creating mod thread is %d\n", thread_control);
            exit(-1);
        }
    }

    //Creating add threads.
    for(loop_index = 0; loop_index < add_thread_count && loop_index < submatrix_needed; loop_index++){

        thread_control = pthread_create(&add_thread_ids[loop_index], NULL, &Add_Thread, (void *) loop_index);

        if(thread_control){
            fprintf(stderr, "Error: return code from creating add thread is %d\n", thread_control);
            exit(-1);
        }
    }

    //Joining generate threads.
    for(loop_index = 0; loop_index < generate_thread_count && loop_index < submatrix_needed; loop_index++){

        thread_control = pthread_join(generate_thread_ids[loop_index], NULL);

        if(thread_control){
            fprintf(stderr, "Error: return code from joining the generate thread is %d\n", thread_control);
            exit(-1);
        }
    }

    //Joining log threads.
    for(loop_index = 0; loop_index < log_thread_count && loop_index < submatrix_needed; loop_index++){

        thread_control = pthread_join(log_thread_ids[loop_index], NULL);

        if(thread_control){
            fprintf(stderr, "Error: return code from joining the log thread is %d\n", thread_control);
            exit(-1);
        }
    }

    //Joining mod threads.
    for(loop_index = 0; loop_index < mod_thread_count && loop_index < submatrix_needed; loop_index++){

        thread_control = pthread_join(mod_thread_ids[loop_index], NULL);

        if(thread_control){
            fprintf(stderr, "Error: return code from joining the mod thread is %d\n", thread_control);
            exit(-1);
        }
    }

    //Joining add threads.
    for(loop_index = 0; loop_index < add_thread_count && loop_index < submatrix_needed; loop_index++){

        thread_control = pthread_join(add_thread_ids[loop_index], NULL);

        if(thread_control){
            fprintf(stderr, "Error: return code from joining the add thread is %d\n", thread_control);
            exit(-1);
        }
    }

    fprintf(stderr, "All threads are completed!!!\n");

    //Destroying mutex and semaphores.
    pthread_mutex_destroy(&generate_mutex);
    pthread_mutex_destroy(&log_mutex);
    pthread_mutex_destroy(&mod_mutex);
    pthread_mutex_destroy(&add_mutex);
    pthread_mutex_destroy(&write_to_screen_mutex);

    sem_destroy(&log_semaphore);
    sem_destroy(&mod_semaphore);
    sem_destroy(&add_semaphore);

    //Printing the results...
    int i, j;

    printf("\nThe matrix is\n");
    for(i = 0; i < matrix_size; i++){

        if(i == 0) printf("\t[");

        for(j = 0; j < matrix_size; j++){

            printf("%d", main_matrix[i][j]);

            if(i == matrix_size - 1 && j == matrix_size - 1)
                printf("]");
            else
                printf(",");
        }

        if(i != matrix_size - 1)
            printf("\n\t");
        else
            printf("\n\n");
    }

    printf("The global sum is : %d\n\n", total_sum);

    //Free the main matrix.
    for(i = 0; i < matrix_size; i++)
        free(main_matrix[i]);
    free(main_matrix);

    //Freeing the queues.
    freeQueue(first_queue, first_queue_push_index);
    freeQueue(second_queue, second_queue_push_index);

    pthread_exit(NULL);
}

void *Generate_Thread(void *argument){

    int generate_thread_index = (int) argument;
    int index_of_generated_matrix;
    int i, j;

    //Deciding which matrix to generate.
    pthread_mutex_lock(&generate_mutex);
    index_of_generated_matrix = index_of_the_matrix_to_generate;
    index_of_the_matrix_to_generate++;
    pthread_mutex_unlock(&generate_mutex);

    //Creating the matrix.
    Matrix *matrix = malloc(sizeof(Matrix));

    matrix->matrix_index = index_of_generated_matrix;
    matrix->global_row_index = index_of_generated_matrix / (matrix_size / 5);
    matrix->global_column_index = index_of_generated_matrix % (matrix_size / 5);

    for(i = 0; i < 5; i++){
        for(j = 0; j < 5; j++){

            matrix->matrix[i][j] = (rand() % 100) + 1; // NOLINT(cert-msc32-c,cert-msc51-cpp,cert-msc30-c,cert-msc50-cpp)
        }
    }

    //After creating the matrix, we will push the matrix into the queue. First we need to check if it is NULL, if it is we need initialize it.
    pthread_mutex_lock(&generate_mutex);
    if(first_queue == NULL)
        first_queue = createQueue();

    enqueue(first_queue, &first_queue_push_index, matrix);
    pthread_mutex_unlock(&generate_mutex);

    //For printing the job...
    pthread_mutex_lock(&write_to_screen_mutex);

    printf("Generator_%d\tGenerator_%d generated following matrix:[", (generate_thread_index + 1), (generate_thread_index + 1));
    for(i = 0; i < 5; i++){
        for(j = 0; j < 5; j++){

            printf("%d", matrix->matrix[i][j]);

            if(j == 4){
                if(i != 4)
                    printf(",\n\t\t\t\t\t\t\t");
                else
                    printf("]\n");
            }else
                printf(",");
        }
    }
    printf("\t\tThis matrix is [%d,%d] submatrix.\n\n", matrix->global_row_index, matrix->global_column_index);

    pthread_mutex_unlock(&write_to_screen_mutex);

    //Incrementing the semaphore.
    sem_post(&log_semaphore);
    sem_post(&mod_semaphore);

    //Now, we will check if extra matrix is needed.
    bool while_condition = true;
    bool generate_another_matrix = false;

    while(while_condition == true){

        pthread_mutex_lock(&generate_mutex);

        if(extra_matrix_needed == 0)
            while_condition = false;
        else{
            generate_another_matrix = true;
            index_of_generated_matrix = index_of_the_matrix_to_generate;
            index_of_the_matrix_to_generate++;
            extra_matrix_needed--;
        }

        pthread_mutex_unlock(&generate_mutex);

        if(generate_another_matrix == true){

            //Create another matrix.
            Matrix *extra_matrix = malloc(sizeof(Matrix));

            extra_matrix->matrix_index = index_of_generated_matrix;
            extra_matrix->global_row_index = index_of_generated_matrix / (matrix_size / 5);
            extra_matrix->global_column_index = index_of_generated_matrix % (matrix_size / 5);

            for(i = 0; i < 5; i++){
                for(j = 0; j < 5; j++){

                    extra_matrix->matrix[i][j] = (rand() % 100) + 1; // NOLINT(cert-msc32-c,cert-msc51-cpp,cert-msc30-c,cert-msc50-cpp)
                }
            }

            //After creating the matrix, we will push the matrix into the queue.
            pthread_mutex_lock(&generate_mutex);
            enqueue(first_queue, &first_queue_push_index, extra_matrix);
            pthread_mutex_unlock(&generate_mutex);

            //And again, print the results.
            pthread_mutex_lock(&write_to_screen_mutex);

            printf("Generator_%d\tGenerator_%d generated following matrix:[", (generate_thread_index + 1), (generate_thread_index + 1));
            for(i = 0; i < 5; i++){
                for(j = 0; j < 5; j++){

                    printf("%d", extra_matrix->matrix[i][j]);

                    if(j == 4){
                        if(i != 4)
                            printf(",\n\t\t\t\t\t\t\t");
                        else
                            printf("]\n");
                    }else
                        printf(",");
                }
            }
            printf("\t\tThis matrix is [%d,%d] submatrix.\n\n", extra_matrix->global_row_index, extra_matrix->global_column_index);

            pthread_mutex_unlock(&write_to_screen_mutex);

            //Incrementing the semaphore.
            sem_post(&log_semaphore);
            sem_post(&mod_semaphore);

            generate_another_matrix = false;
        }
    }

    pthread_exit(NULL);
}

void *Log_Thread(void *argument){

    int log_thread_index = (int) argument;
    int i, j, k, l;
    Matrix *matrix;

    pthread_mutex_lock(&log_mutex);

    if(main_matrix == NULL){

        main_matrix = malloc(matrix_size * sizeof(int *));

        for(i = 0; i < matrix_size; i++)
            main_matrix[i] = malloc(matrix_size * sizeof(int));
    }

    pthread_mutex_unlock(&log_mutex);

    //Waiting for a matrix to be enqueued.
    sem_wait(&log_semaphore);

    //Getting the matrix.
    pthread_mutex_lock(&log_mutex);
    matrix = getFromQueue(first_queue, &queue_log_index);
    pthread_mutex_unlock(&log_mutex);

    //Now, we need to add it to the global matrix.
    for(i = matrix->global_row_index * 5, k = 0; k < 5; i++, k++){
        for(j = matrix->global_column_index * 5, l = 0; l < 5; j++, l++){

            main_matrix[i][j] = matrix->matrix[k][l];
        }
    }

    //For printing the job...
    pthread_mutex_lock(&write_to_screen_mutex);

    printf("Log_%d\t\tLog_%d joined the matrix.\n", (log_thread_index + 1), (log_thread_index + 1));
    printf("\t\tThe matrix joined is [%d,%d] submatrix.\n\n", matrix->global_row_index, matrix->global_column_index);

    pthread_mutex_unlock(&write_to_screen_mutex);

    //Now, we will check if extra log is needed.
    bool while_condition = true;
    bool another_log_needed = false;

    while(while_condition == true){

        pthread_mutex_lock(&log_mutex);

        if(extra_log_needed == 0)
            while_condition = false;
        else{
            another_log_needed = true;
            extra_log_needed--;
        }

        pthread_mutex_unlock(&log_mutex);

        if(another_log_needed == true){

            Matrix *another_matrix;

            //Waiting for a matrix to be enqueued.
            sem_wait(&log_semaphore);

            //Getting the matrix.
            pthread_mutex_lock(&log_mutex);
            another_matrix = getFromQueue(first_queue, &queue_log_index);
            pthread_mutex_unlock(&log_mutex);

            //Now, we need to add it to the global matrix.
            for(i = another_matrix->global_row_index * 5, k = 0; k < 5; i++, k++){
                for(j = another_matrix->global_column_index * 5, l = 0; l < 5; j++, l++){

                    main_matrix[i][j] = another_matrix->matrix[k][l];
                }
            }

            //For printing the job...
            pthread_mutex_lock(&write_to_screen_mutex);

            printf("Log_%d\t\tLog_%d joined the matrix.\n", (log_thread_index + 1), (log_thread_index + 1));
            printf("\t\tThe matrix joined is [%d,%d] submatrix.\n\n", another_matrix->global_row_index, another_matrix->global_column_index);

            pthread_mutex_unlock(&write_to_screen_mutex);

            another_log_needed = false;
        }
    }

    pthread_exit(NULL);
}

void *Mod_Thread(void *argument){

    int mod_thread_index = (int) argument;
    int i, j, mod;
    Matrix *sourceMatrix;

    //Waiting for a matrix to be enqueued.
    sem_wait(&mod_semaphore);

    //Getting the matrix.
    pthread_mutex_lock(&mod_mutex);
    sourceMatrix = getFromQueue(first_queue, &queue_mod_index);
    pthread_mutex_unlock(&mod_mutex);

    //Creating the matrix.
    Matrix *matrix = malloc(sizeof(Matrix));

    matrix->matrix_index = sourceMatrix->matrix_index;
    matrix->global_row_index = sourceMatrix->global_row_index;
    matrix->global_column_index = sourceMatrix->global_column_index;

    for(i = 0; i < 5; i++){
        for(j = 0; j < 5; j++){

            matrix->matrix[i][j] = sourceMatrix->matrix[i][j];
        }
    }

    //Modifying the matrix by taking the mod.
    mod = matrix->matrix[0][0];

    for(i = 0; i < 5; i++){
        for(j = 0; j < 5; j++){

            matrix->matrix[i][j] = matrix->matrix[i][j] % mod;
        }
    }

    //After creating the matrix, we will push the matrix into the queue. First we need to check if it is NULL, if it is we need initialize it.
    pthread_mutex_lock(&mod_mutex);
    if(second_queue == NULL)
        second_queue = createQueue();

    enqueue(second_queue, &second_queue_push_index, matrix);
    pthread_mutex_unlock(&mod_mutex);

    //Incrementing the add semaphore.
    sem_post(&add_semaphore);

    //For printing the job...
    pthread_mutex_lock(&write_to_screen_mutex);

    printf("Mod_%d\t\tMod_%d generated following matrix:\t[", (mod_thread_index + 1), (mod_thread_index + 1));
    for(i = 0; i < 5; i++){
        for(j = 0; j < 5; j++){

            printf("%d", matrix->matrix[i][j]);

            if(j == 4){
                if(i != 4)
                    printf(",\n\t\t\t\t\t\t\t");
                else
                    printf("]\n");
            }else
                printf(",");
        }
    }
    printf("\t\tThis matrix is [%d,%d] submatrix.\n\n", matrix->global_row_index, matrix->global_column_index);

    pthread_mutex_unlock(&write_to_screen_mutex);

    //Now, we will check if extra matrix is needed.
    bool while_condition = true;
    bool another_mod_needed = false;

    while(while_condition == true){

        pthread_mutex_lock(&mod_mutex);

        if(extra_mod_needed == 0)
            while_condition = false;
        else{
            another_mod_needed = true;
            extra_mod_needed--;
        }

        pthread_mutex_unlock(&mod_mutex);

        if(another_mod_needed == true){

            Matrix *another_source_matrix;

            //Waiting for a matrix to be enqueued.
            sem_wait(&mod_semaphore);

            //Getting the matrix.
            pthread_mutex_lock(&mod_mutex);
            another_source_matrix = getFromQueue(first_queue, &queue_mod_index);
            pthread_mutex_unlock(&mod_mutex);

            //Creating the matrix.
            Matrix *another_matrix = malloc(sizeof(Matrix));

            another_matrix->matrix_index = another_source_matrix->matrix_index;
            another_matrix->global_row_index = another_source_matrix->global_row_index;
            another_matrix->global_column_index = another_source_matrix->global_column_index;

            for(i = 0; i < 5; i++){
                for(j = 0; j < 5; j++){

                    another_matrix->matrix[i][j] = another_source_matrix->matrix[i][j];
                }
            }

            //Modifying the matrix by taking the mod.
            mod = another_matrix->matrix[0][0];

            for(i = 0; i < 5; i++){
                for(j = 0; j < 5; j++){

                    another_matrix->matrix[i][j] = another_matrix->matrix[i][j] % mod;
                }
            }

            //After creating the matrix, we will push the matrix into the queue.
            pthread_mutex_lock(&mod_mutex);
            enqueue(second_queue, &second_queue_push_index, another_matrix);
            pthread_mutex_unlock(&mod_mutex);

            //Incrementing the add semaphore.
            sem_post(&add_semaphore);

            //For printing the job...
            pthread_mutex_lock(&write_to_screen_mutex);

            printf("Mod_%d\t\tMod_%d generated following matrix:\t[", (mod_thread_index + 1), (mod_thread_index + 1));
            for(i = 0; i < 5; i++){
                for(j = 0; j < 5; j++){

                    printf("%d", another_matrix->matrix[i][j]);

                    if(j == 4){
                        if(i != 4)
                            printf(",\n\t\t\t\t\t\t\t");
                        else
                            printf("]\n");
                    }else
                        printf(",");
                }
            }
            printf("\t\tThis matrix is [%d,%d] submatrix.\n\n", another_matrix->global_row_index, another_matrix->global_column_index);

            pthread_mutex_unlock(&write_to_screen_mutex);

            another_mod_needed = false;
        }
    }

    pthread_exit(NULL);
}

void *Add_Thread(void *argument){

    int add_thread_index = (int) argument;
    int i, j, local_sum = 0, temp_total_sum;
    Matrix *matrix;

    //Waiting for a matrix to be enqueued.
    sem_wait(&add_semaphore);

    //Getting the matrix.
    pthread_mutex_lock(&add_mutex);
    matrix = getFromQueue(second_queue, &queue_add_index);
    pthread_mutex_unlock(&add_mutex);

    for(i = 0; i < 5; i++){
        for(j = 0; j < 5; j++){

            local_sum += matrix->matrix[i][j];
        }
    }

    //Adding the local sum to the total sum.
    pthread_mutex_lock(&add_mutex);
    temp_total_sum = total_sum;
    total_sum += local_sum;

    //For printing the job...
    pthread_mutex_lock(&write_to_screen_mutex);

    printf("Add_%d\t\tAdd_%d has local sum: %d by [%d,%d] submatrix.\n", (add_thread_index + 1), (add_thread_index + 1), local_sum, matrix->global_row_index, matrix->global_column_index);
    printf("\t\tGlobal sum before/after update: %d/%d\n\n", temp_total_sum, total_sum);

    pthread_mutex_unlock(&write_to_screen_mutex);

    pthread_mutex_unlock(&add_mutex);

    //Now, we will check if extra matrix is needed.
    bool while_condition = true;
    bool another_add_needed = false;

    while(while_condition == true){

        pthread_mutex_lock(&add_mutex);

        if(extra_add_needed == 0)
            while_condition = false;
        else{
            another_add_needed = true;
            extra_add_needed--;
        }

        pthread_mutex_unlock(&add_mutex);

        if(another_add_needed == true){

            Matrix *another_matrix;

            //Waiting for a matrix to be enqueued.
            sem_wait(&add_semaphore);

            //Getting the matrix.
            pthread_mutex_lock(&add_mutex);
            another_matrix = getFromQueue(second_queue, &queue_add_index);
            pthread_mutex_unlock(&add_mutex);

            local_sum = 0;
            for(i = 0; i < 5; i++){
                for(j = 0; j < 5; j++){

                    local_sum += another_matrix->matrix[i][j];
                }
            }

            //Adding the local sum to the total sum.
            pthread_mutex_lock(&add_mutex);
            temp_total_sum = total_sum;
            total_sum += local_sum;

            //For printing the job...
            pthread_mutex_lock(&write_to_screen_mutex);

            printf("Add_%d\t\tAdd_%d has local sum: %d by [%d,%d] submatrix.\n", (add_thread_index + 1), (add_thread_index + 1), local_sum, another_matrix->global_row_index, another_matrix->global_column_index);
            printf("\t\tGlobal sum before/after update: %d/%d\n\n", temp_total_sum, total_sum);

            pthread_mutex_unlock(&write_to_screen_mutex);

            pthread_mutex_unlock(&add_mutex);

            another_add_needed = false;
        }
    }

    pthread_exit(NULL);
}

Matrix **createQueue(){

    Matrix **matrix;

    matrix = malloc(submatrix_needed * sizeof(Matrix *));

    return matrix;
}

void enqueue(Matrix **queue, int *queue_push_index, Matrix *matrix){

    queue[*queue_push_index] = matrix;

    *queue_push_index = *queue_push_index + 1;
}

Matrix *getFromQueue(Matrix **queue, int *queue_index){

    Matrix *to_dequeue = queue[*queue_index];
    *queue_index = *queue_index + 1;

    return to_dequeue;
}

void freeQueue(Matrix **queue, int queue_push_index){

    int i;

    for(i = 0; i < queue_push_index; i++)
        free(queue[i]);

    free(queue);
}
