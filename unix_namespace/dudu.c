#include "unp.h"
#ifdef HAVE_MSGHDR_MSG_CONTROL
int kaka = 1;
#else
int kaka = 0;
#endif

int main(){
    printf("%d\n", kaka);
}
