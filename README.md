# Heap-Manager
Heap manager with "malloc" and "free" functions. Implements dynamic coalescing. 
This is project for Duke University's CS 310 (Operating Systems Course). It implements a heap manager with dynamic memory allocation and achieves high memory utilization. The two main functions are dmalloc, to allocate memory, and dfree, to free memory. 

**How It Works**
Upon initialization, the dmalloc function calls sbrk, which carves out a region of virtual memory. The pointer returned by that function represents the virtual heap region. It also represents the pointer to the "freelist".

*The Freelist*
The freelist is a doubly linked list that maintains the state of the heap. It contains "metadata" headers that act as borders for the heap blocks. These headers contain data about the size of the block, whether it is in use/not in use, and pointers to the previous and next blocks in the free list. Dmalloc and dfree manipulate the freelist to allocate/deallocate memory. 

*Dmalloc*
Dmalloc returns a pointer to the heap block with the requires a certain number of bytes. First, it resets the required size equal to the size of the header plus the required number of bytes. Then, it iterates overy the freelist, checking headers for a size that is greater or equal to the new required size. If the block is of equal size, then perfect! Dmalloc returns the pointer to that block. If ihe block is of greater size, then dmalloc splits the block. In the split function, it sets a two headers: the first being the block of the desired size, and the second being the block that represents the rest of the unused space. The pointer to the first block is then returned back to dmalloc. 

*Dfree*
Dfree frees the block with the pointer passed into it. It first resets the pointer to the pointer minus the header size. Then, it iterates over the freelist and checks if the pointers are equal. Once the desired block is found, it is set to free and coalesced with blocks that are also free. The purpose of the coalesce function is to prevent memory fragmentation. Essentially, it frees up more space between blocks so larger regions of memory can be dmalloced in the future. In the coalesce function, it checks the previous and next blocks and unlinks them from the freelist if they are not in use. 

The 'dmm.h' file is important for configuring the project, like the word size for alignment, and the maximum size of the heap region. 
These tests proved my 'dmm.c' accurately implemented the heap manager with optimized memory utilization. 
