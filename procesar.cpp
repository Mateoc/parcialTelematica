#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>
#include <vector>
 int P1;
 int P2;
 char OP;

using namespace std;
void procesarText(char *text) {

	char sp1[100];
	char sp2[100];

	int i=1;

	while (text[i]!='=') {
	 if (text[i]=='+' || text[i]=='-' || text[i]=='*' || text[i]=='/') {
	    OP = text[i];
	    strncpy(sp1, text, i);
	    strncpy(sp2, text+i+1, strlen(text)-1);
	    break;
	 } else
	    i++;
	}

	P1 = atoi(sp1);
	P2 = atoi(sp2);
}

int operacion(int p1, char op, int p2) {
	int result=-1;
	if (op=='+')
		result = p1+p2;
	else if (op=='-')
		result = p1-p2;
	else if (op=='*')
		result = p1*p2;
	else if (op=='/')
		result = p1+p2;
	return result;
}
 
 struct S {
  char *a;
};
 
int main(int argc, char *argv[])
{
    double t1 = MPI_Wtime();
    MPI_Status status;
    int RC, NUM_PROCS, TASK_ID, NUM_WORKERS;
    // RC: The resulting message from several MPI commands to check for errors.
    // NUM_PROCS: The total number of processes counting Workers and Master.
    // TASK_ID: The ID of this particular process. Master is always 0.
    // NUM_WORKERS: The amount of Workers not counting the Master.
    if((RC = MPI_Init(&argc, &argv)) != MPI_SUCCESS){
        fprintf(stderr, "Cannot initialize MPI_Init. Exiting.\n");
        MPI_Abort(MPI_COMM_WORLD, RC);
        exit(1);
    }
    // Init MPI, pass the argc and argv of the original program to all Workers
    // so that they will be initialized the same way as the Master Node.
    RC = MPI_Comm_rank(MPI_COMM_WORLD, &TASK_ID);
    // Get TASK_ID of this process, each process will have a different ID.
    RC = MPI_Comm_size(MPI_COMM_WORLD, &NUM_PROCS);
    // Get the total number of processes, given via shell with -np flag.
    int LEN;
    char NAME[MPI_MAX_PROCESSOR_NAME];
    // To store the current host name.
    MPI_Get_processor_name(NAME, &LEN);
    NUM_WORKERS = NUM_PROCS - 1;
    // The number of Workers is the amount of total processes, minus 1.
    printf("Hello! I'm process %i out of %i processes, on host %s.\n",
           TASK_ID, NUM_PROCS, NAME);
    int res;
    if(TASK_ID==0){
    FILE *archivo_in;
    FILE *archivo_out;
    archivo_in = fopen("datos_in.txt","r");
    archivo_out = fopen("datos_out.txt","w");
    if (archivo_in == NULL || archivo_out == NULL)
 	exit(1);
	int cont = 0;
	vector <S> v;
	while (feof(archivo_in) == 0)
 	{
		char *linea_in= new char[100];
		cont++;
 		fgets(linea_in,100,archivo_in);
		S s;
		s.a = linea_in;
		v.push_back( s );
 	}
		cont--;
	for(int i = 1; i<=NUM_WORKERS;i++){
		MPI_Send(&cont, 1, MPI_INT, i,
                    1, MPI_COMM_WORLD);
	}
	int rec = 1;
	for(int i = 1; i<cont;i++){
		MPI_Send(v[i].a, 100, MPI_CHAR, rec,
                    2, MPI_COMM_WORLD);
		    
		if(rec<NUM_WORKERS){
		    rec++;	
		}else{
	            rec=1;
		}
	}
	for(int i = 1; i<cont;i++){
		char *linea_in= new char[100];
		MPI_Recv(linea_in, 100, MPI_CHAR, MPI_ANY_SOURCE, 3,
			MPI_COMM_WORLD, &status);
		fputs(linea_in, archivo_out);
	}
	
        fclose(archivo_in);
	fclose(archivo_out);
    }else{
        int cont;
	MPI_Recv(&cont, 1, MPI_INT, 0, 1,
                 MPI_COMM_WORLD, &status);
	vector<S> v;
	for(int i = TASK_ID; i < cont; i+=NUM_WORKERS){
		char *linea_in= new char[100];
		MPI_Recv(linea_in, 100, MPI_CHAR, 0, 2,
			MPI_COMM_WORLD, &status);
		S s;
		s.a = linea_in;
		v.push_back(s);	
		//printf("TASK_ID: %d, \n", TASK_ID);
	}
	for (int i = 0; i<v.size();i++){
		procesarText(v[i].a);
                res = operacion(P1,OP,P2);
		sprintf(v[i].a,"%d%c%d=%d\n",P1, OP, P2,res);
	}
	for (int i = 0; i<v.size();i++){
		MPI_Send(v[i].a, 100, MPI_CHAR, 0,
                    3, MPI_COMM_WORLD);
	}
    }
	 printf("TASK_ID: %d, Total Time: %lf\n", TASK_ID, MPI_Wtime() - t1);
    MPI_Finalize();
	return 0;
}