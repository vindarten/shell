#include <stdio.h>
#include <stdlib.h>

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
int numLet(struct word *fstLet);
void delWrd(struct list **first, struct list **last);
void prtWrd(struct list *first);
char *copyWrd(struct word *fstLet);
void addWrd(
	struct list **first, 
	struct list **last, 
	struct word *fstLet 
);

int main()
{
	struct list *fstWrd = NULL, *lstWrd = NULL;
	struct word *fstLet = NULL;
	int inQts = 0;
	char a;
	
	putchar('>');
	while((a = getchar()) != EOF) {
		switch(a) {
			case ' ': 
				if (inQts) {
					addLet(&fstLet, a);
				}
				else {
					addWrd(&fstWrd, &lstWrd, fstLet);
					delLet(&fstLet);
				}
				break;
			case '\n':
				addWrd(&fstWrd, &lstWrd, fstLet);
				delLet(&fstLet);
				if (inQts) {
					printf("Error: unbalanced quotes.\n>");
				}
				else {
					prtWrd(fstWrd);
				}
				delWrd(&fstWrd, &lstWrd);
				break;
			case '"':
				inQts = !inQts;
				break;
			default:
				addLet(&fstLet, a);
		}
	}
	delWrd(&fstWrd, &lstWrd);
	delLet(&fstLet);	
	putchar('\n');
	return 0;
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

int numLet(struct word *fstLet)
{
	int help = 0;

	while(fstLet != NULL) {
		fstLet = (*fstLet).next;
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

void addWrd(
	struct list **first, 
	struct list **last, 
	struct word *fstLet 
)
{	
	if (fstLet != NULL) {
		if (*first == NULL) {
			*first = malloc(sizeof(**first));
			(**first).word = copyWrd(fstLet);
			(**first).next = NULL;
			*last = *first;
		}
		else {
			(**last).next = malloc(sizeof(**last));
			*last = (**last).next;
			(**last).word = copyWrd(fstLet);
			(**last).next = NULL;
		}
	}
}

void delWrd(struct list **first, struct list **last)
{
	struct list *help;
	
	while(*first != NULL) {
		free((**first).word);
		help = *first;
		*first = (**first).next;
		free(help);
	}
	*last = NULL;
}

void prtWrd(struct list *first)
{
	int i = 0;

	while(first != NULL) {
		while((*first).word[i] != 0) {
			putchar((*first).word[i]);
			i++;
		}
		first = (*first).next;
		i = 0;
		putchar('\n');
	}
	putchar('>');
}
