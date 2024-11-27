// Vishnu Patel
// cs 099
// pate0458

// imports
#define _REENTRANT
#include <pthread.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <semaphore.h>
#include <sys/sem.h>
#include <unistd.h>
#include <stdlib.h>


/* semaphore keys */
#define SEMKEY_EMPTY ((key_t) 400L)
#define SEMKEY_FULL ((key_t) 401L)
#define SEMKEY_MUTEX ((key_t) 402L)
#define SHMKEY ((key_t) 1497)
/* number of semaphores being created */
#define NSEMS 1
/* GLOBAL */
int empty_id;/* semaphore id */
int full_id;
int mutex_id;
/* semaphore buffers */
static struct sembuf OP = {0,-1,0};
static struct sembuf OV = {0,1,0};
struct sembuf *P =&OP;
struct sembuf *V =&OV;
/* semapore union used to generate semaphore */
typedef union{
    int val;
    struct semid_ds *buf;
    ushort *array;
} semunion;
// how the shared memory should look
typedef struct {
    char buffer[15];
    int producter_index;
    int consumer_index;
    int flag;
} shared_mem;
/* POP (wait()) function for semaphore to protect critical section */
// take in sem id to know which semaphore to use
int POP(semaphore_id){
    int status;
    status = semop(semaphore_id, P,1);
    return status;
}
/* VOP (signal()) function for semaphore to release protection */
int VOP(semaphore_id){
    int status;
    status = semop(semaphore_id, V,1);
    return status;
}
// create shared memory variable
shared_mem *total;
// function for thread 1
void* thread1(){
    // char to hold input in file
    char newChar;
    // file to read in
    FILE* fp;
    // open file
    fp= fopen("mytest.dat", "r");
    // while input in file read in next char
    while(fscanf(fp,"%c",&newChar) != EOF){
       // take empty semaphore
        POP(empty_id);
        // take mutex semaphore
        POP(mutex_id);
   
        // add char to buffer
        total->buffer[total->producter_index] = newChar;
        // increment producer index
        // if index more than 15 change circle back to 0
        total->producter_index = total->producter_index +1;
        if(total->producter_index > 14){
            total->producter_index = 0;
        }
        
        // give back mutex semaphore
        VOP(mutex_id);
        // give back full semaphore
        VOP(full_id);
    }
    // change flag one end of file reached
    total->flag = 1;
    // close file
    fclose(fp);
}
// function for thread 2
void* thread2(){
    // sleep 1 to make sure producer is entered first
    sleep(1);
    // while we dont break
    while(1){
        // break if no more input and flag is raised
        if(total->producter_index == total->consumer_index && total->flag == 1){
            break;
        }
        // take full semaphore
        POP(full_id);
        // take mutex semaphore
        POP(mutex_id);
            // remove letter from buffer
            char letter = total->buffer[total->consumer_index];
            // increment consumer index
            total->consumer_index = total->consumer_index+1;
            // if consumer index more than 15 circle back
            if(total->consumer_index > 14){
                total->consumer_index = 0;
            }
            // print next char in buffer
            printf("%c", letter);
            // flush char
            fflush(stdout);
        // return mutex semaphore
        VOP(mutex_id);
        // return empty semaphore
        VOP(empty_id);
        // sleep to allow producter semaphore to read input
        sleep(1);   
    }
}
int main(){
    // intialize values for empty semaphore
    int empty_value, empty_value1;
    // create empty semaphore struct
    semunion empty;
    // initialize empty semaphore val
    empty.val = 1;
    /* Create semaphores */
    empty_id = semget(SEMKEY_EMPTY, NSEMS, IPC_CREAT | 0666);
    if(empty_id < 0) printf("Error in creating the semaphore./n");

    /* Initialize empty semaphore */
    empty_value1 =semctl(empty_id, 0, SETVAL, empty);
    empty_value =semctl(empty_id, 0, GETVAL, empty);

    // Check if setVal and getVal worked 
    if (empty_value1 < 0) printf("Error detected in SETVAL.\n");
    if(empty_value < 0) printf("Error detected in getVal) \n");
    // initialize values for mutex semaphore
    int mutex_value, mutex_value1;
    // create mutex semaphore struct
    semunion mutex;
    // initialize mutex semaphore val to 1
    mutex.val = 1;
    /* Create semaphores */
    mutex_id = semget(SEMKEY_MUTEX, NSEMS, IPC_CREAT | 0666);
    if(mutex_id < 0) printf("Error in creating the semaphore./n");

    /* Initialize empty semaphore */
    empty_value1 =semctl(mutex_id, 0, SETVAL, mutex);
    empty_value =semctl(mutex_id, 0, GETVAL, mutex);

    // Check if setVal and getVal worked 
    if (mutex_value1 < 0) printf("Error detected in SETVAL.\n");
    if(mutex_value < 0) printf("Error detected in getVal) \n");

    // initialize values for full semaphore
    int full_value, full_value1;
    // create full semaphore struct
    semunion full;
    // initialize full semaphore value to 1
    full.val = 1;
    /* Create semaphores */
    full_id = semget(SEMKEY_FULL, NSEMS, IPC_CREAT | 0666);
    if(full_id < 0) printf("Error in creating the semaphore./n");

    /* Initialize full semaphore */
    full_value1 =semctl(full_id, 0, SETVAL, full);
    full_value =semctl(full_id, 0, GETVAL, full);

    // Check if setVal and getVal worked 
    if (full_value1 < 0) printf("Error detected in SETVAL.\n");
    if(full_value < 0) printf("Error detected in getVal) \n");


    // initialize variables for shared memory
    int shmid;
    char *shmadd;
    shmadd = (char*)0;
    //allocates shared memory and exits if fails 
    if(( shmid = shmget(SHMKEY, sizeof(int), IPC_CREAT | 0666)) < 0){
            // allocation failed
            exit(1);
    }
    // attaches shared memory to total variable and exits if fails
    if ((total = (shared_mem *) shmat(shmid,shmadd,0)) == (shared_mem *) -1){
            exit(0);
    }
    // initialize total producer index to 1
    total->producter_index = 0;
    // initialize total consumer index to 1
    total->consumer_index = 0;
    // initilize total flag to 0
    total->flag = 0;
    // initialize all entries in buffer array
    for (int i =0; i < 15; i++){
        total->buffer[i] = 'c';
    }
    pthread_t tid1[1]; /* process id for thread 1 */
    pthread_t tid2[1]; /* process id for thread 2 */
    pthread_attr_t attr[1]; /* attribute pointer array */
    fflush(stdout);
    /* Required to schedule thread independently.*/
    pthread_attr_init(&attr[0]);
    pthread_attr_setscope(&attr[0], PTHREAD_SCOPE_SYSTEM);
    /* end to schedule thread independently */
    /* Create the threads */
    pthread_create(&tid1[0], &attr[0], thread1, NULL);
    pthread_create(&tid2[0], &attr[0], thread2, NULL);
    /* Wait for the threads to finish */
    pthread_join(tid1[0], NULL);
    pthread_join(tid2[0], NULL);
    pthread_exit(NULL);
    if(shmdt(total) ==-1){
            exit(-1);
    }
     // deallocate the shared memory space
    shmctl(shmid, IPC_RMID, NULL);
    // deallocate empty semaophores
    int empty_status =semctl(empty_id, 0, IPC_RMID, empty);
    if(empty_status < 0) printf("Error in removing the semaphore.\n");
    // deallocate full semaphore
    int full_status =semctl(full_id, 0, IPC_RMID, full);
    if(full_status < 0) printf("Error in removing the semaphore.\n");
    // deallocate mutex semaphore
    int mutex_status =semctl(mutex_id, 0, IPC_RMID, mutex);
    if(mutex_status < 0) printf("Error in removing the semaphore.\n");
}