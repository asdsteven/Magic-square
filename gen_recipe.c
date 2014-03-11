/**
 * Used to generate "recipe.h".
 * Recipes elements are of variable lengths :
 * A		- exhaust element A
 * -2 A	B	- states that element[A] < next_element <= B
 *            to avoid symmetric isotopes
 * -3 A B	- calculate element in A + n * B
 * -5 A B	- check if A + n * B is consistent
 * -1		- end of recipe
 *
 * Manual modification to recipe.h is still needed
 */

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
	line[1][0] = d - 1;
	line[1][1] = d - 1;
	for (i = 0; i < d; ++i) {
		line[i + 2][0] = i * d;
		line[i + 2][1] = 1;
		line[i + d + 2][0] = i;
		line[i + d + 2][1] = d;
	}
	
	memset(a, 0, sizeof(a));
	for (i = 0; i <= d * 2 + 2; ++i) {
		answer[i] = 10000;
		ids(0, 0, i);
	}
}

void print_symmetry(int k)
{
	if (k == 0) {
		printf("-2, 0, %d, ", d * d - d / 2 * 4 + 1);
	} else if (k % (d + 1) == 0) {
		if (k / (d + 1) < d / 2)
			printf("-2, %d, %d, ", k - (d + 1), d * d - (d / 2 - k / (d + 1)) * 2 + 1);
		else if (k / (d + 1) >= d - d / 2)
			printf("-2, %d, %d, ", (d - 1 - k / (d + 1)) * (d + 1), d * d);
	} else if (k % (d - 1) == 0) {
		printf("-2, 0, %d, ", d * d);
	}
}

void print_recipe()
{
	int i, j, k;
	memset(a, 0, sizeof(a));
	for (i = 0; i < d * 2 + 2; ++i)
		if (answer[i] == 0) {
			printf("-5, %d, %d,\n", line[list[i]][0], line[list[i]][1]);
		} else {
			k = line[list[i]][0];
			for (j = 0; j < d; ++j) {
				if (!a[k]++) {
					print_symmetry(k);
					if (--answer[i])
						printf("%d, ", k);
					else
						printf("-3, %d, %d,\n", line[list[i]][0], line[list[i]][1]);
				}
				k += line[list[i]][1];
			}
		}
}

void print_grid()
{
	int i, j;
	printf("/**\n");
	for (i = 0; i < d; ++i) {
		printf(" *");
		for (j = 0; j < d; ++j)
			printf(" %2d", i * d + j);
		printf("\n");
	}
	printf(" */\n");		
}

int main()
{
	for (d = 3; d <= 6; ++d) {
		print_grid();
		printf("int recipe_book%d[] = {\n", d);
		gen_recipe();
		print_recipe();
		printf("-1\n};\n\n");
		fflush(stdout);
	}
	return 0;
}
