#include <stdio.h>
#include <stdlib.h>

#define MEGABYTE 1024*1024

#define MEMAMOUNT MEGABYTE*1000

int main(int argc, char *argv[])
{
        void *myblock = NULL;
        int count = 0;

        //while (1)
        //{
            myblock = (void *) malloc(MEMAMOUNT);
            if (!myblock) {
                printf("ERROR allocating %d\n", MEMAMOUNT);
            }
            printf("Currently allocating %d\n", MEMAMOUNT);
        //}

        free(myblock);
        
        exit(0);
}
