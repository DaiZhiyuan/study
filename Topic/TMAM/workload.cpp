#include <random>

extern "C" { 
    void foo(char* a, int n); 
}

const int _200MB = 1024*1024*200;

int main(void) 
{

    char* arrary = (char*)malloc(_200MB);
    for (int index = 0; index < _200MB; index++)
	arrary[index] = 0;

    const int min = 0;
    const int max = _200MB;
    std::default_random_engine generator;
    std::uniform_int_distribution<int> distribution(min, max - 1);

    for (int i = 0; i < 100000000; i++) {
        int random_int = distribution(generator);
        //__builtin_prefetch(arrary + random_int, 0, 1);
        foo(arrary, random_int);
    }

    free(arrary);
    return 0;
}
