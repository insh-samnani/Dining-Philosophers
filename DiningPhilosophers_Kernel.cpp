#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/unistd.h>
#include <linux/semaphore.h>

#define N 5
#define THINKING 2
#define HUNGRY 1
#define EATING 0
#define LEFT (phnum + 4) % N
#define RIGHT (phnum + 1) % N
 
int state[N];
int phil[N] = { 0, 1, 2, 3, 4 };

struct semaphore mutex;
struct semaphore S[N];

void test(int phnum)
{
    if (state[phnum] == HUNGRY && state[LEFT] != EATING && state[RIGHT] != EATING){
        // state that eating
        state[phnum] = EATING;
 
        msleep(2);
        printk("Philosopher %d takes fork %d and %d\n", phnum + 1, LEFT + 1, phnum + 1);
        printk("Philosopher %d is Eating\n", phnum + 1);
 
        // up(&S[phnum]) has no effect
        // during takefork
        // used to wake up hungry philosophers
        // during putfork
        up(&S[phnum]);
    }
}
 
// take up chopsticks
void take_fork(int phnum)
{
    down(&mutex);
 
    // state that hungry
    state[phnum] = HUNGRY;
 
    printk("Philosopher %d is Hungry\n", phnum + 1);
 
    // eat if neighbours are not eating
    test(phnum);
 
    up(&mutex);
 
    // if unable to eat wait to be signalled
    down(&S[phnum]);
 
    msleep(1);
}
 
// put down chopsticks
void put_fork(int phnum)
{
 
    down(&mutex);
 
    // state that thinking
    state[phnum] = THINKING;
 
    printk("Philosopher %d putting fork %d and %d down\n", phnum + 1, LEFT + 1, phnum + 1);
    printk("Philosopher %d is thinking\n", phnum + 1);
 
    test(LEFT);
    test(RIGHT);
 
    up(&mutex);
}

 
int philosopher(void* num)
{
    while (1) {
 
        int* i = num;
 
        msleep(1);
 
        take_fork(*i);
 
        msleep(0);
 
        put_fork(*i);
    }
}

 
asmlinkage long sys_DPP(void)
{
 
    int i;
    //kthread_t thread_id[N];
    struct task_struct *thread_id[N];
    // initialize the semaphores
    sema_init(&mutex,1);
 
    for (i = 0; i < N; i++)
       sema_init(&S[i],0);
 
    for (i = 0; i < N; i++){
        // create philosopher processes
	thread_id[i]= kthread_create(philosopher, (void*) &phil[i], "philosopher_thread");
	if(thread_id[i])
	    wake_up_process(thread_id[i]);
    	else
		kthread_stop(thread_id[i]);
	printk("Philosopher %d is thinking\n", i + 1);
    }
    
    return 0;
}
