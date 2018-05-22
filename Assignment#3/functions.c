/**
* Assignment 3 - User Level Threading
* @author: Ali H. Iqbal
**/
#include <stdio.h>
#include <setjmp.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <math.h>
#include <sys/timeb.h>
#include <setjmp.h>
#include <time.h>

// defined constants
#define JB_SP 6
#define JB_PC 7
#define SECOND 1000000
#define MAX_NO_OF_THREADS 100
#define STACK_SIZE 4096
#define STATUS_SLEEPING 0
#define STATUS_READY 1
#define STATUS_SUSPENDED 2
#define STATUS_RUNNING 3
#define TIME_QUANTUM 1*SECOND


static int run_time = 15000;	/// max instructions for thread
static int curr_schedule = 1;	// 1 for round robin, 2 for lottery scheduling


// global 
static int num_of_threads = 0;	
static int curr_thread_count = 0;	
static int weight_total = 1;
struct timeb t_start, t_stop;		
typedef unsigned long address_t;


//thread weight for lottery scheduling
typedef struct thread_weight		
{
	int min_weight;
	int max_weight;
}thread_weight;

// thread sleeping time
typedef struct sleep_info		
{
	int start_sleeping;
	int sleep_to;
	struct timeb start_s;
	int total;
}sleep_struct;

// thread waiting time
typedef struct wait_info	
{
	int start_waiting;
	int stop_waiting;
	struct timeb start_w, stop_w;
	int total;
}wait_struct;

// thread control block
typedef struct TCB		
{
	
	address_t pc;
	address_t sp;
	thread_weight weight;
	sleep_struct sleep_time;
	wait_struct wait_time;
	sigjmp_buf jbuf;
	int thrd_id;
	int thrd_status;
	int num_bursts;
	int num_waits;
	int num_sleeps;
	int exec_time;
	struct TCB *next;

}TCB;

TCB *head_list = NULL;
TCB *tail_list = NULL;
TCB *curr_thread = NULL;

//functions
void appendList(TCB *new_tcb);
void dispatch(int sig);
void printList();
void yieldCPU(void);
void status(TCB *current);
void CleanUp();
void SleepThread(int sec);
int GetId();
void f( void );	
void g( void );	
int createThread (void (*f) (void));
TCB *findTcb(int random_number);
void go();

#ifdef __x86_64__


address_t tr_address(address_t addr)
{
    address_t ret;
    asm volatile("xor    %%fs:0x30,%0\n"
                 "rol    $0x11,%0\n"
                 : "=g" (ret)
                 : "0" (addr));
    return ret;
}

#else

#define JB_SP 4
#define JB_PC 5


address_t tr_address(address_t addr)
{
    address_t ret;
    asm volatile("xor    %%gs:0x18,%0\n"
                 "rol    $0x9,%0\n"
                 : "=g" (ret)
                 : "0" (addr));
    return ret;
}

#endif


//function used to get status of thread, accepts TCB thread pointer as parameter
void status(TCB *current)
{
	int ex_avg;
	int wait_avg;
	int sleep_total;

	printf("thread: %d\n", current->thrd_id);

	if(current->num_bursts > 0)
	{
		ex_avg = (current->exec_time)/(current->num_bursts);
	}
	else
	{
		ex_avg = 0;
	}

	if(current->num_waits > 0)
	{
		wait_avg = (current->wait_time.total)/(current->num_waits);
	}
	else
	{
		wait_avg = 0;
	}
	if(current->num_sleeps > 0)
	{
		sleep_total = (current->sleep_time.total);
	}
	else
	{
		sleep_total = 0;
	}
	
	printf("  Average execution time\t%d\n", ex_avg);
	printf("    Number of bursts\t%d\n", current->num_bursts);	
	printf("  Average waiting time\t%d\n", wait_avg);
	printf("    Number of waits\t%d\n", current->num_waits);
	printf("  Total sleeping time\t%d\n", sleep_total);
	printf("    Number of sleeps\t%d\n", current->num_sleeps);

}

//function prints info, removes list and exits program
void CleanUp()
{
	TCB *index = head_list;
	int count = 0;
//suspend threads
	while(count < num_of_threads)		
	{
		index->thrd_status = STATUS_SUSPENDED;

		status(index);
		
		index = index->next;
		count++;
	}

	index = head_list;
	TCB *next;
	while(index != tail_list)		
	{
		next = index->next;
		free(index);
		index = next;
	}

	exit(0);			
// exits
} 

//function puts thread to sleep for given time
void SleepThread(int sec)
{
	printf("	SLEEPING\n");

	struct timeb temp_time;
	ftime(&temp_time);

	curr_thread->num_sleeps++;

	curr_thread->sleep_time.start_s = temp_time;
	curr_thread->sleep_time.sleep_to = temp_time.millitm + sec;

	curr_thread->thrd_status = STATUS_SLEEPING;

	yieldCPU();			
}


//fuction to return thread id
int GetId()
{
	return curr_thread->thrd_id;
}


// function handles thread context switching
void yieldCPU(void)					
{
	printf("switching threads\n");

	ftime(&t_stop);//stop exectuting
	curr_thread->exec_time += ( 1000.0 * (t_stop.time - t_start.time) + (t_stop.millitm - t_start.millitm));

	// print thread info
	printf("  Current Thread: %d\n", curr_thread->thrd_id);
	printf("  Thread Status:  %d\n", curr_thread->thrd_status);
	printf("Execution Time: %d\n", curr_thread->exec_time);

	usleep(2*SECOND);//pauses
	
	raise(SIGVTALRM);
}


//function represents thread f
void f(void)			
{
	int i=0;
	while(1)
	{
		++i;
		printf("in f (%d)\n",i);
		if (i % 3 == 0) 
		{
			yieldCPU();
		}
		usleep(SECOND);
	}
}


//function represents thread g
void g( void )
{
	int i = 0;
	while(1)
	{
		++i;
		printf("in g (%d)\n",i);
		if (i % 3 == 0)
		{
			yieldCPU();
		}
		usleep(SECOND);
	}
}


//function create threads along with given arguments
int createThread (void (*f) (void))
{
	TCB* curr_tcb = malloc(sizeof(TCB));

	if(curr_tcb == NULL)				
	{
		curr_tcb->thrd_id = -1;
		num_of_threads++;
	}
	else							
	{
		curr_tcb->thrd_id = curr_thread_count++;
		curr_tcb->pc = (address_t)f;
		curr_tcb->sp = (address_t)malloc(STACK_SIZE);
		curr_tcb->sp = curr_tcb->sp + STACK_SIZE - sizeof(address_t);
		curr_tcb->num_bursts = 0;
		curr_tcb->num_waits = 0;
		curr_tcb->num_sleeps = 0;
		curr_tcb->exec_time = 0;
		
		curr_tcb->sleep_time.sleep_to = 0;
		curr_tcb->sleep_time.start_sleeping = 0;
		curr_tcb->sleep_time.total = 0;

		curr_tcb->wait_time.start_w.millitm = 0;
		curr_tcb->wait_time.stop_w.millitm = 0;
		curr_tcb->wait_time.total = 0;

		printf("set wait time %d\n", curr_tcb->wait_time.total);

		curr_tcb->next = NULL;
		curr_tcb->weight.min_weight = 0;
		curr_tcb->weight.max_weight = 0;
		

		struct timeb temp_time;
		ftime(&temp_time);
		curr_tcb->wait_time.start_w = temp_time;
		curr_tcb->thrd_status = STATUS_READY;   
		num_of_threads++;

		// calculate weight for RR
		if(curr_schedule == 2)			
		{
			curr_tcb->weight.min_weight = weight_total;
			int exponent = curr_tcb->thrd_id;
			curr_tcb->weight.max_weight = weight_total + pow(2,exponent);
			weight_total = curr_tcb->weight.max_weight;
			weight_total++;
		}
			// threads exceeded 
		if(num_of_threads >= MAX_NO_OF_THREADS)	
		{
			CleanUp();
		}
	}	

    
   	sigsetjmp(curr_tcb->jbuf,1);
    	(curr_tcb->jbuf->__jmpbuf)[JB_SP] = tr_address(curr_tcb->sp);
   	(curr_tcb->jbuf->__jmpbuf)[JB_PC] = tr_address(curr_tcb->pc);
	
	sigemptyset(&curr_tcb->jbuf->__saved_mask);

	appendList(curr_tcb);
	return curr_tcb->thrd_id;
}


// find thread with chosen "lottery ticket"
TCB *findTcb(int random_number)
{
	TCB *index = head_list;
	int count = 0;

	while((count < num_of_threads))
	{
		if((random_number >= (index->weight.min_weight)) && (random_number <= (index->weight.max_weight)))
		{	
			break;		
		}
		index = index->next;
		count++;
	}

	if(count == num_of_threads)
	{
		return NULL;
	}
	else
	{
		return index;
	}
}


//function to check sleeping threads which will get woken up
void sleeping_threads()
{
	TCB *index = head_list;
	int count = 0;
	while(count < num_of_threads)
	{
		struct timeb temp_time;
		ftime(&temp_time);

		if((index->thrd_status == STATUS_SLEEPING) && (temp_time.time > curr_thread->sleep_time.sleep_to))
		{
			curr_thread->thrd_status = STATUS_READY;
			curr_thread->wait_time.start_w = temp_time;
			curr_thread->sleep_time.sleep_to = 0;
			curr_thread->sleep_time.total +=  ( 1000.0 * (temp_time.time - curr_thread->sleep_time.start_s.time ) + (temp_time.millitm - curr_thread->sleep_time.start_s.millitm));
		}
		index = index->next;	
		count++;
	} 
}


//function which runs thread scheduler and dispach a signal
void dispatch(int sig)
{
	sleeping_threads();
	printf("Alarm\n");

	//Round-robin scheduling
	if(curr_schedule == 1)		
	{
		if(curr_thread == NULL)
		{
			curr_thread = head_list;
			head_list->thrd_status = STATUS_RUNNING;
			ftime(&t_start);
			curr_thread->wait_time.stop_w = t_start;
			curr_thread->wait_time.total += ( 1000.0 * (curr_thread->wait_time.stop_w.time - curr_thread->wait_time.start_w.time) + (curr_thread->wait_time.stop_w.millitm - curr_thread->wait_time.start_w.millitm));
			siglongjmp(head_list->jbuf, 1);
		}
		else
		{
			if( (curr_thread->exec_time) > run_time )
			{
				CleanUp();
			}

			if(sigsetjmp(curr_thread->jbuf, 1) == 1)
			{
				ftime(&t_start);
				return;
			}

			struct timeb temp_time;
			ftime(&temp_time);
			curr_thread->thrd_status = STATUS_READY;
			curr_thread->wait_time.start_w = temp_time;
			curr_thread = curr_thread->next;

			while(curr_thread->thrd_status != STATUS_READY)
				curr_thread= curr_thread->next;
			curr_thread->thrd_status = STATUS_RUNNING;
			ftime(&t_start);
			curr_thread->wait_time.stop_w = t_start;

			if(curr_thread->wait_time.stop_w.millitm != 0)
			{
				curr_thread->wait_time.total+= ( 1000.0 * (curr_thread->wait_time.stop_w.time - curr_thread->wait_time.start_w.time) + (curr_thread->wait_time.stop_w.millitm - curr_thread->wait_time.start_w.millitm));
				curr_thread->num_waits++;
			}
			curr_thread->num_bursts++;
			siglongjmp(curr_thread->jbuf, 1);	
		}
	} // Lottery scheduling
	else if (curr_schedule == 2)			
	{
		if(curr_thread == NULL)
		{
			curr_thread = head_list;
			head_list->thrd_status = STATUS_RUNNING;
			ftime(&t_start);
			curr_thread->wait_time.stop_w = t_start;
			curr_thread->wait_time.total += ( 1000.0 * (curr_thread->wait_time.stop_w.time - curr_thread->wait_time.start_w.time) + (curr_thread->wait_time.stop_w.millitm - curr_thread->wait_time.start_w.millitm));
			siglongjmp(head_list->jbuf, 1);
		}
		else
		{
			if( (curr_thread->exec_time) > run_time )
			{
				CleanUp();
			}

			if(sigsetjmp(curr_thread->jbuf, 1) == 1)
			{
				return;
			}
		
			curr_thread->thrd_status = STATUS_READY;
			struct timeb start_wait_time;
			ftime(&start_wait_time);
	
			curr_thread->wait_time.start_w = start_wait_time;

			int mod_value = weight_total - 1;
			int chosen_number = ( rand() % mod_value )+ 1;

			TCB *selected_thread = NULL;
			
			do
			{
				selected_thread = findTcb(chosen_number);
				chosen_number = ( rand() % mod_value ) + 1;
			}while(selected_thread->thrd_status != STATUS_READY);

			curr_thread = selected_thread;
			ftime(&t_start);
			curr_thread->wait_time.stop_w = t_start;
			
			if(curr_thread->wait_time.stop_w.millitm != 0)
			{
				curr_thread->wait_time.total += ( 1000.0 * (curr_thread->wait_time.stop_w.time - curr_thread->wait_time.start_w.time) + (curr_thread->wait_time.stop_w.millitm - curr_thread->wait_time.start_w.millitm));
				curr_thread->num_waits++;
			}

			curr_thread->thrd_status = STATUS_RUNNING;
			curr_thread->num_bursts++;

			siglongjmp(curr_thread->jbuf, 1);		
		}
	}
}

//function to print the list
void printList()
{
	TCB *index;

	if(head_list == NULL)
	{
		printf("*Empty list \n");
	}

	printf("  head %p\n", head_list);
	printf("  tail %p\n", tail_list);
	printf("  current %p\n", curr_thread);

	int i = 0;
	for(index = head_list; i < num_of_threads; index = index->next)
	{
		printf("thread address: %p\n", index);
		printf("  id: %d\n", index->thrd_id);
		printf("    status:       %d\n", index->thrd_status);
		printf("    pc address:   %lu\n", index->pc);
		printf("    sp address:   %lu\n", index->sp);
		printf("    next address: %p\n", index->next);
		printf("    min weight:   %d\n", index->weight.min_weight);
		printf("    max weight:   %d\n", index->weight.max_weight);

		i++;	
	}
}


//function add/append newly created thread 
void appendList(TCB *new_tcb)
{
	if( head_list == NULL)
	{
		head_list = new_tcb;
		tail_list = new_tcb;
		new_tcb->next = head_list;

	}
	else
	{
		TCB *index;
		for( index = head_list; index != tail_list; index = index->next);
		index->next = new_tcb;
		tail_list = new_tcb;
		new_tcb->next = head_list;
	}
}


//function gets called by main, creates threads
void go()
{
	signal(SIGVTALRM, dispatch);

	srand(time(NULL));

	struct itimerval tv;
	tv.it_value.tv_sec = 2; //first timer
	tv.it_value.tv_usec = 0; 
	tv.it_interval.tv_sec = 2; //all timers but first one
	tv.it_interval.tv_usec = 0; 
	
	setitimer(ITIMER_VIRTUAL, &tv, NULL);

	createThread(g);
	createThread(f);

	printList();	
	
	while(1);
}

//main function chooses scheduling method
//return int
int main()
{
	printf("Enter number for scheduling method\n");
	printf("1. For Round-robin scheduling\n");
	printf("2. For Lottery scheduling\n");
	scanf("%d",&curr_schedule);
	if(curr_schedule == 1){
		printf("Running round-robin scheduling\n");
	} else if(curr_schedule == 2){
		printf("Running lottery scheduling\n");
	} else {
		curr_schedule = 1;
		printf("Running round-robin scheduling\n");
	}

	go();
	return 0;
}
