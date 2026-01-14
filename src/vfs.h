#ifndef VFS_H
#define VFS_H

#define MAX_FILES 16
#define MAX_NAME 32
#define MAX_CONTENT 128

typedef struct {
    char name[MAX_NAME];
    char content[MAX_CONTENT];
    int used;
} File;

extern File vfs[MAX_FILES];

void vfs_init();
int vfs_touch(const char *name);
int vfs_write(const char *name, const char *text);
const char* vfs_read(const char *name);
void vfs_ls();

#endif
