# Parallel Zip

## Purpose

The purpose of `pzip.cpp` was to learn how to solve the **producer-consumer** with an **unbounded
buffer problem**. The program built upon `wzip.cpp` from a previous project, by adding multi-threading to
zip the files in parallel. Another goal of this program was to learn how to synchronize threads,
using **Semaphores** and **Mutexs**, to output zipped data in the same order as the files that were passed.

### Assumptions

The files that are passed as arguments MUST exist in the **CWD**. If any of the files don't exist, 
an error will be displayed but it won't be handled. This error will corrupt the output, 
so if it's redirected to a file, `wunzip.cpp` will not function as intended.

## Limits

**Advantages:** 
	    Not checking file existance results in a faster program. Having to check if the files exist
	    in the main thread, or opening all the files in the main thread first, would not take advantage
	    of multi-threaded programing. The quicker the main thread creates the 'child' threads, the 
	    faster the overall program will run (including open file operations).

**Disadvantages:** 
- The program is bottlenecked by the synchronized output of the threads.
    Threads will zip in parallel, but will wait for the previous thread before printing. This slows down a thread
	from getting to another task. The solution to this would be writing the output at the end, 
	or passing an array back to the main thread (when it joins) to output right after it joins.
- Program cannot handle large amounts of file due to use of **STL Vector**
- Threads only print 5 bytes at a time

**Solutions:** Addressing the disadvantages above
- Instead of synchronizes threads to print to `stdout`, have threads put output into another buffer then print it all at the end
- Remove packing to prevent excess bytes being stored in the **struct** `zdata`
- Change the write to be the entire buffer size instead of 5 bytes at a time

## I/O

`Input` : The file names to zip

`Output` : The zipped version of the file(s), which uses run-length encoding. Must use shell redirection to get zipped output into a file. 





