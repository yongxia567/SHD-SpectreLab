#ifndef LAB_IPC_H
#define LAB_IPC_H

/********************
 * Lab Parameters *
 ********************/

#define LAB_PAGE_SIZE ((0x1000))

// How many pages should be shared between the client and server?
#define LAB_SHARED_MEMORY_NUM_PAGES ((256))

// Size of the shared memory region in bytes
#define LAB_SHARED_MEMORY_SIZE ((LAB_PAGE_SIZE * LAB_SHARED_MEMORY_NUM_PAGES))

// Maximum secret length (in bytes)
#define LAB_SECRET_MAX_LEN ((64))

/*********************************************
 * Lab Inter-Process Communication structs *
 *********************************************/

// "IPC" is a bit of a misnomer, as we now target the kernel instead of another process

/*
 * lab_command_kind
 * One of the commands we can issue to the server
 */
typedef enum {
	// Non-speculatively leak part of the secret (Part 1)
	COMMAND_PART1,

	// Run the vulnerable method (vulnerable to spectre) (Part 2)
	COMMAND_PART2,

	// Run the second vulnerable method (vulnerable to spectre) but harder! (Part 3)
	COMMAND_PART3
} lab_command_kind;

/*
 * lab_command
 * A command packet for a single action we can request from the kernel
 */
typedef struct lab_command_t {
	// What kind of command is this?
	lab_command_kind kind;

	// Usually this is the user virtual address of the shared memory region
	uint64_t arg1;

	// Usually this is the offset into the secret to access
	uint64_t arg2;
} lab_command;

#endif // LAB_IPC_H
