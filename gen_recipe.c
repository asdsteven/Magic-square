#include <stdio.h>
#include <string.h>

void gen_recipe(int d)
{
	int i, j, k;
	int line[d * 2 + 2][2];
	int stack[14 * 14], top;
	int current[d * 2 + 2], count[d * 2 + 2], cur = 0;
	int answer[d * 2 + 2], list[d * 2 + 2];
	int a[d * d], used[d * 2 + 2];
	int equal = 0, better = 0;
	
	/**
	 * the i(th) line is described by
	 * line[i][0] + n * line[i][1]
	 */
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
	memset(used, 0, sizeof(used));
	for (i = 0; i < d * 2 + 2; ++i)
		answer[i] = 10000;
	
	for (top = 0; top < (d + 1) / 2 + 1; ++top)
		stack[top] = top;
	while (top) {
		k = stack[top - 1];
		if (k >= 10000) { /*visited*/
			used[k -= 10000] = 0;
			--top;
			if (equal == cur)
				--equal;
			--cur;
			j = line[k][0];
			for (i = 0; i < d; ++i) {
				--a[j];
				j += line[k][1];
			}
			continue;
		}
		
		j = line[k][0];
		count[cur] = 0;
		for (i = 0; i < d; ++i) {
			count[cur] += !a[j]++;
			j += line[k][1];
		}
		current[cur++] = k;
		used[k] = 1;
		stack[top - 1] += 10000; /*mark as visited*/
		if (equal == cur - 1)
			if (count[cur - 1] == answer[cur - 1])
				++equal;
			else if (count[cur - 1] > answer[cur - 1])
				continue;
		
		for (i = 0; i < d * 2 + 2; ++i)
			if (!used[i])
				stack[top++] = i;
		if (cur == d * 2 + 2 && equal < cur)
			for (i = 0; i < cur; ++i) {
				answer[i] = count[i];
				list[i] = current[i];
			}
	}
	
	for (i = 0; i < d * 2 + 2; ++i) {
		if (answer[i] == 0) {
			printf("-5, %d, %d,\n", line[list[i]][0], line[list[i]][1]);
			continue;
		}
		j, k = line[list[i]][0];
		for (j = 0; j < d; ++j) {
			if (!a[k]) {
				if (--answer[i])
					printf("%d, ", k);
				a[k] = 1;
			}
			k += line[list[i]][1];
		}
		printf("-3, %d, %d,\n", line[list[i]][0], line[list[i]][1]);
	}
}

int main()
{
	int i;
	for (i = 3; i <= 6; ++i) {
		printf("int recipe_book%d[] = {\n", i);
		gen_recipe(i);
		printf("};\n\n");
		fflush(stdout);
	}
}
