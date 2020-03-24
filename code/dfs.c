int iSize = 6;
char cVertex[] = { 'A','B','C','D','E','F' };
bool bAdj[] = { 0,1,1,1,0,0, 1,0,0,0,1,0, 1,0,0,0,0,1, 1,0,0,0,0,0, 0,1,0,0,0,1, 0,0,1,0,1,0 };
bool bVisited[6 * 6];
//深度优先遍历（递归实现）
void depthFirstTraversal(int v){	    
	if (v < 0 || v >= iSize)	    
	{		        
		return;	    
	}	    
	cout << cVertex[v]<<"\t";	    
	bVisited[v] = true;	    
	for (int i = 0; i < iSize; i++)	    {		       
		if (bVisited[i] == false && bAdj[v*iSize + i] == true）{			            
			depthFirstTraversal(i);		        
		}	    
	}
}
