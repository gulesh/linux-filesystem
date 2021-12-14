#ifndef __JOB_H
#define __JOB_H

#include <sys/types.h>
#include <termios.h>

struct job {
        pid_t pid;
        gid_t gid;
        char** t_arr;
        int background;
        int running;
        int jobid;
	struct termios term;
};

typedef struct job jobtype;

jobtype *initjob();

#endif
