#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include "memsim.h"

static int is_space(char c)
{
    return c == ' ' || c == '\t' || c == '\r' || c == '\n';
}

int *read_references(const char *path, int *count)
{
    FILE *fp = fopen(path, "r");
    if (fp == NULL) {
        perror(path);
        exit(EXIT_FAILURE);
    }

    int cap = 1024;
    int n = 0;
    int *refs = malloc(cap * sizeof(int));
    if (refs == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    char line[256];
    int line_no = 0;
    while (fgets(line, sizeof(line), fp) != NULL) {
        line_no++;

        char *p = line;
        while (is_space(*p)) p++;
        if (*p == '\0') continue;

        char *end;
        errno = 0;
        long val = strtol(p, &end, 10);
        while (is_space(*end)) end++;
        if (errno != 0 || end == p || *end != '\0') {
            fprintf(stderr, "%s:%d: not an integer address: %s", path, line_no, line);
            exit(EXIT_FAILURE);
        }

        if (n == cap) {
            cap *= 2;
            int *bigger = realloc(refs, cap * sizeof(int));
            if (bigger == NULL) {
                perror("realloc");
                exit(EXIT_FAILURE);
            }
            refs = bigger;
        }
        refs[n++] = (int)val;
    }

    fclose(fp);
    *count = n;
    return refs;
}