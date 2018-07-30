#include <stdio.h>  // needed for size_t
#include <unistd.h> // needed for sbrk
#include <assert.h> // needed for asserts
#include "dmm.h"

/* You can improve the below metadata structure using the concepts from Bryant
 * and OHallaron book (chapter 9).
 */

typedef struct metadata {
  /* size_t is the return type of the sizeof operator. Since the size of an
   * object depends on the architecture and its implementation, size_t is used
   * to represent the maximum size of any object in the particular
   * implementation. size contains the size of the data object or the number of
   * free bytes
   */
  size_t size;
  bool freed; 
  struct metadata* next;
  struct metadata* prev; 
} metadata_t;

/* freelist maintains all the blocks which are not in use; freelist is kept
 * sorted to improve coalescing efficiency 
 */

static metadata_t* freelist = NULL;
metadata_t * head = NULL; 
metadata_t * endlist = NULL;
int s = 440; 
char *end;

metadata_t* split (metadata_t * curr, int numbytes){
  //0x1c84e90 -> 0x1c85070
  size_t size = sizeof(metadata_t) + numbytes;
  if((curr->size - size) <= sizeof(metadata_t)){
    return curr; 
  }
  metadata_t* header1 = curr; //making a header1 to start of block to be returned
  size_t oldsize = curr->size; 
  metadata_t* next = curr->next; 
  curr = curr +1; //moved past the header; 
  char *ptr = (char*) curr; 

  ptr = ptr + numbytes; 
  if(s == numbytes){
    //printf("ptr: %p\n", ptr);
  }

  curr = (metadata_t*)ptr; //restore struct pointer from ptr
  metadata_t* header2 = curr; //start header for leftover block
  //initialize new free block 
  header2->size = oldsize - (numbytes + sizeof(metadata_t)); 
  //header2->size = next - header2;
  header1->size = oldsize - header2->size; 
  header1->next = header2; 
  header2->prev = header1; 

  if(next !=NULL){
    next->prev = header2; 
  }
  header2->next = next; 
  header1->freed = false; 
  header2->freed = true; 
  return header1; 

}

void* dmalloc(size_t numbytes) {
  /* initialize through sbrk call first time */
  //printf("numbytes: %zu\n", numbytes);
  //fflush(stdout);
  
  if(freelist == NULL) { 			
    if(!dmalloc_init())
      return NULL;
  }
/* your code here */
  assert(numbytes > 0);
  numbytes = ALIGN(numbytes);
  //printf("numbytes: %zu\n", numbytes);
  metadata_t* curr = freelist; 
  //printf("Curr: %p", curr);
  //fflush(stdout);
  while(curr !=NULL){

    if(curr->size == (numbytes + sizeof(metadata_t)) && curr->freed){
      //printf("is equal\n");
      curr->freed = false;
      return (void*)(curr +1); //points to byte right after header
    }
    if((sizeof(metadata_t) + numbytes) < curr->size  && curr->freed){
      //printf("size: %zu\n", sizeof(metadata_t) + numbytes);
      //printf("Comparing less than size: \n");
      curr = split(curr, numbytes); 
      curr->freed = false;
      return (void*)(curr +1);//points to byte right after header
    }
  curr = curr->next; 
  }
  return NULL;
  }
  /*
 metadata_t* tail = curr; 
 metadata_t* p = sbrk(sizeof(metadata_t) + numbytes); //size of header + size of payload 
 p->size = sizeof(metadata_t) + numbytes; 
 p->freed = false; 
 p->prev = tail; 
 p->next = NULL; 
 //ALIGN? 
 //don't have to coalesce i guess
 return (void *)(p +1);
 */
void coalesce(metadata_t * curr){
  int totalsize; 
  //if at the front of the list
  //printf("freelist: %p\n", freelist);
  //printf("curr: %p\n", curr);
  if(curr == head){
    if(curr->next->freed){
      totalsize = curr->size + curr->next->size; 
      curr->next = curr->next->next; 
      if(curr->next != NULL){
        curr->next->prev = curr; 
      }
      curr->size = totalsize; 
    }
    return; 
  }
  if(curr == endlist){
    if(curr->prev->freed){
      totalsize = curr->size + curr->prev->size; 
      curr = curr->prev; 
      curr->next = NULL; 
      curr->size = totalsize; 
    }
    return;
  }

  if(curr->prev != NULL){
      if(curr->prev->freed){
        totalsize = curr->size + curr->prev->size; 
        curr = curr->prev; 
        curr->next = curr->next->next; 
        if(curr->next != NULL){
          curr->next->prev = curr; 
        }
      curr->size = totalsize; 
      }
  }

  if(curr->next != NULL){
    if(curr->next->freed){
      totalsize = curr->size + curr->next->size;
      curr->next = curr->next->next; 
      if(curr->next != NULL){
        curr->next->prev = curr; 
      }
      curr->size = totalsize; 
    }
  }
  return;
}


void dfree(void* ptr) {
  /* your code here */
  if(ptr == NULL){
    return;
  }
  head = freelist; 
  //printf("ptr: %p\n: ", ptr);
  ptr = ptr - sizeof(metadata_t);
  //printf("ptr: %p\n", ptr);
  metadata_t * curr = freelist; 
  //printf("freelist: %p\n", freelist);
  while(curr != NULL){
    if(ptr == curr && !curr->freed){
      //printf("ptr = curr: %p\n: ", ptr);
      //printf("curr: %p\n", curr);
      curr->freed = true;
      //printf("ptr->freed: %d\n: ", curr->freed); 
      //printf("about to be free\n");
      //printf("freelist: %p\n", freelist);
      //printf("curr: %p\n", curr);
      coalesce(curr);
      return;
    }
    curr = curr->next; 
  }
  return;
}

bool dmalloc_init() {

  /* Two choices: 
   * 1. Append prologue and epilogue blocks to the start and the
   * end of the freelist 
   *
   * 2. Initialize freelist pointers to NULL
   *
   * Note: We provide the code for 2. Using 1 will help you to tackle the 
   * corner cases succinctly.
   */

  //printf("Max heap size: %d\n", MAX_HEAP_SIZE);
  size_t max_bytes = ALIGN(MAX_HEAP_SIZE);
  //printf("max bytes: %zu\n", max_bytes);
  /* returns heap_region, which is initialized to freelist */
  freelist = (metadata_t*) sbrk(max_bytes); //16449536
  end = (char*) freelist; 
  end = end + max_bytes; //16450560, 16450560-16449536 = 1024
  endlist = (metadata_t*)end;
  //printf("end ptr:%p\n", end);
  //printf("freelist ptr: %p\n",freelist);
  //fflush(stdout);
  /* Q: Why casting is used? i.e., why (void*)-1? */
  if (freelist == (void *)-1)
    return false;
  freelist->next = NULL;
  freelist->prev = NULL;
  freelist->freed = true; 
  freelist->size = max_bytes-METADATA_T_ALIGNED;
  //printf("freelist size: %zu\n",freelist->size  );
  return true;
}

/* for debugging; can be turned off through -NDEBUG flag*/
void print_freelist() {
  metadata_t *freelist_head = freelist;
  while(freelist_head != NULL) {
    DEBUG("\tFreelist Size: %zd, Head: %p, Prev: %p, Next: %p, Freed: %d\t",
	  freelist_head->size,
	  freelist_head,
	  freelist_head->prev,
	  freelist_head->next,
    freelist_head->freed);
    freelist_head = freelist_head->next;
  }
  DEBUG("\n");
}

