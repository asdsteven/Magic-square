#include <stdio.h>
#include <string.h>
#include <windows.h>
#include "core.h"

int main(int argc, char **argv)
{
	if (!strcmp(argv[0], "child")) {
		job_volume = atoi(argv[1]);
		child(atoi(argv[2]));
	}
	int i;
	SYSTEM_INFO sysinfo;
	GetSystemInfo(&sysinfo);
	int n = fetch_forks(argc, argv, sysinfo.dwNumberOfProcessors);
	STARTUPINFO si = {sizeof(si)};
	PROCESS_INFORMATION pinfo[n];
	for (i = 0; i < n; ++i) {
		char s[50];
		sprintf(s, "child %d %d", job_volume, i);
		if (CreateProcess(argv[0], s, 0, 0, FALSE,
				IDLE_PRIORITY_CLASS, 0, 0,
				&si, pinfo + i) == FALSE) {
			printf("Fork %d/%d failed with error %d\n",
			i + 1, n, GetLastError());
			break;
		}
	}
	parent(i);
	while (i)
		TerminateProcess(pinfo[--i].hProcess, 0);
	return 0;
}
