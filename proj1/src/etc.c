#include "etc.h"

// Function for help text
void showHelp()
{
  printf("Usage:\n"
  "  isabot [-h|--help] [-v|--verbose] -t|--token <bot_access_token>\n\n"
  "Flags:\n"
  "  -h, --help\t\t\t\t"               "Show help\n"
  "  -v, --verbose\t\t\t\t"            "Verbose output switch\n"
  "  -t, --token <bot_access_token>\t" "Discord bot auth token\n"
  );
}

// Argument parsing
void parseArgs(int argc, char* argv[], bool* show_help, bool* verbose, char** token)
{  
  const struct option long_options[] =
  {
    {"help",    no_argument, NULL, 'h'},
    {"verbose", no_argument, NULL, 'v'},
    {"token",   no_argument, NULL, 't'},
    {NULL,   no_argument, NULL, 0}
  };

  int arg = 0;
  // Load arguments
  while (arg != -1)
  {
    arg = getopt_long(argc, argv, "t::vh", long_options, NULL);
    switch (arg)
    {
      case 't':
        if (!optarg &&
            argv[optind] != NULL &&
            argv[optind][0] != '-')
        { 
          *token = argv[optind++]; 
        }
        else // token.empty()
        {
          fprintf(stderr, "Token cannot be empty.");
          exit(1);
        }
        break;

      case 'v':
        *verbose = true;
        break;

      case 'h':
      case '?':
        *show_help = true;
        break;

      default:
        break;
    }
  }
}