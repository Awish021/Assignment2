
#include "types.h"
#include "stat.h"
#include "user.h"

#define NUM_FORKS 500

int main(void)
{
    int *temp;
    int i, t1, t2;
    
    temp=malloc(40960); //required to allocate more pages to get better results
    *temp=0;

    printf(1,"Please wait!.....\n");
    t1 = uptime();
    t2 = 0;
    for(i = 0; i < NUM_FORKS; i++)
    {
        t1 = uptime();
        if(fork() == 0)
        {
            exit();
        }
        else
        {
           t2 += uptime() - t1;
           waitcow();
        }
    }
    printf(1, "Total uptime for %d forks is %d, \n", NUM_FORKS, t2);

    printf(1,"Please wait some more!.....\n");
    t1 = uptime();
    t2 = 0;
    for(i = 0; i < NUM_FORKS; i++)
    {
        t1 = uptime();
        if(forkcow() == 0)
        {
            exit();
        }
        else
        {
            t2 += uptime() - t1;
            waitcow();
        }
    }
    printf(1, "Total uptime for %d forkcows is %d\n", NUM_FORKS, t2);

    exit();
    return 0;
}
