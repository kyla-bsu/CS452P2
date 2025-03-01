#include "lab.h"
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <limits.h>
#include <ctype.h>
#include <readline/readline.h>
#include <readline/history.h>

// Declare the shell variable globally
struct shell *sh;

/**
    * @brief Set the shell prompt. This function will attempt to load a prompt
    * from the requested environment variable, if the environment variable is
    * not set a default prompt of "shell>" is returned.  This function calls
    * malloc internally and the caller must free the resulting string.
    *
    * @param env The environment variable
    * @return const char* The prompt
    */
char *get_prompt(const char *env) {
     const char *prompt = getenv(env);
     if (!prompt) {
          prompt = "shell>";
     }
     char *result = malloc(strlen(prompt) + 1);
     if (result) {
          strcpy(result, prompt);
     }
     return result;
     free(result);
}

/**
   * Changes the current working directory of the shell. Uses the linux system
   * call chdir. With no arguments the users home directory is used as the
   * directory to change to.
   *
   * @param dir The directory to change to
   * @return  On success, zero is returned.  On error, -1 is returned, and
   * errno is set to indicate the error.
   */

   int change_dir(char **dir) {
    const char *path = NULL;

    if (dir && dir[1]) {
        path = dir[1];
    } else {
        path = getenv("HOME");
        if (!path) {
            struct passwd *pw = getpwuid(getuid());
            if (pw) {
                path = pw->pw_dir;
            }
        }
    }

    if (!path) {
        fprintf(stderr, "cd: could not determine home directory\n");
        errno = ENOENT;
        return -1;
    }

    if (chdir(path) != 0) {
        perror("cd");
        return -1;
    }
    return 0;
}

/**
   * @brief Convert line read from the user into to format that will work with
   * execvp. We limit the number of arguments to ARG_MAX loaded from sysconf.
   * This function allocates memory that must be reclaimed with the cmd_free
   * function.
   *
   * @param line The line to process
   *
   * @return The line read in a format suitable for exec
   */
  #include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

char **cmd_parse(char const *line) {
    long arg_max = sysconf(_SC_ARG_MAX);
    if (arg_max == -1) {
        arg_max = 4096; // Fallback value
    }
    char **argv = malloc((arg_max / 2) * sizeof(char *));
    if (!argv) {
        return NULL;
    }
    int argc = 0;
    char *line_copy = strdup(line);
    if (!line_copy) {
        free(argv);
        return NULL;
    }
    char *token = strtok(line_copy, " \t\n");
    while (token && argc < (arg_max / 2 - 1)) {
        argv[argc++] = strdup(token);
        token = strtok(NULL, " \t\n");
    }
    argv[argc] = NULL;
    free(line_copy);
    return argv;
}

/**
   * @brief Free the line that was constructed with parse_cmd
   *
   * @param line the line to free
   */
  void cmd_free(char **line) {
    if (line) {
        for (int i = 0; line[i] != NULL; i++) {
            free(line[i]);
        }
        free(line);
    }
}

/**
   * @brief Trim the whitespace from the start and end of a string.
   * For example "   ls -a   " becomes "ls -a". This function modifies
   * the argument line so that all printable chars are moved to the
   * front of the string
   *
   * @param line The line to trim
   * @return The new line with no whitespace
   */
char *trim_white(char *line) {
    char *end;
    while (isspace((unsigned char)*line)) line++;
    if (*line == 0) return line;
    end = line + strlen(line) - 1;
    while (end > line && isspace((unsigned char)*end)) end--;
    end[1] = '\0';
    return line;
}



/**
   * @brief Initialize the shell for use. Allocate all data structures
   * Grab control of the terminal and put the shell in its own
   * process group. NOTE: This function will block until the shell is
   * in its own program group. Attaching a debugger will always cause
   * this function to fail because the debugger maintains control of
   * the subprocess it is debugging.
void sh_init(struct shell *sh) {
    // Initialize the global shell variable
    ::sh = sh;
   * @param sh
   */
void sh_init(struct shell *sh) {
    UNUSED(*sh); //to eliminate warning
    // Initialize shell data structures
    // Put the shell in its own process group
    pid_t pid = getpid();
    if (setpgid(pid, pid) == -1) {
        perror("setpgid");
        exit(EXIT_FAILURE);
    }
    tcsetpgrp(STDIN_FILENO, pid);

    // Set the shell prompt
    sh->prompt = get_prompt("SHELL_PROMPT");
}

/**
    * @brief Destroy shell. Free any allocated memory and resources and exit
    * normally.
    *
    * @param sh
    */
void sh_destroy(struct shell *sh) {
     // Free allocated memory and resources
     if (sh) {
          // Assuming sh has dynamically allocated members, free them here
          // Example: free(sh->some_member);
          // free(sh->another_member);
          
          // Finally, free the shell structure itself if it was dynamically allocated
          free(sh);
     }
}

/**
    * @brief Parse command line args from the user when the shell was launched
    *
    * @param argc Number of args
    * @param argv The arg array
    */
void parse_args(int argc, char **argv) {
     for (int i = 1; i < argc; i++) {
          if (strcmp(argv[i], "--help") == 0) {
                printf("Usage: %s [options]\n", argv[0]);
                printf("Options:\n");
                printf("  --help       Display this help message\n");
                // Add more options here
                exit(0);
          }
          int opt;
        while ((opt = getopt(argc, argv, "v")) != -1) {
            switch (opt) {
                case 'v':
                    printf("Project Version: %d.%d\n", lab_VERSION_MAJOR, lab_VERSION_MINOR);
                    exit(EXIT_SUCCESS);
                default:
                    fprintf(stderr, "Usage: %s [-v]\n", argv[0]);
                    exit(EXIT_FAILURE);
            }
        }
     }
}

/**
    * @brief Terminate shell normally
    * 
    * @param status The exit status
    */
void exit_shell(int status) {
     // Clean up any allocated memory and resources
     // Assuming sh is a global or accessible variable
     sh_destroy(sh);
     exit(status);
}

/**
    * @brief Handle end of input (EOF) to terminate the shell
    */
void handle_eof() {
    if (feof(stdin)) {
        exit_shell(0);
    }
}



/**
    * @brief Initialize the history feature for the shell
    */
void init_history() {
    using_history();
}

/**
    * @brief Add a command to the history
    *
    * @param line The command line to add
    */
void add_history_entry(const char *line) {
    add_history(line);
}

/**
    * @brief Print the history of commands entered
    */
void print_history() {
    HIST_ENTRY **hist_list = history_list();
    if (hist_list) {
        for (int i = 0; hist_list[i]; i++) {
            printf("%d: %s\n", i + 1, hist_list[i]->line);
        }
    }
}

/**
   * @brief Takes an argument list and checks if the first argument is a
   * built in command such as exit, cd, jobs, etc. If the command is a
   * built in command this function will handle the command and then return
   * true. If the first argument is NOT a built in command this function will
   * return false.
   *
   * @param sh The shell
   * @param argv The command to check
   * @return True if the command was a built in command
   */
  bool do_builtin(struct shell *sh, char **argv) {
    if (strcmp(argv[0], "exit") == 0) {
        sh_destroy(sh);
        exit(0);
    } else if (strcmp(argv[0], "cd") == 0) {
        return change_dir(&argv[1]) == 0;
    } else if (strcmp(argv[0], "history") == 0) {
        print_history();
        return true;
    }
    // Add more built-in commands here
    return false;
}

