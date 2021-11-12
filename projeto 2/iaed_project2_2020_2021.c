/* Raquel Cardoso - 99314
 * A small program that simulates a hierarchical
 * storing system just like a file system.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * structure to represent a directory
 */

typedef struct directory {
	struct directory *next;			/* the next directory in the subdir list */
	struct directory *parent;		/* parent directory */
	char *name;						/* the directory name */
	char *value;					/* the directory value or NULL */
	struct directory *first_sdir;	/* first subdirectory */
	struct directory *last_sdir;	/* last directory */
} Directory;

/*
 * constants
 */

#define MAX_LINE		65535
#define MAX_CMD			10
#define MAX_PATH_COMPS	256
#define FIRST_DEPTH     0

/*
 * command keywords and their description
 */

#define HELP "help"
#define HELP2 "Imprime os comandos disponíveis.\n"

#define QUIT "quit"
#define QUIT2 "Termina o programa.\n"

#define SET "set"
#define SET2 "Adiciona ou modifica o valor a armazenar.\n"

#define PRINT "print"
#define PRINT2 "Imprime todos os caminhos e valores.\n"

#define FIND "find"
#define FIND2 "Imprime o valor armazenado.\n"

#define LIST "list"
#define LIST2 "Lista todos os componentes de um caminho.\n"

#define SEARCH "search"
#define SEARCH2 "Procura o caminho dado um valor.\n"

#define DELETE "delete"
#define DELETE2 "Apaga um caminho e todos os subcaminhos.\n"

#define NOT_FOUND "not found\n"
#define NO_DATA "no data\n"
#define NO_MEMORY "No memory\n"

/*
 * command numbers
 */
#define INVALID_CMD	0
#define N_COMMANDS	8
#define HELP_CMD	1
#define QUIT_CMD	2
#define SET_CMD		3
#define PRINT_CMD	4
#define FIND_CMD	5
#define LIST_CMD	6
#define SEARCH_CMD	7
#define DELETE_CMD	8

/**
 * duplicate a string using dynamically alocated memory 
 */

char *dupstring(char *str) {
    char *dup = (char *)malloc(strlen(str) + 1);

    if (dup != NULL)
        strcpy(dup, str);
    return dup;
}

/*
 * quick sort by directory name recursive
 */

/* auxiliary swap function */
void swap(char *array[], int one, int other) {
	char *tmp = array[one];
	
	array[one] = array[other];
	array[other] = tmp;
}

/* the sort function */
void string_quicksort(char *str_ptr_array[], int left, int right /* inclusive */) {
	int i, last;
	
	if (left >= right)	/* do nothing if array contains fewer that two elements */
		return;
	/* move partition element to str_ptr_array[0] */
	swap(str_ptr_array, left, (left + right) / 2);
	last = left;
	/* partition */
	for (i = left + 1; i <= right; i++)
		if (strcmp(str_ptr_array[i], str_ptr_array[left]) < 0)
			swap(str_ptr_array, ++last, i);
	/* restore partition element */
	swap(str_ptr_array, left, last);
	/* sort the left and right parts */
	string_quicksort(str_ptr_array, left, last - 1);
	string_quicksort(str_ptr_array, last + 1, right);
}


/**
 * splits a pathname into its path components
 */
int split_pathname(char *pathname, char *paths[], int nelems) {
	char *token, *delimiter = "/";
	int count = 0;
	
	token = strtok(pathname, delimiter);
	while (token != NULL) {
		if (count < nelems)
			paths[count++] = token;
		token = strtok(NULL, delimiter);
	}
	return count;
}

/**
 * 	prints a full directory name
 */
void print_dir_name(Directory *dir) {
	/*base of recursion when the parent of a directory is null*/
	if (dir->parent == NULL) {
		printf("%s", dir->name);
		return;
	}
	print_dir_name(dir->parent);
	if (dir->parent->parent != NULL) {
		putchar('/');
	}
	printf("%s", dir->name);
}

/**
 * exit the program when exhausted memory
 */

void exit_no_memory(void) {
	printf(NO_MEMORY);
	exit(1);
}

/**
 * allocate memory to a directory structure and format it
 */

Directory *new_dir(Directory *parent, char *name, char *value) {
	Directory *dir;

	/* allocate and check for out of memory */
	if ((dir = (Directory *)malloc(sizeof(Directory))) == NULL)
		exit_no_memory();
	/* format the directory structure */
	dir->next = NULL; /* we always insert at the end of sublist dir */
	dir->parent = parent;
	/* allocate memory for directory name */
	if ((dir->name = dupstring(name)) == NULL) {
		exit_no_memory();
	}
	/* if a value was defined, allocate memory for it and copy*/
	if (value != NULL) {
		if ((dir->value = dupstring(value)) == NULL)
			exit_no_memory();
	} else {
		dir->value = NULL;
	}
	/* the sub dir list is initialized as empty */
	dir->first_sdir = dir->last_sdir = NULL;
	return dir;
}

/**
 * locate the sub directory in the specified directory
 */
Directory *locate_subdir(Directory *dir, char *sdir_name) {
	Directory *ptr;

	for (ptr = dir->first_sdir; ptr != NULL; ptr = ptr->next) {
		if (strcmp(sdir_name, ptr->name) == 0)
			return ptr;
	}
	return NULL;
}


/**
 * add a subdir to a directory
 */

Directory *add_subdir(Directory *dir, char *name, char *value) {
	Directory *sdir = new_dir(dir, name, value);

	/* insert the new subdir in the directory subdir list */
	if (dir->first_sdir == NULL) {
		/* insert as the first node */
		dir->first_sdir = dir->last_sdir = sdir;
	} else {
		/* insert at the middle or end of the list */
		dir->last_sdir->next = sdir;
		dir->last_sdir = sdir;
	}
	return sdir;
}

/**
 * find a directory for the given sub-path, recursive 
 */

Directory *find_dir(Directory *dir, char *paths[], int length, int depth) {
	/* when depth = length -1 i reached the recursive base */
	Directory *sdir = locate_subdir(dir, paths[depth]);

	/* if subdir does not exist, return NULL; otherwise, check if this is the
	 * last component of the path, if so return sdir; else call find_dir recusively
	 * with the next component of the path.
	 */
	if (sdir == NULL || depth == length - 1) {
		return sdir;
	}
	/* if this isn't the last component of the path, delegate the find on sdir */
	return find_dir(sdir, paths, length, depth + 1);
}

/*
 * commands implementation
 */


/**
 * help command
 */
void help_cmd() {

    printf("%s: %s", HELP, HELP2);
    printf("%s: %s", QUIT, QUIT2);
    printf("%s: %s", SET, SET2);
    printf("%s: %s", PRINT, PRINT2);
    printf("%s: %s", FIND, FIND2);
    printf("%s: %s", LIST, LIST2);
    printf("%s: %s", SEARCH, SEARCH2);
    printf("%s: %s", DELETE, DELETE2);
}

/**
 * recursive version of the set command
 */

void set_recursive(Directory *dir, char *paths[], int length, int depth, char *value) {
	Directory *sdir;

	if (depth == length-1) {
		/* if exists a subdir of the current dir with the last component of the path,
		change its' value; otherwise create a new subdir with the given name and value
		and add it to the current dir */
		if ((sdir = locate_subdir(dir, paths[depth])) != NULL) {
			/* change the existing value */
			free(sdir->value); /* free(NULL) is ok */
			if (( sdir->value = dupstring(value)) == NULL) {
				exit_no_memory();
			}
		} else {
			/* add a new subdir with the name and value */
			add_subdir(dir, paths[depth], value);
		}
	} else {
		/* the next component of the path is not the last one; if there exists already
		a subdir with the same name delegate the set on it. otherwise, create a new subdir
		and delegate the set on this new dir */
		if ((sdir = locate_subdir(dir, paths[depth])) == NULL) 
			sdir = add_subdir(dir, paths[depth], NULL);

		set_recursive(sdir, paths, length, depth+1, value);
	}
}


/**
 * set command
 */
void set_cmd(char *cmd_args, Directory *root) {
	
	char *pathname, *value, *paths[MAX_PATH_COMPS];
	int length;

	pathname = strtok(cmd_args, " \t");
    /* the value starts after the pathname delimited */
    value = pathname + strlen(pathname) + 1;
	length = split_pathname(pathname, paths, MAX_PATH_COMPS);
	set_recursive(root, paths, length, FIRST_DEPTH, value);
}

/**
 * print command, this is a recursive version
 */
void print_cmd(Directory *dir) {
	Directory *ptr;

	/* print the value of the current dir if it has one*/
	if (dir->value != NULL) {
		print_dir_name(dir);
		printf(" %s\n", dir->value);
	}
	/* in depth curse*/
	for (ptr = dir->first_sdir; ptr != NULL; ptr = ptr->next) {
		print_cmd(ptr);
	}
}

/**
 * find command
 */
void find_cmd(char *cmd_args, Directory *root) {
	char *pathname, *paths[MAX_PATH_COMPS];
	int length;
	Directory *dir;

	pathname = strtok(cmd_args, " \t");
	length = split_pathname(pathname, paths, MAX_PATH_COMPS);
	if ((dir = find_dir(root, paths, length, FIRST_DEPTH)) == NULL) {
		printf(NOT_FOUND);
	} else if (dir->value == NULL) {
		printf(NO_DATA);
	} else {
		printf("%s\n", dir->value);
	}
}

/**
 * auxiliary function used to sort components by name
 */

int cmp_str_ptr(const void *first, const void *second) {
	/* the functions arguments are pointers to string pointers */
	char *first_str = *((char **)first), *second_str = *((char **)second);;

	return strcmp(first_str, second_str);
}

/**
 * list command 
 */
void list_cmd(char *cmd_args, Directory *root) {
	char *pathname, *paths[MAX_PATH_COMPS], **sort_array;
	int length, i;
	Directory *dir, *ptr;
	
	if ((pathname = strtok(cmd_args, " \t")) == NULL) {
		pathname = "/";
	}
	length = split_pathname(pathname, paths, MAX_PATH_COMPS);
	dir = (length == 0) ? root : find_dir(root, paths, length, 0);
	if (dir == NULL) {
		printf("not found\n");
		return;
	}
	if (dir->first_sdir == NULL)
		return;			/* no immediate components */
	/* compute the number of components in the specified directory */
	for (length = 0, ptr = dir->first_sdir; ptr != NULL; ptr = ptr->next)
		length++;
	/* allocate the sort array */
	sort_array = (char **)malloc(length * sizeof(char *));
	if (sort_array == NULL) {
		printf("No memory\n");
		exit(1);
	}	
	/* copy the component names to the sort array */
	for (i = 0, ptr = dir->first_sdir; ptr != NULL; ptr = ptr->next)
		sort_array[i++] = ptr->name;
	
	string_quicksort(sort_array, 0, length - 1);
	
	/* print the components */
	for (i = 0; i < length; i++)
		printf("%s\n", sort_array[i]);
	
	/* release the memory allocated for the "sort array" */
	free(sort_array);
}

/**
 * recursive version of the search command
 */

Directory *search_recursive(Directory *dir, char * value) {
	Directory *ptr, *sdir;

	/* check if the value is at current directory */
	if (dir->value != NULL && strcmp(dir->value, value) == 0) {
		return dir;
	}
	/* scan the children subdir's in order to find the value */
	for (ptr = dir->first_sdir; ptr != NULL; ptr = ptr->next) {
		if ((sdir = search_recursive(ptr, value)) != NULL) {
			return sdir;
		}
	}
	return NULL;
}

/**
 * search command
 */
void search_cmd(char *cmd_args, Directory *root) {
	char *value = cmd_args;
	Directory *dir;

	if ((dir = search_recursive(root, value)) != NULL) {
		print_dir_name(dir);
		putchar('\n');
	} else {
		printf(NOT_FOUND);
	}
}

/**
 * recursive version of the delete command
 */

void delete_recursive(Directory *dir) {
	Directory *ptr, *next;

	/* first delete recursively all the subdirs of this directory */
	for (ptr = dir->first_sdir; ptr != NULL; ptr = next) {
		next = ptr->next;
		delete_recursive(ptr);
	}
	/* then, free all memory used by the node of this directory */
	free(dir->name);
	free(dir->value); /* free(NULL) is ok */
	free(dir);
}

/**
 * free all memory used to store the in memory database
 */

void free_all_memory(Directory *root) {
	delete_recursive(root);
}

/**
 * delete command
 */
void delete_cmd(char *cmd_args, Directory *root) {
	char *pathname, *paths[MAX_PATH_COMPS];
	int length;
	Directory *sdir, *ptr, *next, *prev;

	if ((pathname = strtok(cmd_args, " \t")) == NULL) {
		for (ptr = root->first_sdir; ptr != NULL; ptr = next) {
			next = ptr->next;
			delete_recursive(ptr);
		}
		/* empty the list of subdirs in the root */
		root->first_sdir = root->last_sdir = NULL;
	} else {
		length = split_pathname(pathname, paths, MAX_PATH_COMPS);
		sdir = find_dir(root, paths, length, FIRST_DEPTH);
		if (sdir == NULL) {
			printf(NOT_FOUND);
			return;
		}
		/* remove subdir from the subdir parent's list */
		for (prev = NULL, ptr = sdir->parent->first_sdir; ptr != NULL; prev = ptr, ptr = ptr->next) {
			if (ptr == sdir)
				break;
		}
		/* remove subdir */
		if (prev == NULL) {
			/* remove at the head of list */
			if ((sdir->parent->first_sdir = sdir->next) == NULL) {
				sdir->parent->last_sdir = NULL;
				/* the list is empty now */
			}
		} else {
			/* remove a node at middle or the end of list */
			if ((prev->next = ptr->next) == NULL) {
				/* remove the last node */
				sdir->parent->last_sdir = prev;
			}
		}
		delete_recursive(sdir);
	}
}

/*
 * command table
 */
static char *cmd_table[N_COMMANDS] = { HELP, QUIT, SET, PRINT, FIND, LIST, SEARCH, DELETE };

/**
 * map a command string to a command index
 */
int cmd_str_2_index(const char *cmd_str) {
	int i;
	
	for (i = 0; i < N_COMMANDS; i++) {
		if (strcmp(cmd_str, cmd_table[i]) == 0)
			break;
	}
	return (i < N_COMMANDS) ? (i + 1) : INVALID_CMD;
    /*if (i < N_COMMANDS)
        return i + 1;
    else
        return INVALID_COMMAND; */
}

int main() {
    char cmd_line[MAX_LINE], *cmd_word, *cmd_args;
	Directory *root = new_dir(NULL, "/", NULL);
	
    while (1) {
        /* read the next command line */
        if (fgets(cmd_line, sizeof(cmd_line), stdin) == NULL) {
            /* end of file on standard input */
            free_all_memory(root);
            return 0;
        }
        /* smash the new line that is at the end of the command line*/
        cmd_line[strlen(cmd_line) - 1] = '\0';
        /* delimit the command word */
        cmd_word = strtok(cmd_line, " \t\n");
        if (cmd_word == NULL)
            continue; /* this happens when there's an empty command line */
        /* locate the start of command arguments */
        cmd_args = cmd_word + strlen(cmd_word) + 1;
		/* process command */
		switch (cmd_str_2_index(cmd_word)) {
			case HELP_CMD:			/* print help message */
                help_cmd();
				break;
				
			case QUIT_CMD:			/* quits the program  */
                free_all_memory(root);
				return 0;

			case SET_CMD:			/* set or modify a value */
				set_cmd(cmd_args, root);
				break;
				
			case PRINT_CMD:			/* prints all paths and values */
				print_cmd(root);
				break;
	
			case FIND_CMD:			/* prints the value stored on the given path */
				find_cmd(cmd_args, root);
				break;
	
			case LIST_CMD:			/* lists all the components of a sub-path */
				list_cmd(cmd_args, root);
				break;
			
			case SEARCH_CMD:		/* searchs a path for the given value */
				search_cmd(cmd_args, root);
				break;

			case DELETE_CMD:		/* deletes all paths of the specified sub-path */
				delete_cmd(cmd_args, root);
				break;
			
			default:
				fprintf(stderr, "'%s' is an invalid command word\n", cmd_word);
				break;
		}
	}
	return 0;
}