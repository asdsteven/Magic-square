#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <setjmp.h>
#include <time.h>
#include <math.h>
#include <mysql/mysql.h>
#include "recipe.h"

int D, N, sum, e, lock;
int *recipe;
char a[37], used[37];

char s[336000], *t, *u;
int M;

jmp_buf exception;

MYSQL mysql;

void queryf(const char *format, ...)
{
	static char s[1000];
	va_list arg;
	va_start(arg, format);
	vsprintf(s, format, arg);
	va_end(arg);
	if (mysql_query(&mysql, s))
		longjmp(exception, 1);
}

MYSQL_ROW fetch_row()
{
	MYSQL_RES *result;
	MYSQL_ROW row;
	result = mysql_store_result(&mysql);
	if (!result)
		longjmp(exception, 1);
	row = mysql_fetch_row(result);
	mysql_free_result(result);
	return row;
}

void connect_db()
{
	if (!mysql_real_connect(mysql_init(&mysql),
			"localhost", "client",
			"1234", "db", 0, NULL, 0))
		longjmp(exception, 1);
}

void disconnect_db()
{
	mysql_close(&mysql);
}

int f(int i, int d)
{
	int s = a[i] + a[i + d] + a[i + d + d];
	switch (D) {
	case 6: s += a[i + 5 * d];
	case 5: s += a[i + 4 * d];
	case 4: s += a[i + 3 * d];
	}
	return s;
}

void search(int l)
{
	int A = 1;
	if (recipe[l] == -2) {
		A = a[recipe[l + 1]] + 1;
		l += 2;
	}
	if (recipe[l] < -1) {
		int s = sum + '0' - f(recipe[l + 1], recipe[l + 2]);
		if (recipe[l] == -5) {
			if (s == 0)
				search(l + 3);
		} else if (A <= s && s <= N && !used[s]) {
			int i = recipe[l + 1];
			while (a[i])
				i += recipe[l + 2];
			a[i] = s;
			used[s] = 1;
			search(l + 3);
			used[s] = 0;
			a[i] = '0';
		}
	} else if (recipe[l] == -1 || l == e && lock) {
		if (++M & 8191) {
			u += sprintf(u, "(\'%s\'),", a);
		} else {
			sprintf(u, "(\'%s\')", a);
			u = t;
			if (mysql_query(&mysql, s))
				longjmp(exception, 1);
		}
	} else {
		int *i = a + recipe[l];
		for (*i = A; *i <= N; ++*i)
			if (!used[*i]) {
				used[*i] = 1;
				search(l + 1);
				used[*i] = 0;
			}
		*i = '0';
	}
}

void fetch_job()
{
	MYSQL_ROW row;
	MYSQL_RES *result;
	int i;
	queryf("LOCK TABLES jobs WRITE");
	queryf("SELECT c FROM jobs ORDER BY LENGTH(c), priority, l DESC LIMIT 1");
	row = fetch_row();
	if (!row)
		longjmp(exception, 1);
	D = sqrt(N = strlen(strcpy(a, row[0])));
	queryf("UPDATE jobs SET priority=priority+1 WHERE c=%s", a);
	switch (D) {
	case 3: recipe = recipe_book3; e = 0; break;
	case 4: recipe = recipe_book4; e = 0; break;
	case 5: recipe = recipe_book5; e = 5; break;
	case 6: recipe = recipe_book6; e = 35; break;
	}
	sum = D * (N + 1) / 2;
	memset(used, 0, sizeof(used));
	l = 0;
	lock = 1;
	for (i = 0; i < N; ++i)
		if (a[i] != '0') {
			used[a[i]] = 1;
			l = e;
			lock = 0;
		}
	if (!lock)
		queryf("UNLOCK TABLES");
	for (i = 0; i < N; ++i)
		printf("%2d%c", (int)a[i], (i + 1) % D ? ' ' : '\n');
	fflush(stdout);
	if (l >= e)
		u = t = s + sprintf(s, "INSERT IGNORE INTO solutions%d VALUES ", D);
	else
		u = t = s + sprintf(s, "INSERT IGNORE INTO jobs (c, l) VALUES ");
	M = 0;
	search(l);
	u[-1] = '\0';
	if (u != t && mysql_query(&mysql, s))
		longjmp(exception, 1);
	if (l >= e)
		printf("%d magic squares found.\n", M);
	else
		printf("%d new jobs.\n\n", M);
	fflush(stdout);
	queryf("DELETE FROM jobs WHERE c=%s", a);
	if (lock)
		queryf("UNLOCK TABLES");
}

void h(int t)
{
	printf("%02d:%02d:%02d", t / 3600, t / 60 % 60, t % 60);
}

void child(int n)
{
	int seconds = 0, samples = 0;
	char logfile[50];
	sprintf(logfile, "proc_%d.txt", n);
	if (!freopen(logfile, "w", stdout)) {
		perror("Error making logfile");
		exit(-1);
	}
	while (1) {
		time_t t = time(NULL);
		if (setjmp(exception)) { /*Handle MySQL errors*/
			fprintf(stderr, "\nError [%d] %s\n",
				mysql_errno(&mysql), mysql_error(&mysql));
			printf("\nError [%d] %s\n",
				mysql_errno(&mysql), mysql_error(&mysql));
			disconnect_db();
			t = time(NULL);
			while (time(NULL) - t < 60); /*Wait a minute*/
			continue;
		}
		connect_db();
		fetch_job();
		if (l >= e) {
			t = time(NULL) - t;
			seconds += t;
			samples += 1;
			h(t);
			printf(", ");
			h(seconds);
			printf("/%d = ", samples);
			h(seconds / samples), printf("/job\n\n");
			fflush(stdout);
		}
		disconnect_db();
	}
}

void print_stat()
{
	int i;
	MYSQL_ROW row;
	time_t t;
	for (i = 3; i <= 6; ++i) {
		t = time(NULL);
		queryf("SELECT COUNT(*) FROM solutions%d", i);
		row = fetch_row();
		if (!row)
			longjmp(exception, 1);
		printf("Dimension %d: %15s\t", i, row[0]);
		h(time(NULL) - t);
		putchar('\n');
	}
}

char *ll(int a, int b)
{
	static char s[50];
	if (a)
		sprintf(s, "%6d%09d", a, b);
	else
		sprintf(s, "%15d", b);
	return s;
}

void print(FILE *fp)
{
	MYSQL_ROW row;
	MYSQL_RES *result;
	int i, j = 0, jj = 0;
	time_t t;
	printf("Dimension %d:\n", D);
	printf("Sending query...\t");
	t = time(NULL);
	queryf("SELECT * FROM solutions%d", D);
	h(time(NULL) - t);
	putchar('\n');
	result = mysql_use_result(&mysql);
	if (!result)
		longjmp(exception, 1);
	t = time(NULL);
	while (row = mysql_fetch_row(result)) {
		for (i = 0; i < N; ++i)
			fprintf(fp, "%2d%c",
				row[0][i], (i + 1) % D ? ' ' : '\n');
		fputc('\n', fp);
		if (++j == 1e9) {
			++jj;
			j = 0;
		}
		if (!(j & 8191)) {
			printf("Printed %s\t", ll(jj, j));
			h(time(NULL) - t);
			putchar('\r');
			fflush(stdout);
		}
	}
	printf("Printed %s\t", ll(jj, j));
	h(time(NULL) - t);
	printf("\n\n");
	mysql_free_result(result);
}

void parent(int n)
{
	int action;
	char c;
	FILE *fp = NULL;
	printf(":::::::::Forked %d processes:::::::\n", n);
	loop:
	printf("+-------------------------------------------------+\n");
	printf("|       1       Print statistics		          |\n");
	printf("|       2       Generate magic_square.txt	      |\n");
	printf("|       3       Quit			                  |\n");
	printf("|						                          |\n");
	printf("				                    --------------+\r");
	printf("+------  Action number: ");
	fflush(stdout);
	scanf("%d", &action);
	if (action == 3) {
		puts("Bye.");
		return;
	}
	do { /*clear stdin*/
		c = getchar();
	} while (c != EOF && c != '\n');
	printf("/~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\\\n");
	if (setjmp(exception)) {
		printf("\nError [%d] %s\n", mysql_errno(&mysql), mysql_error(&mysql));
		if (fp != NULL)
			fclose(fp);
	} else {
		connect_db();
		switch (action) {
		case 1:
			print_stat();
			break;
		case 2:
			if (!(fp = fopen("magic_square.txt", "w"))) {
				perror("Can't open magic_square.txt");
				break;
			}
			for (D = 3; D <= 6; ++D) {
				N = D * D;
				print(fp);
			}
			fclose(fp);
			fp = NULL;
			break;
		default:
			puts("What?");
		}
	}
	disconnect_db();
	printf("\\_________________________________________________/\n\n\n");
	goto loop;
}

int fetch_forks(int argc, char **argv, int processors)
{
	printf(
	"Usage: %s [-f n]\n"
	"  -f  If n = 0, 1 process per core is forked (Default)\n"
	"	   If n > 0, exactly n processes are forked\n"
	"	   If n < 0, |n| cores are reserved unused\n"
	"\n"
	"Examples:\n"
	"  %s -f 4    #fork 4 processes\n"
	"  %s -f -1	  #reserve 1 core unused\n"
	"\n", argv[0], argv[0], argv[0]);
	int n = 0;
	int bad = ~argc & 1;
	int i;
	for (i = 1; i + 1 < argc && !bad; i += 2)
		if (!strcmp(argv[i], "-f"))
			n = atoi(argv[i + 1]);
		else
			bad = 1;
	if (bad) {
		puts("Invalid Usage");
		exit(-1);
	}
	if (n == 0)
		return processors;
	else if (n > 0)
		return n;
	else
		return processors + n;
}