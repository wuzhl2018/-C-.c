/*���ȣ�����һ��һ����ͨ��״̬������Ҫʵ����Щ���ݡ�
״̬���洢�ӿ�ʼʱ�̵����ڵı仯�������ݵ�ǰ���룬������һ��״̬������ζ�ţ�
״̬��Ҫ�洢״̬��������루���ǰ���������ת��������������Ӧ��*/

/*����ͼ��ʾ��{s1, s2, s3}��Ϊ״̬����ͷc1/a1��ʾ��s1״̬������Ϊc1ʱ����ת��s2��������a1������
���·�Ϊһ�����룬״̬��Ӧ�������·�Ӧ��*/

/*
��ǰ״̬	����	��һ��״̬	����
  s1	    c1	    s2	        a1
  s2	    c2	    s3	        a2
  s3	    c1	    s2	        a3
  s2	    c2	    s3	        a2
  s3	    c1	    s2	        a3
  s2	    c1	    s_trap	    a_trap
  s_trap	c1	    s_trap	    a_trap
*/

/*��ĳ��״̬��������ʶ�������ʱ����Ĭ�Ͻ�������״̬��������״̬�У������������������붼����������
Ϊ�˱����������Զ��������Ƕ������ǵ�״̬���������ͣ�*/

typedef int State;
typedef int Condition;
 
#define STATES 3 + 1
#define STATE_1 0
#define STATE_2 1
#define STATE_3 2
#define STATE_TRAP 3
 
#define CONDITIONS 2
#define CONDITION_1 0
#define CONDITION_2 1

/*��Ƕ��ʽ�����У����ڴ洢�ռ�Ƚ�С����˰�����ȫ������ɺꡣ���⣬Ϊ�˽���ִ��ʱ��Ĳ�ȷ���ԣ�
����ʹ��O(1)����ת����ģ��״̬����ת�����ȶ�����ת���ͣ�*/

typedef void (*ActionType)(State state, Condition condition);
 
typedef struct
{
    State next;
    ActionType action;
} Trasition, * pTrasition;

/*Ȼ������ͼ�е���ת��ϵ����������ת��һ��������ת�ȶ��������*/

// (s1, c1, s2, a1)
Trasition t1 = {
    STATE_2,
    action_1
};
 
// (s2, c2, s3, a2)
Trasition t2 = {
    STATE_3,
    action_2
};
 
// (s3, c1, s2, a3)
Trasition t3 = {
    STATE_2,
    action_3
};
 
// (s, c, trap, a1)
Trasition tt = {
    STATE_TRAP,
    action_trap
};

/*���еĶ��������û��Լ���ɣ������������һ�������䡣*/

void action_1(State state, Condition condition)
{
    printf("Action 1 triggered.\n");
}

//�������ת��
pTrasition transition_table[STATES][CONDITIONS] = {
/*      c1,  c2*/
/* s1 */&t1, &tt,
/* s2 */&tt, &t2,
/* s3 */&t3, &tt,
/* st */&tt, &tt,
};

/*���ɱ�������е���ת��ϵ��
�����״̬������������Ƕ�����������ô״̬������Ҫ�洢��ǰ״̬�����ˡ����磺*/

typedef struct
{
    State current;
} StateMachine, * pStateMachine;
 
State step(pStateMachine machine, Condition condition)
{
    pTrasition t = transition_table[machine->current][condition];
    (*(t->action))(machine->current, condition);
    machine->current = t->next;
    return machine->current;
}

/*���ǿ��ǵ���һ����ת���ڽ��е�ʱ��ͬʱ������������������ת�����������ݲ�һ�µ����⡣
�ٸ����ӣ�task1(s1, c1/a1 �C> s2)��task2(s2, c2/a2 �C> s3)�Ⱥ�ִ�У��ǿ���˳������s3״̬�ģ�
��������a1���е�ʱ��ִ��Ȩ�ޱ�task2��ռ����task2��ʱ�����ĵ�ǰ״̬����s1��s1����c2�ͽ�������״̬��
�����ᵽ��s3�ˣ�Ҳ����˵��״̬����ת�����˲�ȷ�������ǲ������̵ġ�*/

/*���Ҫ�������״̬��������һ���������С�������һ�����ڴ洢������������С��޸ĺ�Ĵ�������:*/

#define E_OK        0
#define E_NO_DATA   1
#define E_OVERFLOW  2
 
typedef struct
{
    Condition queue[QMAX];
    int head;
    int tail;
    bool overflow;
} ConditionQueue, * pConditionQueue;
 
 
int push(ConditionQueue * queue, Condition c)
{   
    unsigned int flags;
    Irq_Save(flags);
    if ((queue->head == queue->tail + 1) || ((queue->head == 0) && (queue->tail == 0)))
    {
        queue->overflow = true;
        Irq_Restore(flags);
        return E_OVERFLOW;
    }
    else
    {
        queue->queue[queue->tail] = c;
        queue->tail = (queue->tail + 1) % QMAX;
        Irq_Restore(flags);
    }
    return E_OK;
}
 
int poll(ConditionQueue * queue, Condition * c)
{
    unsigned int flags;
    Irq_Save(flags);
    if (queue->head == queue->tail)
    {
        Irq_Restore(flags);
        return E_NO_DATA;
    }
    else
    {
        *c = queue->queue[queue->head];
        queue->overflow = false;
        queue->head = (queue->head + 1) % QMAX;
        Irq_Restore(flags);
    }
    return E_OK;
}
 
typedef struct
{
    State current;
    bool inTransaction;
    ConditionQueue queue;
} StateMachine, * pStateMachine;
 
static State __step(pStateMachine machine, Condition condition)
{
    State current = machine -> current;
    pTrasition t = transition_table[current][condition];
    (*(t->action))(current, condition);
    current = t->next;
    machine->current = current;
    return current;
}
 
State step(pStateMachine machine, Condition condition)
{
    Condition next_condition;
    int status;
    State current;
    if (machine->inTransaction)
    {
        push(&(machine->queue), condition);
        return STATE_INTRANSACTION;
    }
    else
    {
        machine->inTransaction = true;
        current = __step(machine, condition);
        status = poll(&(machine->queue), &next_condition);
        while(status == E_OK)
        {
            __step(machine, next_condition);
            status = poll(&(machine->queue), &next_condition);
        }
        machine->inTransaction = false;
        return current;
    }
}
 
void initialize(pStateMachine machine, State s)
{
    machine->current = s;
    machine->inTransaction = false;
    machine->queue.head = 0;
    machine->queue.tail = 0;
    machine->queue.overflow = false;
}
