#ifndef LOGGER_HEADER

#define LOGGER_HEADER

#include <pthread.h>
#include <stdio.h>



typedef struct message_node{
    char* msg;
    struct message_node* next;
} message_node_t;

typedef struct{
    message_node_t* head; // points to the first element of the queue
    message_node_t* tail; // points to the last element of the queue
    size_t elems; // number of elements in the queue
} message_queue_t;

typedef struct{
    message_queue_t mq1;
    message_queue_t mq2;
    message_queue_t* active_queue;
    
    pthread_mutex_t active_queue_mutex;
    pthread_cond_t cond;
    
    size_t msg_idx;
    int log_msg;

    FILE* file;

    pthread_t thread;

} logger_t;


static void log_to_queue(message_queue_t* mq, const char* msg);
static void log_impl(logger_t* log, const char* msg);
static void init_message_queue(message_queue_t* mq);
int init_logger(logger_t* log, const char* file_name);
void log_message(logger_t* logger, const char* msg);

#endif