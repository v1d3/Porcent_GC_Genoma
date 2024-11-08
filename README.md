1) First, compile the file (semaforo.cpp or MutexCode.cpp).
Example: g++ semaforo.cpp

2) When running the program, you must pass the directory path and the desired threshold as parameters.
Example: ./a.out /home/usr/.../directory 0.5

3) This program reads files from the given directory path and its subdirectories. It then adds the files to a shared queue and prints the files in it that meet the condition of having a threshold that exceeds the given threshold.

4) Since the threads in our code only perform the action of enqueuing and do not need to wait for any prior condition to do so, a mutex is sufficient to synchronize them efficiently. Therefore, no condition variables are used.
