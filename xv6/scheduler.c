#include "types.h"
#include "defs.h"
#include "param.h"
#include "mmu.h"
#include "x86.h"
#include "proc.h"
#include "spinlock.h"
#include "scheduler.h"

struct spinlock sched_data_lock;
struct schedule_data sched_data;

const int stride1 = (1 << 20);
const int quantum = 10000000;


void schedule_init()
{
// cprintf("start s_init\n");
  initlock(&sched_data_lock, "sched_data");
  acquire(&sched_data_lock);
  sched_data.bottom = 0;
  sched_data.global_tickets = 0;
  sched_data.global_stride = 0;
  sched_data.global_pass = 0;
  release(&sched_data_lock);
 // cprintf("end s_init\n");
}

void schedule_update()
{
 // cprintf("start s_update\n");
	acquire(&sched_data_lock);
	static int last_update =0;
	if (last_update == 0)
		last_update = clock();
	int elapsed = clock() - last_update;
	last_update = clock();

	sched_data.global_pass += (sched_data.global_stride * (elapsed / quantum));
	
	release(&sched_data_lock);
 // cprintf("end s_update\n");
}

// requires lock to be held
int heap_contains(struct proc * p)
{
	int i;
	for (i = 0; i <= sched_data.bottom; ++i)
	{
		if (sched_data.heap[i]->pid == p->pid)
		{
			cprintf("Heap_contains found a match for pid %d! Bad news!\n", p->pid);
			return 1;
		}
	}
	return 0;
}

void global_tickets_update(int tickets)
{
 // cprintf("start g_tickets_update\n");
	if (tickets == 0 || sched_data.global_tickets == 0)
		return;
	sched_data.global_tickets += tickets;
	sched_data.global_stride = stride1 / sched_data.global_tickets;
}


struct proc * _queue_remove(int root_index)
{
  //cprintf("start _queue_remove\n");
	acquire(&sched_data_lock);
	struct proc ** heap = sched_data.heap;
	struct proc * top = heap[root_index];
	// reheapify
	heap[root_index] = heap[sched_data.bottom];
	sched_data.heap[sched_data.bottom] = 0;
	sched_data.bottom -= 1;
	int current = root_index;
	while(1)
	{
		int min;
		if (2*current >= sched_data.bottom)
			break;
		else if (2*current + 1 >= sched_data.bottom)
			min =  2*current;
		else
			min = (heap[2*current ]->pass < heap[2*current + 1]->pass) ? 2*current : 2*current + 1;
		
		if (heap[current]->pass > heap[min]->pass)
		{
			struct proc * temp = heap[current];
			heap[current] = heap[min];
			heap[min] = temp;
			current = min;
		}
		else
			break;
	}

	release(&sched_data_lock);
 // cprintf("end _queue_remove\n");
	return top;
}

void schedule_insert(struct proc * p)
{
  //cprintf("start s_insert\n");
//	cprintf("Insert proc %d, size %d\n", p->pid, sched_data.bottom+1);
	schedule_update();
	acquire(&sched_data_lock);
	if (heap_contains(p))
	{
		release(&sched_data_lock);
		return;
	}
	sched_data.bottom++; // bottom points at the last element
	int current = sched_data.bottom;
	sched_data.heap[sched_data.bottom] = p;
	while(current > 1 && sched_data.heap[current]->pass < sched_data.heap[current/2]->pass)
	{
		struct proc* temp = sched_data.heap[current/2];
		sched_data.heap[current/2] = sched_data.heap[current];
		sched_data.heap[current] = temp;
		current = current/2;
	}

	release(&sched_data_lock);
  //cprintf("end s_insert\n");
}

struct proc * schedule_pop()
{
 // cprintf("start s_pop\n");
  acquire(&sched_data_lock);
  if (sched_data.bottom == 0)
  {
    	release(&sched_data_lock);
	return 0;
  }
  release(&sched_data_lock);
   struct proc * p =  _queue_remove(1);
  // cprintf("Pop pid %d, size %d\n", p->pid, sched_data.bottom);
   return p;
  //cprintf("end s_pop\n");
}

void queue_remove(struct proc * p)
{
 // cprintf("start s_remove\n");
	acquire(&sched_data_lock);
	int i = 1;
	for(; i < sched_data.bottom; ++i)
	{
		if (sched_data.heap[i] == p)
		{
			release(&sched_data_lock);
			_queue_remove(i);
			return;
		}
	}

  //cprintf("end s_remove\n");
	release(&sched_data_lock);
}


void schedule_join(struct proc *p)
{
	schedule_update();
	acquire(&sched_data_lock);
//	cprintf("join : %d\n", p->pid);
	
	global_tickets_update(p->tickets);
	p->pass = sched_data.global_pass; //+ (p->elapsed ? (quantum - p->elapsed) : 0);
	//p->elapsed = clock();

	release(&sched_data_lock);
	//cprintf("join\n");
	schedule_insert(p);
}

void schedule_leave(struct proc *p)
{
	schedule_update();
	acquire(&sched_data_lock);

//	cprintf("leave : %d\n", p->pid);
	//p->elapsed = clock() - p->elapsed;
	//p->elapsed = (p->elapsed > 0) ? p->elapsed : 0;

	global_tickets_update(-p->tickets);
	
	release(&sched_data_lock);
	queue_remove(p);
}

// requires proc_table_lock to be held
void mod_tickets(struct proc *p, int tickets)
{
//	cprintf("Mod tickets, %d gets %\n", p->pid, tickets);
	schedule_leave(p);
	p->tickets = tickets;
	p->stride = stride1 / tickets;
    //p->elapsed = 0;
	//cprintf("mod_tickets -> join\n");
	schedule_join(p);
}