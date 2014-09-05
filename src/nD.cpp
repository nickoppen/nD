
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdcl.h>
#include <errno.h>

void debugReset(char * debugFile)
/*
 * Open and close the debug file to create and clear it
 */
{
	FILE * pFile;
	pFile = fopen(debugFile, "w");
	fclose(pFile);

}
void debugdebug(char * fileName, char * msg)
/*
 * Open the debug file and write the message to it
 * Close it again to flush the write buffer
 */
{
	   FILE * pFile;
	   pFile = fopen(fileName, "a");
	   fprintf(pFile, msg);
	   fclose (pFile);

}

void exKernel(void * clHandle, clndrange_t * ndr, char * kernelName, int rows, int cols, cl_uchar * wrkArea, char * debugFile)
{
	cl_kernel krn;
	char msg[255];

	sprintf(msg, "Getting the kernel %s with clsym\n", kernelName);
	debugdebug(debugFile, msg);

	krn = clsym(stdacc, clHandle, kernelName, CLLD_NOW);   // grab the kernel from the program just loaded

	sprintf(msg, "calling clForka with rows: %d and cols: %d\n", rows, cols);
	debugdebug(debugFile, msg);
	clforka(stdacc, 0, krn, ndr, CL_EVENT_WAIT, 1, rows, cols, wrkArea);

	debugdebug(debugFile, (char*)"Transferring memory contents from the Epiphany using clmsync\n");
	clmsync(stdacc, 0, wrkArea, CL_MEM_HOST|CL_EVENT_WAIT);

}

/////////////////////////////////// MAIN PROGRAM ///////////////////////////////////

int main (int argc, char * argv[])
{

   char kernFile[] = "../src/nD.cl";	// the default location of the cl file - can be over ridden by the first command line arguement
   char debugFile[] = "./debug";		// the debug file location
   char msg[255];						// space for the debug message where a program variable is to be written to the debug file
   char * clFile;						// the cl file string actually used (either argv or kernFile)
   void * openHandle;					// the return value to clOpen
   int	bytesPerCore = 16;				// how many bytes we want each core to process
   int workItems = 32; 					// the total number workItems (threads sent to the Epiphany)
   int	i;								// loop counter
   cl_uchar * wrkArea1D;					// the pointer to the malloc'd space (bytesPerCore * workItems)
   cl_uchar * wrkArea2D;

//	These variables will be useful when I get getDeviceInfo and getBuildInfo working
//   unsigned int space;					// the space required for clGetInfo style calls
//   cl_build_status bOk;					// the return value from clGetProgramBuildInfo
//   size_t computeUnits;					// return from GetDeviceInfo
//   char strInfo[20];

   FILE * pFile;

   if(argc == 2)
	   clFile = argv[1];
   else
	   clFile = kernFile;

   pFile = fopen(clFile, "r");
   if (pFile == NULL)
   {
	   printf("Opening the Kernel file: %s produced an error(%d). Make sure that the source code variable kern has a valid path to the cl code and that the code is readable.\n", clFile, errno);
	   exit(0);
   }
   else
	   fclose(pFile);	// only open the file to check that it is there and readable


   debugReset(debugFile);
   debugdebug(debugFile, (char*)"How many devices do we have?\n");


   sprintf(msg, "About to malloc wrkArea1D: %d\n", workItems * bytesPerCore);
   debugdebug(debugFile, msg);
   wrkArea1D = (cl_uchar*) clmalloc(stdacc, workItems * bytesPerCore, 0);
   wrkArea2D = (cl_uchar*) clmalloc(stdacc, workItems * bytesPerCore, 0);

   for (i=0; i < workItems * bytesPerCore; i++)
	   wrkArea2D[i] = wrkArea1D[i] = 0;

   sprintf(msg, "Well malloc worked! Opening kernel file:%s\n", clFile);
   debugdebug(debugFile, msg);
   openHandle = clopen(stdacc, clFile, CLLD_NOW);
   // open the standard accellerator context (i.e. the Epiphany chip, reading in the .cl file and compiling it immediately

   clndrange_t ndr1D = clndrange_init1d(NULL,							// global offset (always zero)
		   	   	   	   	   	   	   ((size_t)workItems),					// total number of threads (get_global_id will return 0 to workItems
		   	   	   	   	   	   	   ((size_t)bytesPerCore));				// How many bytes do we tell the kernel to process via get_local_size(0)
   exKernel(openHandle, &ndr1D, (char*)"k_init1D", workItems, bytesPerCore, wrkArea1D, debugFile);

   clndrange_t ndr2D = clndrange_init2d(NULL,							// global offset (always zero)
		   	   	   	   	   	   	   ((size_t)workItems),					// total number of threads (get_global_id will return 0 to workItems
		   	   	   	   	   	   	   ((size_t)(bytesPerCore/4)),			// How many bytes do we tell the kernel to process via get_local_size(0)
		   	   	   	   	   	   	   NULL,								// another useless global offset
		   	   	   	   	   	   	   ((size_t)workItems),					// a value that does not seem to do anything useful
		   	   	   	   	   	   	   ((size_t)(bytesPerCore/4)));			// How many rows to process per call returned by get_local_size(1)

   exKernel(openHandle, &ndr2D, (char*)"k_init2D", workItems, bytesPerCore, wrkArea2D, debugFile);


   // ============================================================================================================
   // show the results
   // ============================================================================================================

   printf("The 1D data:\n");
   for(i=0; i < workItems * bytesPerCore; i++)
   {
	   printf("%u\t", wrkArea1D[i]);
	   if(((i+1) % bytesPerCore) == 0)
		   printf("\n");
   }

   printf("The 2D data:\n");
   for(i=0; i < workItems * bytesPerCore; i++)
   {
	   printf("%u\t", wrkArea2D[i]);
	   if(((i+1) % bytesPerCore) == 0)
		   printf("\n");
   }

   clfree(wrkArea1D);
   clfree(wrkArea2D);

   return 0;
}


