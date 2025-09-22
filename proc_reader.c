#include "proc_reader.h"
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>


int main(void) {
    // List processes
    list_process_directories();

    // Show system info
    show_system_info();

    // Compare file reading methods
    compare_file_methods();

    // Example: read info for a specific PID
    // read_process_info("1");  // uncomment to test PID 1

    return 0;
}
int is_number(const char *s) {
    if (!s || !*s) return 0;
    for (int i = 0; s[i]; i++) {
        if (!isdigit((unsigned char)s[i])) return 0;
    }
    return 1;
}

int list_process_directories(void) {

DIR *dir;
    struct dirent *entry;
    int count = 0;

    dir = opendir("/proc");
    if (!dir) {
        perror("opendir /proc");
        return -1;  // error
    }
    

    printf("Process directories in /proc:\n");
    printf("%-8s %-20s\n", "PID", "Type");
    printf("%-8s %-20s\n", "---", "----");
    
     while ((entry = readdir(dir)) != NULL) {
        if (is_number(entry->d_name)) {
            printf("%-8s %-20s\n", entry->d_name, "process");
            count++;
        }
    }

    if (closedir(dir) != 0) {
        perror("closedir /proc");
        return -1;
    }

    printf("\nTotal processes found: %d\n", count);
    return 0; // Replace with proper error handling
}

int read_process_info(const char* pid) {
    char filepath[256];
    int fd;
    ssize_t n;
    char buf[8192];

    // Create the path to /proc/[pid]/status
    snprintf(filepath, sizeof(filepath), "/proc/%s/status", pid);

    printf("\n--- Process Information for PID %s ---\n", pid);

    fd = open(filepath, O_RDONLY);
    if (fd < 0) {
        perror("open status");
        return -1;
    }
    n = read(fd, buf, sizeof(buf) - 1);
    if (n < 0) {
        perror("read status");
        close(fd);
        return -1;
    }
    buf[n] = '\0';
    close(fd);

    // Print selected fields from /proc/[pid]/status
    char *saveptr;
    char *line = strtok_r(buf, "\n", &saveptr);
    while (line) {
        if (strncmp(line, "Name:", 5) == 0 ||
            strncmp(line, "State:", 6) == 0 ||
            strncmp(line, "Tgid:", 5) == 0 ||
            strncmp(line, "Ngid:", 5) == 0 ||
            strncmp(line, "Pid:", 4) == 0 ||
            strncmp(line, "PPid:", 5) == 0 ||
            strncmp(line, "TracerPid:", 10) == 0 ||
            strncmp(line, "Uid:", 4) == 0 ||
            strncmp(line, "Gid:", 4) == 0) {
            printf("%s\n", line);
        }
        line = strtok_r(NULL, "\n", &saveptr);
    }

    // Create the path to /proc/[pid]/cmdline
    snprintf(filepath, sizeof(filepath), "/proc/%s/cmdline", pid);

    printf("\n--- Command Line ---\n");

    fd = open(filepath, O_RDONLY);
    if (fd < 0) {
        perror("open cmdline");
        return -1;
    }
    n = read(fd, buf, sizeof(buf) - 1);
    if (n > 0) {
        buf[n] = '\0';
        for (ssize_t i = 0; i < n; i++) {
            if (buf[i] == '\0') buf[i] = ' ';
        }
        printf("%s\n", buf);
    } else {
        printf("(empty)\n");
    }
    close(fd);

    printf("\n"); // readability
    return 0; // Replace with proper error handling
}

int show_system_info(void) {
    const int MAX_LINES = 10;
    char line[512];
    FILE *f;

    printf("\n--- CPU Information (first %d lines) ---\n", MAX_LINES);

    f = fopen("/proc/cpuinfo", "r");
    if (!f) {
        perror("fopen /proc/cpuinfo");
        return -1;
    }

    for (int i = 0; i < MAX_LINES && fgets(line, sizeof(line), f); i++) {
        printf("%s", line);
    }
    fclose(f);

    printf("\n--- Memory Information (first %d lines) ---\n", MAX_LINES);

    f = fopen("/proc/meminfo", "r");
    if (!f) {
        perror("fopen /proc/meminfo");
        return -1;
    }

    for (int i = 0; i < MAX_LINES && fgets(line, sizeof(line), f); i++) {
        printf("%s", line);
    }
    fclose(f);

    return 0; // Replace with proper error handling
}


void compare_file_methods(void) {
    const char* test_file = "/proc/version";

    printf("Comparing file reading methods for: %s\n\n", test_file);

    printf("=== Method 1: Using System Calls ===\n");
    read_file_with_syscalls(test_file);

    printf("\n=== Method 2: Using Library Functions ===\n");
    read_file_with_library(test_file);

    printf("\nNOTE: Run this program with strace to see the difference!\n");
    printf("Example: strace -e trace=openat,read,write,close ./lab2\n");
}

// Read file using system calls
int read_file_with_syscalls(const char* filename) {
    int fd;
    char buffer[1024];
    ssize_t bytes_read;

    // Open the file
    fd = open(filename, O_RDONLY);
    if (fd == -1) {
        perror("open");
        return -1;
    }

    // Read in a loop
    while ((bytes_read = read(fd, buffer, sizeof(buffer) - 1)) > 0) {
        buffer[bytes_read] = '\0';  // null-terminate
        printf("%s", buffer);
    }

    if (bytes_read == -1) {
        perror("read");
        close(fd);
        return -1;
    }

    if (close(fd) == -1) {
        perror("close");
        return -1;
    }

    return 0;
}

// Read file using C library
int read_file_with_library(const char* filename) {
    FILE *fp;
    char buffer[1024];

    fp = fopen(filename, "r");
    if (fp == NULL) {
        perror("fopen");
        return -1;
    }

    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        printf("%s", buffer);
    }

    if (ferror(fp)) {
        perror("fgets");
        fclose(fp);
        return -1;
    }

    if (fclose(fp) != 0) {
        perror("fclose");
        return -1;
    }

    return 0;
}

// Check if string contains only digits

