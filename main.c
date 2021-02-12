/***
 **
 * @filename : main.c
 *
 * @brief    : main Module Source File
 *
 * @Date     : 2020-12-10 11:16:29
 * @Author   : Junbo Zheng
 * @E-mail   : zheng_junbo@foxmail.com
 * @Version  : v1.0
 * @Company  : X Comm. Co., Ltd.
 * @Summary  :
 *
 * @Function : 1.
 *
 * @Log      : 1. 2020-12-10 11:16:29 : zhengjunbo - Create
 */

#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

#include "timer.h"


static void semphore_setter(timer_argv_t sem_p){
	sem_post(sem_p);
}
 
static void* timed_thread(void *threadid){
    sem_t sem;

	if (sem_init(&sem, 0, 0) == -1){
		pthread_exit("sem_init failed");
	}

	timer_handle_t *p_timer = fn_timer_create(e_timer_type_periodic, semphore_setter, &sem);
    if (NULL == p_timer){
        sem_destroy(&sem);
        pthread_exit("Timer create failed");
    }

	struct timespec ts[101];

	fn_timer_start(p_timer, 1);  

    sem_wait(&sem);//ignore first event for more stable timing 

	int i;
	for(i=0;i<101;++i){
		sem_wait(&sem);
        //usleep(1000-75);//very unstable, and a lot of drift
		clock_gettime(CLOCK_REALTIME, &ts[i]);
        //do something at a 1 ms interval
	}

	fn_timer_stop(p_timer);

    printf("timer values (us):\n");
    int t0_us = ts[0].tv_sec * 1000000 + ts[0].tv_nsec /1000;
	for(i=0;i<100;++i){
        int t_us = ts[i+1].tv_sec * 1000000 + ts[i+1].tv_nsec /1000 - t0_us;
		printf("%d\n",t_us);
	}

    printf("timer errors (us):\n");
    int min=0,max=0;
	for(i=0;i<100;++i){
        int t_us = ts[i+1].tv_sec * 1000000 + ts[i+1].tv_nsec /1000 - t0_us;
        int error_us = t_us -(i+1)*1000;
        if(error_us < min){
            min = error_us;
        }
        if(error_us > max){
            max = error_us;
        }
		printf("%d\n",error_us);
	}

    printf("min %d\n",min);
    printf("max %d\n",max);

    fn_timer_delete(p_timer);
    sem_destroy(&sem); 
    pthread_exit("Succes!");
 }
 
int main(void) {
    pthread_t timed_thread_id;
    pthread_create(&timed_thread_id, NULL, timed_thread,NULL);

    const char * return_value;
    pthread_join(timed_thread_id,(void**)&return_value);
    if(return_value){
        puts(return_value);
    }
 
    return 0;
}
 