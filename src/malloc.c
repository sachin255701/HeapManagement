/* malloc.c
 * A heap management library.
 */
#include <assert.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <limits.h>
#include <string.h>
#include <stdlib.h>

/* Macros */
#define ALIGN4(s)           (((((s) - 1) >> 2) << 2) + 4)
#define BLOCK_DATA(b)       ((b) + 1)
#define BLOCK_HEADER(ptr)   ((struct block *)(ptr) - 1)

/* Metadata structure */
struct block {
    size_t        size;
    struct block *next;
    bool          free;
};

/* Global variables */
struct block *FreeList = NULL;

/* Global couunter for reporting status */
int successful_malloc_count = 0;
int successful_free_count = 0;
int reuse_count = 0;
int new_block_count = 0;
int split_block_count = 0;
int coalease_block_count = 0;
int blocks_in_list = 0;
int total_mem_requested = 0;
int max_heap_size = 0;
int at_exit = 0;
int call_atexit = 0;

/* Function to report stats at the exit of application */
void report_stats(void) {
    if(at_exit == 0) {
    	printf("mallocs: %d\n", successful_malloc_count);
    	printf("frees: %d\n", successful_free_count);
    	printf("reuses: %d\n", reuse_count);
    	printf("grows: %d\n", new_block_count);
    	printf("splits: %d\n", split_block_count);
    	printf("coaleases: %d\n", coalease_block_count);
    	printf("blocks: %d\n", blocks_in_list);
    	printf("requested: %d\n", total_mem_requested);
    	printf("max heap: %d\n", max_heap_size);
    	at_exit++;
    }
}

/* 
 * Function to find a free block.
 * Function implements various management methods:
 * 1. Fisrt fit
 * 2. Best fit
 * 3. Worst fit
 */
struct block *find_free(struct block **last, size_t size) {
    struct block *curr = FreeList;

#if defined FIT && FIT == 0
    /* First fit */
    while (curr && !(curr->free && curr->size >= size)) {
        *last = curr;
        curr  = curr->next;
    }
#endif

#if defined FIT && FIT == 1
    /* Best fit */
    struct block *best_fit_block = NULL;
    int best_fit_size = INT_MAX;
    int diff_size = 0;

    while(curr != NULL) {
	diff_size = ((curr -> size) - size);
	if((curr -> free == true) && (diff_size >= 0)) {
	    if(diff_size < best_fit_size) {
		best_fit_size = diff_size;
		best_fit_block = curr;
	    }
	}
	*last = curr;
	curr = curr -> next;
    }
    curr = best_fit_block;
#endif

#if defined FIT && FIT == 2
    /* Worst fit */
    struct block *worst_fit_block = NULL;
    int worst_fit_size = 0;
    int diff_size = 0;

    while(curr != NULL) {
	diff_size = ((curr -> size) - size);
	if((curr -> free == true) && (diff_size >= 0)) {
	    if(diff_size > worst_fit_size) {
		worst_fit_size = diff_size;
		worst_fit_block = curr;
	    }
	}
	*last = curr;
	curr = curr -> next;
    }
    curr = worst_fit_block;
#endif

    return curr;
}

/* Function that allocates requiredspace on heap*/
struct block *grow_heap(struct block *last, size_t size) {
    /* Request more space from OS */
    struct block *curr = (struct block *)sbrk(0);
    struct block *prev = (struct block *)sbrk(sizeof(struct block) + size);

    assert(curr == prev);

    /* OS allocation failed */
    if (curr == (struct block *)-1) {
        return NULL;
    } else {
	new_block_count++;
    }

    /* Update FreeList if not set */
    if (FreeList == NULL) {
        FreeList = curr;
    }

    /* Attach new block to prev block */
    if (last) {
        last->next = curr;
    }

    blocks_in_list++;
    max_heap_size += size;

    /* Update block metadata */
    curr->size = size;
    curr->next = NULL;
    curr->free = false;
    return curr;
}

/* Function called by applications to request memory */
void *malloc(size_t size) {
    /* Align to multiple of 4 */
    size = ALIGN4(size);
    total_mem_requested += size;

    /* Handle 0 size */
    if (size == 0) {
        return NULL;
    }

    /* Look for free block */
    struct block *last = FreeList;
    struct block *next = find_free(&last, size);
    struct block *temp = NULL;

    /* Check is blocks can be split */
    if(next != NULL && ((next -> size - size) > sizeof(struct block))) {
	temp = (struct block*) ((void *)next + sizeof(struct block) + size);
	temp -> size = next -> size - size - sizeof(struct block);
	temp -> free = true;
	temp -> next = next -> next;
	next -> next = temp;
	next -> size = size;
	split_block_count++;
	blocks_in_list++;
    }

    /* Could not find free block, so grow heap */
    if (next == NULL) {
        next = grow_heap(last, size);
    } else {
	reuse_count++;
    }

    /* Could not find free block or grow heap, so just return NULL */
    if (next == NULL) {
        return NULL;
    }
    
    /* Mark block as in use */
    next->free = false;

    successful_malloc_count++;
    /* Return data address associated with block */
    return BLOCK_DATA(next);
}

/* Function that coaleases the free blocks in the list */
void coalesce_blocks() {
    struct block *curr = FreeList;

    while((curr != NULL) && (curr -> next != NULL)) {
	if((curr -> free == true) && (curr -> next -> free == true)) {
	    curr -> size += curr -> next -> size + sizeof(struct block);
	    curr -> next = curr -> next -> next;
	    coalease_block_count++;
	    blocks_in_list--;
	}
	curr = curr -> next;
    }

}

/* Function called by applications to release memory */
void free(void *ptr) {
    /* Call atexit funtion */
    if(call_atexit < 10) {
        atexit(report_stats);
	call_atexit++;
    }

    if (ptr == NULL) {
        return;
    }

    /* Make block as free */
    struct block *curr = BLOCK_HEADER(ptr);
    assert(curr->free == 0);
    curr->free = true;

    coalesce_blocks();

    successful_free_count++;
}

/* Function called by applications to initialized request memomory 
 * void* calloc (size_t nitems, size_t size);
 */
void *calloc (size_t nitems, size_t size) {
    //atexit(report_stats);
    void *retptr = NULL;
    size_t bytes = 0;

    if((nitems == 0) || (size == 0)) {
	return NULL;
    }
    
    bytes = nitems * size;

    retptr = malloc(bytes);

    if(retptr == NULL) {
	return retptr;
    }

    memset(retptr, 0, bytes);

    return retptr;
}
