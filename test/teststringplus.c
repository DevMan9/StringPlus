#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../source/stringplus.x"

#define _TEST(acc, line, func, ...)           \
    do                                        \
    {                                         \
        int last = acc;                       \
        acc += func(__VA_ARGS__);             \
        if (last != acc)                      \
        {                                     \
            printf("Line %d: FAIL!\n", line); \
        }                                     \
    } while (0)

#define TEST(acc, func, ...) _TEST(acc, __LINE__, func, __VA_ARGS__)

int StringCheck(char *result, char *expected)
{
    return strcmp(result, expected) == 0 ? 0 : 1;
}

int PatternCheck(char ***result, char ***expected)
{
    for (char ***r = result, ***e = expected; *r != NULL; r++, e++)
    {
        for (char **r2 = *r, **e2 = *e; *r2 != NULL; r2++, e2++)
        {
            printf("%s =?= %s\n", *r2, *e2);
            if (strcmp(*r2, *e2) != 0)
            {
                goto fail;
            }
        }
    }
    return 0;
fail:
    return 1;
}

int main()
{
    int failed = 0;
    char *tmp_string = NULL;

    PrintfCreateString(&tmp_string, "Moocow!");
    TEST(failed, StringCheck, tmp_string, "Moocow!");
    free(tmp_string);

    PrintfCreateString(&tmp_string, "The number is %d", 42);
    TEST(failed, StringCheck, tmp_string, "The number is 42");
    free(tmp_string);

    PrintfCreateString(&tmp_string, "");
    StringAppend(&tmp_string, "Base");
    TEST(failed, StringCheck, tmp_string, "Base");

    StringAppend(&tmp_string, "Append");
    TEST(failed, StringCheck, tmp_string, "BaseAppend");

    StringAppend(&tmp_string, "");
    TEST(failed, StringCheck, tmp_string, "BaseAppend");
    free(tmp_string);

    if (failed != 0)
    {
        printf("Test cases failed.\n");
        exit(EXIT_FAILURE);
    }
    else
    {
        printf("All tests passed!\n");
        exit(EXIT_SUCCESS);
    }
    return 0;
}