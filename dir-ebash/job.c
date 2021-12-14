#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

#include "job.h"

jobtype *initjob(){
        jobtype * job = (jobtype *) malloc(sizeof(jobtype));
        if (job == NULL){
                printf("failure to malloc in initjob()\n");
        }
}

