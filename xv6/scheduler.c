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

unsigned clock();

void schedule_init()
{
  initlock(&sched_data_lock, "sched_data");
  sched_data.bottom = 0;
}

void schedule_update()
{
	acquire(&sched_data_lock);
	static int last_update = clock();
	int elapsed;

	elapsed = clock() - last_update;
	last_update = clock();

	sched_data.global_pass += (sched_data.global_stride * elapsed) / quantum;
	
	release(&sched_data_lock);
}

void global_tickets_update(int tickets)
{
	sched_data.global_tickets += tickets;
	sched_data.global_stride = stride1 / sched_data.global_tickets;
}


struct proc * _queue_remove(int root_index)
{
	acquire(&sched_data_lock);
	struct proc * heap[] = sched_data.heap;
	struct proc * top = heap[0];
	// reheapify
	heap[root_index] = heap[sched_data.bottom--];
	int current = root_index;
	while(1)
	{
		int min;
		if (2*current + 1 >= sched_data.bottom)
			break;
		else if (2*current + 2 >= sched_data.bottom)
			min = 2 * current + 1;
		else
			min = (heap[2*current + 1]->pass > heap[2*current + 2]->pass) ? 2*current + 1 : 2*current + 2;
		
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
	return top;
}

void queue_insert(struct proc * p)
{
	acquire(&sched_data_lock);
	int current = sched_data.bottom;
	sched_data.heap[sched_data.bottom++] = p;
	while(current && sched_data.heap[current]->pass > sched_data.heap[(current - 1)/2]->pass)
	{
		struct proc* temp = sched_data.heap[(current-1)/2];
		sched_data.heap[(current-1)/2] = sched_data.heap[current];
		sched_data.heap[current] = temp;
		current = (current-1)/2;
	}

	release(&sched_data_lock);
}

struct proc * queue_pop()
{
	return _queue_remove(0);
}

void queue_remove(struct proc * p)
{
	acquire(&sched_data_lock);
	int i = 0;
	for(; i < sched_data.bottom; ++i)
	{
		if (sched_data.heap[i] == p)
		{
			release(&sched_data_lock);
			_queue_remove(i);
			return;
		}
	}

	release(&sched_data_lock);
}


void schedule_join(struct proc *p)
{
	acquire(&sched_data_lock);
	schedule_update();
	p->pass = sched_data.global_pass + p->remaining;
	p->elapsed = clock();
	global_tickets_update(p->tickets);

	release(&sched_data_lock);
	queue_insert(p);
}

void schedule_leave(struct proc *p)
{
	acquire(&sched_data_lock);
	schedule_update();
	p->elapsed = clock() - p->elapsed;
	p->elapsed = (p->elapsed > 0) ? p->elapsed : 0;

	global_tickets_update(-p->tickets);
	
	release(&sched_data_lock);
	queue_remove(p);
}

void schedule_init_proc(struct proc *p, int tickets)
{
	p->tickets = tickets;
	p->stride = stride1 / tickets;
}