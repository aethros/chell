#include "util.h"

void test_isSpace()
{
    for (size_t i = 9; i < 14; i++)
    {
        printf("Test: isSpace returned %d for argument %c\n", isSpace((char)i), (char)i);
    }

    printf("Test: isSpace returned %d for argument %c\n", isSpace((char)0x20), (char)0x20);
}

void run_test_Whitespace(char* text, int len)
{
    addrPair_t pair = trimAlgorithm(text, len);
    if(!isSpace(((char*)pair.start)[0])
    && !isSpace(((char*)pair.start)[pair.endOffset]))
    {
        printf("whitespace test passed!\n");
    }
    else
    {
        printf("whitespace test failed on string: %s\n", text);
    }
}

void test_trimWhitespace()
{
    // case 1:
    char* text = "char* str";
    int len = 10;
    run_test_Whitespace(text, len);

    // case 2:
    char* text1 = "   char*         str              ";
    int len1 = 35;
    run_test_Whitespace(text1, len1);

    // case 3:
    char* text2 = "   char*         str";
    int len2 = 21;
    run_test_Whitespace(text2, len2);

    // case 4:
    char* text3 = " char*         str";
    int len3 = 19;
    run_test_Whitespace(text3, len3);

    // case 5:
    char* text4 = " char*         str ";
    int len4 = 20;
    run_test_Whitespace(text4, len4);

    // case 6:
    char* text5 = "char*         str ";
    int len5 = 19;
    run_test_Whitespace(text5, len5);
}

void run_test_TokenCount(char* text, size_t length, int expected)
{
    addrPair_t pair = trimAlgorithm(text, length);
    int actual = getTokenCount((char*) pair.start, pair.endOffset);
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
    test_isSpace();
    test_trimWhitespace();
    test_getTokenCount();
    return 0;
}