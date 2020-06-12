
double Bound(int i)
{
	int rp = 0;
	while (i <= n) {
		rp += v[i];
		i++;
	}
	return cp + rp;
}

void Backtrack(int t)
{
	if (t > n) {
		for (j = 1;j <= n; j++) {
			bestx[i] = x[j];
		}
		bestp = cp;
		return;
	}
	if (cw + w[i] <= W) {
		x[t] = 1;
		cw+=w[t];
		cp+=v[t];
		Backtrack(t+1);
		cw-=w[t];
		cp-=v[t];
	}
	if (Bound(t+1) > bestp) {
		x[t] = 0;
		Backtrack(t+1);
	}
}
