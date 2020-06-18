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
	struct cmnd *next;
};

struct cmnds {
	struct cmnd *fstCmd;
	char *input;
	char *output;
	int append;
	int bckGrd;
	int err;
};

struct hlpStr {
	struct word *fstLet;
	char llast;
	char last;
	char next;
	int inQts;
	int gt;
	int lt;
};

struct pidList {
	int pid;
	struct pidList *next;
};

enum {input, output};
enum {right, unbQts, inSpec, outSpec, redir, nlUnexp, wrArg, nllCmd};

void rmvZmb(int x);
void entLet(void);
void initAll(struct cmnds *cmds, struct hlpStr *av);
char entChr(struct hlpStr *av);
int cllHnd(struct cmnds *cmds, struct hlpStr *av);
int tstCon(char a);
void addLet(struct word **fstLet, char a);
void spcHnd(struct cmnds *cmds, struct hlpStr *av);
void addWrd(struct list **fstWrd, struct word **fstLet);
char *copyWrd(struct word **fstLet);
int numLet(struct word *fstLet);
int ltsHnd(struct cmnds *cmds, struct hlpStr *av);
int gtsHnd(struct cmnds *cmds, struct hlpStr *av);
int barHnd(struct cmnds *cmds, struct hlpStr *av);
void endHnd(struct cmnds *cmds, struct hlpStr *av);
void printe(int err);
void prpCll(struct cmnds *cmds);
char **copyLst(struct list *fstWrd); 
int numWrd(struct list *fstWrd);
void cllCmd(struct cmnds *cmds, char **arrWrd);
int cmpStr(const char *a, const char *b);
void chdHnd(char **arrWrd);
int openfd(struct cmnds *cmds, int i);
void clsfdc(struct cmnds *cmds, int fdi, int fdo);
void clsfdp(struct cmnds *cmds, int fdi, int fdo);
void cllCnv(struct cmnds *cmds);
int fstRun(struct cmnds *cmds, int fdio[2]);
struct cmnd *midRun(struct cmnds *cmds, struct pidList **pids, int fdio[2]);
int lstRun(struct cmnds *cmds, struct cmnd *help, int fd[2]);
void dupCls(int fdio, int fd[2]);
void flipList(struct cmnds *cmds);
int numCmd(struct cmnd *fstCmd);
void addPid(struct pidList **pids, int pid);
void delPid(struct pidList **pids, int pid);
void ersVar(struct cmnds *cmds, struct hlpStr *av);
void delCmd(struct cmnd **fstCmd);
void delWrd(struct list **fstWrd);
void delLet(struct word **fstLet);

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
	struct cmnds cmds;
	struct hlpStr av;
	initAll(&cmds, &av);
	while(entChr(&av) != EOF) {
		if (!cmds.err) {
			if (cllHnd(&cmds, &av)) {
				continue;
			}
		}
		if (av.next == '&' || av.next == '\n') {
			endHnd(&cmds, &av);
			if (av.next == '\n') {
				putchar('>');
			}
			ersVar(&cmds, &av);
			continue;
		}
		addLet(&av.fstLet, av.next);
	}
	ersVar(&cmds, &av);
}

void initAll(struct cmnds *cmds, struct hlpStr *av)
{
	(*cmds).fstCmd = malloc(sizeof(*(*cmds).fstCmd));
	(*(*cmds).fstCmd).fstWrd = NULL;
	(*(*cmds).fstCmd).next = NULL;
	(*cmds).input = (*cmds).output = NULL;
	(*cmds).append = (*cmds).bckGrd = (*cmds).err = 0;
	(*av).fstLet = NULL;
	(*av).llast = (*av).last = (*av).next = ' ';	
	(*av).inQts = (*av).gt = (*av).lt = 0;
}

char entChr(struct hlpStr *av)
{
	(*av).llast = (*av).last;
	(*av).last = (*av).next;
	(*av).next = getchar();
	return (*av).next;
}

int cllHnd(struct cmnds *cmds, struct hlpStr *av)
{
	char a;
	a = (*av).next;
	if (a == '"') {
		(*av).inQts = !((*av).inQts);
		return -1;
	}
	if (tstCon(a) && (*av).inQts) {
		addLet(&(*av).fstLet, a);
		return -1;
	}
	if (a == ' ') {
		spcHnd(cmds, av);
		return -1;
	}
	if (a == '<') {
		(*cmds).err = ltsHnd(cmds, av);
		return -1;
	}
	if (a == '>') {
		(*cmds).err = gtsHnd(cmds, av);
		return -1;
	}
	if (a == '|') {
		(*cmds).err = barHnd(cmds, av);
		return -1;
	}
	return 0;
}

int tstCon(char a)
{
	return (a == ' ' || a == '&' || a == '>' || a == '<' || a == '|');
}

void addLet(struct word **fstLet, char a)
{
	struct word *help;
	help = malloc(sizeof(*help));
	(*help).letter = a;
	(*help).next = *fstLet;
	*fstLet = help;
}

void spcHnd(struct cmnds *cmds, struct hlpStr *av)
{
	if (!(*av).gt && !(*av).lt) {
		addWrd(&(*(*cmds).fstCmd).fstWrd, &(*av).fstLet);
		return;
	}
	if ((*av).fstLet != NULL) {
		if ((*av).gt) {
			(*cmds).output = copyWrd(&(*av).fstLet);
			(*av).gt = 0;
		} else {
			(*cmds).input = copyWrd(&(*av).fstLet);
			(*av).lt = 0;
		}
	}
}

void addWrd(struct list **fstWrd, struct word **fstLet)
{
	struct list *help;
	if (*fstLet != NULL) {
		help = malloc(sizeof(*help));
		(*help).word = copyWrd(fstLet);
		(*help).next = *fstWrd;
		*fstWrd = help;
	}
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

int numLet(struct word *fstLet)
{
	int help = 0;
	while(fstLet != NULL) {
		fstLet = (*fstLet).next;
		help++;
	}
	return help;
}

int ltsHnd(struct cmnds *cmds, struct hlpStr *av)
{
	if ((*av).lt && (*av).fstLet == NULL) {
		return redir;
	}
	if ((*cmds).input != NULL || (*av).lt) {
		return inSpec;
	}
	if (!(*av).gt) {
		addWrd(&(*(*cmds).fstCmd).fstWrd, &(*av).fstLet);
		(*av).lt = 1;
		return 0;
	}
	if ((*av).fstLet == NULL) {
		return redir;
	}
	(*cmds).output = copyWrd(&(*av).fstLet);
	(*av).gt = 0;
	(*av).lt = 1;
	return 0;		
}

int gtsHnd(struct cmnds *cmds, struct hlpStr *av)
{
	if ((*av).last == '>') {
		if ((*av).llast == '>') {
			return redir;
		}
		(*cmds).append = 1;
		return 0;
	}
	if ((*av).gt && (*av).fstLet == NULL) {
		return redir;
	}
	if ((*cmds).output != NULL || (*av).gt) {
		return outSpec;
	}
	if (!(*av).lt) {
		addWrd(&(*(*cmds).fstCmd).fstWrd, &(*av).fstLet);
		(*av).gt = 1;
		return 0;
	}
	if ((*av).fstLet == NULL) {
		return redir;
	}
	(*cmds).input = copyWrd(&(*av).fstLet);
	(*av).lt = 0;
	(*av).gt = 1;
	return 0;			
}

int barHnd(struct cmnds *cmds, struct hlpStr *av)
{
	struct cmnd *help;
	if (!(*av).gt && !(*av).lt) {
		addWrd(&(*(*cmds).fstCmd).fstWrd, &(*av).fstLet);
	} else { 
		if ((*av).fstLet == NULL) {
			return nlUnexp;
		}
		if ((*av).gt) {
			(*cmds).output = copyWrd(&(*av).fstLet);
			(*av).gt = 0;
		} else {
			(*cmds).input = copyWrd(&(*av).fstLet);
			(*av).lt = 0;
		}
	}
	if ((*(*cmds).fstCmd).fstWrd == NULL) {
		return nllCmd;
	}
	help = malloc(sizeof(*help));
	(*help).fstWrd = NULL;
	(*help).next = (*cmds).fstCmd;
	(*cmds).fstCmd = help;
	return 0;
}

void endHnd(struct cmnds *cmds, struct hlpStr *av)
{
	if ((*av).next == '\n' && (*av).inQts) {	
		(*cmds).err = unbQts;
	}
	if ((*cmds).err) {
		printe((*cmds).err);
		return;
	}
	if (!(*av).gt && !(*av).lt) {
		addWrd(&(*(*cmds).fstCmd).fstWrd, &(*av).fstLet);
	} else { 
		if ((*av).fstLet == NULL) {
			printe(nlUnexp);
			return;
		}
		if ((*av).gt) {
			(*cmds).output = copyWrd(&(*av).fstLet);
		} else {
			(*cmds).input = copyWrd(&(*av).fstLet);
		}
	}
	if ((*(*cmds).fstCmd).fstWrd == NULL && (*(*cmds).fstCmd).next != NULL) {
		printe(nllCmd);
		return;
	}
	(*cmds).bckGrd = ((*av).next == '\n') ? 0 : 1;
	prpCll(cmds);
}
	
void printe(int err)
{
	if (err == wrArg) {
		printf("cd: wrong number of arguments\n");
		return;
	}
	if (err == nllCmd) {
		printf("Invalid null command\n");
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

void prpCll(struct cmnds *cmds)
{	
	if ((*(*cmds).fstCmd).fstWrd != NULL) {
		if (!(*cmds).bckGrd) {	
			signal(SIGCHLD, SIG_DFL);
		}
		if ((*(*cmds).fstCmd).next == NULL) {
			char **arrWrd;
			arrWrd = copyLst((*(*cmds).fstCmd).fstWrd);
			cllCmd(cmds, arrWrd);
			free(arrWrd);
		} else {
			cllCnv(cmds);
		}
		if (!(*cmds).bckGrd) {
			signal(SIGCHLD, rmvZmb);
		}
	}
}

char **copyLst(struct list *fstWrd)
{
	char **arrWrd;
	int i, n;
	n = numWrd(fstWrd);
	arrWrd = malloc((n+1)*sizeof(*arrWrd));
	for(i = n-1; i >=0; i--) {
		arrWrd[i] = (*fstWrd).word;	
		fstWrd = (*fstWrd).next;
	}
	arrWrd[n] = NULL;
	return arrWrd;
}

int numWrd(struct list *fstWrd)
{
	int help = 0;
	while(fstWrd != NULL) {
		fstWrd = (*fstWrd).next;
		help++;
	}
	return help;
}

void cllCmd(struct cmnds *cmds, char **arrWrd)
{
	int pid, fdi, fdo;
	if (cmpStr(arrWrd[0],"cd")) {
		chdHnd(arrWrd);
		return;
	}
	if ((fdo = openfd(cmds, output)) == -1) {
		perror((*cmds).output);
		return;
	}
	if ((fdi = openfd(cmds, input)) == -1) {
		perror((*cmds).input);
		return;
	}
	pid = fork();
	if (!pid) {
		clsfdc(cmds, fdi, fdo);
		execvp(arrWrd[0],arrWrd);
		perror(arrWrd[0]);
		exit(1);
	}
	clsfdp(cmds, fdi, fdo);
	if (!(*cmds).bckGrd) {
		while(wait(NULL) != pid)
		{}
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

void chdHnd(char **arrWrd)
{
	int i = 0;
	while(arrWrd[i] != NULL) {
		i++;
	}
	if (i != 2) {
		printe(wrArg);
	} else {
		if (chdir(arrWrd[1])) {
			perror(arrWrd[1]);
		}
	}
}

int openfd(struct cmnds *cmds, int i)
{
	int fd = 0;
	if (i == output) {
		if ((*cmds).output != NULL) {
			if ((*cmds).append) {
				fd = open((*cmds).output, O_WRONLY|O_CREAT|O_APPEND, 0666);
			} else {
				fd = open((*cmds).output, O_WRONLY|O_CREAT|O_TRUNC, 0666);
			}
		}
	} else {
		if ((*cmds).input != NULL) {
			fd = open((*cmds).input, O_RDONLY, 0666);
		}
	}
	return fd;
}

void clsfdc(struct cmnds *cmds, int fdi, int fdo)
{
	if ((*cmds).input != NULL) {
		dup2(fdi, 0);
		close(fdi);
	}
	if ((*cmds).output != NULL) {
		dup2(fdo, 1);
		close(fdo);
	}
}

void clsfdp(struct cmnds *cmds, int fdi, int fdo)
{
	if ((*cmds).input != NULL) {
		close(fdi);
	}
	if ((*cmds).output != NULL) {
		close(fdo);
	}
}

void cllCnv(struct cmnds *cmds)
{
	struct pidList *pids = NULL;
	struct cmnd *help;
	int pid;
	int fd[2];
	flipList(cmds); 
	if ((fd[0] = openfd(cmds, input)) == -1) {
		perror((*cmds).input);
		return;
	}
	pid = fstRun(cmds, fd);
	if (!(*cmds).bckGrd) {
		addPid(&pids, pid);
	}
	help = midRun(cmds, &pids, fd);
	if ((fd[1] = openfd(cmds, output)) == -1) {
		perror((*cmds).output);
		return;
	}
	pid = lstRun(cmds, help, fd);
	if (!(*cmds).bckGrd) {
		addPid(&pids, pid);
		while (pids != NULL) {
			pid = wait(NULL);
			delPid(&pids, pid);
		}
	}
}

int fstRun(struct cmnds *cmds, int fdio[2])
{
	char **arrWrd;
	int fd[2];
	int pid;
	arrWrd = copyLst((*(*cmds).fstCmd).fstWrd);
	pipe(fd);
	pid = fork();
	if (!pid) {
		if ((*cmds).input != NULL) {
			dup2(fdio[0], 0);
			close(fdio[0]);
		}
		dup2(fd[1],1);
		close(fd[0]);
		close(fd[1]);
		execvp(arrWrd[0],arrWrd);
		perror(arrWrd[0]);
		exit(1);
	}
	free(arrWrd);
	if ((*cmds).input != NULL) {
		close(fdio[0]);
	}
	close(fd[1]);
	fdio[0] = fd[0];
	return pid;
}

struct cmnd *midRun(struct cmnds *cmds, struct pidList **pids, int fdio[2])
{
	struct cmnd *help;
	char **arrWrd;
	int fd[2];
	int pid, n, i;
	help = (*cmds).fstCmd;
	n = numCmd(help);
	for(i = 0; i < n-2; i++) {
		help = (*help).next;
		pipe(fd);
		arrWrd = copyLst((*help).fstWrd);
		pid = fork();
		if (!pid) {
			dupCls(fdio[0], fd);
			execvp(arrWrd[0],arrWrd);
			perror(arrWrd[0]);
			exit(1);
		}
		free(arrWrd);
		if (!(*cmds).bckGrd) {
			addPid(pids, pid);
		}
		close(fdio[0]);
		close(fd[1]);
		fdio[0] = fd[0];
	}
	return (*help).next;
}

int lstRun(struct cmnds *cmds, struct cmnd *help, int fd[2])
{
	char **arrWrd;
	int pid;
	arrWrd = copyLst((*help).fstWrd);
	pid = fork();
	if (!pid) {
		if ((*cmds).output != NULL) {
			dup2(fd[1], 1);
			close(fd[1]);
		}
		dup2(fd[0],0);
		close(fd[0]);
		execvp(arrWrd[0],arrWrd);
		perror(arrWrd[0]);
		exit(1);
	}
	free(arrWrd);
	if ((*cmds).output != NULL) {
		close(fd[1]);
	}
	close(fd[0]);
	return pid;
}

void dupCls(int fdio, int fd[2])
{
	dup2(fdio, 0);
	close(fdio);
	dup2(fd[1],1);
	close(fd[0]);
	close(fd[1]);
}

void flipList(struct cmnds *cmds)
{
	struct cmnd *help;
	struct cmnd *last;
	struct cmnd *new = NULL;
	last = (*cmds).fstCmd;
	while(last != NULL) {
		help = malloc(sizeof(*help));
		(*help).fstWrd = (*last).fstWrd;
		(*help).next = new;
		new = help;
		help = last;
		last = (*last).next;
		free(help);
	}
	(*cmds).fstCmd = new;
}

int numCmd(struct cmnd *fstCmd)
{
	int help = 0;
	while(fstCmd != NULL) {
		fstCmd = (*fstCmd).next;
		help++;
	}
	return help;
}

void addPid(struct pidList **pids, int pid)
{
	struct pidList *help;
	help = malloc(sizeof(*help));
	(*help).pid = pid;
	(*help).next = *pids;
	*pids = help;
}

void delPid(struct pidList **pids, int pid)
{
	struct pidList *help;
	if ((**pids).pid == pid) {
		help = *pids;
		*pids = (**pids).next;
		free(help);
		return;
	}
	struct pidList *list;
	list = *pids;
	while((*list).next != NULL) {
		if ((*(*list).next).pid == pid) {
			help = (*list).next;
			(*list).next = (*(*list).next).next;
			free(help);
			return;
		}
		list = (*list).next; 
	}
}

void ersVar(struct cmnds *cmds, struct hlpStr *av)
{
	delCmd(&(*cmds).fstCmd);
	(*cmds).fstCmd = malloc(sizeof(*(*cmds).fstCmd));
	(*(*cmds).fstCmd).fstWrd = NULL;
	(*(*cmds).fstCmd).next = NULL;
	if ((*cmds).output != NULL) {
		free((*cmds).output);
		(*cmds).output = NULL;
	}
	if ((*cmds).input != NULL) {	
		free((*cmds).input);
		(*cmds).input = NULL;
	}
	(*cmds).append = (*cmds).bckGrd = (*cmds).err = 0;
	delLet(&((*av).fstLet)); 
	(*av).llast = (*av).last = (*av).next = ' ';	
	(*av).inQts = (*av).gt = (*av).lt = 0;
}

void delCmd(struct cmnd **fstCmd)
{
	struct cmnd *help;
	while(*fstCmd != NULL) {
		delWrd(&(**fstCmd).fstWrd);
		help = *fstCmd;
		*fstCmd = (**fstCmd).next;
		free(help);
	}
}

void delWrd(struct list **fstWrd)
{
	struct list *help;
	while(*fstWrd != NULL) {
		free((**fstWrd).word);
		help = *fstWrd;
		*fstWrd = (**fstWrd).next;
		free(help);
	}
}

void delLet(struct word **fstLet)
{
	struct word *help;
	while(*fstLet != NULL) {
		help = *fstLet;
		*fstLet = (**fstLet).next;
		free(help);
	}
}
