#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>

struct word {
	char letter;
	struct word *next;
};

struct list {
	char *word;
	struct list *next;
};

struct cmnd {
	struct list *fstWrd;
	int bckGrd;
};

void entLet(void);
void rmvZmb(int x);
void addLet(struct word **first, char a);
void delLet(struct word **first);
void addWrd(struct list **first, struct word **fstLet);
void delWrd(struct list **first);
void cllCmd(struct cmnd *cmd);
void sprHnd(struct cmnd **cmd, struct word **fstLet, char a);
char *copyWrd(struct word *first);
char **copyLst(struct list *first); 
int numLet(struct word *first);
int numWrd(struct list *first);
int cmpStr(const char *a, const char *b);

int main()
{
	signal(SIGCHLD, rmvZmb);
	putchar('>');
	entLet();
	putchar('\n');
	return 0;
}

void rmvZmb(int x)
{
	while(waitpid(-1, NULL, WNOHANG) > 0)
	{}
}

void entLet(void)
{
	struct cmnd *fstCmd = NULL;
	struct word *fstLet = NULL;
	int inQts = 0;
	char a;
	
	fstCmd = malloc(sizeof(*fstCmd));
	(*fstCmd).fstWrd = NULL;
	while((a = getchar()) != EOF) {
		if (a == '"') {
			inQts = !inQts;
			continue;
		}
		if ((a == ' ' || a == '&') && inQts) {
			addLet(&fstLet, a);
			continue;
		}
		if (a == ' ') {
			addWrd(&((*fstCmd).fstWrd), &fstLet);
			continue;
		}
		if (a == '\n' && inQts) {	
			printf("Error: unbalanced quotes.\n>");
			inQts = 0;
			delWrd(&((*fstCmd).fstWrd));
			delLet(&fstLet);	
			continue;
		}
		if (a == '&' || a == '\n') {
			sprHnd(&fstCmd, &fstLet, a);
			continue;
		}
		addLet(&fstLet, a);		
	}
	delWrd(&((*fstCmd).fstWrd));
	delLet(&fstLet);	
	free(fstCmd);
}

void sprHnd(struct cmnd **cmd, struct word **fstLet, char a)
{
	addWrd(&((**cmd).fstWrd), fstLet);
	if ((**cmd).fstWrd != NULL) {
		if (a == '\n') {	
			(**cmd).bckGrd = 0;
			signal(SIGCHLD, SIG_DFL);
		} else {
			(**cmd).bckGrd = 1;
		}
		cllCmd(*cmd);
		if (a == '\n') {
			signal(SIGCHLD, rmvZmb);
		}
		delWrd(&((**cmd).fstWrd));
	}
	if (a == '\n') {
		putchar('>');
	}
}

void cllCmd(struct cmnd *cmd)
{
	char **arrWrd;
	int help;
	
	arrWrd = copyLst((*cmd).fstWrd);
	if (cmpStr(arrWrd[0],"cd")) {
		help = numWrd((*cmd).fstWrd);
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
		if (!(*cmd).bckGrd) {
			while(wait(NULL) != help)
			{}
		}
	}
	free(arrWrd);
}

int cmpStr(const char *a, const char *b)
{
	int i = 0;

	while(a[i] == b[i] && a[i] != 0) {
		i++;
	}
	return (a[i] == b[i]);
}

char **copyLst(struct list *first)
{
	char **arrWrd;
	int i, n;

	n = numWrd(first);
	arrWrd = malloc((n+1)*sizeof(*arrWrd));
	for(i = n-1; i >=0; i--) {
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

void addWrd(struct list **first, struct word **fstLet)
{	
	if (*fstLet != NULL) {
		if (*first == NULL) {
			*first = malloc(sizeof(**first));
			(**first).word = copyWrd(*fstLet);
			(**first).next = NULL;
		} else {
			struct list *help;
			help = malloc(sizeof(*help));
			(*help).word = copyWrd(*fstLet);
			(*help).next = *first;
			*first = help;
		}
	}
	delLet(fstLet);
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
