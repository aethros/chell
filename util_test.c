#include "util.h"

void run_test_TokenCount(char* text, size_t length, int expected)
{
    int actual = getTokenCount(text, ' ', length);
    if (expected == actual)
    {
        printf("token test passed!\n");
    }
    else
    {
        printf("token test failed on string: %s\nexpected: %d, actual: %d\n\n", text, expected, actual);
    }
}

void test_getTokenCount()
{
    // case 1:
    char* text = "char* str";
    int len = 10;
    run_test_TokenCount(text, len, 2);

    // case 2:
    char* text1 = "   char*         str              ";
    int len1 = 35;
    run_test_TokenCount(text1, len1, 2);

    // case 3:
    char* text2 = "  hello   my name is          jim   ";
    int len2 = 37;
    run_test_TokenCount(text2, len2, 5);

    // case 4:
    char* text3 = "o say can you see";
    int len3 = 18;
    run_test_TokenCount(text3, len3, 5);

    // case 5:
    char* text4 = "ls -alG ../foo";
    int len4 = 15;
    run_test_TokenCount(text4, len4, 3);

    // case 6:
    char* text5 = "mkdir -p /usr/bin/file";
    int len5 = 23;
    run_test_TokenCount(text5, len5, 3);
}

int main(void){
    test_getTokenCount();
    return 0;
}
