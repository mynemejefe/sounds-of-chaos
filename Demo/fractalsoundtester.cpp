#include "fractalsoundtester.h"
#include <iostream>

void FractalSoundTester::FractalSoundConstructor()
#define ASSERT_EQ(actual, expected)                                 \
    if(actual != expected){                                         \
        std::cout << "Assert failed while running " << __func__     \
                  << ", at line " << __LINE__ << std::endl          \
                  << "Expected: " << expected                       \
                  << "(" << #expected << ")" << std::endl           \
                  << "Actual: " << actual                           \
                  << "(" << #actual << ")" << std::endl;            \
        return false;                                               \
    }

#define ASSERT_NE(actual, expected)                                 \
    if(actual == expected){                                         \
        std::cout << "Assert failed while running " << __func__     \
                  << ", at line " << __LINE__ << std::endl          \
                  << "Expected: " << expected                       \
                  << "(" << #expected << ")" << std::endl           \
                  << "Actual: " << actual                           \
                  << "(" << #actual << ")" << std::endl;            \
        return false;                                               \
    }

#define ASSERT_TRUE(arg)                                            \
    if(!(arg)) {                                                    \
        std::cout << "Assert failed while running " << __func__     \
                  << ", at line " << __LINE__ << std::endl          \
                  << "Expected " << arg << "(" << #arg << ")"       \
                  << " to be true, but it was false" << std::endl;  \
        return false;                                               \
    }

#define ASSERT_FALSE(arg)                                           \
    if(arg) {                                                       \
        std::cout << "Assert failed while running " << __func__     \
                  << ", at line " << __LINE__ << std::endl          \
                  << "Expected " << arg << "(" << #arg << ")"       \
                  << " to be false, but it was true" << std::endl;  \
        return false;                                               \
    }
{
    int FS = 10;
    FractalSound* fractalSound = new FractalSound(FS);

    if (fractalSound->GetFs() == FS) {
        std::cout << "Test run successfully" << std::endl;
    }

    fractalSound->~FractalSound();
}
