## XV6 Labs - for Operating System

Source: https://pdos.csail.mit.edu/6.828/2021/xv6.html

### Lab 1 Utilities

Including some user-space programs 

#### sleep.c

Nothing to say, just use the system call **sleep**.

#### pingpong.c

Using **fork** we create a child process, using **pipe** we connect a pair of file descriptors. Then using **read** and **write** for process communication.

#### pirmes.c

Find primes in 2, 35 using the sieve of Eratosthenes in each pipeline and print out the first prime. Need to control the amount of piped file descriptors.

![alt text](https://github.com/LanFear-Li/labs-xv6/Image/sieve.gif)

#### find.c

A unix-like **find** program. Traverse the file directory recursively. Using **stat** and **dirent** for file manipulation.

#### xargs.c

Nothing to say, see my 3-days 69-lines bullshit code in user/xargs.c.

