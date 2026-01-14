#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include "vfs.h"

#define BUF_SIZE 128
#define HISTORY_SIZE 10

char history[HISTORY_SIZE][BUF_SIZE];
int history_index = 0;
int history_pos = -1;  // Keeps track of current history position

void handle_arrow_up(char *buf) {
    if (history_pos > 0) {
        history_pos--;
        strncpy(buf, history[history_pos], BUF_SIZE);
        buf[BUF_SIZE - 1] = '\0';  // Ensure null-terminated
    }
}

void handle_arrow_down(char *buf) {
    if (history_pos < history_index - 1) {
        history_pos++;
        strncpy(buf, history[history_pos], BUF_SIZE);
        buf[BUF_SIZE - 1] = '\0';  // Ensure null-terminated
    } else {
        buf[0] = '\0';  // Clear buffer when reaching the end of history
    }
}

void handle_tab_autocomplete(char *buf) {
    // We are going to assume the user is trying to autocomplete a filename
    char *cmd = strtok(buf, " ");
    if (cmd && strcmp(cmd, "echo") == 0) {
        // Look for files that match the current text after "echo "
        char *filename = strtok(NULL, " ");
        if (filename) {
            // For simplicity, autocomplete the first match (VFS)
            for (int i = 0; i < MAX_FILES; i++) {
                if (vfs[i].used && strncmp(vfs[i].name, filename, strlen(filename)) == 0) {
                    snprintf(buf, BUF_SIZE, "%s %s", cmd, vfs[i].name);
                    break;
                }
            }
        }
    }
}

int main() {
    char buf[BUF_SIZE];
    const char *prompt = "> ";

    vfs_init();
    write(1, "Welcome to WASI Shell!\n", 23);

    while (1) {
        write(1, prompt, 2);

        // Read user input (simulate raw key presses here)
        ssize_t n = read(0, buf, BUF_SIZE - 1);
        if (n <= 0) break;  // EOF

        buf[n - 1] = '\0';  // Remove newline character

        // Handle arrow key commands (up and down arrow)
        if (n == 1 && buf[0] == 27) {  // Escape sequence starts with 27
            char arrow_buf[3];
            int arrow_n = read(0, arrow_buf, 3);
            if (arrow_n == 3) {
                if (arrow_buf[1] == '[') {
                    if (arrow_buf[2] == 'A') {  // Up arrow
                        handle_arrow_up(buf);
                    } else if (arrow_buf[2] == 'B') {  // Down arrow
                        handle_arrow_down(buf);
                    }
                }
            }
        }

        // Handle tab autocomplete
        if (n == 1 && buf[0] == '\t') {  // Tab key
            handle_tab_autocomplete(buf);
        }

        // If the user presses Enter, save command to history
        if (n == 1 && buf[0] == '\n') {
            // Save the command to history
            if (history_index < HISTORY_SIZE) {
                strncpy(history[history_index], buf, BUF_SIZE);
                history_index++;
            } else {
                // Shift all history up and add new command at the end
                for (int i = 1; i < HISTORY_SIZE; i++) {
                    strncpy(history[i - 1], history[i], BUF_SIZE);
                }
                strncpy(history[HISTORY_SIZE - 1], buf, BUF_SIZE);
            }
            history_pos = history_index;  // Reset position
        }

        // Handle commands like "echo", "ls", "touch", etc.
        if (strncmp(buf, "echo ", 5) == 0) {
            char *arrow = strstr(buf, " > ");
            char *arrow_append = strstr(buf, " >> ");
            if (arrow || arrow_append) {
                char *text = buf + 5;  // after "echo "
                char *filename;
                int append_mode = 0;
                if (arrow_append) {
                    append_mode = 1;
                    *arrow_append = '\0';
                    filename = arrow_append + 4;  // after " >> "
                } else {
                    *arrow = '\0';
                    filename = arrow + 3;  // after " > "
                }
                int idx = vfs_touch(filename);  // create if needed
                if (!append_mode) {
                    vfs_write(filename, text);  // overwrite
                } else {
                    const char *old = vfs_read(filename);
                    char buffer[MAX_CONTENT];
                    if (old) {
                        snprintf(buffer, MAX_CONTENT, "%s\n%s", old, text);
                    } else {
                        snprintf(buffer, MAX_CONTENT, "%s", text);
                    }
                    vfs_write(filename, buffer);
                }
                continue;
            }
        }

        // Handle command parsing (ls, touch, cat, etc.)
        char *cmd = strtok(buf, " ");
        char *arg = strtok(NULL, " ");
        if (!cmd) continue;

        if (strcmp(cmd, "exit") == 0) break;
        else if (strcmp(cmd, "ls") == 0) vfs_ls();
        else if (strcmp(cmd, "touch") == 0 && arg) vfs_touch(arg);
        else if (strcmp(cmd, "cat") == 0 && arg) {
            const char *content = vfs_read(arg);
            if (content) {
                write(1, content, strlen(content));
                write(1, "\n", 1);
            } else {
                write(1, "File not found\n", 15);
            }
        } else {
            write(1, "Unknown command\n", 16);
        }
    }

    write(1, "Goodbye!\n", 9);
    return 0;
}
