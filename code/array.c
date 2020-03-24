int x[100];

void myarray(int t)
{
	if (t>n) {
		//print array
	}
	for (i=t;i<=n;i++) {
		swap(x[t], x[i]);
		myarray(t+1);
		swap(x[t], x[i]);
	}
}
