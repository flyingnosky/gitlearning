int map[N][M];
int visited[N][M];
int step_x[]= {-1,1,0,0},step_y[]= {0,0,-1,1}; //上下左右四个方向
int flag;

void DFS(int x, int y)
{
	visited[x][y] = 1;
	if (map[x][y] == 'T') {
		flag = 1;
		return;
	}
	for (int i = 0; i < 4; i++) {
		int xx = x +step_x[i];
		int yy = y +step_y[i];
		if (map[xx][yy] != '*' && !visited[xx][yy] &&
			xx >= 0 && xx < n && yy >= 0 && y<m) {
			DFS(xx, yy);
		}
	}
}
