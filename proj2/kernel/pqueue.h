
struct task_t {
    int priority;  // Task priority (higher value means higher priority)
    int pid;       // Process ID
    int x;
    int y;
    char op; // Supports "+", "-", "*", "/"
    int *result; // Stores the computation result
    int *error; 
};

struct pq_node {
    struct task_t task;   // The task associated with the node
    struct pq_node* next; // Pointer to the next node in the queue
};

struct pqueue {
    struct pq_node* head;      // Head of the linked list representing the queue
    struct spinlock* lock;     // Lock for thread safety
};

// Initializes the priority queue
void pqueue_init(struct pqueue* pq);

// Pushes a task onto the priority queue (preserving priority order)
int pqueue_push(struct pqueue* pq, struct task_t* task);

// Pops the highest-priority task from the queue
struct task_t* pqueue_pop(struct pqueue* pq);

// Checks if the queue is empty
int pqueue_empty(struct pqueue* pq);
