#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/time.h>

void copy_file(const char *src, const char *dest, int buf_size){
    // open the source file
    // read only permission
    int src_file = open(src, O_RDONLY);
    if (src_file == -1){
        perror("Cannot open source file");
        exit(1);
    }

    // open destination file 
    // overwrite if exist 
    int dest_file = open(dest, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    if (dest_file == -1){
        perror("Cannot open destination file");
        close(src_file);
        exit(2);
    }

    // allocate a buffer
    char *buffer = (char *)malloc(buf_size);
    if (buffer == NULL){
        perror("cannot allocate memory for buffer");
        close(src_file);
        close(dest_file);
        exit(3);
    }

    // read from source file and write to destination file 
    size_t b_read, b_write;

    // loop through contents 
    while ((b_read = read(src_file, buffer, buf_size)) > 0){
        b_write = write(dest_file, buffer, b_read);
        if (b_write != b_read){
            perror("Cannot write to destination file");
            free(buffer);
            close(src_file);
            close(dest_file);
            exit(4);
        }
    }
    if (b_read == -1){
        perror("Cannot read from source file");
    }

    close(src_file);
    close(dest_file);
    free(buffer);
}

// get the user and system time 
long calculate_time(struct timeval *start, struct timeval *end) {
    return ((end->tv_sec - start->tv_sec) * 1000000L + end->tv_usec) - start->tv_usec;
}

int main(int argc, char *argv[]) {
    if (argc != 3){
        fprintf(stderr, "Usage: %s <source_file> <destination_file>\n", argv[0]);
        exit(1);
    }

    // open a file to record values 
    FILE *log_file = fopen("/home/teresayuexinzhang/CSCE313_Labs/HW/Union_FS/timing_log.txt", "a");
    if (log_file == NULL){
        perror("Cannot open log file");
        exit(3);
    }

    // write to log file 
    fprintf(log_file, "Buffer size (bytes)\tTotal Time (us)\n");

    // test all the values and list them in a txt file 
    int buf_size = 1;
    for(int i = 0; i < 13; i++){
        buf_size = 2 << i;

        struct timeval start, end;
        gettimeofday(&start, NULL); // start time 

        copy_file(argv[1], argv[2], buf_size);

        gettimeofday(&end, NULL);// end time 

        // total time 
        long elapsed_time = calculate_time(&start, &end);

        // log into the file 
        fprintf(log_file, "%d\t%ld\n", buf_size, elapsed_time);
        printf("Buffer size: %d bytes, Time: %ld us\n", buf_size, elapsed_time);
    }

    // close the log file 
    fclose(log_file);

    return 0;
}


// #include <sys/types.h>
// #include <sys/wait.h>
// #include <unistd.h>
// #include <fcntl.h>
// #include <stdio.h>
// #include <stdlib.h>
// #include <sys/stat.h>
// #include <sys/time.h>
// #include <string.h> // Include string.h for using fstat

// void copy_file(const char *src, const char *dest, int buf_size) {
//     // Open the source file with read-only permission
//     int src_file = open(src, O_RDONLY);
//     if (src_file == -1) {
//         perror("Cannot open source file");
//         exit(1);
//     }

//     // Open destination file with overwrite if it exists
//     int dest_file = open(dest, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
//     if (dest_file == -1) {
//         perror("Cannot open destination file");
//         close(src_file);
//         exit(2);
//     }

//     // Allocate a buffer
//     char *buffer = (char *)malloc(buf_size);
//     if (buffer == NULL) {
//         perror("Cannot allocate memory for buffer");
//         close(src_file);
//         close(dest_file);
//         exit(3);
//     }

//     // Read from source file and write to destination file
//     size_t b_read, b_write;

//     // Loop through contents
//     while ((b_read = read(src_file, buffer, buf_size)) > 0) {
//         b_write = write(dest_file, buffer, b_read); // Write the number of bytes read
//         if (b_write != b_read) {
//             perror("Cannot write to destination file");
//             free(buffer);
//             close(src_file);
//             close(dest_file);
//             exit(4);
//         }
//     }
//     if (b_read == -1) {
//         perror("Cannot read from source file");
//     }

//     // Close files and free buffer
//     close(src_file);
//     close(dest_file);
//     free(buffer);
// }

// // Get the user and system time 
// long calculate_time(struct timeval *start, struct timeval *end) {
//     return ((end->tv_sec - start->tv_sec) * 1000000L + end->tv_usec) - start->tv_usec;
// }

// // Check if the log file is empty
// int is_log_file_empty(const char *file_path) {
//     struct stat st;
//     if (stat(file_path, &st) == 0) {
//         return st.st_size == 0; // Return true if the file is empty
//     }
//     return 1; // If the file does not exist, consider it "empty"
// }

// int main(int argc, char *argv[]) {
//     if (argc != 3) {
//         fprintf(stderr, "Usage: %s <source_file> <destination_file>\n", argv[0]);
//         exit(1);
//     }

//     // Open a file to record values in append mode
//     const char *log_file_path = "/home/teresayuexinzhang/CSCE313_Labs/HW/Union_FS/timing_log.txt";
//     FILE *log_file = fopen(log_file_path, "a");
//     if (log_file == NULL) {
//         perror("Cannot open log file");
//         exit(3);
//     }

//     // Write header to log file if it's empty
//     if (is_log_file_empty(log_file_path)) {
//         fprintf(log_file, "Buffer size (bytes)\tTotal Time (us)\n");
//     }

//     // Set the buffer size to 4KB (4096 bytes)
//     int buf_size = 4096;

//     struct timeval start, end;
//     gettimeofday(&start, NULL); // Start time 

//     copy_file(argv[1], argv[2], buf_size); // Perform the copy operation

//     gettimeofday(&end, NULL); // End time 

//     // Total time 
//     long elapsed_time = calculate_time(&start, &end);

//     // Log into the file
//     fprintf(log_file, "%d\t%ld\n", buf_size, elapsed_time);
//     printf("Buffer size: %d bytes, Time: %ld us\n", buf_size, elapsed_time);

//     // Close the log file 
//     fclose(log_file);

//     return 0;
// }
