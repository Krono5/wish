#include <fcntl.h>
#include "wish.h"

char *read_string;
char *path[PATH_SIZE];
const char *operator = ">";
char *redirect_args;

enum redirection {
    none,       // no redirection
    standalone, // redirection with spaces
    internal    // redirection without spaces
};

int main(int argc, char *argv[]) {
    size_t buffer_size = 50;
    bool exit = false;
    char *separated_components[buffer_size];
    char command_path[PATH_SIZE];


    // set up initial path
    path[0] = "/bin";
    read_string = malloc(buffer_size * sizeof(char));
    redirect_args = malloc(sizeof(char));

    for (int i = 0; i < buffer_size; ++i) {
        separated_components[i] = NULL;
    }

    // if there is nothing in the command line, attempt to read file and redirect to stdin
    if (argc != 1) {
        stdin = freopen(argv[1], "r", stdin);
        // Could not open file
        if (stdin == NULL) {
            print_error();
            exit_shell();
        }
    }

    while (exit == false) {
        // interactive mode
        if (argc == 1) {
            printf("wish> ");
        }
        // Get input from stdin
        int result = (int) getline(&read_string, &buffer_size, stdin);
        break_string(read_string, separated_components);

        //----------------Checking for redirects----------------
        if (search_redirect(separated_components) == standalone) {

            int i = 0;
            while (strcmp(separated_components[i], operator) != 0) {
                i++;
            }
            if (i == 0 || separated_components[i + 2] != NULL || separated_components[i + 1] == NULL) {
                print_error();
                exit_shell();
            } else {
                redirect_args = realloc(redirect_args, sizeof (separated_components[i+1]));
                strcpy(redirect_args, separated_components[i + 1]);
//                redirect_output(separated_components[i+1]);
            }
        } else if (search_redirect(separated_components) == internal) {
            restructure_components(separated_components);
        }
        //---------------------------------------------------------

        if (strcmp(separated_components[0], "exit") == 0 || result == -1) {
            if (separated_components[1] != NULL) {
                print_error();
            } else {
                exit = true;
            }
        } else if (strcmp(separated_components[0], "cd") == 0) {
            change_directory(separated_components);
        } else if (strcmp(separated_components[0], "path") == 0) {
            change_path(separated_components);
        } else {
            // any other shell command
            pid_t return_pid = fork();
            if (return_pid < 0) {
                // fail to fork
                exit_shell();
            } else if (return_pid == 0) {
                // in the universe of the child
                if (check_path(command_path, separated_components) == false) {
                    print_error();
                }

                int fw = open(redirect_args, O_WRONLY);
                dup2(fw, STDOUT_FILENO);
                dup2(fw, STDERR_FILENO);
                close(fw);

                execv(command_path, separated_components);
                exit = true;
            } else {
                // in the universe of the parent
                wait(NULL);
            }
        }
    }

    exit_shell();
    return EXIT_SUCCESS;
}

/**
 * Break an input_string string into components removing the newline if it exists
 * @param input_string input_string string to break
 * @param components address of where to stash the components
 */
void break_string(char *input_string, char *components[]) {
    char delimiter = ' ';
    int i = 0;
    bool exit = false;

    // remove newline if it exists
    input_string[strcspn(input_string, "\n")] = 0;
    while (exit == false) {
        components[i] = strsep(&input_string, &delimiter);
        if (components[i] == NULL) {
            exit = true;
        } else {
            i++;
        }
    }
}

/**
 * Close and free file pointers and exit gracefully
 */
void exit_shell() {
    fclose(stdin);
    fclose(stdout);
    fclose(stderr);
    free(read_string);
    free(redirect_args);
    exit(EXIT_SUCCESS);
}

/**
 * Print an error message to stderr
 */
void print_error() {
    char *error_message = "An error has occurred\n";
    write(STDERR_FILENO, error_message, strlen(error_message));
}

/**
 * Built in command Change Directory
 * @param components input arguments of which directory to go to
 */
void change_directory(char *components[]) {
    // if no arguments to cd or cd with more than 1 argument
    if (components[1] == NULL || components[2] != NULL) {
        print_error();
    }
    // if directory does not exist
    if (chdir(components[1]) == -1) {
        errno = ENOTDIR;
    }
}

/**
 * Change the existing path variable
 * @param components arguments for the new path to be changed to
 */
void change_path(char *components[]) {
    bool exit = false;
    int i = 0;

    while (exit == false) {
        if (components[i] == NULL) {
            // If nothing is entered into the path, set path to nothing
            path[i] = NULL;
            exit = true;
        } else {
            // Otherwise, set path variable to everything entered in path
            path[i] = malloc(strlen(components[i]));
            strcpy(path[i], components[i]);
            i++;
        }
    }
}

/**
 * Check if an argument exists in the current path
 * @param check_path full path to check for an argument
 * @param components input arguments to check if command can be executed in the path
 * @return true with the full path if exists within path. false if it does not exist in path
 */
bool check_path(char *check_path, char *components[]) {
    for (int i = 0; i < PATH_SIZE; ++i) {
        if (path[i] != NULL) {
            strcpy(check_path, path[i]);
            strcat(check_path, "/");
            strcat(check_path, components[0]);
            if (access(check_path, X_OK) == 0) {
                return true;
            }
        }
    }
    return false;
}

enum redirection search_redirect(char *components[]) {
    int i = 0;
    int num_operators = 0;
    bool internal_flag = false;
    while (components[i] != NULL) {
        if (strcmp(components[i], operator) == 0) {
            num_operators++;
        } else if (strstr(components[i], operator) != NULL) {
            internal_flag = true;
            num_operators++;
        }
        i++;
    }
    if (num_operators == 1) {
        if (internal_flag) {
            return internal;
        } else {
            return standalone;
        }
    } else if (num_operators == 0) {
        return none;
    } else {
        print_error();
        exit_shell();
        return none;
    }
}

void restructure_components(char *components[]) {
    int i = 0;
    while (strstr(components[i], operator) == NULL) {
        i++;
    }
    strcpy(components[i], strtok(components[i], operator));
//    redirect_output(strtok(NULL , operator));
    char* tempstr = strtok(NULL, operator);
    redirect_args = realloc(redirect_args, sizeof(tempstr));
    strcpy(redirect_args, tempstr);
}

//void redirect_output(char *redirect_args) {
//    int fw = creat(redirect_args, O_WRONLY);
//    dup2(fw, STDOUT_FILENO);
//    dup2(fw, STDERR_FILENO);
//    close(fw);
//}
