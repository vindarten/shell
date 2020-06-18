#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

struct word {
	char letter;
	struct word *next;
};

struct list {
	char *word;
	struct list *next;
};

void addLet(struct word **first, char a);
void delLet(struct word **first);
void addWrd(struct list **first, struct word *fstLet);
void delWrd(struct list **first);
void cllCmd(struct list *first);
char *copyWrd(struct word *first);
char **copyLst(struct list *first); 
int numLet(struct word *first);
int numWrd(struct list *first);
int cmpStr(const char *a, const char *b);

int main()
{
	struct list *fstWrd = NULL;
	struct word *fstLet = NULL;
	int inQts = 0;
	char a;
	
	putchar('>');
	while((a = getchar()) != EOF) {
		switch(a) {
			case ' ': 
				if (inQts) {
					addLet(&fstLet, a);
				} else {
					addWrd(&fstWrd, fstLet);
					delLet(&fstLet);
				}
				break;
			case '\n':
				addWrd(&fstWrd, fstLet);
				delLet(&fstLet);
				if (inQts) {
					printf("Error: unbalanced quotes.\n>");
				} else {
					if (fstWrd != NULL) {
						cllCmd(fstWrd);
					}
				}
				delWrd(&fstWrd);
				break;
			case '"':
				inQts = !inQts;
				break;
			default:
				addLet(&fstLet, a);
		}
	}
	delWrd(&fstWrd);
	delLet(&fstLet);	
	putchar('\n');
	return 0;
}

void cllCmd(struct list *first)
{
	char **arrWrd;
	int help;
	
	arrWrd = copyLst(first);
	if (cmpStr(arrWrd[0],"cd")) {
		help = numWrd(first);
		if (help != 2) {
			printf("cd: wrong number of arguments\n");
		} else {
			if (chdir(arrWrd[1])) {
				perror(arrWrd[1]);
			}
		}
	} else {
		help = fork();
		if (!help) {
			execvp(arrWrd[0],arrWrd);
			perror(arrWrd[0]);
			exit(1);
		}
		wait(NULL);
	}
	free(arrWrd);
	putchar('>');
}

int cmpStr(const char *a, const char *b)
{
	int i = 0;

	while(a[i] && b[i]) {
		if (a[i] != b[i]) {
			return 0;
		}
		i++;
	}
	if (a[i] != b[i]) {
		return 0;
	}
	return -1;
}

char **copyLst(struct list *first)
{
	char **arrWrd;
	int i, n;

	n = numWrd(first);
	arrWrd = malloc((n+1)*sizeof(*arrWrd));
	for(i = 0; i <= n-1; i++) {
		arrWrd[i] = (*first).word;
		first = (*first).next;
	}
	arrWrd[n] = NULL;
	return arrWrd;
}

void addLet(struct word **first, char a)
{
	struct word *help;

	help = malloc(sizeof(*help));
	(*help).letter = a;
	(*help).next = *first;
	*first = help;
}

void delLet(struct word **first)
{
	struct word *help;

	while(*first != NULL) {
		help = *first;
		*first = (**first).next;
		free(help);
	}
}

int numLet(struct word *first)
{
	int help = 0;

	while(first != NULL) {
		first = (*first).next;
		help++;
	}
	return help;
}

int numWrd(struct list *first)
{
	int help = 0;

	while(first != NULL) {
		first = (*first).next;
		help++;
	}
	return help;
}

char *copyWrd(struct word *fstLet)
{
	char *word;
	int i, n;
	
	n = numLet(fstLet);
	word = malloc(n+1);
	for(i = n - 1; i >= 0; i--) {
		word[i] = (*fstLet).letter;
		fstLet = (*fstLet).next;
	}
	word[n] = 0;
	return word;
}

void addWrd(struct list **first, struct word *fstLet)
{	
	if (fstLet != NULL) {
		if (*first == NULL) {
			*first = malloc(sizeof(**first));
			(**first).word = copyWrd(fstLet);
			(**first).next = NULL;
		} else {
			struct list *help = *first;
			while((*help).next != NULL) {
				help = (*help).next;
			}
			(*help).next = malloc(sizeof(*help));
			help = (*help).next;
			(*help).word = copyWrd(fstLet);
			(*help).next = NULL;
		}
	}
}

void delWrd(struct list **first)
{
	struct list *help;
	
	while(*first != NULL) {
		free((**first).word);
		help = *first;
		*first = (**first).next;
		free(help);
	}
}
