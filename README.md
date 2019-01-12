The main purpose of this project was to become familiar with ***multithreaded programs*** and also how to deal with ***sychronization problems*** between threads. 
In this project, we have four types of threads, each is responsible for a different job:

  - ***Generate Thread*** : These threads will generate a 5x5 matrix and fill it with random values.
  - ***Log Thread*** : These threads will get the 5x5 matrix generated by the generate thread and allocate a bigger matrix and put these smaller matrices inside.
  - ***Mod Thread*** : These threads will get the 5x5 matrix generated by the generate thread and find modula of each number by the first number in the matrix.
  - ***Add thread*** : These threads will get the output of Mod Thread and find a local sum of each 5x5 matrix and add it to the global sum.

*Here are the program details given by the customer:*

- The user will execute the program specifying *the size of the matrix* (N – a multiply of 5) to be generated and the ***number of threads*** to be created of each kind.
- You will then create the specified number of ***Generate Threads***. Generate Threads will create a 5x5 matrix and fill it with random values 
(you can think the upper limit is 100 for random values), and put it inside a *global queue*. 
These submatrices will be used to create the bigger matrix whose size is specified by the user at program execution. 
So the Generate Threads has to create (N/5)*(N/5) submatrices. If you have a smaller number of threads, 
then each thread has to create another submatrix until the specified number is achieved.
Note that the Generate Thread to be executed has to find which submatrix to generate before generation. 
So each Generate Thread has to check the last submatrix assigned and then generate the one after that. 
You have to keep track of and store this information with the submatrices created.
- The ***Log Threads*** will read from the *global queue* that is being updated by the *Generate Threads*. 
Then the first Log Thread to run will *allocate a matrix of size NxN* array and all of them fill the matrix with the numbers from submatrices in the appropriate slots.
- The ***Mod Threads*** will also read from the *global queue* that is being updated by the *Generate Threads*. 
Each Mod Thread will replace the values inside submatrices with the ***remainder*** of when they are divided by the first number in the submatrix (the number at the zeroth row and zeroth column). 
Note that the first numbers in all of the submatrices will be zeros after this operation. 
The updated version of the submatrices will be put inside another global queue.
- ***Add Threads*** will read from the *global queue* that is being updated by Mod Threads. 
They will find the local sum for each matrix and update a global sum value.
- The ***main thread*** will wait for all the threads to finish execution and print the matrix of size NxN (produces by the Log Threads) and the ***global sum*** on an output file.
- All the threads have to print information about their job on the screen.
- Note that when a Generate Thread starts generating a submatrix, it has to get which submatrix to generate. 
You have to make sure that *no multiple* Generate Threads create same submatrix.
- Note that there are *two global queues* requested and the queues can be accessed by many threads of different types so a ***synchronization*** should be done seperately for each queue. This can be achieved by using the ***semophores***.
- Note that multiple Log Threads have to be executed *concurrently*.
- Note that when Log *Threads* and *Mod Threads* can access the *global queue* at the same time since they are not updating the value.
- The *main thread* is responsible for creating threads and waiting for the completion of them.
- Both *global queues* have to be created at the very first time a thread needs to use it by that thread.
- ***Preserving consistency*** and ***preventing deadlocks*** are major issues to be considered.
- Multiple simultaneous operations on different parts of the same matrix and queue should be allowed in your solution.
- Your program will be executed as follows:  
X Example:  
`./prog.out -d 30 -n 15 5 8 6`  
X The -d option represents the size of the matrix to be created (a multiple of 5), -n option represents the numbers of threads to be created of each type (Generate, Log, Mod, Add, respectively).