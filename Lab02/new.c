#include <pthread.h> // posix threads 
#include <stdio.h>
#include <stdlib.h> //rand
#include <assert.h>
#include <semaphore.h>  /* required for semaphores */

/* to compile use -lpthread */

#define HOWMANY_PRODUCERS     80
#define HOWMANY_CONSUMERS     10
// SIZE * HOWMANY_PRODUCERS should be divisible by HOMANY_CONSUMERS


/* functions */ 
void insert_item(int); // add the value to buffer 
int remove_item(void); // remove the item from the buffer

// /**********************
//  *                       Stuff for the buffer                 *
//  **********************/
#define SIZE 100  
int freeSlot = 0; 

// initialize the mutex (static initialization) 
pthread_mutex_t mutex_for_buffer = PTHREAD_MUTEX_INITIALIZER; 
/*********************/

/*********************
*                     Debug stuff                            *
**********************/
//#define DEBUG   1
#ifdef DEBUG
#define PRINT printf 
#else 
#define PRINT(...) 
#endif 

sem_t sFree, sFull; 


/**
 * add the given item to the buffer 
 * do not call if the buffer is full 
 */ 


typedef struct node{
  int value;
  struct node * next;
} Node;

Node * head = NULL;


void insert_item(int value){

     assert(!pthread_mutex_lock(&mutex_for_buffer)); 
     {
        Node * temp = (Node*)malloc(sizeof(struct node));
        temp->value = value;
        temp->next = head;
        freeSlot++;
        head = temp;
     }
     assert(!pthread_mutex_unlock(&mutex_for_buffer)); 
    
}

int remove_item(void){
    Node * temp;
    temp = head;
    assert(!pthread_mutex_lock(&mutex_for_buffer)); 
    assert(temp != NULL);
    {
        head = head->next;
        assert(freeSlot > 0);
        freeSlot--;
    } 
    assert(!pthread_mutex_unlock(&mutex_for_buffer));
    return temp->value;
}


/** 
 * produce some value 
 * insert to the buffer using the insert_item function 
 * sleep for a random time 
 * repeat SIZE times
 */
void * producer(void * p) 
{ 
  int i = 0, val; 
  while(i++ < SIZE) { 
    val = rand() % 20; // produce value 
    sem_wait(&sFree); 
    insert_item(val);  // insert to buffer 
    sem_post(&sFull); 
    usleep(rand() % 10); 
  }
  PRINT("%s: Done\n", _func_); 
}

/** 
 * remove a value from the buffer 
 * using the remove_item function 
 * consume the item, sleep for random time 
 * repeat 
 */

void * consumer(void *p) 
{ 
  int i = 0; 
  while(i++ < (SIZE * HOWMANY_PRODUCERS/HOWMANY_CONSUMERS)) { 
    sem_wait(&sFull); 
    remove_item(); // remove from buffer 
    sem_post(&sFree);  
    usleep(rand() % 10); 
  }
  PRINT("%s: Done\n", _func_); 
}


int main() 
{
  pthread_t prods[HOWMANY_PRODUCERS]; 
  pthread_t cons[HOWMANY_CONSUMERS]; 

  pthread_attr_t attr; 
 
  pthread_attr_init(&attr); // set default attributes 

  assert(!sem_init(&sFree, 0/*shared between threads*/, SIZE-1)); 
  assert(!sem_init(&sFull, 0/*shared between threads*/, 0)); 
	 
  int i; 
  for(i=0; i<HOWMANY_PRODUCERS; i++) 
    pthread_create(&prods[i], &attr, producer, NULL); // create producers thread

  for(i=0; i<HOWMANY_CONSUMERS; i++) 
    pthread_create(&cons[i], &attr, consumer, NULL); // create new thread

  void *dummy; 
  for(i=0; i<HOWMANY_PRODUCERS; i++) 
    pthread_join(prods[i], &dummy); 

  for(i=0; i<HOWMANY_CONSUMERS; i++) 
    pthread_join(cons[i], &dummy); 

  printf("%s: all threads exited (solution %s)\n",
   __func__, 
   freeSlot == 0 ? "worked!" : "failed"); 

  pthread_mutex_destroy(&mutex_for_buffer); 
  sem_destroy(&sFree); 
  sem_destroy(&sFull);   
}