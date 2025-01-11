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
int greater(int a, int b);
mlist_t list;


void printMemList(const mblock_t* head);


int main(int argc, char argv[]) {
  list.head = NULL;
  void * p1 = mymalloc(10);
  void * p2 = mymalloc(100);
  void * p3 = mymalloc(200);
  void * p4 = mymalloc(500);
  printMemList(list.head);
  myfree(p3); p3 = NULL;
  myfree(p2); p2 = NULL;
  void * p5 = mymalloc(150);
  void * p6 = mymalloc(500);
  myfree(p4); p4 = NULL;
  myfree(p5); p5 = NULL;
  myfree(p6); p6 = NULL;
  myfree(p1); p1 = NULL;

  printMemList(list.head);
} 

void myfree(void * ptr) {
  mblock_t * Block = (mblock_t *)(ptr - MBLOCK_HEADER_SZ);
  int prev = 0;
  if (Block->prev != NULL) {
    CoallesceWithPreviousBlock(Block);
    prev = 1;
  }
  if (Block->next != NULL && prev == 0) {
    CoallesceWithNextBlock(Block);
  }
  else if (Block->next != NULL && prev == 1) {
    CoallesceWithNextBlock(Block->prev);
  }
}

void CoallesceWithPreviousBlock(mblock_t * FreedBlock){
  if (FreedBlock->prev->status == 0) {
    FreedBlock->prev->next = FreedBlock->next;
    FreedBlock->next->prev = FreedBlock->prev;
    FreedBlock->prev->size = (MBLOCK_HEADER_SZ + FreedBlock->size + FreedBlock->prev->size);
    FreedBlock->prev->status = 0;
  }
  else {
    return;
  }
}

void CoallesceWithNextBlock(mblock_t * FreedBlock) {
  if (FreedBlock->next->status == 0) {
    FreedBlock->size = FreedBlock->size + MBLOCK_HEADER_SZ + FreedBlock->next->size;
    FreedBlock->next = FreedBlock->next->next;
    if (FreedBlock->next != NULL) {
      FreedBlock->next->prev = FreedBlock;
    }
    FreedBlock->status = 0;
  }
  else {
    return;
  }
}

void * mymalloc(size_t size) {
  mblock_t * Block = FindFreeBlockOfSize(size);
  return &(Block->payload);                            // return void * payload of block
}


mblock_t * FindFreeBlockOfSize(size_t size) {  
  mblock_t * Block = list.head;
  while (1) {
    if (Block == NULL) {                  // if list is empty allocate 1KB block 
      mblock_t * NewBlock = GrowHeapBySize(greater(1000, size + MBLOCK_HEADER_SZ));
      size_t OldBlockSize = NewBlock->size;
      NewBlock->size = size;
      NewBlock->status = 1;
      SplitBlockAtSize(NewBlock, OldBlockSize - MBLOCK_HEADER_SZ - size);
      return NewBlock;
    }
    else if (Block->size == (size + MBLOCK_HEADER_SZ)) {       // block with exactly enough required size found
      size_t OldBlockSize = Block->size;
      mblock_t * NewBlock = (mblock_t*)(&(Block->payload));
      NewBlock->next = Block->next;
      Block->next = NewBlock;
      NewBlock->prev = Block;
      Block->size = 0;
      Block->status = 0;
      NewBlock->size = size;
      NewBlock->status = 1;
      SplitBlockAtSize(NewBlock, (OldBlockSize - MBLOCK_HEADER_SZ - size));
      return NewBlock;
    }
    else if (Block->size > (size + (2 * MBLOCK_HEADER_SZ) + 1)) {       // block has enough space for block of requested size and additional block with 1 byte payload.
      size_t OldBlockSize = Block->size;
      mblock_t * NewBlock = (mblock_t*)(&(Block->payload));
      NewBlock->next = Block->next;
      Block->next = NewBlock;
      NewBlock->prev = Block;
      Block->size = 0;
      Block->status = 0;
      NewBlock->size = size;
      NewBlock->status = 1;
      SplitBlockAtSize(NewBlock, (OldBlockSize - MBLOCK_HEADER_SZ - size));
      return NewBlock;
    }
    else if (Block->next == NULL) {      // if no block with enough size, allocate 1KB block
      mblock_t * NewBlock = GrowHeapBySize(greater(1000, size + MBLOCK_HEADER_SZ));
      size_t OldBlockSize = NewBlock->size;
      NewBlock->size = size;
      NewBlock->status = 1;
      SplitBlockAtSize(NewBlock, OldBlockSize - MBLOCK_HEADER_SZ - size);
      return NewBlock;
    }
    else {                              // check next block in list
      Block = Block->next;
    } 
  }
}

void SplitBlockAtSize(mblock_t * block, size_t NewSize) {
  mblock_t * NewBlock = (mblock_t *)((char *)(&(block->payload)) + block->size);
  NewBlock->size = NewSize;
  NewBlock->prev = block;
  NewBlock->next = block->next;
  block->next = NewBlock;
}


mblock_t * GrowHeapBySize(size_t size) {
  if (list.head == NULL) {                                                        // empty list case
    mblock_t * NewLastBlock = (mblock_t *)sbrk(MBLOCK_HEADER_SZ + size);
    if (NewLastBlock == (void *)-1) {
        exit(1);
    }
    NewLastBlock->prev = NULL;
    NewLastBlock->next = NULL;
    NewLastBlock->size = size - MBLOCK_HEADER_SZ;
    NewLastBlock->status = 0;
    NewLastBlock->payload = (void *)((char *)NewLastBlock + MBLOCK_HEADER_SZ);
    list.head = NewLastBlock;
    return NewLastBlock; 
  }
  else {                                                                        // non-empty list case
    mblock_t * LastBlock = FindLastMemoryListBlock();
    mblock_t * NewLastBlock = (mblock_t *)sbrk(MBLOCK_HEADER_SZ + size);
    if (NewLastBlock == (void *)-1) {
        exit(1);
    }
    NewLastBlock->next = NULL;
    NewLastBlock->prev = LastBlock;
    NewLastBlock->size = size - MBLOCK_HEADER_SZ;
    NewLastBlock->status = 0;
    NewLastBlock->payload = (void *)((char *)NewLastBlock + MBLOCK_HEADER_SZ);
    LastBlock->next = NewLastBlock;
    return NewLastBlock;
  }
}


mblock_t * FindLastMemoryListBlock() {  
  mblock_t * TraversalBlock = list.head;
  while (TraversalBlock->next != NULL) {
    TraversalBlock = TraversalBlock->next;
  }
  return TraversalBlock;
}

int greater(int a, int b) {
  return (a > b) ? a : b;
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
