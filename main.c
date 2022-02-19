#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <malloc.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

void break_string(char *input, char *components[]);

void exit_shell();

void change_directory(char *components[]);

void change_path(char *components[]);

void list_directory();

char *input_line;
char *path[30];

int main(int argc, char *argv[]) {
    size_t buffer_size = 50;
    bool exit = false;
    int line_num = 1;
    char *separated_strings[99];

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
        } else if (strcmp(separated_strings[0], "ls") == 0){

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
    free(input_line);
    exit(0);
}

void change_directory(char *components[]) {
    if (components[1] == NULL || components[2] != NULL) {
        fprintf(stderr,"An error has occurred\n");
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
