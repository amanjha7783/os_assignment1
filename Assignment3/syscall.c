#include <unistd.h>

int main() {
    write(1, "Hello System Call\n", 18);
    return 0;
}

