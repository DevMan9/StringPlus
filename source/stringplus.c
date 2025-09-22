#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "stringplus.x"

void PrintfCreateString(char **ret, char *format, ...)
{
    va_list args;
    va_start(args, format);
    int size = 1 + vsnprintf(NULL, 0, format, args);
    (*ret) = malloc(size);
    vsprintf((*ret), format, args);
    va_end(args);
}

void StringAppend(char **base, char *append)
{
    size_t base_length = strlen(*base);
    size_t append_length = strlen(append);
    (*base) = realloc((*base), 1 + base_length + append_length);
    memcpy((*base) + base_length, append, 1 + append_length);
}

struct TrieNode
{
    struct TrieNode *children[256];
    struct TrieNode *failure_link;
    char **matches;
    size_t match_count;
};

static void CreateTrieNode(struct TrieNode **ret)
{
    (*ret) = malloc(sizeof(**ret));
    memset((*ret)->children, 0, sizeof((*ret)->children));
    (*ret)->failure_link = NULL;
    (*ret)->match_count = 0;
    (*ret)->matches = calloc(1, sizeof(*((*ret)->matches)));
}

static void _AhoCorasickAddPattern(struct TrieNode *root, char *pattern)
{
    struct TrieNode *current_node = root;
    char *current_character = pattern;

    while (*current_character != '\0')
    {
        struct TrieNode **next_node = &(current_node->children[(unsigned char)(*current_character)]);
        if ((*next_node) == NULL)
        {
            CreateTrieNode(next_node);
        }
        current_node = *next_node;
        current_character++;
    }

    current_node->matches[current_node->match_count] = pattern;
    current_node->match_count++;
    current_node->matches = realloc(current_node->matches, (1 + current_node->match_count) * sizeof(*(current_node->matches)));
    current_node->matches[current_node->match_count] = NULL;
}

static void _push(void *array, size_t *size, size_t *count, void *item)
{
    void ***v_array = (void ***)array;
    (*v_array)[*count] = item;
    (*count)++;
    if ((*count) >= (*size))
    {
        (*size) <<= 1;
        (*v_array) = realloc((*v_array), (*size) * sizeof(**v_array));
    }
    (*v_array)[*count] = NULL;
}

static void _pop(void *array, size_t *count, void *item)
{
    void **v_array = (void **)array;
    void **v_item = (void **)item;
    (*count)--;
    (*v_item) = v_array[*count];
    v_array[*count] = NULL;
}

static void _AhoCorasickBuildFailures(struct TrieNode *root)
{
    size_t queue_size = 16;
    size_t queue_count = 0;
    struct TrieNode **queue = malloc(queue_size * sizeof(*queue));

    root->failure_link = root;
    _push(&queue, &queue_size, &queue_count, root);

    size_t queue_index = 0;
    while (queue_index < queue_count)
    {
        struct TrieNode *current_node = queue[queue_index];
        for (int c = 0; c < 256; c++)
        {
            struct TrieNode *child = current_node->children[c];
            if (child == NULL)
            {
                continue;
            }

            struct TrieNode *fail_node = current_node->failure_link;

            // Repeatedly follow the failure_link until we find a node with a matching transition.
            while (fail_node != root && fail_node->children[c] == NULL)
            {
                fail_node = fail_node->failure_link;
            }

            if (fail_node->children[c] != NULL && fail_node->children[c] != child)
            {
                child->failure_link = fail_node->children[c];
            }
            else
            {
                child->failure_link = root;
            }

            _push(&queue, &queue_size, &queue_count, child);
        }
        queue_index++;
    }
}

static void _AhoCorasickBuildTrie(char **patterns, struct TrieNode **ret)
{
    CreateTrieNode(ret);
    for (char **pattern = patterns; *pattern != NULL; pattern++)
    {
        _AhoCorasickAddPattern((*ret), *pattern);
    }
    _AhoCorasickBuildFailures(*ret);
}

static void _AhoCorasickDestroyTrie(struct TrieNode **trie_address)
{
}

static void _AhoCorasickSearch(struct TrieNode *root, char *string, char ****ret)
{
    size_t ret_size = 1 + strlen(string);
    (*ret) = malloc(ret_size * sizeof(**ret));
    size_t *counts = calloc(ret_size, sizeof(*counts));
    for (size_t i = 0; i < ret_size - 1; i++)
    {
        (*ret)[i] = calloc(1, sizeof(*((*ret)[i])));
    }

    struct TrieNode *current_node = root;
    for (char *c = string; *c != '\0'; c++)
    {
        unsigned char uc = *c;

        while ((current_node != root) && (current_node->children[uc] == NULL))
        {
            current_node = current_node->failure_link;
        }

        if (current_node->children[uc] != NULL)
        {
            current_node = current_node->children[uc];
        }

        for (struct TrieNode *check_node = current_node; check_node != root; check_node = check_node->failure_link)
        {
            for (char **pattern = check_node->matches; *pattern != NULL; pattern++)
            {
                size_t pattern_length = strlen(*pattern);
                size_t index = 1 + c - string - pattern_length;
                (*ret)[index][counts[index]] = *pattern;
                counts[index]++;
                (*ret)[index] = realloc((*ret)[index], (1 + counts[index]) * sizeof(*((*ret)[index])));
            }
        }
    }
}

static void _AhoCorasick(char *string, char **patterns, char ****ret)
{
    struct TrieNode *trie = NULL;
    _AhoCorasickBuildTrie(patterns, &trie);
    _AhoCorasickSearch(trie, string, ret);
    _AhoCorasickDestroyTrie(&trie);
}

void StringEnclosedBy(char *string, char *open, char *close, char **ret, char **the_rest)
{
    char ***occurances = NULL;
    char *patterns[] = {open, close, NULL};
    _AhoCorasick(string, patterns, &occurances);

    size_t open_count = 0;
    size_t opens_size = 16;
    char **opens = malloc(opens_size * sizeof(*opens));
    char *start = NULL;
    char *end = NULL;
    char *best_start = NULL;
    char *best_end = NULL;
    size_t minimal_enclosure = -1;
    for (char ***occurance = occurances; *occurance != NULL; occurance++)
    {
        // printf("\n%s\n", string + (occurance - occurances));
        for (char **pattern = *occurance; *pattern != NULL; pattern++)
        {
            // printf("%s\n", *pattern);
            if (*pattern == open)
            {
                // printf("push!\n");
                _push(&opens, &opens_size, &open_count, string + (occurance - occurances));
            }
            else if (*pattern == close)
            {
                if (open_count > 0)
                {
                    // printf("pop!\n");
                    _pop(opens, &open_count, &start);
                    end = string + (occurance - occurances);
                    // printf("%zu, %zu\n", open_count, minimal_enclosure);
                    if (open_count < minimal_enclosure)
                    {
                        best_start = start;
                        best_end = end;
                        minimal_enclosure = open_count;
                    }
                }
            }
        }
    }
    if (minimal_enclosure == -1)
    {
        goto nothing_enclosed;
    }
    best_start += strlen(open);

    size_t ret_length = best_end - best_start;
    (*ret) = malloc(1 + ret_length);
    memcpy((*ret), best_start, ret_length);
    (*ret)[ret_length] = '\0';

    if (the_rest != NULL)
    {
        *the_rest = best_end;
    }
    goto end;
nothing_enclosed:
    (*ret) = NULL;
    (*the_rest) = string;
end:
    return;
}

void StringAllMatches(char *string, char **patterns, char ****ret)
{
    _AhoCorasick(string, patterns, ret);
}