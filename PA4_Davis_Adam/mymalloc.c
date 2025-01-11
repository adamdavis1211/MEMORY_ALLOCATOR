#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define MBLOCK_HEADER_SZ offsetof(mblock_t, payload)
#define BLOCK_SIZE sizeof(mblock_t)

typedef struct _mblock_t {  // data block within the memory list.
   struct _mblock_t * prev;
   struct _mblock_t * next;
   size_t size;
   int status;
   void * payload;
} mblock_t;

typedef struct _mlist_t {   // data structure containing memory list.
   mblock_t * head;
} mlist_t;

void * mymalloc(size_t size);   // size: requested allocation size.
                               // return value: pointer to virtual address location which was allocated and can be safely used. null if allocation failed.

void myfree(void * ptr);    // ptr: pointer to virtual address location which was previously allocated and should now be de-allocated.
mblock_t * FindLastMemoryListBlock(); 
mblock_t * FindFreeBlockOfSize(size_t size);    // "first-fist" strategy
void SplitBlockAtSize(mblock_t * block, size_t NewSize);
void CoallesceWithPreviousBlock(mblock_t * FreedBlock);
void CoallesceWithNextBlock(mblock_t * FreedBlock);
mblock_t * GrowHeapBySize(size_t size);
void printMemList(const mblock_t* head);

mlist_t list;

int main() {
 list.head = NULL;
 GrowHeapBySize(2);
 GrowHeapBySize(8);
 printMemList(list.head);
}


void * mymalloc(size_t size) {
   mblock_t * FindFreeBlockOfSize(size);
}


mblock_t * FindFreeBlockOfSize(size_t size) {
   mblock_t * TraversalBlock = list.head;
   while (1) {
     if (TraversalBlock->size >= size) {
         return TraversalBlock;
     }
     else if (TraversalBlock->next == NULL) {
         return GrowHeapBySize(size);
     }
     else {
        TraversalBlock = TraversalBlock->next;
     }
   }
}


mblock_t * GrowHeapBySize(size_t size) {
 if (list.head == NULL) {
   mblock_t * NewLastBlock = (mblock_t *)sbrk(MBLOCK_HEADER_SZ + size);
   if (NewLastBlock == (void *)-1) {
       exit(1);
   }
   NewLastBlock->prev = NULL;
   NewLastBlock->next = NULL;
   NewLastBlock->size = size;
   NewLastBlock->status = 0;
   NewLastBlock->payload = (void *)((char *)NewLastBlock + MBLOCK_HEADER_SZ);
   list.head = NewLastBlock;
   return NewLastBlock;
 }
 else {
   mblock_t * LastBlock = FindLastMemoryListBlock();
   mblock_t * NewLastBlock = (mblock_t *)sbrk(MBLOCK_HEADER_SZ + size);
   if (NewLastBlock == (void *)-1) {
       exit(1);
   }
   NewLastBlock->next = NULL;
   NewLastBlock->prev = LastBlock;
   NewLastBlock->size = size;
   NewLastBlock->status = 0;
   NewLastBlock->payload = (void *)((char *)NewLastBlock + MBLOCK_HEADER_SZ);
   LastBlock->next = NewLastBlock;
 }
}


mblock_t * FindLastMemoryListBlock() {  // working so far.
 mblock_t * TraversalBlock = list.head;
 while (TraversalBlock->next != NULL) {
   TraversalBlock = TraversalBlock->next;
 }
 return TraversalBlock;
}


void printMemList(const mblock_t* head) {
 const mblock_t* p = head;
 size_t i = 0;
 while(p != NULL) {
   printf("[%ld] p: %p\n", i, (void*)p);
   printf("[%ld] p->size: %ld\n", i, p->size);
   printf("[%ld] p->status: %s\n", i, p->status > 0 ? "allocated" : "free");
   printf("[%ld] p->prev: %p\n", i, (void*)p->prev);
   printf("[%ld] p->next: %p\n", i, (void*)p->next);
   printf("___________________________\n");
   ++i;
   p = p->next;
 }
 printf("===========================\n");
}
