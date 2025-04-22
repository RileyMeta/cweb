#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parsefile.h"

bool show_extensions = false;  // Default value

// Simple Config file parser, WIP
void parse_config(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        fprintf(stderr, "Error opening config file: %s\n", filename);
        return;
    }

    char line[256];
    while (fgets(line, sizeof(line), file)) {
        line[strcspn(line, "\n")] = 0;

        if (strncmp(line, "show_extensions=", 16) == 0) {
            if (strcmp(line + 16, "true") == 0) {
                show_extensions = true;
            } else if (strcmp(line + 16, "false") == 0) {
                show_extensions = false;
            }
        }
    }

    fclose(file);
}
