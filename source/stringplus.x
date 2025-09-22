#ifndef STANDARD_STRINGPLUS_X_1_0_0
#define STANDARD_STRINGPLUS_X_1_0_0

// Allocates a new string to store the printed string.
void PrintfCreateString(char **ret, char *format, ...);

// Reallocates base and appends append to the end of it.
void StringAppend(char **base, char *append);

// Returns the first substring in string enclosed by the given open (left) and close (right) strings.
// If nesting occurs, returns the outermost enclosure.
void StringEnclosedBy(char *string, char *open, char *close, char **ret, char **the_rest);

// Returns a NULL terminated array equal to the size of the given string.
// Each element is itself a NULL terminated array of pattern strings that start at that index.
void StringAllMatches(char *string, char **patterns, char ****ret);

#endif // STANDARD_STRINGPLUS_X_1_0_0
