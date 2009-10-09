
struct schedule_data {
	int bottom;
	int global_tickets;
	int global_stride;
	int global_pass;
	struct proc * heap[NPROC+1];
};

void schedule_insert(struct proc*);
struct proc * schedule_pop();
void schedule_init();
void schedule_join(struct proc *p);
void schedule_leave(struct proc *p);
void mod_tickets(struct proc *p, int tickets);

extern const int quantum;
extern const int stride1;
extern struct schedule_data sched_data;

unsigned clock();
