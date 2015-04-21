#include <sys/time.h>
#include <sys/resource.h>
#include <stdio.h>
#include <errno.h>


/**
 *  We want core files!
 */
int
EnableCoreFiles(void)
{
    //--------------------------------
    struct rlimit rl;
    int rc;
    //--------------------------------

    getrlimit(RLIMIT_CORE, &rl);

    rl.rlim_cur = RLIM_INFINITY;

    rc = setrlimit(RLIMIT_CORE, &rl);

    if (rc < 0){
        perror("setrlimit");
    }

    return rc;
}



