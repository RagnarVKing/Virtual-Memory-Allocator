#include "vma.h"

//creeaza o lista
list_t *
dll_create(unsigned int data_size)
{
	list_t *list = malloc(sizeof(list_t));

	list->head = NULL;
	list->data_size = data_size;
	list->size = 0;

	return list;
}

//creeaza un nod
dll_node_t *
create_node(void *new_data, unsigned int data_size)
{
	dll_node_t *node = malloc(sizeof(dll_node_t));
	node->data = malloc(data_size);

	memcpy(node->data, new_data, data_size);

	return node;
}

//returneaza al n-lea nod
dll_node_t *
dll_get_nth_node(list_t *list, unsigned int n)
{
	dll_node_t *current = list->head;
	
	if (list->size == 0)
		return 0;

	for (unsigned int i = 0; i < n % list->size; i++)
		current = current->next;

	return current;
}

//adauga un nod pe pozitia n
void dll_add_nth_node(list_t *list, unsigned int n, const void *new_data)
{
	dll_node_t *node = create_node((void *)new_data, list->data_size);

	if (list->size == 0) {

		list->head = node;
		node->next = node;
		node->prev = node;

		list->size++;

		return;
	}
	if (n == 0) {

		node->next = list->head;
		node->prev = list->head->prev;

		list->head->prev->next = node;
		list->head->prev = node;
		list->head = node;

		list->size++;

		return;
	}
	if (n > list->size)
		n = list->size;

	dll_node_t *current = list->head;

	for (unsigned int i = 0; i < n - 1; i++)
		current = current->next;

	node->prev = current;
	node->next = current->next;

	current->next->prev = node;
	current->next = node;

	list->size++;
}

//sterge un nod de pe pozitia n
dll_node_t *
dll_remove_nth_node(list_t *list, unsigned int n)
{
	if (list->size == 0)
		return 0;

	if (n == 0) {

		dll_node_t *current = list->head;

		list->head = list->head->next;
		list->head->prev = list->head->prev->prev;
		list->head->prev = list->head;

		list->size--;

		return current;
	}
	if (n >= list->size - 1)
		n = list->size - 1;

	dll_node_t *current = list->head;

	for (unsigned int i = 0; i < n - 1; i++)
		current = current->next;

	dll_node_t *aux = current->next;

	current->next->next->prev = current;
	current->next = current->next->next;

	list->size--;

	return aux;
}

//returneaza marimea listei
unsigned int
dll_get_size(list_t *list)
{
	return list->size;
}

//elibereaza memoria alocata pentru lista
void dll_free(list_t **pp_list)
{
	list_t *list = *pp_list;

	if (list->size > 0) {

		dll_node_t *current = list->head;
		dll_node_t *aux;

		for (unsigned int i = 0; i < list->size; i++) {
			aux = current->next;
			free(current->data);
			free(current);
			current = aux;
		}
	}

	free(*pp_list);
}

//afiseaza lista de int-uri
void dll_print_int_list(list_t *list)
{
	dll_node_t *current = list->head;

	for (unsigned int i = 0; i < list->size; i++) {
		printf("%d ", *(int *)current->data);
		current = current->next;
	}
	printf("\n");
}

//afiseaza lista de string-uri
void dll_print_string_list(list_t *list)
{
	dll_node_t *current = list->head->prev;

	for (unsigned int i = 0; i < list->size - 1; i++) {
		printf("%s ", (char *)current->data);
		current = current->prev;
	}
	printf("\n");
}

//creeaza arena
arena_t *alloc_arena(const uint64_t size)
{
	arena_t *arena = malloc(sizeof(arena_t));

	arena->arena_size = size;

	arena->alloc_list = dll_create(sizeof(block_t));

	arena->alloc_list->size = 0;
	arena->alloc_list->head = NULL;

	return arena;
}

//elibereaza toata memoria din arena, cu tot cu arena
void dealloc_arena(arena_t *arena)
{
	if (arena->alloc_list->size) {

		dll_node_t *currentb = arena->alloc_list->head;
		dll_node_t *currentmb;
		dll_node_t *auxb;
		dll_node_t *auxmb;

		for (unsigned int i = 0; i < arena->alloc_list->size; i++) {
			auxb = currentb->next;
			currentmb =
			((list_t *)(((block_t *)currentb->data)->miniblock_list))->head;

			for (unsigned int j = 0;
			j < ((list_t *)((block_t *)currentb->data)->miniblock_list)->size;
			j++) {

				auxmb = currentmb->next;

				free(((miniblock_t *)currentmb->data)->rw_buffer);
				free(currentmb->data);
				free(currentmb);

				currentmb = auxmb;
			}

			free(((block_t *)currentb->data)->miniblock_list);
			free(currentb->data);
			free(currentb);

			currentb = auxb;
		}
	}

	free(arena->alloc_list);
	free(arena);
}

//creeaza un miniblock si, daca este cazul, un block
void alloc_block(arena_t *arena
, const uint64_t address, const uint64_t size)
{
	dll_node_t *currentb = arena->alloc_list->head;

	if (address >= arena->arena_size) {

		printf("The allocated address is outside the size of arena\n");
		return;

	} else if (address + size > arena->arena_size) {

		printf("The end address is past the size of the arena\n");

		return;

	}

	for (unsigned int i = 0; i < arena->alloc_list->size; i++) {
		if (address >= ((block_t *)currentb->data)->start_address &&
			address < ((block_t *)currentb->data)->start_address +
			((block_t *)currentb->data)->size) {
				
			printf("This zone was already allocated.\n");
			return;

		} else if (address < ((block_t *)currentb->data)->start_address &&
			address + size > ((block_t *)currentb->data)->start_address) {

			printf("This zone was already allocated.\n");
			return;

		}

		currentb = currentb->next;

	}

	int left = 0, right = 0, poz;
	currentb = arena->alloc_list->head;

	dll_node_t *leftb;
	dll_node_t *rightb;

	for (unsigned int i = 0; i < arena->alloc_list->size; i++) {
		if (address == ((block_t *)currentb->data)->start_address +
		((block_t *)currentb->data)->size) {

			left++;
			leftb = currentb;

		}
		if (address + size == ((block_t *)currentb->data)->start_address) {

			right++;
			rightb = currentb;

		}

		currentb = currentb->next;

	}

	currentb = arena->alloc_list->head;

	if (left == 0 && right == 0) {
		if (arena->alloc_list->size == 0) {

			block_t *block = malloc(sizeof(block_t));

			block->start_address = address;
			block->size = size;

			block->miniblock_list = dll_create(sizeof(miniblock_t));
			miniblock_t *miniblock = malloc(sizeof(miniblock_t));
			miniblock->rw_buffer = malloc(1);

			miniblock->start_address = address;
			miniblock->size = size;
			miniblock->perm = 6;

			dll_add_nth_node(block->miniblock_list, 0, miniblock);
			dll_add_nth_node(arena->alloc_list, 0, block);

			free(miniblock);
			free(block);

		} else if (address < ((block_t *)currentb->data)->start_address) {

			block_t *block = malloc(sizeof(block_t));

			block->start_address = address;
			block->size = size;

			block->miniblock_list = dll_create(sizeof(miniblock_t));
			miniblock_t *miniblock = malloc(sizeof(miniblock_t));
			miniblock->rw_buffer = malloc(1);

			miniblock->start_address = address;
			miniblock->size = size;
			miniblock->perm = 6;

			dll_add_nth_node(block->miniblock_list, 0, miniblock);
			dll_add_nth_node(arena->alloc_list, 0, block);

			free(miniblock);
			free(block);

		} else {
			for (unsigned int i = 0; i < arena->alloc_list->size; i++) {
				if (address > ((block_t *)currentb->data)->start_address +
				((block_t *)currentb->data)->size)
					poz = i + 1;

				currentb = currentb->next;
			}

			block_t *block = malloc(sizeof(block_t));

			block->start_address = address;
			block->size = size;

			block->miniblock_list = dll_create(sizeof(miniblock_t));
			miniblock_t *miniblock = malloc(sizeof(miniblock_t));
			miniblock->rw_buffer = malloc(1);

			miniblock->start_address = address;
			miniblock->size = size;
			miniblock->perm = 6;

			dll_add_nth_node(block->miniblock_list, 0, miniblock);
			dll_add_nth_node(arena->alloc_list, poz, block);

			free(miniblock);
			free(block);

		}
	} else if (left == 0 && right == 1) {

		((block_t *)rightb->data)->start_address -= size;
		((block_t *)rightb->data)->size += size;

		miniblock_t *miniblock = malloc(sizeof(miniblock_t));
		miniblock->rw_buffer = malloc(1);

		miniblock->start_address = address;
		miniblock->size = size;
		miniblock->perm = 6;
		
		dll_add_nth_node(((block_t *)rightb->data)->miniblock_list
		, 0, miniblock);

		free(miniblock);

	} else if (left == 1 && right == 0) {

		((block_t *)leftb->data)->size += size;

		miniblock_t *miniblock = malloc(sizeof(miniblock_t));
		miniblock->rw_buffer = malloc(1);

		miniblock->start_address = address;
		miniblock->size = size;
		miniblock->perm = 6;

		dll_add_nth_node(((block_t *)leftb->data)->miniblock_list
		, ((list_t *)((block_t *)leftb->data)->miniblock_list)->size + 5
		, miniblock);

		free(miniblock);

	} else if (left == 1 && right == 1) {
		
		((block_t *)leftb->data)->size += size +
		((block_t *)rightb->data)->size;

		leftb->next = rightb->next;
		rightb->next->prev = rightb->prev;

		miniblock_t *miniblock = malloc(sizeof(miniblock_t));
		miniblock->rw_buffer = malloc(1);

		miniblock->start_address = address;
		miniblock->size = size;
		miniblock->perm = 6;

		dll_node_t *miniblock_node = create_node(miniblock
		, sizeof(miniblock_t));

		dll_node_t *leftb_head =
		((list_t *)((block_t *)leftb->data)->miniblock_list)->head;

		leftb_head->prev->next =
		miniblock_node;

		miniblock_node->prev =
		((list_t *)((block_t *)leftb->data)->miniblock_list)->head->prev;

		miniblock_node->next =
		((list_t *)((block_t *)rightb->data)->miniblock_list)->head;

		((list_t *)((block_t *)leftb->data)->miniblock_list)->head->prev =
		((list_t *)((block_t *)rightb->data)->miniblock_list)->head->prev;

		dll_node_t *rightb_head =
		((list_t *)((block_t *)rightb->data)->miniblock_list)->head;

		rightb_head->prev->next =
		((list_t *)((block_t *)leftb->data)->miniblock_list)->head;

		((list_t *)((block_t *)rightb->data)->miniblock_list)->head->prev =
		miniblock_node;
		((list_t *)((block_t *)leftb->data)->miniblock_list)->size +=
		((list_t *)((block_t *)rightb->data)->miniblock_list)->size + 1;

		free(miniblock);
		free(((block_t *)rightb->data)->miniblock_list);
		free(rightb->data);
		free(rightb);

		arena->alloc_list->size--;
	}
}

//sterge un miniblock si, daca este cazul, un block
//, eliberand toata memoria folosita pentru acestea
void free_block(arena_t *arena, const uint64_t address)
{
	dll_node_t *currentb = arena->alloc_list->head;
	dll_node_t *currentmb;

	if (address > arena->arena_size || arena->alloc_list->size == 0) {

		printf("Invalid address for free.\n");

		return;

	}
	if (address < ((block_t *)currentb->data)->start_address ||
		address >= ((block_t *)currentb->prev->data)->start_address +
		((block_t *)currentb->prev->data)->size) {

		printf("Invalid address for free.\n");

		return;

	}
	for (unsigned int i = 0; i < arena->alloc_list->size; i++) {

		currentmb =
		((list_t *)(((block_t *)currentb->data)->miniblock_list))->head;

		for (unsigned int j = 0;
		j < ((list_t *)((block_t *)currentb->data)->miniblock_list)->size;
		j++) {
			if (address == ((miniblock_t *)currentmb->data)->start_address)
				goto jump;
				
			currentmb = currentmb->next;
		}

		currentb = currentb->next;
	}

	printf("Invalid address for free.\n");
	return;

jump:

	for (unsigned int i = 0; i < arena->alloc_list->size; i++) {
		if (address >= ((block_t *)currentb->data)->start_address &&
			address <= ((block_t *)currentb->data)->start_address +
			((block_t *)currentb->data)->size)
			break;

		currentb = currentb->next;

	}

	currentmb =
	((list_t *)(((block_t *)currentb->data)->miniblock_list))->head;

	if (((list_t *)((block_t *)currentb->data)->miniblock_list)->size == 1) {

		free(((miniblock_t *)currentmb->data)->rw_buffer);
		free((miniblock_t *)currentmb->data);
		free(currentmb);
		free(((block_t *)currentb->data)->miniblock_list);

		currentb->next->prev = currentb->prev;
		currentb->prev->next = currentb->next;

		if (currentb == arena->alloc_list->head)
			arena->alloc_list->head = currentb->next;

		arena->alloc_list->size--;

		free(currentb->data);
		free(currentb);

	} else if (address == ((block_t *)currentb->data)->start_address) {

		((block_t *)currentb->data)->start_address =
		((miniblock_t *)currentmb->next->data)->start_address;

		((block_t *)currentb->data)->size -=
		((miniblock_t *)currentmb->data)->size;

		currentmb->next->prev = currentmb->prev;
		currentmb->prev->next = currentmb->next;

		((list_t *)((block_t *)currentb->data)->miniblock_list)->head =
		currentmb->next;

		((list_t *)((block_t *)currentb->data)->miniblock_list)->size--;
		free(((miniblock_t *)currentmb->data)->rw_buffer);
		free(currentmb->data);
		free(currentmb);

	} else if (address ==
	((miniblock_t *)((dll_node_t *)currentmb->prev)->data)->start_address) {

		((block_t *)currentb->data)->size -=
		((miniblock_t *)((dll_node_t *)currentmb->prev)->data)->size;

		currentmb = currentmb->prev;
		currentmb->prev->next = currentmb->next;
		currentmb->next->prev = currentmb->prev;

		((list_t *)((block_t *)currentb->data)->miniblock_list)->size--;
		free(((miniblock_t *)currentmb->data)->rw_buffer);

		free(currentmb->data);
		free(currentmb);

	} else if (address > ((miniblock_t *)currentmb->data)->start_address &&
				address <
				((miniblock_t *)((dll_node_t *)currentmb->prev)->data)->start_address) {

		int sizee;
		dll_node_t *aux = currentmb;
		currentmb = currentmb->next;

		for (unsigned int i = 0;
		i < ((list_t *)((block_t *)currentb->data)->miniblock_list)->size - 2;
		i++) {
			if (((miniblock_t *)currentmb->data)->start_address == address) {

				sizee = i + 2;

				break;
			}

			currentmb = currentmb->next;
		}

		int ssize = ((block_t *)currentb->data)->start_address +
		((block_t *)currentb->data)->size -
		((miniblock_t *)((dll_node_t *)currentmb->next)->data)->start_address;

		((block_t *)currentb->data)->size =
		((miniblock_t *)currentmb->data)->start_address -
		((block_t *)currentb->data)->start_address;

		int ssizee;
		ssizee =
		((list_t *)((block_t *)currentb->data)->miniblock_list)->size - sizee;

		((list_t *)((block_t *)currentb->data)->miniblock_list)->size =
		sizee - 1;

		currentmb->prev->next = aux;
		dll_node_t *auxx = aux->prev;

		aux->prev = currentmb->prev;
		currentmb->next->prev = auxx;

		auxx->next = currentmb->next;
		auxx = auxx->next;

		dll_node_t *node = malloc(sizeof(dll_node_t));
		node->data = malloc(sizeof(block_t));

		((block_t *)node->data)->start_address =
		((miniblock_t *)currentmb->next->data)->start_address;
		((block_t *)node->data)->size = ssize;

		((block_t *)node->data)->miniblock_list =
		dll_create(sizeof(miniblock_t));

		((list_t *)((block_t *)node->data)->miniblock_list)->size = ssizee;
		((list_t *)((block_t *)node->data)->miniblock_list)->head = auxx;

		node->next = currentb->next;

		node->prev = currentb;
		currentb->next->prev = node;

		currentb->next = node;

		arena->alloc_list->size++;

		free(((miniblock_t *)currentmb->data)->rw_buffer);
		free(currentmb->data);
		free(currentmb);
	}
}

//citeste dintr-un block, daca miniblokurile respective au aceasta permisiune
void read(arena_t *arena, uint64_t address, uint64_t size)
{
	dll_node_t *currentb = arena->alloc_list->head;

	if (address > arena->arena_size || arena->alloc_list->size == 0) {

		printf("Invalid address for read.\n");

		return;
	}
	if (address < ((block_t *)currentb->data)->start_address ||
		address >= ((block_t *)currentb->prev->data)->start_address +
		((block_t *)currentb->prev->data)->size) {

		printf("Invalid address for read.\n");

		return;
	}
	for (unsigned int i = 0; i < arena->alloc_list->size - 1; i++) {
		if (address >= ((block_t *)currentb->data)->start_address +
			((block_t *)currentb->data)->size && address <
			((block_t *)currentb->next->data)->start_address) {

			printf("Invalid address for read.\n");

			return;

		}

		currentb = currentb->next;

	}

	currentb = arena->alloc_list->head;

	uint64_t ssize = size;

	for(unsigned int i = 0; i < arena->alloc_list->size; i++)
	{
		if(address >= ((block_t *)currentb->data)->start_address && address < ((block_t *)currentb->data)->start_address + ((block_t *)currentb->data)->size)
			break;

		currentb = currentb->next;

	}

	dll_node_t *currentmb;
	ssize = size;

	currentmb = ((list_t *)((block_t *)currentb->data)->miniblock_list)->head;

	for (unsigned int i = 0;
		i < ((list_t *)((block_t *)currentb->data)->miniblock_list)->size;
		i++) {
		if (address >= ((miniblock_t *)currentmb->data)->start_address &&
			address < ((miniblock_t *)currentmb->data)->start_address +
			((miniblock_t *)currentmb->data)->size) {
			if (ssize <= ((miniblock_t *)currentmb->data)->start_address +
				((miniblock_t *)currentmb->data)->size - address) {
				if (((miniblock_t *)currentmb->data)->perm == 0 ||
					((miniblock_t *)currentmb->data)->perm == 1 ||
					((miniblock_t *)currentmb->data)->perm == 4 ||
					((miniblock_t *)currentmb->data)->perm == 5) {

					printf("Invalid permissions for read.\n");

					return;

				}
				goto cop;

			} else {
				uint64_t aaddress =
				address - ((miniblock_t *)currentmb->data)->start_address;
				int k = 0;

				while (ssize) {
					if (((miniblock_t *)currentmb->data)->perm == 0 ||
						((miniblock_t *)currentmb->data)->perm == 1 ||
						((miniblock_t *)currentmb->data)->perm == 4 ||
						((miniblock_t *)currentmb->data)->perm == 5) {
						printf("Invalid permissions for read.\n");
						return;
					}

					ssize--;

					if (ssize == 0)
						goto cop;

					if (aaddress ==
						((miniblock_t *)currentmb->data)->size - 1) {
						currentmb = currentmb->next;
						aaddress = 0;
						goto nextt;
					}
					aaddress++;
				nextt:
					k++;
				}
				goto cop;
			}
		}

		currentmb = currentmb->next;
	}

cop:

	currentb = arena->alloc_list->head;
	ssize = size;

	for (unsigned int i = 0; i < arena->alloc_list->size; i++) {
		if (address >= ((block_t *)currentb->data)->start_address &&
			address < ((block_t *)currentb->data)->start_address +
			((block_t *)currentb->data)->size) {
			if (address + size > ((block_t *)currentb->data)->start_address +
				((block_t *)currentb->data)->size) {
				ssize = ((block_t *)currentb->data)->start_address +
				((block_t *)currentb->data)->size - address;

				printf("Warning: size was bigger than the block size. ");

				printf("Reading %lu characters.\n", ssize);

				break;
			}
		}

		currentb = currentb->next;
	}

	currentmb = ((list_t *)((block_t *)currentb->data)->miniblock_list)->head;
	for (unsigned int i = 0; i < ((list_t *)((block_t *)currentb->data)->miniblock_list)->size; i++) {
		if (address >= ((miniblock_t *)currentmb->data)->start_address && address < ((miniblock_t *)currentmb->data)->start_address + ((miniblock_t *)currentmb->data)->size) {
			if (ssize <= ((miniblock_t *)currentmb->data)->start_address + ((miniblock_t *)currentmb->data)->size - address) {
				for (unsigned int i = 0, j = address - ((miniblock_t *)currentmb->data)->start_address; i < ssize; i++, j++) {
					printf("%c", ((uint8_t *)((miniblock_t *)currentmb->data)->rw_buffer)[j]);
				}

				printf("\n");

				return;
				
			} else {

				uint64_t aaddress = address - ((miniblock_t *)currentmb->data)->start_address;
				int k = 0;

				while (ssize) {
					printf("%c", ((uint8_t *)((miniblock_t *)currentmb->data)->rw_buffer)[aaddress]);
					ssize--;

					if (ssize == 0) {
						printf("\n");
						return;
					}

					if (aaddress == ((miniblock_t *)currentmb->data)->size - 1) {
						currentmb = currentmb->next;
						aaddress = 0;
						goto next;
					}

					aaddress++;
				next:
					k++;
				}
				printf("\n");
				return;
			}
		}
		currentmb = currentmb->next;
	}
}

//scrie intr-un block, daca miniblokurile respective au aceasta permisiune
void write(arena_t *arena, const uint64_t address, const uint64_t size, int8_t *data)
{
	dll_node_t *currentb = arena->alloc_list->head;

	if (address > arena->arena_size || arena->alloc_list->size == 0) {
		printf("Invalid address for write.\n");
		return;
	} else {
		if (address < ((block_t *)currentb->data)->start_address || address >= ((block_t *)currentb->prev->data)->start_address + ((block_t *)currentb->prev->data)->size) {
			printf("Invalid address for write.\n");
			return;
		}
		for (unsigned int i = 0; i < arena->alloc_list->size - 1; i++) {
			if (address >= ((block_t *)currentb->data)->start_address + ((block_t *)currentb->data)->size && address < ((block_t *)currentb->next->data)->start_address) {
				printf("Invalid address for write.\n");
				return;
			}
			currentb = currentb->next;
		}
	}

	currentb = arena->alloc_list->head;
	uint64_t ssize = size;

	for(unsigned int i = 0; i < arena->alloc_list->size; i++)
	{
		if(address >= ((block_t *)currentb->data)->start_address && address < ((block_t *)currentb->data)->start_address + ((block_t *)currentb->data)->size)
			break;
		currentb = currentb->next;
	}

	dll_node_t *currentmb;
	ssize = size;

	currentmb = ((list_t *)((block_t *)currentb->data)->miniblock_list)->head;

	for (unsigned int i = 0; i < ((list_t *)((block_t *)currentb->data)->miniblock_list)->size; i++) {
		if (address >= ((miniblock_t *)currentmb->data)->start_address && address < ((miniblock_t *)currentmb->data)->start_address + ((miniblock_t *)currentmb->data)->size) {
			if (ssize <= ((miniblock_t *)currentmb->data)->start_address + ((miniblock_t *)currentmb->data)->size - address) {
				if (((miniblock_t *)currentmb->data)->perm == 0 || ((miniblock_t *)currentmb->data)->perm == 1 || ((miniblock_t *)currentmb->data)->perm == 4 || ((miniblock_t *)currentmb->data)->perm == 5) {
					printf("Invalid permissions for write.\n");
					return;
				}
				goto corp;
			} else {
				
				uint64_t aaddress = address - ((miniblock_t *)currentmb->data)->start_address;
				int k = 0;

				while (ssize) {
					if (((miniblock_t *)currentmb->data)->perm == 0 || ((miniblock_t *)currentmb->data)->perm == 1 || ((miniblock_t *)currentmb->data)->perm == 4 || ((miniblock_t *)currentmb->data)->perm == 5) {
						printf("Invalid permissions for write.\n");
						return;
					}
					ssize--;
					if (ssize == 0) {
						goto corp;
					}
					if (aaddress == ((miniblock_t *)currentmb->data)->size - 1) {
						currentmb = currentmb->next;
						aaddress = 0;
						goto nexxt;
					}
					aaddress++;
				nexxt:
					k++;
				}
				goto corp;
			}
		}
		currentmb = currentmb->next;
	}
corp:

	currentb = arena->alloc_list->head;
	ssize = size;
	for (unsigned int i = 0; i < arena->alloc_list->size; i++) {
		if (address >= ((block_t *)currentb->data)->start_address && address < ((block_t *)currentb->data)->start_address + ((block_t *)currentb->data)->size) {
			if (address + size > ((block_t *)currentb->data)->start_address + ((block_t *)currentb->data)->size) {
				ssize = ((block_t *)currentb->data)->start_address + ((block_t *)currentb->data)->size - address;
				printf("Warning: size was bigger than the block size. Writing %lu characters.\n", ssize);
			}
			break;
		}
		currentb = currentb->next;
	}

	currentmb = ((list_t *)((block_t *)currentb->data)->miniblock_list)->head;
	for (unsigned int i = 0; i < ((list_t *)((block_t *)currentb->data)->miniblock_list)->size; i++) {
		if (address >= ((miniblock_t *)currentmb->data)->start_address && address < ((miniblock_t *)currentmb->data)->start_address + ((miniblock_t *)currentmb->data)->size) {
			if (ssize <= ((miniblock_t *)currentmb->data)->start_address + ((miniblock_t *)currentmb->data)->size - address) {
				((miniblock_t *)currentmb->data)->rw_buffer = realloc(((miniblock_t *)currentmb->data)->rw_buffer, ((miniblock_t *)currentmb->data)->size);
				for (unsigned int i = 0, j = address - ((miniblock_t *)currentmb->data)->start_address; i < ssize; i++, j++) {
					((uint8_t *)((miniblock_t *)currentmb->data)->rw_buffer)[j] = data[i];
				}
				return;
			} else {
				uint64_t aaddress = address - ((miniblock_t *)currentmb->data)->start_address;
				int k = 0;
				if (((miniblock_t *)currentmb->data)->size != 1) {
					((miniblock_t *)currentmb->data)->rw_buffer = realloc(((miniblock_t *)currentmb->data)->rw_buffer, ((miniblock_t *)currentmb->data)->size);
				}

				while (ssize) {
					((uint8_t *)((miniblock_t *)currentmb->data)->rw_buffer)[aaddress] = data[k];
					ssize--;
					if (ssize == 0) {
						return;
					}

					if (aaddress == ((miniblock_t *)currentmb->data)->size - 1) {
						currentmb = currentmb->next;
						if (((miniblock_t *)currentmb->data)->size != 1) {
							((miniblock_t *)currentmb->data)->rw_buffer = realloc(((miniblock_t *)currentmb->data)->rw_buffer, ((miniblock_t *)currentmb->data)->size);
						}

						aaddress = 0;
						goto cont;
					}
					aaddress++;
				cont:
					k++;
				}
				return;
			}
		}
		currentmb = currentmb->next;
	}
}

//afiseaza tot ce contine arena
void pmap(const arena_t *arena)
{
	if (arena->alloc_list->size == 0) {
		printf("Total memory: 0x%lX bytes\n", arena->arena_size);
		printf("Free memory: 0x%lX bytes\n", arena->arena_size);
		printf("Number of allocated blocks: %u\n", arena->alloc_list->size);
		printf("Number of allocated miniblocks: 0\n");
	} else {
		printf("Total memory: 0x%lX bytes\n", arena->arena_size);
		uint64_t free_size = 0;
		int num = 0;
		dll_node_t *currentb = arena->alloc_list->head;
		for (unsigned int i = 0; i < arena->alloc_list->size; i++) {
			free_size += ((block_t *)currentb->data)->size;
			currentb = currentb->next;
			num += ((list_t *)((block_t *)currentb->data)->miniblock_list)->size;
		}
		free_size = arena->arena_size - free_size;
		printf("Free memory: 0x%lX bytes\n", free_size);
		printf("Number of allocated blocks: %u\n", arena->alloc_list->size);
		printf("Number of allocated miniblocks: %d\n\n", num);
		dll_node_t *currentmb;
		currentb = arena->alloc_list->head;
		for (unsigned int i = 0; i < arena->alloc_list->size; i++) {
			printf("Block %d begin\n", i + 1);
			printf("Zone: 0x%lX - 0x%lX\n", ((block_t *)currentb->data)->start_address, ((block_t *)currentb->data)->start_address + ((block_t *)currentb->data)->size);
			currentmb = ((list_t *)((block_t *)currentb->data)->miniblock_list)->head;
			for (unsigned int j = 0; j < ((list_t *)((block_t *)currentb->data)->miniblock_list)->size; j++) {
				printf("Miniblock %d:\t\t0x%lX\t\t-\t\t0x%lX\t\t| ", j + 1, ((miniblock_t *)currentmb->data)->start_address, ((miniblock_t *)currentmb->data)->start_address + ((miniblock_t *)currentmb->data)->size);
				if (((miniblock_t *)currentmb->data)->perm == 0) {
					printf("---");
				} else if (((miniblock_t *)currentmb->data)->perm == 1) {
					printf("--X");
				} else if (((miniblock_t *)currentmb->data)->perm == 2) {
					printf("-W-");
				} else if (((miniblock_t *)currentmb->data)->perm == 3) {
					printf("-WX");
				} else if (((miniblock_t *)currentmb->data)->perm == 4) {
					printf("R--");
				} else if (((miniblock_t *)currentmb->data)->perm == 5) {
					printf("R-X");
				} else if (((miniblock_t *)currentmb->data)->perm == 6) {
					printf("RW-");
				} else if (((miniblock_t *)currentmb->data)->perm == 7) {
					printf("RWX");
				}
				printf("\n");
				currentmb = currentmb->next;
			}
			currentb = currentb->next;
			printf("Block %d end\n", i + 1);
			if (i != arena->alloc_list->size - 1) {
				printf("\n");
			}
		}
	}
}

//schimba permisiunile miniblokurilor
void mprotect(arena_t *arena, uint64_t address, int8_t *permission)
{

	dll_node_t *currentb = arena->alloc_list->head;
	dll_node_t *currentmb;
	if (address > arena->arena_size || arena->alloc_list->size == 0) {
		printf("Invalid address for mprotect.\n");
		return;
	} else {
		if (address < ((block_t *)currentb->data)->start_address || address >= ((block_t *)currentb->prev->data)->start_address + ((block_t *)currentb->prev->data)->size) {
			printf("Invalid address for mprotect.\n");
			return;
		}
		for (unsigned int i = 0; i < arena->alloc_list->size; i++) {
			currentmb = ((list_t *)(((block_t *)currentb->data)->miniblock_list))->head;
			for (unsigned int j = 0; j < ((list_t *)((block_t *)currentb->data)->miniblock_list)->size; j++) {
				if (address == ((miniblock_t *)currentmb->data)->start_address) {
					goto hop;
				}
				currentmb = currentmb->next;
			}
			currentb = currentb->next;
		}
	}
	printf("Invalid address for mprotect.\n");
	return;
hop:
	((miniblock_t *)currentmb->data)->perm = 0;
	char *token = strtok((char *)permission, " ");
	while (token) {
		if (strlen(token) < 5) {
			token = strtok(NULL, " ");
		}
		if (strncmp(token, "PROT_NONE", 9) == 0) {
			((miniblock_t *)currentmb->data)->perm = 0;
			break;
		} else if (strncmp(token, "PROT_READ", 9) == 0) {
			((miniblock_t *)currentmb->data)->perm += 4;
		} else if (strncmp(token, "PROT_WRITE", 10) == 0) {
			((miniblock_t *)currentmb->data)->perm += 2;
		} else if (strncmp(token, "PROT_EXEC", 9) == 0) {
			((miniblock_t *)currentmb->data)->perm += 1;
		}
		token = strtok(NULL, " ");
	}
}
