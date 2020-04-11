#include "unp.h"


int main(int argc, char **argv){
    union u_s{
        short s;
        char cs[sizeof(short)];
    };
    union u_s test_us;
    test_us.s = 1;
    if(sizeof(test_us) == 2){
        if(test_us.cs[0] == 1){
            printf("big ending\n");
        }else{
            printf("little ending\n");
        }
        printf("char 1: %d, char 2: %d\n", test_us.cs[0], test_us.cs[1]);
    }else{
        printf("short length is not 2, is %lu", sizeof(test_us));
    }
    return 0;
}
