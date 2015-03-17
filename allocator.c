//Min Jee Son z3330687
//Milestone 3
//17/04/14

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "allocator.h"

typedef unsigned char byte;
static byte *memory = NULL;

typedef struct header Header;

struct header{
  int magic;
  int size;
  Header *next;
  Header *prev;
};

#define HEADER_SIZE sizeof(Header)

static Header *free_list_ptr;

void merge(Header *header);

void allocator_init(u_int32_t size){
  int powerOfTwo;
  
  powerOfTwo = pow(2, ceil(log2(size)));

  memory = (byte*)malloc(powerOfTwo);
  if(memory==NULL){
    fprintf(stderr, "Error - The requested memory could not be allocated\n");
    abort();
  }
  free_list_ptr = (Header*)memory;
  free_list_ptr->magic=0xDEADBEEF;
  free_list_ptr->size=powerOfTwo;
  free_list_ptr->next = free_list_ptr;
  free_list_ptr->prev = free_list_ptr;
}

void allocator_end(void){
  free(memory);
  memory = NULL;
}

void *allocator_malloc(u_int32_t n){
  Header *pointer = free_list_ptr;
  Header *smallestSize = NULL; // points to the Header with the smallest size that fits n+HEADER_SIZE
  Header *new = NULL;
  Header *curr = NULL;
  Header *temp = NULL;
  
  int s;
  void *chosen_ptr = NULL; // the pointer to the allocated region
  
  // smallestSize remains NULL if the first free_list_ptr is not big enough
  if(pointer->size >= HEADER_SIZE + n){
    smallestSize = pointer;
  }
  
  pointer = pointer->next;
  
  // check all sizes until the pointer returns to the first free_list_ptr encountered
  while(pointer!=free_list_ptr){
    //Check that the memory is not corrupted
    if(pointer->magic != 0xDEADBEEF){
      fprintf(stderr, "Error - Memory has been corrupted. \n");
      abort();
    }
    
    //Check for the header with the smallest size that can fit n + HEADER_SIZE
    if(pointer->size >= HEADER_SIZE + n){
      
      if(smallestSize == NULL)
        smallestSize = pointer;
      
      else if(pointer->size < smallestSize->size)
        smallestSize = pointer;
    }
    pointer = pointer->next;
    
  }
  
  // return NULL if there is no free_list_ptr size big enough for n + HEADER_SIZE
  if(smallestSize == NULL){
    printf("There is no free space big enough for the required bytes.\nNo memory was malloc'd.\n");
    return NULL;
  }
  
  s = smallestSize->size;
  chosen_ptr = smallestSize;
  curr = smallestSize;
  
  while(s/2 > HEADER_SIZE + n){
    
    new = (void*)(chosen_ptr + s/2);
    curr->size = s/2;
    new->magic = 0xDEADBEEF;
    new->size = s/2;
    new->next = curr->next;
    curr->next = new;
    new->prev = curr;
    temp = new->next;
    temp->prev = new;
    
    s = s/2;
  }
  
  //if selected region is the only free region, return NULL
  if(smallestSize->next == smallestSize){
    printf("There's only one free region. No memory was malloc'd");
    return NULL;
  }
  
  //remove the selected region from the free region
  new = curr->prev;
  new->next = curr->next;
  curr->magic = 0xDEAFBEAD;
  curr = new->next;
  curr->prev = new;
  
  if((void*)(free_list_ptr) == chosen_ptr){
    free_list_ptr = curr;
  }
  return((void*)(chosen_ptr + HEADER_SIZE));
  
}

void allocator_free(void *object){
  Header *freeHeader = NULL;
  Header *temp = NULL;
  Header *pointer = free_list_ptr;
  
  
  if(object == NULL){
    fprintf(stderr, "Error - The requested pointer to be freed is a nullpointer. No memory was freed.\n");
    abort();
  }
  
  freeHeader = (void*)(object - HEADER_SIZE);
  //return if the memory block is already free
  if(freeHeader->magic == 0xDEADBEEF){
    return;
  }
  if(freeHeader->magic != 0xDEAFBEAD){
    fprintf(stderr, "Error - The memory has been corrupted.\n");
    abort();
  }
  
  freeHeader->magic = 0xDEADBEEF;
  
  if(freeHeader < pointer){
    freeHeader->next = pointer;
    temp = pointer->prev;
    temp->next = freeHeader;
    freeHeader->prev = temp;
    pointer->prev = freeHeader;
    free_list_ptr = freeHeader;
    merge(freeHeader);
    return;
  }
  pointer = pointer->next;
  
  while((pointer!=free_list_ptr)&&(freeHeader > pointer)){
    pointer = pointer->next;
  }
  freeHeader->next = pointer;
  temp = pointer->prev;
  temp->next = freeHeader;
  freeHeader->prev = temp;
  pointer->prev = freeHeader;

  merge(freeHeader);
  
}

void merge(Header *freedHeader){

  long int offset =(byte*)freedHeader - memory;
  Header *prevH = NULL;
  Header *nextH = NULL;
  
  /* If the offset of the freedheader is not a multiple of the freedheader's size*2, then the memory has been split at that offset. So if available, merge with the previous free header.
  */
    
  if((byte*)(freedHeader->prev) == (byte*)(freedHeader) - freedHeader->size && freedHeader->size == freedHeader->prev->size){
    prevH = freedHeader->prev;
    if(offset%((freedHeader->size)*2) !=0){
      prevH->size *= 2;
      prevH->next = freedHeader->next;
      prevH->next->prev = prevH;
      freedHeader->prev = NULL;
      freedHeader->next = NULL;
      return;
    }
  }
  
/* If the offset of the freedheader is a multiple of the freedheader's size*2, then the memory has not been previously split at the offset. So if available, merge with the next free header.
 */
  if((byte*)(freedHeader->next) == (byte*)(freedHeader) + freedHeader->size && freedHeader->size == freedHeader->next->size){
    nextH = freedHeader->next;
    if(nextH!=NULL && offset%((freedHeader->size)*2) == 0){
      freedHeader->size *= 2;
      freedHeader->next = nextH->next;
      freedHeader->next->prev = freedHeader;
      nextH->prev = NULL;
      nextH->next = NULL;
      return;
    }
  }
}


