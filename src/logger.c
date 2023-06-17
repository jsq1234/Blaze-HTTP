#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

#include "logger.h"

static
void init_message_queue(message_queue_t* mq){
    mq->head = NULL;
    mq->tail = NULL;
}

static
void log_impl(logger_t* log, const char* msg){
    
    pthread_mutex_lock(&log->active_queue_mutex);

    log_to_queue(log->active_queue, msg);

    log->log_msg = 1;

    pthread_cond_signal(&log->cond);

    pthread_mutex_unlock(&log->active_queue_mutex);

}

static
void log_to_queue(message_queue_t* mq, const char* msg){
    
    message_node_t* node = malloc(sizeof(message_node_t));
    
    node->msg = malloc(strlen(msg)+1);
    node->next = NULL;

    strcpy(node->msg,msg);

    if( mq->tail == NULL ){
        mq->head = node;
        mq->tail = node;
    }else{
        mq->tail->next = node;
        mq->tail = node;
    }
}

void* logger_thread(void* param){
    logger_t* log = (logger_t*)param;

    message_queue_t* inactive_queue;

    while(1){
        pthread_mutex_lock(&log->active_queue_mutex);

        while(!log->log_msg){
            //wait until we are told to log the messages into log file
            pthread_cond_wait(&log->cond,&log->active_queue_mutex);
        }

        // Here, we swap the active queue with the unactive queue
        // This will enable us to put new logs into our now active queue
        // while our previous queue gets logged in the log file by this thread
        inactive_queue = log->active_queue;
        log->active_queue = (log->active_queue == &log->mq1) ? &log->mq2 : &log->mq1;

        // now the main thread can use this 'new' active queue 
        // while we log into the file, that's why we release the lock
        pthread_mutex_unlock(&log->active_queue_mutex);

        // log all the messages that are in the queue into the log file
        message_node_t* node = inactive_queue->head;
        
        while(node){

            fprintf(log->file,"%s\n",node->msg);
            fflush(log->file);

            message_node_t* temp = node;
            
            node = node->next;

            free(temp->msg);
            free(temp);

        }

        inactive_queue->head = NULL;
        inactive_queue->tail = NULL;
        
        pthread_mutex_lock(&log->active_queue_mutex);
        
        log->log_msg = 0;
        
        pthread_mutex_unlock(&log->active_queue_mutex);

    }
}


int init_logger(logger_t* log, const char* file_name){
    
    // setting this variable to 1 logs the message that are in the active queue
    // into the log file named as file_name
    log->log_msg = 0; 

    init_message_queue(&log->mq1);
    init_message_queue(&log->mq2);
    
    log->active_queue = &log->mq1;
    
    if( pthread_mutex_init(&log->active_queue_mutex,NULL) < 0 ){
        perror("mutex initialization failed.\n");
        return -1;
    }

    if( pthread_cond_init(&log->cond,NULL) < 0 ){
        perror("pthread_cond_t initialization failed.\n");
        return -1;
    }
    
    log->file = fopen(file_name,"a+");
    
    if( log->file == NULL ){
        perror("opening/creating log file failed\n");
        return -1;
    }

    if( pthread_create(&log->thread,NULL,logger_thread,log) < 0 ){
        perror("Failed to create thread.\n");
        return -1;
    }
    return 0;

}

void log_message(logger_t* logger, const char* msg){
    log_impl(logger,msg);
}