#include <stdio.h>
#include <stdlib.h>

#define MEMAMOUNT 20000000

int main(int argc, char *argv[])
{
        void *myblock = NULL;
        int count = 0;

        myblock = (void *) calloc(MEMAMOUNT, 128);
        if (!myblock) {
            printf("ERROR allocating %d\n", MEMAMOUNT);
        }
        printf("Currently allocating %d\n", MEMAMOUNT);

        free(myblock);
        
        exit(0);
}
