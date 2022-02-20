#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <malloc.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <wait.h>

void break_string(char *input, char *components[]);

void exit_shell();

void print_error();

void change_directory(char *components[]);

void change_path(char *components[]);

void check_path(char *command_path, char *components[]);

char *input_line;
char *path[30];
char error_message[30] = "An error has occurred\n";

int main(int argc, char *argv[]) {
    size_t buffer_size = 50;
    bool exit = false;
    int line_num = 1;
    char *separated_strings[99];
    char command_path[50];

    // set up initial path
    path[1] = "/bin";

    input_line = malloc(buffer_size * sizeof(char));

    // if there is nothing in the command line, attempt to read file
    if (argc != 1) {
        stdin = freopen(argv[1], "r", stdin);
    }
    while (exit == false) {
        if (argc == 1) {
            printf("wish> ");
        }
        // Get input from stdin
        int result = (int) getline(&input_line, &buffer_size, stdin);
        break_string(input_line, separated_strings);

        if (strcmp(separated_strings[0], "exit") == 0 || result == -1) {
            exit = true;
        } else if (strcmp(separated_strings[0], "cd") == 0) {
            change_directory(separated_strings);
        } else if (strcmp(separated_strings[0], "path") == 0) {
            change_path(separated_strings);
        } else{
            int rc = fork();

            if(rc < 0){
                // fail to fork
                exit_shell();
            } else if(rc == 0){
                // in the universe of the child
                check_path(command_path, separated_strings);
                execv(command_path, separated_strings);
            } else {
                // in the universe of the parent
                wait(NULL);
            }
        }
    }

    exit_shell();
    return 0;
}

void break_string(char *input, char *components[]) {
    char delimiter = ' ';
    int i = 0;
    bool exit = false;

    // remove newline if it exists
    input[strcspn(input, "\n")] = 0;
    while (exit == false) {
        components[i] = strsep(&input, &delimiter);
        if (components[i] == NULL) {
            exit = true;
        } else {
            i++;
        }
    }
}

void exit_shell() {
    fclose(stdin);
    fclose(stdout);
    fclose(stderr);
    free(input_line);
    exit(0);
}

void print_error() {
    write(STDERR_FILENO, error_message, strlen(error_message));
}

void change_directory(char *components[]) {
    if (components[1] == NULL || components[2] != NULL) {
        print_error();
    }
    if (chdir(components[1]) == -1) {
        errno = ENOTDIR;
    }
}

void change_path(char *components[]) {
    bool exit = false;
    int i = 1;

    while (exit == false) {
        if (components[i] == NULL) {
            exit = true;
        } else {
            path[i] = malloc(strlen(components[i]) + 1);
            strcpy(path[i], components[i]);
            i++;
        }
    }
}

void check_path(char *command_path, char *components[]){

    for (int i = 1; i < 30; ++i) {
        strcpy(command_path, path[i]);
        strcat(command_path, "/");
        strcat(command_path, components[0]);
        if(access(command_path, X_OK) == 0){
            return;
        }
    }
    printf("nothing works");
}
