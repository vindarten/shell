#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <fcntl.h>

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
	char *input;
	char *output;
	int append;
	int bckGrd;
	int err;
};

struct hlpStr {
	char llast;
	char last;
	char next;
	int inQts;
	int gt;
	int lt;
};

enum {input, output};
enum {right, unbQts, inSpec, outSpec, redir, nlUnexp, wrArg};

void entLet(void);
void rmvZmb(int x);
void printe(int err);
void addLet(struct word **first, char a);
void delLet(struct word **first);
void addWrd(struct list **first, struct word **fstLet);
void delWrd(struct list **first);
void cllCmd(struct cmnd *cmd, char **arrWrd);
void chdHnd(struct cmnd *cmd, char **arrWrd);
void clsfdc(struct cmnd *cmd, int fdi, int fdo);
void clsfdp(struct cmnd *cmd, int fdi, int fdo);
void prpCll(struct cmnd **cmd, char a);
void ersVar(struct cmnd **cmd, struct word **fstLet, struct hlpStr *auxVar);
void spcHnd(struct cmnd **cmd, struct word **fstLet, struct hlpStr *auxVar);
void endHnd(struct cmnd **cmd, struct word **fstLet, struct hlpStr *auxVar);
int cllHnd(struct cmnd **cmd, struct word **fstLet, struct hlpStr *auxVar);
int ltsHnd(struct cmnd **cmd, struct word **fstLet, struct hlpStr *auxVar);
int gtsHnd(struct cmnd **cmd, struct word **fstLet, struct hlpStr *auxVar);
int openfd(struct cmnd *cmd, int i);
int numLet(struct word *first);
int numWrd(struct list *first);
int cmpStr(const char *a, const char *b);
int tstCon(char a);
char entChr(struct hlpStr *auxVar);
char *copyWrd(struct word **first);
char **copyLst(struct list *first); 
struct cmnd *crtCmd(void);

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
	struct cmnd *cmd = NULL;
	struct word *fstLet = NULL;
	struct hlpStr auxVar;
	ersVar(NULL, NULL, &auxVar);
	cmd = crtCmd();
	while(entChr(&auxVar) != EOF) {
		if (!(*cmd).err) {
			if (cllHnd(&cmd, &fstLet, &auxVar)) {
				continue;
			}
		}
		if (auxVar.next == '&' || auxVar.next == '\n') {
			endHnd(&cmd, &fstLet, &auxVar);
			if (auxVar.next == '\n') {
				putchar('>');
			}
			ersVar(&cmd, &fstLet, &auxVar);
			continue;
		}
		addLet(&fstLet, auxVar.next);
	}
	ersVar(&cmd, &fstLet, &auxVar);
	free(cmd);
}

void ersVar(struct cmnd **cmd, struct word **fstLet, struct hlpStr *auxVar)
{
	if (cmd != NULL && fstLet != NULL) {
		delWrd(&((**cmd).fstWrd));
		delLet(fstLet); 
		if ((**cmd).output != NULL) {
			free((**cmd).output);
			(**cmd).output = NULL;
		}
		if ((**cmd).input != NULL) {	
			free((**cmd).input);
			(**cmd).input = NULL;
		}
		(**cmd).append = (**cmd).bckGrd = (**cmd).err = 0;
	}
	(*auxVar).llast = (*auxVar).last = (*auxVar).next = ' ';	
	(*auxVar).inQts = (*auxVar).gt = (*auxVar).lt = 0;
}

struct cmnd *crtCmd(void)
{
	struct cmnd *cmd;
	cmd = malloc(sizeof(*cmd));
	(*cmd).fstWrd = NULL;
	(*cmd).input = (*cmd).output = NULL;
	(*cmd).append = (*cmd).bckGrd = (*cmd).err = 0;
	return cmd;
}

char entChr(struct hlpStr *auxVar)
{
	(*auxVar).llast = (*auxVar).last;
	(*auxVar).last = (*auxVar).next;
	(*auxVar).next = getchar();
	return (*auxVar).next;
}

int cllHnd(struct cmnd **cmd, struct word **fstLet, struct hlpStr *auxVar)
{
	char a;
	a = (*auxVar).next;
	if (a == '"') {
		(*auxVar).inQts = !((*auxVar).inQts);
		return -1;
	}
	if (tstCon(a) && (*auxVar).inQts) {
		addLet(fstLet, a);
		return -1;
	}
	if (a == ' ') {
		spcHnd(cmd, fstLet, auxVar);
		return -1;
	}
	if (a == '<') {
		(**cmd).err = ltsHnd(cmd, fstLet, auxVar);
		return -1;
	}
	if (a == '>') {
		(**cmd).err = gtsHnd(cmd, fstLet, auxVar);
		return -1;
	}
	return 0;
}

void spcHnd(struct cmnd **cmd, struct word **fstLet, struct hlpStr *auxVar)
{
	if (!(*auxVar).gt && !(*auxVar).lt) {
		addWrd(&((**cmd).fstWrd), fstLet);
		return;
	}
	if (*fstLet != NULL) {
		if ((*auxVar).gt) {
			(**cmd).output = copyWrd(fstLet);
			(*auxVar).gt = 0;
		} else {
			(**cmd).input = copyWrd(fstLet);
			(*auxVar).lt = 0;
		}
	}
}

int ltsHnd(struct cmnd **cmd, struct word **fstLet, struct hlpStr *auxVar)
{
	if ((*auxVar).lt && *fstLet == NULL) {
		return redir;
	}
	if ((**cmd).input != NULL || (*auxVar).lt) {
		return inSpec;
	}
	if (!(*auxVar).gt) {
		addWrd(&((**cmd).fstWrd), fstLet);
		(*auxVar).lt = 1;
		return 0;
	}
	if (*fstLet == NULL) {
		return redir;
	}
	(**cmd).output = copyWrd(fstLet);
	(*auxVar).gt = 0;
	(*auxVar).lt = 1;
	return 0;		
}

int gtsHnd(struct cmnd **cmd, struct word **fstLet, struct hlpStr *auxVar)
{
	if ((*auxVar).last == '>') {
		if ((*auxVar).llast == '>') {
			return redir;
		}
		(**cmd).append = 1;
		return 0;
	}
	if ((*auxVar).gt && *fstLet == NULL) {
		return redir;
	}
	if ((**cmd).output != NULL || (*auxVar).gt) {
		return outSpec;
	}
	if (!(*auxVar).lt) {
		addWrd(&((**cmd).fstWrd), fstLet);
		(*auxVar).gt = 1;
		return 0;
	}
	if (*fstLet == NULL) {
		return redir;
	}
	(**cmd).input = copyWrd(fstLet);
	(*auxVar).lt = 0;
	(*auxVar).gt = 1;
	return 0;			
}

void endHnd(struct cmnd **cmd, struct word **fstLet, struct hlpStr *auxVar)
{
	if ((*auxVar).next == '\n' && (*auxVar).inQts) {	
		(**cmd).err = unbQts;
	}
	if ((**cmd).err) {
		printe((**cmd).err);
		return;
	}
	if (!(*auxVar).gt && !(*auxVar).lt) {
		addWrd(&((**cmd).fstWrd), fstLet);
		prpCll(cmd, (*auxVar).next);
		return;
	} 
	if (*fstLet == NULL) {
		printe(nlUnexp);
		return;
	} 
	if ((*auxVar).gt) {
		(**cmd).output = copyWrd(fstLet);
	} else {
		(**cmd).input = copyWrd(fstLet);
	}
	prpCll(cmd, (*auxVar).next);
}	

void prpCll(struct cmnd **cmd, char a)
{	
	char **arrWrd;
	if ((**cmd).fstWrd != NULL) {
		if (a == '\n') {	
			signal(SIGCHLD, SIG_DFL);
		} else {
			(**cmd).bckGrd = 1;
		}
		arrWrd = copyLst((**cmd).fstWrd);
		cllCmd(*cmd, arrWrd);
		free(arrWrd);
		if (a == '\n') {
			signal(SIGCHLD, rmvZmb);
		}
	}
}

void cllCmd(struct cmnd *cmd, char **arrWrd)
{
	int pid, fdi, fdo;
	if (cmpStr(arrWrd[0],"cd")) {
		chdHnd(cmd, arrWrd);
		return;
	}
	if ((fdo = openfd(cmd, output)) == -1) {
		perror((*cmd).output);
		return;
	}
	if ((fdi = openfd(cmd, input)) == -1) {
		perror((*cmd).input);
		return;
	}
	pid = fork();
	if (!pid) {
		clsfdc(cmd, fdi, fdo);
		execvp(arrWrd[0],arrWrd);
		perror(arrWrd[0]);
		exit(1);
	}
	clsfdp(cmd, fdi, fdo);
	if (!(*cmd).bckGrd) {
		while(wait(NULL) != pid)
		{}
	}
}

void chdHnd(struct cmnd *cmd, char **arrWrd)
{
	int help;
	help = numWrd((*cmd).fstWrd);
	if (help != 2) {
		printe(wrArg);
	} else {
		if (chdir(arrWrd[1])) {
			perror(arrWrd[1]);
		}
	}
}

int openfd(struct cmnd *cmd, int i)
{
	int fd = 0;
	if (i == output) {
		if ((*cmd).output != NULL) {
			if ((*cmd).append) {
				fd = open((*cmd).output, O_WRONLY|O_CREAT|O_APPEND, 0666);
			} else {
				fd = open((*cmd).output, O_WRONLY|O_CREAT|O_TRUNC, 0666);
			}
		}
	} else {
		if ((*cmd).input != NULL) {
			fd = open((*cmd).input, O_RDONLY, 0666);
		}
	}
	return fd;
}

void clsfdc(struct cmnd *cmd, int fdi, int fdo)
{
	if ((*cmd).input != NULL) {
		dup2(fdi, 0);
		close(fdi);
	}
	if ((*cmd).output != NULL) {
		dup2(fdo, 1);
		close(fdo);
	}
}

void clsfdp(struct cmnd *cmd, int fdi, int fdo)
{
	if ((*cmd).input != NULL) {
		close(fdi);
	}
	if ((*cmd).output != NULL) {
		close(fdo);
	}
}

void printe(int err)
{
	if (err == wrArg) {
		printf("cd: wrong number of arguments\n");
		return;
	}
	printf("Syntax error: ");
	switch (err) {
		case unbQts:
			printf("unbalanced quotes.\n>");
			break;
		case inSpec:
			printf("the input file is already specified\n");
			break;
		case outSpec:
			printf("the output file is already specified\n");
			break;
		case redir:
			printf("redirection unexpected (expecting word)\n");
			break;
		case nlUnexp:
			printf("newline unexpected (expecting word)\n");
			break;
		default:
			printf("unknown error\n");
	}
}

int cmpStr(const char *a, const char *b)
{
	int i = 0;
	while(a[i] == b[i] && a[i] != 0) {
		i++;
	}
	return (a[i] == b[i]);
}

int tstCon(char a)
{
	return (a == ' ' || a == '&' || a == '>' || a == '<');
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

char *copyWrd(struct word **fstLet)
{
	char *word;
	int i, n;
	n = numLet(*fstLet);
	word = malloc(n+1);
	for(i = n - 1; i >= 0; i--) {
		word[i] = (**fstLet).letter;
		*fstLet = (**fstLet).next;
	}
	word[n] = 0;
	delLet(fstLet);
	return word;
}

void addWrd(struct list **first, struct word **fstLet)
{	
	if (*fstLet != NULL) {
		if (*first == NULL) {
			*first = malloc(sizeof(**first));
			(**first).word = copyWrd(fstLet);
			(**first).next = NULL;
		} else {
			struct list *help;
			help = malloc(sizeof(*help));
			(*help).word = copyWrd(fstLet);
			(*help).next = *first;
			*first = help;
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
