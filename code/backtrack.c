#include <stdio.h>

int weight[100] = {0};
int x[100] = {0};
int n, s, cw=0;
void backtrack(int t)
{
	if (t > n-1) {
/*		if (cw + weight[t] == s) {
			x[t]=1;
			printf("YES\n");
			return;
		} else {
			x[t]=0;
			printf("NO\n");
			return;
		} */
		printf("NO\n");
		return;
	}
	if (cw + weight[t] < s) {
		x[t] = 1;
		cw += weight[t];
		backtrack(t+1);
		cw -= weight[t];
	} else if (cw + weight[t] == s) {
		x[t] = 1;
		printf("YES\n");
		for (int i=0;i<n;i++) {
			printf("%d ", x[i]);
		}
		printf("\n");
		return ;
	} else {
		x[t] = 0;
		backtrack(t+1);
	}
	
}

void knapsack(int s, int n)
{
	int sum=0;
	int i;

	for (i = 0; i < n; i++)
		sum += weight[i];
	backtrack(0);
}
int main()
{
	int i;

	scanf("%d %d\n", &n, &s);
	for (i = 0; i < n; i++) {
		scanf("%d", &weight[i]);
	}
	knapsack(s, n);
	return 0;
}
