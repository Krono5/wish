//
// Created by TySto on 2/20/2022.
//

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <wait.h>
#include <fcntl.h>
#include <ctype.h>

#ifndef WISH_H
#define WISH_H

void break_string(char *input_string, char *components[]);

void exit_shell();

void print_error();

void change_directory(char *components[]);

void change_path(char *components[]);

bool check_path(char *check_path, char *components[]);

bool is_redirection(char *components[]);

void restructure_components(char *components[]);

char *format_string(char *input_line, size_t buffer_size);

char *getInput(size_t buffer_size, char *input_line);


#define PATH_SIZE 30

#endif //WISH_H
