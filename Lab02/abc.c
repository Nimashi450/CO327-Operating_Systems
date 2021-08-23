#include <pthread.h> // posix threads 
#include <stdio.h>
#include <stdlib.h> //rand
#include <assert.h>


#include <pthread.h>    /* required for pthreads */
#include <semaphore.h>  /* required for semaphores */

/* to compile use -lpthread */
void * producer(void *); 
void * consumer(void *); 

#define HOWMANY_PRODUCERS     80
#define HOWMANY_CONSUMERS     10
// SIZE * HOWMANY_PRODUCERS should be divisible by HOMANY_CONSUMERS
pthread_mutex_t lock;
pthread_cond_t cond;


/* functions */ 
//int insert_item(int); // add the value to buffer 
//int remove_item(void); // remove the item from the buffer
typedef struct _Node{
    struct _Node* next;
    int data;
}node_t,*node_p,**node_pp;


node_p alloc_node(int x){ 
    node_p p=(node_p)malloc(sizeof(node_t));
    if(!p){
        perror("malloc");
        exit(1);
    }   
 
   p->data=x;
   p->next=NULL;
   return p;
}
/**************************************************************
 *                       Stuff for the buffer                 *
 **************************************************************/
/*#define SIZE 100 
int buffer[SIZE]; 
int freeSlot = 0; */

// initialize the mutex (static initialization) 
pthread_mutex_t mutex_for_buffer = PTHREAD_MUTEX_INITIALIZER; 
/*************************************************************/

/*************************************************************
*                     Debug stuff                            *
**************************************************************/
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
void insert_item(node_p h,int x) 
{ 
  /*assert(!pthread_mutex_lock(&mutex_for_buffer)); 
  {
    assert(freeSlot < (SIZE - 1));
    buffer[freeSlot] = val; 
    freeSlot ++; 
  }
  assert(!pthread_mutex_unlock(&mutex_for_buffer)); 
  return 0; // success*/
   node_p p=alloc_node(x);
    p->next=h->next;
    h->next=p;
}

/** 
 * remove an item from the buffer 
 * Do not call if there are no items to remove 
 */
 int  is_empty(node_p head)
  {     
      return (head->next==NULL)?1:0;
  }
void remove_item(node_p head,int* x) 
{ /*
  int val; 
  assert(!pthread_mutex_lock(&mutex_for_buffer)); 
  {
    assert(freeSlot > 0); // there should be something to reomve
    freeSlot --; 
    val = buffer[freeSlot]; 
  }
  assert(!pthread_mutex_unlock(&mutex_for_buffer)); 
  return val; // removed value  */
  if(!is_empty(head)){
        node_p p=head->next;
        *x=p->data;
        head->next=p->next;
        free(p);
    }   
  
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
    
    node_p head=(node_p)p;
    val = rand() % 20; // produce value 
    pthread_mutex_lock(&lock);
    sem_wait(&sFree); 
    insert_item(head,val);  // insert to buffer 
    sem_post(&sFull);
    pthread_mutex_unlock(&lock); 
    usleep(rand() % 10); 
  }
  PRINT("%s: Done\n", __func__); 
}

/** 
 * remove a value from the buffer 
 * using the remove_item function 
 * consume the item, sleep for random time 
 * repeat 
 */

void * consumer(void *p) 
{ 
  int i = 0,val; 
  node_p head=(node_p)p;
  while(i++ < (SIZE * HOWMANY_PRODUCERS/HOWMANY_CONSUMERS)) { 
    pthread_mutex_lock(&lock);
    sem_wait(&sFull); 
    remove_item(head,&val); // remove from buffer 
    sem_post(&sFree); 
    pthread_mutex_unlock(&lock); 
    usleep(rand() % 10); 
  }
  PRINT("%s: Done\n", __func__); 
}


int main() 
{
  pthread_t prods[HOWMANY_PRODUCERS]; 
  pthread_t cons[HOWMANY_CONSUMERS]; 
  node_p head;

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
