#include "vma.h"

int main(void)
{
	arena_t *arena;
	while (1) {
		char command[16];
		scanf("%s", command);
		uint64_t arena_size, address, size;
		if (strncmp(command, "ALLOC_ARENA", 11) == 0) {

			scanf("%lu", &arena_size);
			arena = alloc_arena(arena_size);

		} else if (strncmp(command, "DEALLOC_ARENA", 13) == 0) {

			dealloc_arena(arena);
			break;

		} else if (strncmp(command, "ALLOC_BLOCK", 11) == 0) {

			scanf("%lu %lu", &address, &size);
			alloc_block(arena, address, size);
			
		} else if (strncmp(command, "FREE_BLOCK", 10) == 0) {

			scanf("%lu", &address);
			free_block(arena, address);

		} else if (strncmp(command, "READ", 4) == 0) {

			scanf("%lu %lu", &address, &size);
			read(arena, address, size);

		} else if (strcmp(command, "WRITE") == 0) {

			scanf("%lu %lu", &address, &size);
			int8_t *data = malloc(size * sizeof(char));

			for (uint64_t i = 0; i < size; i++) {
				data[i] = getc(stdin);
				if (i == 0 && data[i] != '\n')
					data[i] = getc(stdin);
			}

			write(arena, address, size, data);
			free(data);

		} else if (strncmp(command, "PMAP", 4) == 0) {

			pmap(arena);

		} else if (strncmp(command, "MPROTECT", 8) == 0) {

			scanf("%lu%*c", &address);
			int8_t permission[100];

			fgets((char *)permission, sizeof(permission), stdin);
			mprotect(arena, address, permission);

		} else {

			printf("Invalid command. Please try again.\n");

		}
	}
	
	return 0;
}
