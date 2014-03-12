#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <setjmp.h>
#include <time.h>
#include <math.h>
#include <mysql/mysql.h>
#include "recipe.h"

int D, N, sum, e, E;
int *recipe;
char a[37], c[37], used[37];

char *s, *u;
unsigned M, len;

int sym[] = {0, 1, 0, 8, 32, 32, 192};

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

char* fetch_item()
{
	MYSQL_RES *result;
	MYSQL_ROW row;
	result = mysql_store_result(&mysql);
	if (!result)
		longjmp(exception, 1);
	row = mysql_fetch_row(result);
	mysql_free_result(result);
	if (!row)
		longjmp(exception, 1);
	return row[0];
}

void connect_db()
{
	if (!mysql_real_connect(mysql_init(&mysql),
			"127.0.0.1", "client",
			"1234", "db", 0, NULL, 0))
		longjmp(exception, 1);
}

void disconnect_db()
{
	mysql_close(&mysql);
}

int f(int i, int d)
{
	int s = a[i] + a[i += d] + a[i += d];
	switch (D) {
	case 6: s += a[i += d];
	case 5: s += a[i += d];
	case 4: s += a[i += d];
	}
	return s;
}

void search(int l)
{
	if (l == e || E) {
		int i = u - s;
		E = E || recipe[e] != -1;
		if (i + 41 >= len) {
			char *p = calloc(len *= 2, sizeof(char));
			if (p == NULL) {
				perror("\n");
				exit(-1);
			}
			u = p + i;
			memcpy(p, s, i);
			free(s);
			s = p;
		}
		if (M++) {
			*u++ = ',';
		} else {
			for (i = 0; i < N; ++i)
				c[i] = a[i] + '0';
			c[N] = 0;
		}
		*u++ = '(';
		*u++ = '\"';
		for (i = 0; i < N; ++i)
			*u++ = a[i] + '0';
		*u++ = '\"';
		*u++ = ')';
		return;
	}
	int A = 1, B = N;
	if (recipe[l] == -2) {
		A = a[recipe[l + 1]] + 1;
		B = recipe[l + 2];
		l += 3;
	}
	if (recipe[l] < -1) {
		int s = sum - f(recipe[l + 1], recipe[l + 2]);
		if (recipe[l] == -5) {
			if (s == 0)
				search(l + 3);
		} else if (A <= s && s <= B && !used[s]) {
			int i = recipe[l + 1];
			while (a[i])
				i += recipe[l + 2];
			a[i] = s;
			used[s] = 1;
			search(l + 3);
			used[s] = 0;
			a[i] = 0;
		}
	} else {
		char *i = a + recipe[l];
		for (*i = A; *i <= B; ++*i)
			if (!used[*i]) {
				used[*i] = 1;
				search(l + 1);
				used[*i] = 0;
			}
		*i = 0;
	}
}

void fetch_job()
{
	int i, size, l;
	queryf("LOCK TABLES jobs WRITE");
	queryf("SELECT c FROM jobs ORDER BY LENGTH(c), priority, "
		"LENGTH(REPLACE(c, \"0\", \"\")) DESC, c LIMIT 1");
	strcpy(a, fetch_item());
	queryf("UPDATE jobs SET priority=priority+1 WHERE c=\"%s\"", a);
	queryf("UNLOCK TABLES");
	N = strlen(a);
	D = sqrt(N);
	sum = D * (N + 1) / 2;
	switch (D) {
	case 3: recipe = recipe_book3; e = 0; break;
	case 4: recipe = recipe_book4; e = 0; break;
	case 5: recipe = recipe_book5; e = 8; break;
	case 6: recipe = recipe_book6; e = 40; break;
	}
	memset(used, 0, sizeof(used));
	for (i = 0, size = 0; i < N; ++i)
		if (a[i] -= '0') {
			used[a[i]] = 1;
			++size;
		}
	for (i = 0; i < N; ++i)
		printf("%2d%c", a[i], (i + 1) % D ? ' ' : '\n');
	fflush(stdout);
	for (l = 0; size; )
		switch (recipe[l]) {
		case -2: l += 3; break;
		case -3: l += 3; --size; break;
		case -5: l += 3; --size; break;
		default: l += 1; --size; break;
		}
	if (l == e) {
		u = s + sprintf(s, "INSERT INTO solutions%d VALUES ", D);
		while (recipe[e] != -1)
			++e;
	} else {
		u = s + sprintf(s, "INSERT INTO jobs (c) VALUES ");
	}
	M = 0;
	E = 0;
	search(l);
	for (i = 0; i < N; ++i)
		a[i] += '0';
	if (recipe[e] == -1)
		queryf("LOCK TABLES jobs WRITE, solutions%d WRITE", D);
	else
		queryf("LOCK TABLES jobs WRITE");
	queryf("SELECT COUNT(*) FROM jobs WHERE c=\"%s\"", a);
	if (atoi(fetch_item())) {
		if (M) {
			if (recipe[e] == -1)
				queryf("SELECT COUNT(*) FROM solutions%d WHERE c=\"%s\"", D, c);
			else
				queryf("SELECT COUNT(*) FROM jobs WHERE c=\"%s\"", c);
			if (!atoi(fetch_item()) && mysql_real_query(&mysql, s, u - s))
				longjmp(exception, 1);
		}
		queryf("DELETE FROM jobs WHERE c=\"%s\"", a);
	}
	queryf("UNLOCK TABLES");
	if (recipe[e] == -1)
		printf("%d magic squares found.\n", M);
	else
		printf("%d new jobs.\n\n", M);
	fflush(stdout);
}

void h(int t)
{
	printf("%02d:%02d:%02d", t / 3600, t / 60 % 60, t % 60);
}

void child(int n)
{
	int seconds[7], samples[7], max[7];
	char logfile[50];
	sprintf(logfile, "proc_%d.txt", n);
	if (!freopen(logfile, "w", stdout)) {
		perror("Error making logfile");
		exit(-1);
	}
	len = 262144;
	s = calloc(len, sizeof(char));
	if (s == NULL) {
		perror("\n");
		exit(-1);
	}
	memset(seconds, 0, sizeof(seconds));
	memset(samples, 0, sizeof(samples));
	memset(max, 0, sizeof(max));
	connect_db();
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
			connect_db();
			continue;
		}
		fetch_job();
		if (recipe[e] == -1) {
			t = time(NULL) - t;
			seconds[D] += t;
			samples[D] += 1;
			if (t > max[D])
				max[D] = t;
			h(t);
			printf(", ");
			h(max[D]);
			printf(", ");
			h(seconds[D]);
			printf("/%d = ", samples[D]);
			h(seconds[D] / samples[D]);
			printf("/job\n\n");
			fflush(stdout);
		}
	}
}

void print_stat()
{
	int i;
	MYSQL_ROW row;
	time_t t;
	for (i = 3; i <= 6; ++i) {
		t = time(NULL);
		queryf("SELECT %d*COUNT(*) FROM solutions%d", sym[i], i);
		printf(" Dimension %d: %15s\t", i, fetch_item());
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
			fprintf(fp, "%2d%c", row[0][i], (i + 1) % D ? ' ' : '\n');
		fputc('\n', fp);
		if (++j == 1e9) {
			++jj;
			j = 0;
		}
		if (~j & 8191) {
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
	printf("+------------------------------------------------+\n");
	printf("|    1    Print statistics                       |\n");
	printf("|    2    Generate element_magic_square.txt      |\n");
	printf("|    3    Quit                                   |\n");
	printf("|                                                |\n");
	printf("                                    -------------+\r");
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
	printf("/~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\\\n");
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
	printf("\\________________________________________________/\n\n\n");
	goto loop;
}

int fetch_forks(int argc, char **argv, int processors)
{
	printf(
	"Usage: %s [-f n]\n"
	"  -f  If n = 0, 1 process per core is forked (Default)\n"
	"      If n > 0, exactly n processes are forked\n"
	"      If n < 0, |n| cores are reserved unused\n"
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
