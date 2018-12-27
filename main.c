#pragma GCC diagnostic ignored "-Wpointer-to-int-cast"
#pragma GCC diagnostic ignored "-Wint-to-pointer-cast"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
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

typedef struct{

    Matrix *front;
    int front_index;
    Matrix *rear;
    int rear_index;
    int size;
    Matrix **elements;

}Queue_Struct;

//Global variables.
int matrix_size;

int index_of_the_matrix_to_generate = 0;
int submatrix_needed;
int extra_matrix_needed;

Queue_Struct *first_queue = NULL;
Queue_Struct *second_queue = NULL;

//Mutex and Semaphores
pthread_mutex_t generate_mutex;

//Function declarations.
void enqueue(Queue_Struct *, Matrix *);
void freeQueue(Queue_Struct *);
void *Generate_Thread(void *);

bool queue_isFull(Queue_Struct *);
bool queue_isEmpty(Queue_Struct *);

Queue_Struct *createQueue();
Matrix *dequeue(Queue_Struct *);

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

    //Deciding how many submatrix we're gonna need.
    submatrix_needed = (int) pow(matrix_size / 5, 2); // NOLINT(bugprone-integer-division)

    /**
     * After deciding the number of submatrix needed, we need to check if the generate thread count is enough to create these matrices. If not, we need to generate the rest
     * by the same generate threads.
     */

    if(submatrix_needed > generate_thread_count)
        extra_matrix_needed = submatrix_needed - generate_thread_count;

    pthread_t generate_thread_ids[generate_thread_count];
    pthread_t log_thread_ids[log_thread_count];
    pthread_t mod_thread_ids[mod_thread_count];
    pthread_t add_thread_ids[add_thread_count];

    //Initializing mutex and semaphores.
    pthread_mutex_init(&generate_mutex, NULL);

    //Creating generate threads.
    for(loop_index = 0; loop_index < generate_thread_count; loop_index++){

        thread_control = pthread_create(&generate_thread_ids[loop_index], NULL, &Generate_Thread, (void *) loop_index);

        if(thread_control){
            fprintf(stderr, "Error: return code from creating thread is %d\n", thread_control);
            exit(-1);
        }
    }

    //Joining generate threads.
    for(loop_index = 0; loop_index < generate_thread_count; loop_index++){

        thread_control = pthread_join(generate_thread_ids[loop_index], NULL);

        if(thread_control){
            fprintf(stderr, "Error: return code from joining the thread is %d\n", thread_control);
            exit(-1);
        }
    }

    fprintf(stderr, "All threads are completed!!!\n");

    //Destroying mutex and semaphores.
    pthread_mutex_destroy(&generate_mutex);

    //Freeing the queues.
    freeQueue(first_queue);

    pthread_exit(NULL);
}

void *Generate_Thread(void *argument){

    int generate_thread_index = (int) argument;
    int index_of_generated_matrix;
    int i, j;

    //Creating the matrix.
    Matrix *matrix = malloc(sizeof(Matrix));

    //Deciding which matrix to generate.
    pthread_mutex_lock(&generate_mutex);
    index_of_generated_matrix = index_of_the_matrix_to_generate;
    index_of_the_matrix_to_generate++;
    pthread_mutex_unlock(&generate_mutex);

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

    enqueue(first_queue, matrix);
    pthread_mutex_unlock(&generate_mutex);

    //For printing the job...
    pthread_mutex_lock(&generate_mutex);

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

    pthread_mutex_unlock(&generate_mutex);

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
            enqueue(first_queue, extra_matrix);
            pthread_mutex_unlock(&generate_mutex);

            //And again, print the results.
            pthread_mutex_lock(&generate_mutex);

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

            pthread_mutex_unlock(&generate_mutex);

            generate_another_matrix = false;
        }
    }

    pthread_exit(NULL);
}

Queue_Struct *createQueue(){

    Queue_Struct *queue;

    queue = malloc(sizeof(Queue_Struct));
    queue->front = NULL;
    queue->rear = NULL;
    queue->size = 0;
    queue->front_index = 0;
    queue->rear_index = 0;
    queue->elements = malloc(submatrix_needed * sizeof(Matrix *));

    return queue;
}

bool queue_isFull(Queue_Struct *queue){

    return (submatrix_needed == queue->size);
}

bool queue_isEmpty(Queue_Struct *queue){

    return (0 == queue->size);
}

void enqueue(Queue_Struct *queue, Matrix *matrix){

    if(queue_isFull(queue)){
        fprintf(stderr, "Something went wrong, the queue shouldn't be full!!!\n");
        exit(-1);
    }

    if(queue->size == 0){
        queue->front_index = 0;
        queue->rear_index = 0;
    }else
        queue->rear_index++;

    queue->elements[queue->rear_index] = matrix;
    queue->rear = matrix;

    if(queue->front_index == 0) queue->front = matrix;

    queue->size++;
}

Matrix *dequeue(Queue_Struct *queue){

    if(queue_isEmpty(queue)){
        fprintf(stderr, "Something went wrong, the queue shouldn't be empty!!!\n");
        exit(-1);
    }

    Matrix *to_dequeue = queue->front;
    queue->front_index++;
    queue->front = queue->elements[queue->front_index];

    queue->size--;

    return to_dequeue;
}

void freeQueue(Queue_Struct *queue){

    int i = queue->size;

    for( ; i > 0; i--){

        free(queue->elements[i - 1]);
    }

    free(queue->elements);
    free(queue);
}
