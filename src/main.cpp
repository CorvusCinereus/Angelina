#include "Angelina.h"

#ifdef __ANDROID__
extern "C" void slint_main()
#else
int main(int argc, char **argv)
#endif
{
    Angelina angelina;

    angelina.run();
}