#include <string.h>
#include <unistd.h>
#include "vfs.h"

File vfs[MAX_FILES];

void vfs_init() {
    for (int i = 0; i < MAX_FILES; i++) {
        vfs[i].used = 0;
    }
}

// Find file index by name
int vfs_find(const char *name) {
    for (int i = 0; i < MAX_FILES; i++) {
        if (vfs[i].used && strcmp(vfs[i].name, name) == 0) return i;
    }
    return -1;
}

// Create or overwrite a file
int vfs_touch(const char *name) {
    int idx = vfs_find(name);
    if (idx >= 0) return idx; // already exists

    for (int i = 0; i < MAX_FILES; i++) {
        if (!vfs[i].used) {
            vfs[i].used = 1;
            strncpy(vfs[i].name, name, MAX_NAME-1);
            vfs[i].name[MAX_NAME-1] = '\0';
            vfs[i].content[0] = '\0';
            return i;
        }
    }
    return -1; // no space left
}

// Write content to file
int vfs_write(const char *name, const char *text) {
    int idx = vfs_find(name);
    if (idx < 0) return -1;
    strncpy(vfs[idx].content, text, MAX_CONTENT-1);
    vfs[idx].content[MAX_CONTENT-1] = '\0';
    return 0;
}

// Read file content
const char* vfs_read(const char *name) {
    int idx = vfs_find(name);
    if (idx < 0) return NULL;
    return vfs[idx].content;
}

// List all files
void vfs_ls() {
    for (int i = 0; i < MAX_FILES; i++) {
        if (vfs[i].used) {
            write(1, vfs[i].name, strlen(vfs[i].name));
            write(1, "\n", 1);
        }
    }
}
