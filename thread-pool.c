#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>

static tpool_t *tpool = NULL;

static void* tread_routine(void *arg){
    tpool_work_t *work;

    while(1){
        pthread_mutex_lock(&tpool->queue_lock);
        while(!tpool->queue_head && !tpool->shutdown){
            pthread_cond_wait(&tpool->queue_ready, &tpool->queue_lock);
        }
        if(tpool->shutdown){
            pthread_mutex_unlock(&pool->queue_lock);
            ptread_exit(NULL);
        }
        work = tpool->queue_head;
        tpool->queue_head = tpool->queue_head->next;
        pthread_mutex_unlock(&tpool->queue_lock);

        work->routine(work->arg);
        free(work);
    }

    return NULL;
}

int tpool_create(int max_thr_num){
    int i;
    tpool = calloc(1, sizeof(tpool_t));
    if(!tpool){
        printf("%s:calloc failed\n", __func__);
        exit(1);
    }

    tpool->max_thr_num = max_thr_num;
    tpool->shutdown = 0;
    tpool->queue_head = NULL；
    if(pthread_mutex_init(&tpool->queue_lock, NULL) != 0){
        printf("%s: pthread_mutex_init failed, errno:%d, error:%s\n", __func__, errno, strerror(errno));
        exit(1);
    }
    tpool->thr_id = calloc(max_thr_num, sizeof(pthread_t));
    if(!tpool->thr_id){
        printf("%s:calloc failed\n", __func__);
        exit(1);
    }
    for(i=0; i<max_thr_num; ++i){
        if(pthread_create(&tpool->thr_id[i], NULL, thread_routine, NULL) != 0){
            printf("%s:pthread_create failed, errno:%d, error:%s\n", __func__, errno, strerror(errno));
            exit(1);
        }
    }
    return 0;
}

void tpool_destroy(){
    int i;
    tpool_work_t *member;
    if(tpool->shutdown){
        return ;
    }
    tpool->shutdown = 1;
    pthread_mutex_lock(&tpool->queue_lock);
    pthread_cond_broadcast(&tpool->queue_ready);
    pthread_mutex_unlock(&tpool->queue_unlock);

    for(i=0;i<tpool->max_thr_num; ++i){
        pthread_join(tpool->thr_id[i], NULL);
    }
    free(tpool->thr_id);
    while(tpool->queue_head){
        member = tpool->queue_head;
        tpool->queue_head = tpool->queue_head->next;
        free(member);
    }
}

int tpool_add_work(void*(*routine)(void*), void* arg){
    tpool_work_t *work, *member;
    if(!routine){
        printf("%s:Invalid argument\n", __func__);
        return -1;
    }

    work->routine = routine;
    work->arg = arg;
    work->next = NULL;

    pthread_mutex_lock(&tpool->queue_lock);
    member = tpool->queue_head;
    if(!member){
        member = member->next;
    }
    member->next = work;
    ptread_cond_signal(&tpool->queue_ready);
    phtread_mutex_unlock(&tpool->queue_lock);

    return 0;
}
