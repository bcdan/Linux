#include <stdio.h>


char* test(){
    return "stam";
}

int main() {
    char * something = test();
    printf("%s!\n",something);
    return 0;
}
