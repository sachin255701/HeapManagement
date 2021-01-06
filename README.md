# HeapManagement
A project to demonstrate heap management strategies.

The directory structure of this project:
```
HeapManagement
    \_____ Makefile
    \_____ README.md
    \_____ lib
            \_____ libmalloc-bf.so    
            \_____ libmalloc-wf.so   
            \_____ libmalloc-ff.so
    \_____ src
            \_____ malloc.c
```

This initial implementation provides feature to allocate memory on heap (malloc) and free that memory (free). Internally, library is responsible for maintaining metadata of the allocated blocks. 

In this project demonstrates metadata approach to keep track of allocated chunks of memory. The metadata for each allocated block is stored in the following way:
```
struct block {
    size_t        size; /* field indicates the size of allocated block */
    struct block *next; /* field points to the next allocated block */
    bool          free; /* field indicates whether the block is free for use or not */
};
```

Using above data structure, linked list of allocated blocks is maintained. Before growing the heap to allocate memory, this list is searched for free blocks. If a free block is found that block is resused instead of growing heap. Library supports three types of strategies for block reuse:  
1. First Fit method
2. Best Fit method
3. Worst Fit method.

This library also supports capturing of statistics of the ```malloc``` and ```free``` functions and print it when the application exits. This statistical data is helpful to understand the efficiency of the various above mentioned stategies.  

## Compilation and execution
Compile all three shared libraries using following command:
```
make all
``` 
Shared libraries can be found in directory ```lib```.

Once you have the library, you can use it to override the existing malloc by using the ```LD_PRELOAD``` trick:
```
# Implementation of library
$ env LD_PRELOAD=lib/libmalloc-ff.so cat README.md
$ env LD_PRELOAD=lib/libmalloc-bf.so cat README.md
$ env LD_PRELOAD=lib/libmalloc-wf.so cat README.md
```

## Future Work
1. Add support for ```calloc``` and ```realloc``` functions.  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;```calloc``` implementation added.  
2. Compare the statistics of three strategies using various test applications.  
3. Add support to make the library thread safe.  
