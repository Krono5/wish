#include "wish.h"

char *read_string;
char *path[PATH_SIZE];
const char *operator = ">";
char *redirect_args;

int main(int argc, char *argv[]) {
    size_t buffer_size = 50;
    bool exit = false;
    char *separated_components[buffer_size];
    char command_path[PATH_SIZE];

    // set up initial path
    path[0] = "/bin";
    read_string = malloc(buffer_size * sizeof(char));

    for (int i = 0; i < buffer_size; ++i) {
        separated_components[i] = NULL;
    }

    // if there is nothing in the command line, attempt to read file and redirect to stdin
    if (argc != 1) {
        FILE * input_file = NULL;
        input_file = freopen(argv[1], "r", stdin);
        if(argv[2] != NULL){
            // Multiple batch files
            print_error();
            return (EXIT_FAILURE);
        }else if (input_file == NULL) {
            // Could not open file
            print_error();
            return (EXIT_FAILURE);
        } else{
            stdin = input_file;
        }
    }

    while (exit == false) {
        // interactive mode
        if (argc == 1) {
            printf("wish> ");
        }
        // Get input from stdin

        char* formattedString = format_string(read_string, buffer_size);

        break_string(formattedString, separated_components);
        //----------------Checking for redirects----------------
        bool redirection = is_redirection(separated_components);
        //---------------------------------------------------------

        if (strcmp(separated_components[0], "exit") == 0) {
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
            if (check_path(command_path, separated_components) == false) {
                print_error();
            }
            pid_t return_pid = fork();
            if (return_pid < 0) {
                // fail to fork
                exit_shell();
            } else if (return_pid == 0) {
                // in the universe of the child
                if (redirection) {
                    creat(redirect_args, S_IRWXU);
                }
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

int getInput(size_t buffer_size){
    return (int) getline(&read_string, &buffer_size, stdin);
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
        } else if (strcmp(components[i], "") == 0) {
            components[i] = NULL;
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
    free(redirect_args);
    free(read_string);
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

bool is_redirection(char *components[]) {
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
            // SPACES IN BETWEEN >
            restructure_components(components);
        } else {
            // NO SPACES IN BETWEEN >
            int index = 0;
            while (strcmp(components[index], operator) != 0) {
                index++;
            }
            if (index == 0 || components[index + 2] != NULL || components[index + 1] == NULL) {
                print_error();
                exit_shell();
            } else {
                redirect_args = realloc(redirect_args, sizeof(components[index + 1]));
                strcpy(redirect_args, components[index + 1]);
            }
        }
        return true;
    } else if (num_operators == 0) {
        return false;
    } else {
        print_error();
        exit_shell();
        return false;
    }
}

void restructure_components(char *components[]) {
    int i = 0;
    while (strstr(components[i], operator) == NULL) {
        i++;
    }
    strcpy(components[i], strtok(components[i], operator));
    char *tempstr = strtok(NULL, operator);
    redirect_args = realloc(redirect_args, sizeof(tempstr));
    strcpy(redirect_args, tempstr);
}

char * format_string(char* input_line, size_t buffer_size){
    bool exit = false;
    int stringIndex;
    while (exit == false){
        stringIndex = 0;
        int result = getInput(buffer_size);
        if(result == -1){
            exit_shell();
        }
        while (isspace(input_line[stringIndex]) != 0){
            stringIndex++;
        }
        if (stringIndex != strlen(input_line)) {
            exit = true;
        }
    }
    return input_line + stringIndex;
}