#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
// Argument loading
#include <getopt.h>

// Function for help text
void showHelp();

// Argument parsing
void parseArgs(int argc, char* argv[], bool* show_help, bool* verbose, char** token);
