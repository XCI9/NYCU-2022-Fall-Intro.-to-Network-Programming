#include <cstdio>
#include <cstdlib>

int main() {
    srand(0x7f96fb91);
    for (int i{ 0 }; i < 20; i++)
        printf("%x\n", rand());
}