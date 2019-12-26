#include "sched.h"
#include <linux/sched.h>

/*
YuYe Zhu 516030910148
Main WRR scheduling program.
I delete various unnecessary parts so it may seems a little bit short.
*/

extern char *task_group_path(struct task_group *tg);

static void dequeue_task_wrr(struct rq *rq, struct task_struct *p, int flags)
{
	struct sched_wrr_entity *wrr_se;
	struct wrr_rq *wrr_rq;

	wrr_se = &p->wrr;
	wrr_rq = &rq->wrr;

	list_del_init(&wrr_se->run_list);
	--wrr_rq->wrr_nr_running;
	dec_nr_running(rq);
}



static void enqueue_task_wrr(struct rq *rq, struct task_struct *p, int flags)
{
	struct sched_wrr_entity *wrr_se;
	struct wrr_rq *wrr_rq;
	struct list_head *queue;

	wrr_se = &p->wrr;
	wrr_rq = &rq->wrr;
	queue = &wrr_rq->wrr_rq_list;


	list_add_tail(&wrr_se->run_list, queue);
	wrr_rq->wrr_nr_running++;

	inc_nr_running(rq);
	//printk("enqueue_task_wrr 2 %d\n",p->pid);
}

static void requeue_task_wrr(struct rq *rq, struct task_struct *p, int flags)
{
	//printk("requeue_task_wrr 1 %d\n",p->pid);
	struct sched_wrr_entity *wrr_se;
	struct wrr_rq *wrr_rq;
	struct list_head *head;

	wrr_se = &p->wrr;
	wrr_rq = &rq->wrr;
	head = &wrr_rq->wrr_rq_list;

	if (sizeof(wrr_rq->wrr_nr_running) == 1)
		return;

	list_move_tail(&wrr_se->run_list, head);

	//printk("requeue_task_wrr 1 %d\n",p->pid);
}

//same as requeue
static void yield_task_wrr(struct rq *rq)
{
	requeue_task_wrr(rq, rq->curr , 0);
}

static struct task_struct *pick_next_task_wrr(struct rq *rq)
{
	//printk("pick_next_task_wrr 1\n");
	struct sched_wrr_entity *wrr_se;
	struct task_struct *p;
	struct wrr_rq *wrr_rq;
	struct list_head *queue;

	wrr_rq = &rq->wrr;

	if (!wrr_rq->wrr_nr_running) // return NULL if no wrr tasks
		return NULL;

	queue = &wrr_rq->wrr_rq_list;
	wrr_se = list_entry(queue->next, struct sched_wrr_entity, run_list); //pick a task
	return container_of(wrr_se, struct task_struct, wrr); //return task_struct
	//printk("pick_next_task_wrr 2\n");
}

static void put_prev_task_wrr(struct rq *rq, struct task_struct *prev) 
{
	//leave it dummy
}

static void task_tick_wrr(struct rq *rq, struct task_struct *p, int queued)
{
	printk("task_tick_wrr pid:%d timeslice: %d\n",p->pid,p->wrr.time_slice);
	struct sched_wrr_entity *wrr_se;

	wrr_se = &p->wrr;
	if (--p->wrr.time_slice)
		return;
	//char *tmp=task_group_path(p->sched_task_group);

	//printk("tmp: %s",tmp);
	//printk("task_group_path: %s",task_group_path(p->sched_task_group));

	if(task_group_path(p->sched_task_group) [1] != 'b') //Foreground
	{
		printk("WRR Fore Ground\n");
		p->wrr.time_slice = WRR_TIMESLICE_FG;
	}
	else //Background
	{
		printk("WRR Back Ground\n");
		p->wrr.time_slice = WRR_TIMESLICE_BG;
	}

	if (wrr_se->run_list.prev != wrr_se->run_list.next) { //the same in rt.c
		requeue_task_wrr(rq, p, 0);
		set_tsk_need_resched(p);
		return;
	}
}

static void set_curr_task_wrr(struct rq *rq)
{
	struct task_struct *p;

	p = rq->curr;
	p->se.exec_start = rq->clock_task;

}

static void check_preempt_curr_wrr(struct rq *rq,
				struct task_struct *p, int flags)
{
	if (p->prio < rq->curr->prio) {
		resched_task(rq->curr);
		return;
	}
}

static void switched_to_wrr(struct rq *rq, struct task_struct *p)
{
	struct sched_wrr_entity *wrr_se;

	wrr_se = &p->wrr;
	if(task_group_path(p->sched_task_group) [1] != 'b')
		p->wrr.time_slice = WRR_TIMESLICE_FG;
	
	else
		p->wrr.time_slice = WRR_TIMESLICE_BG;
	
	INIT_LIST_HEAD(&wrr_se->run_list);
}

static void prio_changed_wrr(struct rq *rq, struct task_struct *p, int old)
{ }


static unsigned int get_rr_interval_wrr(struct rq *rq, struct task_struct *p)
{
	if (p == NULL)
		return -EINVAL;

	if(task_group_path(p->sched_task_group) [1] != 'b')
		return WRR_TIMESLICE_FG;
	else
		return WRR_TIMESLICE_BG;
}

void init_wrr_rq(struct wrr_rq *wrr_rq, struct rq *rq)
{
	wrr_rq->wrr_nr_running = 0;
	INIT_LIST_HEAD(&wrr_rq->wrr_rq_list);
}


const struct sched_class wrr_sched_class = {
	.next			= &fair_sched_class,
	.dequeue_task		= dequeue_task_wrr,
	.enqueue_task		= enqueue_task_wrr,
	.yield_task		= yield_task_wrr,
	.check_preempt_curr	= check_preempt_curr_wrr,
	.pick_next_task		= pick_next_task_wrr,
	.put_prev_task		= put_prev_task_wrr,

#ifdef CONFIG_SMP
	.select_task_rq		= select_task_rq_wrr,
	.rq_online		= rq_online_wrr,
	.rq_offline		= rq_offline_wrr,
	.task_woken		= task_woken_wrr,
	.switched_from		= switched_from_wrr,
	.pre_schedule		= pre_schedule_wrr,
	.post_schedule		= post_schedule_wrr,
#endif

	.set_curr_task		= set_curr_task_wrr,
	.task_tick		= task_tick_wrr,

	.get_rr_interval	= get_rr_interval_wrr,

	.prio_changed		= prio_changed_wrr,
	.switched_to		= switched_to_wrr,
};



