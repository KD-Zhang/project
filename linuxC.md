# 进程
- fork:用于创建一个子进程
- exit：用于终止一个进程
- exec：用于执行一个应用程序
- wait：将父进程挂起，等待子进程终止
- getpid：获取当前进程ID
- nice：改变进程的优先级

```c
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>

int main(void)
{
  pid_t pid;
  printf("Process Creation Student\n");
  pid = fork();
  switch(pid)
  {
    case 0:
    printf("child process is running, curpid is %d, parentpid is %d\n", pid,getppid());
    break;
    case -1: 
    printf("fork error");
    default:
    printf("parent process is running, childid is %d, parentpid is %d\n", pid, getpid());
    break;
  }
    
  return 0;
}
```
- 如果一个进程的父进程优先于子进程结束，子进程就是一个孤儿进程，它由init进程收养，成为init进程的子进程
- 如果子进程优先于父进程退出时，如果父进程没有调用wait和waitpid函数，子进程就会进入僵死状态。
```
int nice(); #改变进程优先级
```
## 守护进程
守护进程是指在后台执行的，没有终端与之相连的进程，他独立于控制终端，通常周期性地执行某种任务。
守护进程的执行方式有多种，它可以在linux系统启动时从脚本/etc/rc.d中启动。可以从作业规划进程crond启动，还可以由用户终端执行
编写创建守护进程的程序时，要尽量避免产生不必要的交互。编写守护进程时，有如下要点：
- 让程序后台执行。方法是调用一个fork产生子进程，然后退出父进程。
- 调用setsid创建一个新对话。控制终端，登录会话和进程组通常是由父进程继承下来的，守护进程要摆脱他们不受他们影响，其方法就是调用setsid使其成为会话组长。
- 禁止进程重新打开终端。经过上述步骤，进程已经成为一个无终端的会话组长，但是它可以重新申请打开一个终端。为了避免这种情况发生，可以通过进程不再是会话组长来实现。再一次通过fork创建子进程，使调用fork进程退出
- 关闭不再需要的文件描述符。创建子进程从父进程继承打开的文件描述符。如不关闭，将浪费系统资源。先得到最高的的文件描述符值，然后用一个循环，关闭从0到最高文件描述符值的所有文件描述符。
- 将当前目录更改为根目录。当守护进程当前工作目录在一个装配文件系统中时，该文件系统不能被拆卸。一般需将工作目录改为根目录。
- 将文件创建时使用屏蔽字设置为0。进程从创建它的父进程那里继承的文件创建屏蔽字可能拒绝某些许可权。为防止这点，使用umask(0),将屏蔽字清0。
- 处理sigchld信号。这一点不是必须的，但是某些进程，特别是服务器进程往往在请求到来时生成子进程处理请求。如果父进程不等待子进程结束，子进程将成为僵尸进程，从而占用系统资源。如果父进程等待子进程结束，将增加父进程的负担，影响服务器进程的并发性能。在Linux下，可以简单的将sigchld 信号操作设置为sig_ign。这样，子进程结束时不会产生僵尸进程。
```c
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>

int init_daemon(void)
{
  int pid;
  int i;
  /***忽略终端IO信号，STOP信号***/
  signal(SIGTTOU, SIG_IGN);
  signal(SIGTTIN, SIG_IGN);
  signal(SIGTSTP, SIG_IGN);
  signal(SIGHUP, SIG_IGN);

  pid = fork();
  if(pid > 0)
  {
    exit(0); #结束父进程
  }
  else if(pid < 0)
  {
    return -1; 
  }
  /**新建进程组，摆脱终端**/
  setsid();
  /**不是进程组长，同时让该进程无法在打开一个新的终端**/
  pid = fork();
  if(pid > 0)
  {
    exit(0); #结束父进程
  }
 }
  else if(pid < 0)
  {
    return -1;
  }
  for(i = 0; i < NOFILE; i++)
    close(i);
  chdir("/");
  umask(0);
  signal(SIGCHLD, SIG_IGN);
  return 0;
}

int main(void)
{
   time_t now;
   init_daemon();
   syslog(LOG_USER|LOG_INFO, "测试守护进程: \n");
   while(1)
   {
      sleep(8);
      time(&now);
      syslog(LOG_USER|LOG_INFO, "系统时间: \n", ctime(&now));

      return 0;
 }
}
```
- 使用syslog 首先要下载syslogd服务，配置/etc/syslog
# 线程
```c
#创建线程
int pthread_create(pthread_t *thread, const pthread_attr_t *attr, void *(*start_routine) (void *), void *arg);
#参数：
thread：该参数是个指针，当线程创建成功时，返回创建线程的ID
attr：该参数用于指定线程的属性，NULL表示使用默认属性。
start_routine：该参数作为一个函数指针，指向线程创建后要调用的函数。这个被线程调用的函数也被成为线程函数
arg：该参数指向传递给线程函数的参数。
```
```c

pthread_t pthread_self(void);  #获取本线程ID
/***
The pthread_equal() function compares two thread identifiers.
***/
int pthread_equal(pthread_t t1, pthread_t t2); #判断两个线程ID是否指向同一线程

int pthread_once(pthread_once_t *once_control,void (*init_routine)(void));#保证线程函数init_routine只在进程中执行一次
pthread_once_t once_control = PTHREAD_ONCE_INIT;
```
创建新线程
```c
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
int* thread(void* arg)
{
  pthread_t newthid;

  newthid = pthread_self();
  printf("this is a new thread, thread id = %lu\n", newthid);
  return NULL;
}
int main(int argc, char* argv[])
{
  pthread_t thid;
  printf("main thread ID is %lu", pthread_self()); //%u 十进制无符号整数 
  if(pthread_create(&thid, NULL, (void*)thread, NULL) != 0)
  {
    printf("pthread_create error\n");
    exit(1);
  }
  sleep(1);
  exit(0);
}
```
在某些情况下，函数执行次数被限制为1次，这种情况下就要调用pthread_once函数
```c
#include<stdio.h>
#include <unistd.h>
#include<stdlib.h>
#include<pthread.h>
pthread_once_t once = PTHREAD_ONCE_INIT;

void run()
{
  printf("function run is running %lu\n", pthread_self());

}

void* thread1(void* arg)
{
  pthread_t thid = pthread_self();
  printf("Current thread id %lu\n", thid);
  pthread_once(&once, run);
  printf("thread1 ends\n");
}


void* thread2(void* arg)
{

  pthread_t thid = pthread_self();
  printf("Current thread id %lu\n", thid);
  pthread_once(&once, run);
  printf("thread2 ends\n");
}
int main(int argc, char* argv[])
{
  pthread_t thid1, thid2;

  pthread_create(&thid1, NULL, thread1, NULL);

  pthread_create(&thid2, NULL, thread2, NULL);

  sleep(3);
  printf("main thread exit \n");
  exit(0);
}
```

线程终止
Linux下有两种方式可以使线程终止。第一种，通过return从线程函数返回，第二种是通过调用函数pthread_exit()使线程退出。
```c
void pthread_exit(void *retval);
```
有两种特殊情况需要注意：一种情况，在主线程中，如果从main函数返回或者调用了exit函数退出主线程，则整个进程终止，此时进程中的所有线程也将终止，因此在主线程中不能过早的从main函数返回；另一种情况是，如果主线程调用phread_exit函数，则仅仅是主线程消亡，进程不会结束，进程内其他线程也不会终止，直到所有线程结束，进程才会结束。

```c
int pthread_join(pthread_t thread, void **retval);
```
函数pthread_join用来等待一个线程结束。pthread_join()的调用者将会被挂起，并等待th线程终止。如果retval的值不为NULL,则*retval = retval。需要注意的是，一个线程仅允许一个线程使用  phtread_join来等待他的终止。并且被等待的线程应处于join状态，即非detached状态，detached状态是指对某个线程执行pthread_detach后的所处状态。处于detached状态的线程，无法实现phtread_join。
主线程和子线程之间通常定义两种关系：
可会合(joinable)：
这种关系下，主线程需要明确执行等待操作。在子线程结束后，主线程的等待操作执行完毕，子线程和主线程会合。这时主线程继续执行等待操作之后的下一步操作。主线程必须会合可会合的子线程，Thread类中，这个操作通过在主线程的线程函数内部调用子线程对象的wait()函数实现。这也就是上面加上三个wait()调用后显示正确的原因。必须强调的是，即使子线程能够在主线程之前执行完毕，进入终止态，也必需显示执行会合操作，否则，系统永远不会主动销毁线程，分配给该线程的系统资源（线程id或句柄，线程管理相关的系统资源）也永远不会释放。
相分离(detached)：
顾名思义，这表示子线程无需和主线程会合，也就是相分离的。这种情况下，子线程一旦进入终止态，系统立即销毁线程，回收资源。无需在主线程内调用wait()实现会合。Thread类中，调用detach()使线程进入detached状态。
- 线程私有数据
在多线程环境下，进程内所有线程共享进程的数据空间，因此全局变量为所有线程共有。在程序设计中时需要保存线程自己的全局变量，这种特殊的变量仅在某个线程内部有效。如常见的错误码。errno不应该是一个局部变量，几乎每个函数应该都可以访问它，但是他又不可能是一个全局变量，否则在一个线程里输出的很可能输出的很可能是另外一个进程出错信息。这个问题可以通过创建线程的私有数据(TSD)来解决。在线程内部，线程私有数据可以被各个函数访问，但是它对其他线程是屏蔽的。
线程私有数据采用了一种被成为一键多值的技术，即一个键对应多个数值。访问数据时通过键值来访问，好像是对一个变量进行访问，其实在访问不同的数据。使用线程私有数据时，首先要为每个线程创建一个相关联的键。在各个线程内部，都使用这个公用的键来指代线程数据，但是在不同的线程中，这个键代表的数据是不同的。操作线程私有数据的函数主要有4个：
```c
int pthread_key_create(pthread_key_t *key, void (*destructor)(void*)); ##创建一个键
```
从Linux的TSD池中分配一项，将其值赋给key供以后访问使用。
它的第一个参数key为指向键值的指针。
第二个参数为一个函数指针。如果指针不为空，则在线程退出时将以key所关联的数据为参数，调用destr_function释放分配的缓冲区。key一旦建立，所有的线程都可以访问它。但各线程可根据自己的需要往key中填入不同的值。这就相当于提供一个同名而不同值的全局变量，一键多值。一键多值靠的是一个关键数据结构数组，即TSD池，其结构如下。
```c
static struct pthread_key_struct pthread_keys[PTHREAD_KEYS_MAX] = { { 0, NULL } };
```
创建一个TSD就相当于将结构数组中的某一项设置为"in_use",并将其索引返回给*key,然后设置destrcutor函数为destr_functiion。
```c
int pthread_setspecific(pthread_key_t key, const void *value);##为一个键设置线程私有数据
```
该函数的value的值与  key相关联。用pthread_setspecific为一个键指定新的线程数据时，线程必须显示放原有线程数据用以回收空间。
```c
void *pthread_getspecific(pthread_key_t key);##从一个键读取线程私有数据
```
通过该函数得到key相关联的数据
```c
int pthread_key_delete(pthread_key_t key);##删除一个键
```
该函数用来删除一个键，删除后，键所占的内存将被释放。需要注意的是，键占用的内存被释放，与该键关联的线程所占用的内存并不被释放。因此，线程数据的释放必须在释放键之前完成。
```c
#include<stdio.h>
#include <unistd.h>
#include<stdlib.h>
#include<string.h>
#include<pthread.h>

pthread_key_t key;

void* thread2(void *arg)
{
  int tsd = 5;

  printf("the thread2 %ld is running\n", pthread_self());

  pthread_setspecific(key,(void*)tsd);
  printf("thread2 %ld returns %p\n", pthread_self(), pthread_getspecific(key));

}

void* thread1(void *arg)
{
  int tsd = 1;
  pthread_t thid2;

  printf("the thread1 %ld is running\n", pthread_self());
  pthread_setspecific(key, (void*)tsd);
  pthread_create(&thid2, NULL, thread2, NULL);
  sleep(5);
  printf("thread1 %ld retrurns %p\n", pthread_self(), pthread_getspecific(key));
}
int main(int argc, char* argv[])
{
  pthread_t thid1;
  printf("the main thread begins running\n");
  pthread_key_create(&key, NULL);

  pthread_create(&thid1, NULL, thread1, NULL);
  sleep(8);
  pthread_key_delete(key);
  printf("the main thread end\n");

  return 0;

}
```
## 线程同步
线程最大的特点就是资源的共享性，然而资源共享的同步问题是多线程编程的难点。Linux系统提供了多种处理线程间同步的问题，其中常用的有互斥锁，条件变量和异步信号。
#### 互斥锁
互斥锁通过锁的机制来实现线程间的同步。在同一时刻它通常之允许一个线程执行一个关键部分代码。

```c
//初始化一个互斥锁
int pthread_mutex_init(pthread_mutex_t *restrict mutex,const pthread_mutexattr_t *restrict attr);

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
```
使用互斥锁前必须先进行初始化。初始化有两种方式，一种是静态赋值法，将宏结构常量PTHREAD_MUTEX_INITIALIZER赋值互斥锁。
另一种方式是通过pthread_mutex_init函数初始化互斥锁。其中attr表示互斥锁属性，如果NULL则使用默认属性。
|  属性  |  含义  |
|:--------|:---------|
|pthread_mutex_timed_np|普通锁:当一个线程加锁后，其余请求锁的线程形成等待队列，解锁后按优先级获得锁|
|pthread_mutex_recursive_np|嵌套锁:允许一个线程对同一个锁多次加锁，并通过多次unlock解锁，如果不是同线程请求，则在解锁时重新竞争|
|pthread_mutex_errorcheck_np|检错锁:在同一线程请求同一锁的情况下，返回edeadlk,否则执行的动作与类型pthread_mutex_timed_np相同|
|pthread_mutex_adaptive_np|适应锁:解锁后重新竞争|
初始化以后，就可以给互斥锁加锁了。加锁有两个函数
```c
//加锁，如果不成功，阻塞等待
int pthread_mutex_lock(pthread_mutex_t *mutex);
```
```c
//测试加锁，如果不成功则立即返回，错误码为EBUSY
int pthread_mutex_trylock(pthread_mutex_t *mutex);
```
用pthread_mutex_lock加锁时，如果mutex已经被锁住，当前尝试加锁的线程就会被阻塞，直到互斥锁被其他线程释放。当pthread_mutex_lock函数返回成功时，
说明互斥锁已经被当前线程成功加锁。pthread_mutex_trylock函数则不同，如果mutex已经被加锁，它将立即返回，返回错误码为ebusy,而不是阻塞等待。
```c
//解锁
int pthread_mutex_unlock(pthread_mutex_t *mutex);
```
用pthread_mutex_unlock函数解锁时，要满足两个条件:一是互斥锁必须处于加锁状态，二是调用本函数的线程必须是给本函数的线程必须是给互斥锁加锁的线程。
解锁后如果如果有其他线程在等待互斥锁，等待队列中的第一个线程将获得互斥锁。
当一个互斥锁使用完毕后，必须进行清除。清除锁时，要求当前处于开放状态。若锁处于锁定状态，函数返回ebusy。该函数成功执行时，返回0.
在Linux中，互斥锁并不占用内存，因此pthread_mutex_destroy除了解除到互斥锁的状态以外没有其他操作。

```c
//注销一个互斥锁
int pthread_mutex_destroy(pthread_mutex_t *mutex);
```
```c
pthread_mutex_t number_mutex;
int globalnumber;

void write_globalnumber()
{
  pthread_mutex_lock(&number_mutex);
  globalnumber++;
  pthread_mutex_unlock(&number_mutex);
}

int read_write_globalnumber()
{
  
  int temp;
  pthread_mutex_lock(&number_mutex);
  temp = globalnumber;
  pthread_mutex_unlock(&number_mutex);
  return(temp);
}

```
### 条件变量
条件变量是利用线程间共享的全局变量进行同步的一种机制。条件变量宏观上类似if语句，符合条件就能执行某段程序，否则只能等待条件成立。
使用条件变量  主要包含两个动作，一个等待使用资源的线程等待"条件变量被设置为真";另一个线程在使用资源后"设置条件为真",这样就可以保证线程间同步了。这样就存在一个问题，
就是要保证条件变量能被正确修改，条件变量要受到特殊的保护，实际中使用互斥锁扮演者这样一个保护者的角色。Linux也提供了一系列对条件变量操作的函数。
与互斥锁一样，条件变量的初始化也有两种方式，一种是静态赋值法，将宏常量PTHREAD_COND_INITIALIZER赋值给条件。另一种使用pthread_cond_init函数。
```c
int pthread_cond_init(pthread_cond_t *restrict cond, const pthread_condattr_t *restrict attr);
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
```
attr参数是条件变量的属性，由于没有得到实现，所以它的值通常是NULL。
等待条件成立有两个函数,pthread_cond_wait 和pthread_cond_timewait。
```c
int pthread_cond_timedwait(pthread_cond_t *restrict cond,pthread_mutex_t *restrict mutex,const struct timespec *restrict abstime);
int pthread_cond_wait(pthread_cond_t *restrict cond,pthread_mutex_t *restrict mutex);
```
pthread_cond_wait函数释放由mutex指向的互斥锁，同时使当前线程关于cond指向的条件变量阻塞，直到条件被信号唤醒。
通常条件表达式在互斥锁的保护下求值，如果条件表达式为假，那么线程基于条件变量阻塞。当一个线程改变条件变量的值时，条件变量获得一个信号，使得等待条件变量的线程退出阻塞状态。
pthread_cond_timedwait函数和pthread_cond_wait函数用法类似。差别在于pthread_cond_timedwait函数将阻塞直到条件变量获得信号或者经由abstime指定的时间。也就是说，
如果在给定时刻前条件没有满足，则返回etimeout，结束等待。
线程被条件变量阻塞后，可通过函数pthread_cond_signal和pthread_cond_broadcast激活
```c
int pthread_cond_signal(pthread_cond_t *cond);
int pthread_cond_broadcast(pthread_cond_t *cond);
```
pthread_cond_signal激活一个等待条件成立的线程，存在多个等待线程时，按顺序激活其中一个，而pthread_cond_broadcast则激活所有等待的线程。
当一个条件变量不再使用时，需要将其清除。清除一个条件变量通过调用pthread_cond_destroy实现
```c
int pthread_cond_destroy(pthread_cond_t *cond);
```
pthread_cond_destroy函数清除由cond指向的条件变量。注意：只有在没有线程等待该条件变量的时候才能清除这个条件变量，否则返回ebusy

```c
void pthread_cleanup_push(void (*routine)(void *),void *arg);
void pthread_cleanup_pop(int execute);
```
```c
#include<stdio.h>
#include<unistd.h>
#include<pthread.h>

pthread_mutex_t mutex;
pthread_cond_t cond;


void* thread1(void* arg)
{

  pthread_cleanup_push(pthread_mutex_unlock, &mutex);
  while(1)
  {
    printf("thread1 is running\n");
    pthread_mutex_lock(&mutex);
    pthread_cond_wait(&cond, &mutex);
    printf("thread1 applied the condition\n");
    pthread_mutex_unlock(&mutex);
    sleep(4);
  }

  pthread_cleanup_pop(0);
}
void* thread2(void* arg)
{ 
  printf("thread2 is running\n");
  
  pthread_mutex_lock(&mutex);
  pthread_cond_wait(&cond, &mutex);
  printf("thread2 applied the condition\n");
  pthread_mutex_unlock(&mutex);
  sleep(1);
}
int main(int argc, char* argv[])
{


  pthread_t tid1, tid2;

  printf("condition variable study.\n");
  pthread_mutex_init(&mutex, NULL);

  pthread_cond_init(&cond, NULL);

  pthread_create(&tid1, NULL, (void*)thread1, NULL);
  pthread_create(&tid2, NULL, (void*)thread2, NULL);

  do
  {
    pthread_cond_signal(&cond);

  }while(1);

  sleep(30);

  pthread_exit(0);
}
```
从运行结果来看，thread1和thread2通过条件变量同时运行。在线程thread1中可以看到条件变量使用时要配合互斥锁使用。这样可以防止多个线程同时请求pthread_cond_wait。
调用pthread_cond_wait前必须由本线程加锁。而在更新条件等待队列以前，mutex保持锁定状态，并在线程挂起前解锁。


## 进程间通信
进程间的地址空间是各自独立的，因此进程间交互数据必须采用专门的通信机制。特别是在大型的应用系统中，往往需要多个进程相互协作共同完成一个任务。这就需要使用进程间通信。
Linux下进程间通信的方法基本上从Unix平台上继承而来的，Linux操作系统不但继承了system V IPC通信机制，还继承了基于套接字的进程通信机制。前者是贝尔实验室对unix早期的
进程间通信手段的改进和扩充，其通信的进程局限于单台计算机内，后者则突破了这一局限，通信的进程可以运行在不同的主机上，也就是进行网络通信。
- 管道：管道是一种半双工的通信方式，数据只能单方向流动，而且只能在具有亲缘关系的进程间使用。
管道是一种两个进程间进行单向通信的机制。因为管道传递数据的单向性，管道又称之为半双工管道。管道的这一特点决定了其使用的局限性。
- 数据只能由一个进程流向另一个进程；如果要进行全双工通信，需要建立两个通道。
- 管道只能用于父子进程或者兄弟进程间的通信。无亲缘关系的进程不能使用管道。
除了以上局限性，管道还有其他的一些不足。如管道没有名字，管道的缓冲区大小是受限的。管道所传送的无格式的字节流。这就要求管道的输入方和输出方事先约定好数据的格式。虽然
有那么多不足。但是对于一些简单的进程间通信，管道是完全胜任的。
Linux下创建管道通过调用函数pipe来完成

```c
int pipe(int pipefd[2]);
```
管道两端可分别用描述符pipefd[0]以及pipefd[1]来描述。需要注意的是，管道两端的任务是固定的，一端只能用于读，由描述符pipefd[0]表示，称为管道读端；另一端只能用于写，由文件描述符
pipefd[1]来表示。称其为管道写端。如果试图从管道写端读或者从读端写都将导致出错。
管道是一种文件，因此对文件操作的I/O函数都可以用于管道，如read，write等。
- 从管道中读数据
如果某进程要读取管道中的数据，那么该进程关闭pipefd[1],同时向管道写数据则应关闭pipefd[0],因为管道只能用于具有亲缘关系的进程间的通信，在个进程进行通信时，他们共享文件描述符。在使用前，应及时的关闭不需要的另一端，以免意外发生。
进程在管道读端读取，如果管道的写端不存在，则读进程认为已经读到数据的末尾。读函数返回读出的字节数为0；管道的写端如果存在，且请求读取的字节数大于PIPE_BUF,则返回管道中现有的所有数据。
如果请求的字节数不大于PIPE_BUF,则返回管道中现有
- 向管道中写数据
如果某进程希望向管道中写入数据，那么该进程应该关闭pipefd0文件描述符，同时管道另一端的进程关闭pipefd1。向管道中写数据时，Linux不保证写入的原子性。管道缓冲区一旦有空闲区域，写进程就会试图向管道中写数据。如果读进程不读走缓冲区域的数据，那么写操作将一直阻塞等待。
在写管道时，如果要求写的字节数小于Pipe_buf,则多进程对同一管道的写操作不会交错进行。但是，如果多个进程同时写一个管道，而且某些进程要求写的字节数超过pipe_buf所能容纳时，则多个写的操作可能会交错。

```c
include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<unistd.h>

void child_rw_pipe(int readfd, int writefd)
{
  char* message1 = "from child process.\n";
  write(writefd, message1, strlen(message1) + 1); 

  char message2[100];
  read(readfd, message2, 100);
  printf("child process read from pipe:%s\n", message2);
}
void parent_rw_pipe(int readfd, int writefd)
{
  char* message1 = "from parent process.\n";
  write(writefd, message1, strlen(message1) + 1); 
  char message2[100];
  read(readfd, message2, 100);
  printf("parent process read from pipe:%s\n", message2);
}
int main(int argc, char* argv[])
{

  int pipe1[2]. pipe2[2];
  pid_t pid;
  int stat_val;

  printf("realize full-duplex commication:\n\n");

  if(pipe(pipe1))
  {
    printf("pipe1 failed\n");
    exit(1);
  }

  if(pipe(pipe2))
  {
    printf("pipe2 failed\n");
    exit(1);
  }

  pid = fork();
  switch(pid)
  {
    case -1:
      printf("fork error\n");
      exit(1);
    case 0:
      /***子进程关闭Pipe1写端，关闭pipe2读端***/
      close(pipe1[1]);
      close(pipe2[0]);
      child_rw_pipe(pipe1[0], pipe2[1]);
    default:
      close(pipe1[0]);
      close(pipe2[1]);
      parent_rw_pipe(pipe2[0], pipe1[1]);
      wait(&stat_val);
      exit(0);
  }
  return 0;
}
```
- 有名管道：有名管道也是半双工的通信方式，但是它允许无亲缘关系进程间通信
```c
int mkfifo(const char *pathname, mode_t mode);
```
- 信号量:信号量是一个计数器，可以用来控制多个进程对公共资源的访问。它常作为一种锁机制，防止某进程正在访问共享资源时，其他进程也访问该资源。因此主要作为进程间以及同一进程
内不同线程之间的同步手段。
- 共享内存：共享内存就是映射一段能被其他进程所访问的内存。这段共享内存由一个进程创建但是多个进程都可以访问。共享内存是最快的IPC方式。它是针对其他进程间通信方式运行效率低
而专门设计的。它往往与其他通信机制，如信号量，配合使用，来实现进程间的同步和通信。
- 共享内存
```c
int shmget();#获取共享内存
void * shmat(); #将共享内存加到进程空间中，返回值是共享内存的地址

```

- 消息队列:消息队列由由消息的链表，存放在内核中并由消息队列标识符标识。消息队列克服了信号量传递信息量少。管道只能承载无格式字节流以及缓冲区大小受限等缺点。
1. 创建消息队列
消息队列是随着内核存在而存在，每个消息队列在系统范围内对应唯一的键值。要获得一个消息队列的描述符，只需提供该消息队列的键值即可。该值通常由函数ftok返回。
该函数定义在sys/ipc.h中
```c
key_t ftok(const char *pathname, int proj_id);
```
根据pathname和proj_id两个参数生成唯一值。该函数执行成功会返回一个键值。msgget根据这个键值创建一个消息队列或者访问一个已知存在的消息队列。
```c
int msgget(key_t key, int msgflg);
```
2.写消息队列
```c
int msgsnd(int msqid, const void *msgp, size_t msgsz, int msgflg);
```
3.读消息队列
```c
ssize_t msgrcv(int msqid, void *msgp, size_t msgsz, long msgtyp,int msgflg);
```
```
ftok();  #获取唯一的键值
msgget(); #创建消息队列
msgsnd(); #发送消息
msgrcv(); #接受消息
```
- 信号：信号是一种比较复杂的通信方式。用于通知接收进程某个事件已经发生。
网络编程
tcp数据包格式
- 源端口: 占16比特，写入源端口号，用来 标识发送该TCP报文段的应用进程。
- 目的端口： 占16比特，写入目的端口号，用来标识接收该TCP报文段的应用进程。
- 序号: 占32比特，取值范围[0,2^32-1]，序号增加到最后一个后，下一个序号就又回到0。指出本TCP报文段数据载荷的第一个字节的序号。
- 确认号： 占32比特，取值范围[0,2^32-1]，确认号增加到最后一个后，下一个确认号就又回到0。指出期望收到对方下一个TCP报文段的数据载荷的第一个字节的序号，同时也是对之前收到的所有数据的确认。若确认号=n，则表明到序号n-1为止的所有数据都已正确接收，期望接收序号为n的数据。
- 确认标志位ACK： 取值为1时确认号字段才有效；取值为0时确认号字段无效。TCP规定，在连接建立后所有传送的TCP报文段都必须把ACK置1。
- 数据偏移: 占4比特，并以4字节为单位。用来指出TCP报文段的数据载荷部分的起始处距离TCP报文段的起始处有多远。这个字段实际上是指出了TCP报文段的首部长度。
- 窗口： 占16比特，以字节为单位。指出发送本报文段的一方的接收窗。
- 同步标志位SYN： 在TCP连接建立时用来同步序号。
- 终止标志位FIN： 用来释放TCP连接
- 复位标志位RST： 用来复位TCP连接。
三次握手：
第一次握手：客户端发送 syn 包(syn=j)到服务器，并进入 SYN_SEND 状态，等待服务器确认；
第二次握手：服务器收到 syn 包，必须确认客户的 SYN（ack=j+1），同时自己也发送一个 SYN 包（syn=k），即 SYN+ACK 包，此时服务器进入 SYN_RECV 状态；
第三次握手：客户端收到服务器的SYN＋ACK包，向服务器发送确认包ACK(ack=k+1)，此包发送完毕，客户端和服务器进入 ESTABLISHED 状态，完成三次握手。
四次挥手：
四次挥手即终止TCP连接，就是指断开一个TCP连接时，需要客户端和服务端总共发送4个包以确认连接的断开。在socket编程中，这一过程由客户端或服务端任一方执行close来触发。
由于TCP连接是全双工的，因此，每个方向都必须要单独进行关闭，这一原则是当一方完成数据发送任务后，发送一个FIN来终止这一方向的连接，收到一个FIN只是意味着这一方向上没有数据流动了，即不会再收到数据了，但是在这个TCP连接上仍然能够发送数据，直到这一方向也发送了FIN。首先进行关闭的一方将执行主动关闭，而另一方则执行被动关闭。
中断连接端可以是客户端，也可以是服务器端。

第一次挥手：客户端发送一个FIN=M，用来关闭客户端到服务器端的数据传送，客户端进入FIN_WAIT_1状态。意思是说"我客户端没有数据要发给你了"，但是如果你服务器端还有数据没有发送完成，则不必急着关闭连接，可以继续发送数据。
第二次挥手：服务器端收到FIN后，先发送ack=M+1，告诉客户端，你的请求我收到了，但是我还没准备好，请继续你等我的消息。这个时候客户端就进入FIN_WAIT_2 状态，继续等待服务器端的FIN报文。
第三次挥手：当服务器端确定数据已发送完成，则向客户端发送FIN=N报文，告诉客户端，好了，我这边数据发完了，准备好关闭连接了。服务器端进入LAST_ACK状态。
第四次挥手：客户端收到FIN=N报文后，就知道可以关闭连接了，但是他还是不相信网络，怕服务器端不知道要关闭，所以发送ack=N+1后进入TIME_WAIT状态，如果Server端没有收到ACK则可以重传。服务器端收到ACK后，就知道可以断开连接了。客户端等待了2MSL后依然没有收到回复，则证明服务器端已正常关闭，那好，我客户端也可以关闭连接了。最终完成了四次握手。

- socket函数用来创建一个套接字
```
int socket(int domain, int type, int protocol);
参数：
domain：用来指定创建套接字所使用的协议族
        AF_UNIX:创建只在本机内进行通信的套接字
        AF_INET:使用ipv4 tcp/ip协议
        AF_INET6:使用IPv6 tc/ip协议
type：用来指定套接字类型
      SOCK_STREAM:创建tcp流套接字
      SOCK_DGRAM:创建UDP数据报套接字
      SOCK_RAW:创建原始套接字
protocol 一般设置为0，表示通过参数domain，和type指定的套接字类型来确定使用的协议
eg：
int sock_fd
sock_fd = socket(AF_INET, SOCK_STREAM, 0);  #tcp套接字
sock_fd = socket(AF_INET, SOCK_DGRAM, 0); #udp套接字
```
- 函数connect用来在一个指定套接字上创建一个连接
```
int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
参数：
sockfd：sockfd是由函数socket创建的套接字，如果该套接字是SOCK_STREAM  类型，则connect函数用于向服务器发送连接请求，服务器的ip地址和端口号
        由参数addr指定。如果该套接字是SOCK_DGRAM类型的套接字，则connect函数并不是建立真正的连接，它只告诉内核与该套接字进行的目的地址。只有该目的地址发过来的数据才会被该socket接受。
addr：是一个地址结构体
addrlen：地址结构体的长度
```
- 绑定套接字
```
int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
socket只创建一个套接字，这个套接字工作在哪个端口上，程序并没有指定。服务器的ip和端口号一般是固定的，因此在服务器的程序中，使用bind函数
参数：
addr：指定了sockfd将绑定到本地的地址，
```
- 监听套接字
```
int listen(int sockfd, int backlog);
由函数socket创建的套接字是主动套接字，这种套接字可以用来主动请求连接到某个服务器(通过connect)。但是作为服务器端的程序，通常在某个端口 上监听等待客户端的连接请求。在服务器端，一般先调用socket创建一个主动套接字，然后调用bind将该套接字绑定到某个端口上，接着再调用函数listen将该套接字转换为监听套接字，等待来自于客户端的连接请求。
参数：一般多个客户端连接到一个服务器，服务器向这些客户端提供某种服务。服务器设置一个连接队列，记录已经建立的连接，参数backlog指定了该连接队列的最大长度，如果连接队列已经达到最大，之后的连接请求将会被服务器拒绝

```
- 接受连接
```
int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
参数：
sockfd是由函数socket创建，经函数bind绑定到本地的某个端口上，然后通过listen转化而来的套接字。
addr：用来保存发起连接主机的地址和端口
addrlen:是指addr所指向的结构体的大小。
执行成功返回一个新的代表客户端的套接字。出错则直接返回-1.
如果参数sockfd所指定的套接字被设置为阻塞方式，且连接请求队列为空，则accept将被阻塞到那里直到有连接请求到达为止。如果参数sockfd所指定的套接字被设置为非阻塞方式，，则如果队列为空，accept将立即返回为-1
```
- 发送数据
```
函数send用来在TCP套接字上发送数据，只能对处于连接状态的套接字使用
ssize_t send(int sockfd, const void *buf, size_t len, int flags);
参数：
sockfd为建立好连接的套接字描述符，即accept函数的返回值。
buf指向存放待发送数据的缓冲区。
len待发送数据的长度
flags：
MSG_OOB：在指定的套接字上发送带外数据。该类型的套接字必须支持带外数据
MSG_DONTROUTE:通过最直接的路径发送数据，而忽略下层协议的路由设置

```
- 接收数据
```
函数recv用来在tcp套接字上接收数据
ssize_t recv(int sockfd, void *buf, size_t len, int flags);
函数recv从参数sockfd所指定的套接字描述符上的数据，并保存到buf所指定的缓冲区，参数len是缓冲区的长度。
参数flags为控制选项，一般设置为0或者取一下数值
MSG_OOB：请求接收带外数据
MSG_PEEK:只查看数据而不读出
MSG_WAITALL:只有在接收缓冲区满的时候才返回。

```
- 关闭套接字
```
int close(int fd);
关闭一个套接字
int shutdown(int sockfd, int how);
函数shutdown也用来关闭一个套接字，参数how指定了关闭方式
SHUT_RD:将连接上的读通道关闭
SHUT_WR
SHUT_RDWR
```
创建套接字以后，就可以利用它来传输数据，但有时可能对套接字的工作方式有特殊的要求，此时就需要修改套接字的属性。
```
int getsockopt(int sockfd, int level, int optname,void *optval, socklen_t *optlen);
int setsockopt(int sockfd, int level, int optname, const void *optval, socklen_t optlen);
参数：
sockfd：作为一个套接字
level：进行套接字选项操作的层次，可以取SOL_SOCKET（通用套接字），IPPROTO_IP(IP层套接字)，IPPROTO_TCP(TCP层套接字)。等值。一般取SOL_SOCKET与特定协议不相关的操作。
```
不同机器内部对变量的字节存储顺序不同，有的采用大端模式，有的采用小端模式，大端模式是指高字节数据存放在地址处，低字节数据存放在高地址处。
在网络上传输数据时，由于数据传输的两端可能对应不同的硬件平台，采用的存储字节顺序也可能不一致，因此tcp/ip协议规定了在网络上必须采用网络字节顺序(也是大端模式)，通过大小端原理分析，对于char型数据，由于只占一个字节，不存在这个问题，这也是我们把数据缓冲区定义成char型的原因之一。而对于ip，端口号等非char型数据，必须在数据发送到网络上之前将其转成大端模式，在接收数据之后再将其转换成符合接收端主机的存储模式，Linux系统为大小端模式提供了4个函数，在shell下输入"man byteorder "可获取它们的函数原型。
```
uint32_t htonl(uint32_t hostlong);
uint16_t htons(uint16_t hostshort);
uint32_t ntohl(uint32_t netlong);
uint16_t ntohs(uint16_t netshort);

h--host 主机
n--network 网络字节顺序
```
通常我们习惯于使用字符串形式的网络格式，(如 “192.168.1.1”)，然而在网络数据传输时，需要使用的是二进制形式且为网络字节顺序的ip地址。Linux系统为网络地址的格式转换提供了一系列函数，在shell下输入"man inet",可获取他们的函数原型
```
int inet_aton(const char *cp, struct in_addr *inp);
将参数cp所指向的字符串形式的ip地址格式，转换为二进制的网络字节顺序的ip格式。转换结果存放在inp所指向的空间中。
in_addr_t inet_addr(const char *cp);
与inet_aton功能类似，执行成功后的结果返回。该函数已经过时。
in_addr_t inet_network(const char *cp);
char *inet_ntoa(struct in_addr in);
struct in_addr inet_makeaddr(in_addr_t net, in_addr_t host);
in_addr_t inet_lnaof(struct in_addr in);
in_addr_t inet_netof(struct in_addr in);
```
在客户端/服务器模型中，服务器需要处理多个客户端的连接请求，此时就需要使用多路复用。实现多路复用最简单的方法是采用非阻塞方式套接字，服务器不断的查询各个套接字的状态，如果有数据到达，则读出数据，如果没有则查看下一个套接字，这种方式虽然简单，但是在轮询过程中浪费了大量cpu时间，效率特别低。
另一种方法是服务器进程并不主动的询问套接字状态，而是向系统登记希望监听的套接字，然后阻塞。当套接字上有事件发生时，系统通知服务哪个套接字上发生了什么事件。服务器查询对应的套接字并处理
```
int select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout);
参数：
nfds：指要监听的文件描述符数
readfds：需要监视的可读文件描述符集合
timeout：阻塞时间，如果在阻塞时间内，没有时间发生，则返回0
void FD_CLR(int fd, fd_set *set);
int  FD_ISSET(int fd, fd_set *set);
void FD_SET(int fd, fd_set *set);
void FD_ZERO(fd_set *set);

```
客户端
```
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>    
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/socket.h>
#include <string.h>
#define SERV_PORT 4507
#define LISTENQ 12
#define USERNAME 0
#define PASSWD 1

void send_data(int connfd, const char *string)
{
  if(send(connfd, string, strlen(string), 0) < 0)
  {
    printf("send error\n");
  }

}
int main(int argc, char* argv[])
{
  int sockfd;
  int connfd;
  int optval;
  int ret;
  int flag_recv = USERNAME;
  pid_t pid;
  socklen_t cli_len;
pid_t pid;
  socklen_t cli_len;
  struct sockaddr_in serv_addr;
  struct sockaddr_in cli_addr;
  char recv_buf[128];

  /*int socket(int domain, int type, int protocol);*/
  sockfd = socket(AF_INET, SOCK_STREAM, 0);

  if(sockfd < 0)
  {
    printf("socket error \n");

  }

  /*
  设置该套接字为可以重新绑定端口
  Linux系统中，如果socket绑定了一个端口，该socket正常关闭或程序异常退出一段时间内，该端口依然维持
  原来的绑定状态，其他程序无法绑定该端口，如果设置了SO_REUSEADDR选项则可以避免这个问题
  int setsockopt(int sockfd, int level, int optname,const void *optval, socklen_t optlen);
  */
  optval = 1;
  if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR,(void *)&optval, sizeof(int)) < 0)
  {
    printf(" setsockopt error \n");
  }
  //初始化服务端地址结构
  memset(&serv_addr, 0, sizeof(struct sockaddr_in));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(SERV_PORT);
  serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);//泛指本地所有ip
  //将套接字绑定到本地端口
  if(bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(struct sockaddr_in)) < 0)
  {
    printf("bind error");
  }

  //将套接字转化为监听套接字
  if(listen(sockfd, LISTENQ) < 0)
  {
    printf("listen error \n");
  }

  cli_len = sizeof(struct sockaddr_in);

  while(1)
  {
    //通过accpt接收接收客户端连接请求，并返回连接套接字用于收发数据
    connfd = accept(sockfd, (struct sockaddr*)&cli_addr, &cli_len);
    if(connfd < 0)
    {
      printf("accept error\n");
    }

    printf("accpet a new client, ip:%s\n",inet_ntoa(cli_addr.sin_addr));
    //创建子进程处理刚刚接收到的连接请求
 {
      //子进程
      while(1)
      {
        if((ret =  recv(connfd, recv_buf, sizeof(recv_buf), 0)) < 0)
        {
          printf("recv error\n");
        }
        if(flag_recv == USERNAME)//接收到的是用户名
        {
          printf("recv usrname...\n");
        }
        else if(flag_recv == PASSWD)
        {
          printf("recv passwd....\n");
        } 
        close(sockfd);
        close(connfd);
        exit(0);
      }
    }
    else
    {
      close(connfd);
    }
  } 
return 0;
```
客户端
```
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>    
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/socket.h>
#include <string.h>
int my_recv(int connfd, char* data_buf, int len)
{
  char recv_buf[128];
  char* pread;
  int len_remain = 0;
  int i;

  if(len_remain <= 0)
  {
    if((len_remain = recv(connfd, recv_buf, sizeof(recv_buf), 0)) < 0)
    {   
      printf("recv error \n");
      exit(1);
    }   
    else if(len_remain = 0) //目的计算机端的socket关闭
    {   
      return 0;
    }   
    pread = recv_buf;
  }

  for(i = 0; *pread != '\n'; i++)
  {
    if(i > len)
    {
      return -1;
    }
    data_buf[i] = *pread++;
    len_remain--;
  }

  len_remain--;
  pread++;

  return i;

}

int main(int argc, char* argv[])
{

  struct sockaddr_in serv_addr;
  int serv_port;
  int connfd;
  int i;
  int ret;
  char recv_buf[128];

  if(argc != 5)
  {
    printf("Usage:[-p] [serv_port] [-a] [serv_address] \n");
    exit(1);
  }

  memset(&serv_addr, 0, sizeof(struct sockaddr_in));
for(i = 1; i < argc; i++)
  {
    if(strcmp("-p", argv[i]) == 0)
    {
      serv_port = atoi(argv[i+1]);
      if(serv_port < 0 || serv_port > 65535)
      {
        printf("invalid serv_addr.sin_port \n");
        exit(1);
      }
      else
      {
        serv_addr.sin_port= htons(serv_port);
      }
      continue;
    }
    if(strcmp("-a", argv[i]) == 0)
    {
      if(inet_aton(argv[i + 1], &serv_addr.sin_addr) == 0)
      {
        printf("invalid server ip address\n");
        exit(1);
      }
    }
    continue;
  }
if(serv_addr.sin_port == 0 || serv_addr.sin_addr.s_addr == 0)
  {

    printf("Usage:[-p] [serv_port] [-a] [serv_address] \n");
    exit(1);
  }
//创建tcp套接字
  connfd = socket(AF_INET, SOCK_STREAM, 0);
  if(connfd < 0)
  {
    printf("socket error\n");
    exit(1);
  }
  if(connect(connfd, (struct sockaddr*)&serv_addr, sizeof(struct sockaddr)) < 0)
  {
    printf("connect error\n");
    exit(1);
  }

  if((ret = my_recv(connfd, recv_buf, sizeof(recv_buf))) < 0)
  {
    printf("data is too long\n");
  }

  close(connfd);
  return 0;
}
```






















