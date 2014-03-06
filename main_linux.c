#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/resource.h>
#include "core.h"

int main(int argc, char **argv)
{
	int i;
	int n = fetch_forks(argc, argv, sysconf(_SC_NPROCESSORS_ONLN));
	pid_t pid[n];
	for (i = 0; i < n; ++i) {
		pid[i] = fork();
		if (pid[i] == 0) {
			/*lowest scheduling priority*/
			setpriority(PRIO_PROCESS, 0, 19);
			child(i);
		}
		if (pid[i] == -1) {
			printf("Fork %d/%d failed\n", i + 1, n);
			break;
		}
	}
	parent(i);
	while (i)
		kill(pid[--i], SIGINT);
	return 0;
}
