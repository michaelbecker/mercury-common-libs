#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "Calculator.h"


typedef bool(*TEST)(void);


struct TEST_CASE{
    const char *Name;
    TEST Test;
    bool RunAlways;
};


#include "test-list.h"

bool Verbose = false;


/**
*  The number was emprically determined and is somewhat based
*  on the number of significant digits stored in a unit test's
*  Expected Results.
*/
bool
DoubleEqual(double a, double b)
{
    //
    //  Special code to handle numbers near zero.
    //
    if ((a < 0.0000001) && (a > -0.0000001)){

        if (fabs(a - b) <= 0.00000001)
            return true;
        else
            return false;
    }
    else{

        if (fabs((a - b) / a) <= 0.00001)
            return true;
        else
            return false;
    }
}



void 
PrintResults(double *Data, int NumberSignals, void *UserData)
{
    (void)UserData;

    printf(" { ");
    for (int i = 0; i < NumberSignals; i++){
        if (Data[i] > SM_DBL_MAX){
            printf("%11s, ", "-----------");
        }
        else{
            printf("%11.6f, ", Data[i]);
        }
    }
    printf(" },\n");
}


bool ExecuteTest(char *Command)
{
    int i = 0;
    bool TestPass;

    while (TestCases[i].Name != NULL){
        if (strcmp(TestCases[i].Name, Command) == 0){
            TestPass = TestCases[i].Test();
            return TestPass;
        }
        else{
            i++;
        }
    }

    return false;
}



void ExecuteAllTests(char *UnitTestProgramName)
{
    int i = 0;
    pid_t pid;
    int status;

    while (TestCases[i].Name != NULL){

        //
        //  Skip these
        //  
        if (TestCases[i].RunAlways == false){
            i++;
            continue;
        }

        pid = fork();

        if (pid == -1){
            abort();
        }
        else if (pid == 0){
            char *ExecVector[3];
            
            ExecVector[0] = UnitTestProgramName;
            ExecVector[1] = (char *)TestCases[i].Name;
            ExecVector[2] = NULL;

            if (Verbose)
                printf("running %s %s\n", ExecVector[0], ExecVector[1]); 
            execv(UnitTestProgramName, ExecVector);

            printf("FAILED TO RUN! Errno: %d\n", errno);
            abort();
        }
        else{

            if (waitpid(pid, &status, 0) == -1){
                //
                //  The OS is broke somehow?
                //
                printf("waitpid() failed! %d\n", errno);
                abort();
            }


            //
            //  Ok, the script exited clean, but we can still have 
            //  errors from the script
            //
            if (WIFEXITED(status)){

                //
                //  Extract the return code from the script
                //
                status = WEXITSTATUS(status);
                
                if (status){
                    printf("UNIT TEST %s FAILED!\n", (char *)TestCases[i].Name);
                }
                else{
                    if (Verbose)
                        printf("Unit test %s passed.\n", (char *)TestCases[i].Name);
                }
            }

            if (WIFSIGNALED(status)){
                printf("UNIT TEST %s DIED BY SIGNAL %d!\n", (char *)TestCases[i].Name, WTERMSIG(status));
            }
        }

        i++;
    }

}




int
main(int argc, char *argv[])
{
    bool TestPass;

    char *v = getenv("SM_VERBOSE");

    if (v == NULL){
    }
    else{
        Verbose = true;
    }


    if (argc > 1) {
        TestPass = ExecuteTest(argv[1]);
        if (TestPass){
            return 0;
        }
        else{
            return 1;
        }
    }
    else {
        if (Verbose)
            printf("Unit tests for Signal Manager calculator.\n\n");
        ExecuteAllTests(argv[0]);
    }
}



