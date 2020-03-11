#include<sys/socket.h>
#include<sys/wait.h> 
#include <stdio.h>
#include <unistd.h>
#include<stdlib.h> 
#include<fcntl.h> 

#include <errno.h>

#include "lib/message.h"
#include "lib/logging.h"

extern int errno;


#define PIPE(fd) socketpair(AF_UNIX, SOCK_STREAM, PF_UNIX, fd)

//////////////////////////////////////////////////////////////////////////////
///////////////////////////////onursehitoglu//////////////////////////////////
void server(int p1[], int p2[], int pid1, int pid2)
{
	fd_set readset;
	char mess[40];

    int buf[8];
	
	

	ii* test_input_info = new ii{};
	ii* test_input_info2 = new ii{};

	int m, r;
	int open[2] = {1,1}; /* keep a flag for if pipe is still open */

	close(p1[1]); /* close unused ends */
	close(p2[1]);

	m = ((p1[0] > p2[0]) ? p1[0] : p2[0]) + 1; /* maximum of file descriptors */
	
	printf("CONTROL\n");



	while (open[0] || open[1]) {
		/* following initializes a blocking set for select() */
		FD_ZERO(&readset);
		if (open[0]) FD_SET(p1[0],&readset);
		if (open[1]) FD_SET(p2[0],&readset);


		/* the following code will block until any of them have data to read */
		select(m, &readset, NULL,NULL,NULL);  /* no wset, eset, timeout */

		/* now readset contains the fd's with available data */
		if (FD_ISSET(p1[0], &readset)) {  /* check if first pipe is in readset */
			
			//r = read(p1[0], buf, 8);
			cm* first_client_message;
			
			r = read(p1[0], first_client_message, sizeof(cm));
			
			if (r == 0)  	/* EOF */
				open[0] = 0;
			else{
				//printf("Buffer form child 1: %d %d \n", buf[0], buf[1] );



				printf("Struct form child 1: %d %d \n", first_client_message->message_id, first_client_message->params.delay);

				//pid_t* process_id_1;

				//read(pid1[0], process_id_1, sizeof(pid_t));


				test_input_info->type = first_client_message->message_id;
				test_input_info->pid = pid1;//*process_id_1;
				test_input_info->info = first_client_message->params;
				
				pid_t pid = getpid();

  				printf("PID 01: %d \n", pid);


				print_input(test_input_info, 0); // 0 will be replaced by the client id assigned to the bidder
				

			}

		}



		/* following if is not in else since both may have data to read */
		if (FD_ISSET(p2[0], &readset)) {  /* check if second pipe is in readset */

			cm* first_client_message;

			r = read(p2[0], first_client_message, sizeof(cm));
			//r = read(p2[0], buf, 8);


			int test;

		
			if ( r == 0 )  /* EOF */
				open[1] = 0;
			else{


				printf("Struct form child 2: %d %d \n", first_client_message->message_id, first_client_message->params.delay);


				
				//*test_input_info = {1, 9999, first_client_message->params};
				test_input_info2->type = first_client_message->message_id;
				test_input_info2->pid  = pid2;
				test_input_info2->info = first_client_message->params;
				
				pid_t pid = getpid();

  				printf("PID 02: %d \n", pid);


				print_input(test_input_info2, 1); // 0 will be replaced by the client id assigned to the bidder
			}
		}


	}

}
///////////////////////////////onursehitoglu//////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

int main()
{


	//////////////////////IO REDIRECTION///////////////////////
    //int file_desc = open("Test_Output.txt",O_WRONLY | O_APPEND); 
      

    //dup2(file_desc, STDOUT_FILENO) ; //  STDOUT_FILENO = 1


    int starting_bid, minimum_increment, number_of_bidders;
 	
  	int input_fds = open("./sample_to_work_on/inp1.txt", O_RDONLY);

    if(dup2(input_fds, STDIN_FILENO) < 0) {
    	printf("ERROR");
    	
  	}
  	
  	scanf("%d %d %d", &starting_bid, &minimum_increment, &number_of_bidders);
  	//scanf("%d %d", &first_bidder_executable, &minimum_increment);
 	
 
  	printf("%d , %d , %d\n", starting_bid, minimum_increment , number_of_bidders);
    
	//////////////////////IO REDIRECTION///////////////////////




	int fd1[2];
	int fd2[2];
	
	int pipe1 = PIPE(fd1); //socket creation 1
	int pipe2 = PIPE(fd2); //socket creation 2

	int pid1[2];
	int pid2[2];
	
	int pipe3 = PIPE(pid1); //socket creation for pid1 (process id for child process 1)
	int pipe4 = PIPE(pid2); //socket creation for pid2 (process id for child process 2)

	int file_desc_1 = open("child_1_output.txt",O_WRONLY | O_APPEND); 

	int file_desc_2 = open("child_2_output.txt",O_WRONLY | O_APPEND); 

	if (pipe1 == 0 && pipe2 == 0 && pipe3 == 0 && pipe4 == 0)
	{
		printf("Socket Pair is succesfully created.\n");

		int w; // parameter for wait()

		int fork_return1, fork_return2;

		if (fork_return2 = fork()) {
			if (fork_return1 = fork())
			{
				printf("PARENT\n");  /* Parent process */
				printf(" FORK RETURNS: %d %d \n", fork_return1, fork_return2 );

				server(fd1,fd2,fork_return1,fork_return2);
				
				wait(&w);
				wait(&w);
			}else{												/* Child process 1*/
				printf("Child process: Bidder 1\n");

				char *args[3];
				args[0] = "Bidder";  /* Convention! Not required!! */
				args[1] = "1111";
				args[2] = NULL;

				pid_t pid = getpid();

  				printf("PID 1: %d \n", pid);
  

				//dup2(file_desc, STDOUT_FILENO) ; //  STDOUT_FILENO = 1

  				//write(pid1[1],&pid,2);

  				//close(pid1[1]);
				
				dup2(fd1[1], STDOUT_FILENO) ; //  STDOUT_FILENO = 1


				execv("/home/furkan/Desktop/ceng334/HW1/bin/Bidder", args);
				


			}
		} else {       /* Child process 2*/

			printf("Child process: Bidder 2\n");
			char *args[3];
			args[0] = "Bidder";  /* Convention! Not required!! */
			args[1] = "2222";
			args[2] = NULL; 

			//dup2(file_desc_2, STDOUT_FILENO) ; //  STDOUT_FILENO = 1

			pid_t pid = getpid();

			printf("PID 2: %d \n", pid);

			//write(pid2[1],&pid,2);

			//close(pid2[1]);


			dup2(fd2[1], STDOUT_FILENO) ; //  STDOUT_FILENO = 1


			execv("/home/furkan/Desktop/ceng334/HW1/bin/Bidder", args);

		}



	}else{
		printf("Socket Pair error!.\n");

	}

	


	return 0;
}