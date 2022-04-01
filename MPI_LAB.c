// must compile with: mpicc  -std=c99 -Wall -o checkdiv 
#include <mpi.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>


int main(int argc, char *argv[]){
  
unsigned int x, A, B;
//unsigned int i; //loop index
FILE * fp; //for creating the output file
char filename[100]=""; // the file name


double start_p1, end_p1, start_p2, end_p2;
double time_for_p1; 

//Needed for MPI
int comm_sz;
int my_rank;


/////////////////////////////////////////

start_p1 = clock();
// Check that the input from the user is correct.
if(argc != 4){

  printf("usage:  ./checkdiv A B x\n");
  printf("A: the lower bound of the range [A,B]\n");
  printf("B: the upper bound of the range [A,B]\n");
  printf("x: divisor\n");
  exit(1);
}  

A = (unsigned int)atoi(argv[1]); 
B = (unsigned int)atoi(argv[2]); 
x = (unsigned int)atoi(argv[3]);
 

// The arguments to the main() function are available to all processes and no need to send them from process 0.
// Other processes must, after receiving the variables, calculate their own range.


/////////////////////////////////////////

//initalizing MPI Coms
MPI_Init(NULL, NULL);
MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);
MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);



/////////////////////////////////////////
//start of part 1
start_p1 = MPI_Wtime();
// The main computation part starts here


//VERY IMPORTANT TO READ THIS
//determining size the total number of elements
//note the following explinations of lines 70-71 below
//1. to ensure that size is consistent accross processes 
//        I add extra values to be check to the total count
//        as to enable MPI_Gather to work properly 
//        and for each value to be checked
//2. I ensure that the extra values beyond the range of B 
//   are not added to the file by insering an if statment 
//   in part2.
int total = (B - A) + (comm_sz - (B-A)%comm_sz);
int size = total/comm_sz;


//array is the local array of values on each process
//in addition each process will calculate its own starting and ending index
int *array; 
int startIndex = (my_rank * size) + A;
int endIndex = (startIndex + size);
int aryIndex = 0;

//allocating space for the local array of values
array = malloc(sizeof(int) * (size));

for(int k = (startIndex); k < endIndex; k++){
  if(k%x == 0){
    array[aryIndex] = k;
    aryIndex++;
  }else{
    array[aryIndex] = -1000;
    aryIndex++;
  }
}

int *b = NULL;
if(my_rank == 0){
  b = malloc((total) * sizeof(int));
}

//sending local array to process 0
//very important
MPI_Gather(array, size, MPI_INT, b, size, MPI_INT, 0, MPI_COMM_WORLD);
  
// end of the main compuation part
end_p1 = MPI_Wtime();


/////////////////////////////////////
//sending the max


// Use reduction operation to get MAX of (end_p1 - start_p1) among processes 
// and send it to process 0 as time_for_p1
double max = (end_p1 - start_p1);

MPI_Reduce(&max, &time_for_p1, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
/////////////////////////////////////



//end of part 1
/////////////////////////////////////////


/////////////////////////////////////////
//start of part 2

start_p2 = MPI_Wtime();


//forming the filename
//only process 0 should be handling this as specified in the lab documents
if(my_rank == 0){
  strcpy(filename, argv[2]);
strcat(filename, ".txt");

if( !(fp = fopen(filename,"w+t")))
{
  printf("Cannot create file %s\n", filename);
  exit(1);
}
}


//Write the numbers divisible by x in the file as indicated in the lab description.


//writing to the text file.
size++;
if(my_rank == 0){
  for(int j = 0; j < (total); j++){

    if(b[j] <= B && b[j] >= A){
      fprintf(fp, "%d \n", b[j]);
    }
  }

  //only process 0 should be closing to avoid segmentation fault
  //a common error I was facing
  fclose(fp);
}



end_p2 = MPI_Wtime();


//end of part 2
/////////////////////////////////////////
/* Print  the times of the two parts */



//process 0 stores the max time for part 1
//hence it makes sense to only include this
if(my_rank == 0){
  printf("time of part1 = %lf s    part2 = %lf s\n", 
       (double)(time_for_p1),
       (double)(end_p2-start_p2) );
}

//MPI Finalize has documentation that states that only a return statement 
//should follow MPI_Finalize
MPI_Finalize();
return 0;
}

