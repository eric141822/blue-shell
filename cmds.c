#include "cmds.h"

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <getopt.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <time.h>

char *custom_cmds[] = {"exit", "help", "hello", "cflags", "mkdir", "rmdir", "ls", "cd"};

void clearScreen() {
    printf("\033[H\033[J");
}

void printBlueShell(int state) {
    clearScreen();
    switch (state) {
        case 0:
            printf("\033[34m      ^      \033[0m\n");
            printf("\033[34m     / \\     \033[0m\n");
            printf("\033[34m <---------> \033[0m\n");
            printf("\033[34m     \\ /     \033[0m\n");
            printf("\033[34m      v      \033[0m\n");
            break;
        case 1:
            printf("\033[34m <---------> \033[0m\n");
            printf("\033[34m     / \\     \033[0m\n");
            printf("\033[34m      ^      \033[0m\n");
            printf("\033[34m     \\ /     \033[0m\n");
            printf("\033[34m <---------> \033[0m\n");
            break;
        case 2:
            printf("\033[34m      v      \033[0m\n");
            printf("\033[34m     \\ /     \033[0m\n");
            printf("\033[34m <---------> \033[0m\n");
            printf("\033[34m     / \\     \033[0m\n");
            printf("\033[34m      ^      \033[0m\n");
            break;
        case 3:
            printf("\033[34m <---------> \033[0m\n");
            printf("\033[34m     \\ /     \033[0m\n");
            printf("\033[34m      v      \033[0m\n");
            printf("\033[34m     / \\     \033[0m\n");
            printf("\033[34m <---------> \033[0m\n");
            break;
    }
}

int num_custom_cmds()
{
    return sizeof(custom_cmds) / sizeof(char *);
}

int shell_cd(char **argv)
{
    if (argv[1] == NULL)
    {
        fprintf(stderr, "cd: requires an argument\n");
    }
    else
    {
        if (chdir(argv[1]) != 0)
        {
            fprintf(stderr, "cd: directory does not exist\n");
        }
    }
    return 1;
}

int shell_exit(void)
{
    return 0;
}
int shell_help(void)
{
    printf("Welcome to Blueshell!\n");
    printf("Custom commands supported:\n");
    for (int i = 0; i < num_custom_cmds(); i++)
    {
        printf("%s\n", custom_cmds[i]);
    }
    return 1;
}

int shell_hello(char **argv)
{
    if (argv[1] == NULL)
    {
        printf("cmd: hello requires an argument, how can I say hello without your name? :)\n");
    }
    else
    {
        printf("Hello, %s!\n", argv[1]);
    }
    return 1;
}

int shell_cflags(char **argv)
{
    if (argv[1] == NULL)
    {
        fprintf(stderr, "cflags: requires an argument\n");
    }
    else
    {
        if (strcasecmp(argv[1], "C") == 0)
        {
            printf("-Wall -Wextra -Wundef -Wshadow -Wpointer-arith -Wcast-align -pedantic -O3\n");
        }
        else if (strcasecmp(argv[1], "C++") == 0 || strcasecmp(argv[1], "CXX") == 0 || strcasecmp(argv[1], "CPP") == 0)
        {
            printf("-Wall -pedantic -Wshadow -O3 -std=c++17\n");
            printf("Add -g for debug symbols\n");
        }
        else
        {
            fprintf(stderr, "cflags: invalid argument\n");
        }
    }

    return 1;
}

int shell_mkdir(char **argv)
{
    if (argv[1] == NULL)
    {
        fprintf(stderr, "mkdir: requires an argument\n");
    }

    struct stat st = {0};

    if (stat(argv[1], &st) == -1)
    {
        mkdir(argv[1], 0700);
    }
    else
    {
        fprintf(stderr, "mkdir: directory already exists\n");
    }

    return 1;
}

int shell_rmdir(char **argv)
{
    if (argv[1] == NULL)
    {
        fprintf(stderr, "rmdir: requires an argument\n");
    }

    struct stat st = {0};

    if (stat(argv[1], &st) == -1)
    {
        fprintf(stderr, "rmdir: directory does not exist\n");
    }
    else
    {
        rmdir(argv[1]);
    }
    return 1;
}

int shell_ls(char **argv)
{
    // get opts.
    int opt;
    int lflag = 0;
    int argc;
    for (argc = 0; argv[argc] != NULL; argc++)
        ;
    while ((opt = getopt(argc, argv, "l")) != -1)
    {
        switch (opt)
        {
        case 'l':
            lflag = 1;
            break;
        default:
            fprintf(stderr, "ls: invalid option\n");
            return 1;
        }
    }

    char *path = argv[optind];
    // reset optind.
    optind = 1;
    struct stat st;

    if (stat(path, &st) == -1)
    {
        fprintf(stderr, "ls: directory does not exist\n");
        return 1;
    }
    // get dir.
    DIR *dr = opendir(path);
    struct dirent *de;

    if (dr)
    {
        while ((de = readdir(dr)) != NULL)
        {
            if (lflag)
            {
                struct stat fst;
                stat(de->d_name, &fst);
                printf((S_ISDIR(fst.st_mode)) ? "d" : "-");
                printf((fst.st_mode & S_IRUSR) ? "r" : "-");
                printf((fst.st_mode & S_IWUSR) ? "w" : "-");
                printf((fst.st_mode & S_IXUSR) ? "x" : "-");
                printf((fst.st_mode & S_IRGRP) ? "r" : "-");
                printf((fst.st_mode & S_IWGRP) ? "w" : "-");
                printf((fst.st_mode & S_IXGRP) ? "x" : "-");
                printf((fst.st_mode & S_IROTH) ? "r" : "-");
                printf((fst.st_mode & S_IWOTH) ? "w" : "-");
                printf((fst.st_mode & S_IXOTH) ? "x" : "-");
                printf("\t%ld", fst.st_size);
                // remove \n from ctime.
                char *tstr = ctime(&fst.st_ctime);
                if (tstr[strlen(tstr) - 1] == '\n')
                {
                    tstr[strlen(tstr) - 1] = '\0';
                }
                printf("\t%s", tstr);
                printf((S_ISDIR(fst.st_mode)) ? "\t\033[34m%s\033[0m\n" : "\t%s\n", de->d_name);
            }
            else
            {
                printf("%s\n", de->d_name);
            }
        }
        closedir(dr);
    }

    return 1;
}


int shell_blue_shell(void) {
    int state = 0;

    for (int i = 0; i < 50; i++) {
        printBlueShell(state);
        usleep(500000);  // Delay for 500ms
        state = (state + 1) % 4;
    }

    return 1;
}
