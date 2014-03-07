#include <stdio.h>
#include <string.h>

int d;
int line[14][2];
int a[36];
int answer[15], list[14];

int ids(int n, int used, int depth)
{
	if (n == d * 2 + 2)
		return 1;
	int i, j, s, x;
	for (i = 0; i < d * 2 + 2; ++i)
		if (~used & 1 << i) {
			s = 0;
			x = line[i][0];
			for (j = 0; j < d; ++j) {
				s += !a[x]++;
				x += line[i][1];
			}
			if (s <= answer[n]) {
				answer[n] = s;
				if (n < depth && ids(n + 1, used | 1 << i, depth)) {
					list[n] = i;
					return 1;
				}
			}
			x = line[i][0];
			for (j = 0; j < d; ++j) {
				--a[x];
				x += line[i][1];
			}
		}
	return 0;
}

void gen_recipe()
{
	int i, j, k;
	
	line[0][0] = 0;
	line[0][1] = d + 1;
	for (i = 0; i < d; ++i) {
		line[i + 1][0] = i * d;
		line[i + 1][1] = 1;
		line[i + d + 1][0] = i;
		line[i + d + 1][1] = d;
	}
	line[d * 2 + 1][0] = d - 1;
	line[d * 2 + 1][1] = d - 1;
	
	memset(a, 0, sizeof(a));
	for (i = 0; i <= d * 2 + 2; ++i) {
		answer[i] = 10000;
		ids(0, 0, i);
	}
	
	memset(a, 0, sizeof(a));
	for (i = 0; i < d * 2 + 2; ++i)
		if (answer[i] == 0) {
			printf("-5, %d, %d,\n", line[list[i]][0], line[list[i]][1]);
		} else {
			k = line[list[i]][0];
			for (j = 0; j < d; ++j) {
				if (!a[k]++ && --answer[i])
					printf("%d, ", k);
				k += line[list[i]][1];
			}
			printf("-3, %d, %d,\n", line[list[i]][0], line[list[i]][1]);
		}
}

int main()
{
	for (d = 3; d <= 6; ++d) {
		printf("int recipe_book%d[] = {\n", d);
		gen_recipe();
		printf("};\n\n");
		fflush(stdout);
	}
}
