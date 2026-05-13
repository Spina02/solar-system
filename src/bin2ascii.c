#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <limits.h>

void bin2ascii(char* filename_bin, int ndt) {
    // check if the file has .bin extension
    const char* dot = strrchr(filename_bin, '.');
    if (!dot || strcmp(dot, ".bin") != 0) {
        fprintf(stderr, "Error: input file must be a .bin file\n");
        return;
    }

    // Prepare ASCII filename buffer
    char filename_ascii[128];

    // Remove extension and append .txt
    size_t len = (size_t)(dot - filename_bin);
    if (len > sizeof(filename_ascii) - 5) { // 4 for ".txt", 1 for '\0'
        fprintf(stderr, "Filename too long\n");
        return;
    }

    // Open the binary file for reading
    FILE* bin_file = fopen(filename_bin, "rb");
    if (bin_file == NULL) {
        perror("fopen");
        return;
    }

    // copy the name without file type suffix
    strncpy(filename_ascii, filename_bin, len);
    filename_ascii[len] = '\0';
    strcat(filename_ascii, ".txt");

    // Open the ascii file for writing
    FILE* ascii_file = fopen(filename_ascii, "w");
    if (ascii_file == NULL) {
        perror("fopen");
        fclose(bin_file);
        return;
    }

    double bin_line[7];
    char ascii_line[256];
    size_t nread, nwrite;

    int line = 0;
    while ((nread = fread(bin_line, sizeof(double), 7, bin_file)) == 7) {
        if ((line++ % ndt)) continue;
        int n = snprintf(ascii_line, sizeof(ascii_line), "%lf %lf %lf %lf %lf %lf %lf\n", bin_line[0], bin_line[1], bin_line[2], bin_line[3], bin_line[4], bin_line[5], bin_line[6]);
        if (n < 0 || (size_t)n >= sizeof(ascii_line)) {
            fprintf(stderr, "error while formatting ascii line\n");
            fclose(bin_file);
            fclose(ascii_file);
            return;
        }
        nwrite = fwrite(ascii_line, sizeof(char), (size_t)n, ascii_file);
        if (nwrite != (size_t)n) {
            printf("error while writing on file\n");
            fclose(bin_file);
            fclose(ascii_file);
            return;
        }
    }
    // write the last line
    int n = snprintf(ascii_line, sizeof(ascii_line), "%lf %lf %lf %lf %lf %lf %lf\n", bin_line[0], bin_line[1], bin_line[2], bin_line[3], bin_line[4], bin_line[5], bin_line[6]);
    if (n < 0 || (size_t)n >= sizeof(ascii_line)) {
        fprintf(stderr, "error while formatting ascii line\n");
        fclose(bin_file);
        fclose(ascii_file);
        return;
    }
    nwrite = fwrite(ascii_line, sizeof(char), (size_t)n, ascii_file);
    if (nwrite != (size_t)n) {
        printf("error while writing on file\n");
        fclose(bin_file);
        fclose(ascii_file);
        return;
    }

    fclose(bin_file);
    fclose(ascii_file);
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        printf("Missing arguments, usage:\n\tbin2ascii Ndt folder [filename]\nIf no filename is specified, all the folder will be converted\n");
        return 1;
    }

    int ndt = atoi(argv[1]);
    if (ndt <= 0) {
        fprintf(stderr, "Ndt must be > 0\n");
        return 1;
    }

    char* folder = argv[2];

    char path[PATH_MAX];

    if (argc == 4) {
        char* filename = argv[3];
        int n = snprintf(path, sizeof(path), "%s/%s", folder, filename);
        if (n < 0 || (size_t)n >= sizeof(path)) {
            fprintf(stderr, "Path too long: %s/%s\n", folder, filename);
            return 1;
        }
        bin2ascii(path, ndt);
        return 0;
    }

    DIR* dir = opendir(folder);
    struct dirent * entry;

    if (!dir) {
        perror("opendir");
        return 1;
    }

    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;
        const char* ext = strrchr(entry->d_name, '.');
        if (!ext || strcmp(ext, ".bin") != 0)
            continue; // skip non-binary files
        int n = snprintf(path, sizeof(path), "%s/%s", folder, entry->d_name);
        if (n < 0 || (size_t)n >= sizeof(path)) {
            fprintf(stderr, "Path too long: %s/%s\n", folder, entry->d_name);
            continue;
        }
        bin2ascii(path, ndt);
    }
    
    closedir(dir);
    return 0;
}