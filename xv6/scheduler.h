
struct schedule_data {
	int bottom;
	int global_tickets;
	int global_stride;
	int global_pass;
	struct proc * heap[NPROC];
};

void schedule_queue_insert(struct proc * p);
struct proc * schedule_queue_pop();
void schedule_init();
void schedule_join(struct proc *p);
void schedule_leave(struct proc *p);
void schedule_init_proc(struct proc *p, int tickets);

extern const int quantum;
extern const int stride1;