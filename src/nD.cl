
__kernel void k_init1D(
   int initVal,
   int rows,				// only to make the call consistent with init2D
   int cols,				// only to make the call consistent with init2D
   __global uchar* g_data
)
{
   int i, j;
   int idx;
   int wrk;

   j = get_global_id(0);					// j is the thread id
   wrk = get_local_size(0);					// wrk is how many bytes this call is to process
   idx = j * wrk;							// idx is the start of the array that this call is to process
   g_data[idx] = j;							// remember j 
   g_data[idx+1] = wrk;						// remember wrk

   for (i = idx+2; i < (idx + wrk); i++)	// set all of the other values to the initVal parameter to show that we've been here 
   {
		g_data[i] = initVal;
   }
}

__kernel void k_init2D(
   int initVal,
   int rows,
   int cols,
   __global uchar* g_data
)
{
   int i, j, callx, cally;
   int startPos;
   int colsPerCall, rowsPerCall;
   int chunksPerRow;
   
   int tileRow, tileSize, tileOffset;

   callx = get_global_id(0);
   cally = get_global_id(1);
   
   colsPerCall = get_local_size(0);
   rowsPerCall = get_local_size(1);
   
   chunksPerRow = cols / colsPerCall;
   
   tileRow = callx /chunksPerRow;		// break the linear data store into tiles
   tileSize = rowsPerCall * cols;		// of size local_size(0) times local_size(1)
   tileOffset = callx % chunksPerRow;	// in order to figure out where to start

   startPos = (tileRow * tileSize) + (tileOffset * colsPerCall);

//	to see the local variables uncomment these lines and comment out the for loop below   
//   i = callx * 16;
//   g_data[i++] = callx;
//   g_data[i++] = cally;
//   g_data[i++] = cols;
//   g_data[i++] = rows;
//   g_data[i++] = colsPerCall;
//   g_data[i++] = rowsPerCall;
//   g_data[i++] = chunksPerRow;
//   g_data[i++] = tileRow;
//   g_data[i++] = tileSize;
//   g_data[i++] = tileOffset;
//   g_data[i++] = startPos;
   
   
   for (i = startPos; i < (startPos + (cols * rowsPerCall)); i = i + cols)	 
   {
		for (j = i; j < (i + colsPerCall); j++)
			g_data[j] = initVal;

   }
	g_data[startPos] = callx;		// over write the first value with the call id (get_global_id(0))
	g_data[startPos+1] = cally;		// over write the second value with (get_global_id(1))
}

