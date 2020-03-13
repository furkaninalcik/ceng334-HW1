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

#define DATA1 "In Xanadu, did Kublai Khan..."
#define DATA2 "A stately pleasure dome decree..."

#define PIPE(fd) socketpair(AF_UNIX, SOCK_STREAM, PF_UNIX, fd)

//////////////////////////////////////////////////////////////////////////////
///////////////////////////////onursehitoglu//////////////////////////////////
void server(int p1[], int p2[], int pid1, int pid2, int out_fd1[])
{
	fd_set readset;
	char mess[40];

    int buf[8];
	
	

	ii* test_input_info = new ii{};
	ii* test_input_info2 = new ii{};

	oi* test_output_info = new oi{};

	int m, r;
	int open[2] = {1,1}; /* keep a flag for if pipe is still open */

	close(p1[1]); /* close unused ends */
	close(p2[1]);

	m = ((p1[0] > p2[0]) ? p1[0] : p2[0]) + 1; /* maximum of file descriptors */
	
	printf("CONTROL\n");

	cei* test_connection_established_info = new cei{};

	sm* test_server_message = new sm{};



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



				printf("Struct form child 1: %d %d \n", first_client_message->message_id, first_client_message->params.status);
				printf("Struct form child 1: %d %d \n", first_client_message->message_id, first_client_message->params.bid);
				printf("Struct form child 1: %d %d \n", first_client_message->message_id, first_client_message->params.delay);

				//printf("FIRST CLIENT MESSAGE: %d\n", first_client_message->params.delay);
				//printf("FIRST CLIENT MESSAGE: %d\n", first_client_message->params.status);
				//printf("FIRST CLIENT MESSAGE: %d\n", first_client_message->params.bid);

				//pid_t* process_id_1;

				//read(pid1[0], process_id_1, sizeof(pid_t));


				test_input_info->type = first_client_message->message_id;
				test_input_info->pid = pid1;//*process_id_1;
				test_input_info->info = first_client_message->params;
				
				pid_t pid = getpid();

  				//printf("PID 01: %d \n", pid);


				print_input(test_input_info, 0); // 0 will be replaced by the client id assigned to the bidder
					

				//cei test_connection_established_info ;//= {0,0,0,5};

				
				test_connection_established_info->client_id = 0; 
				test_connection_established_info->starting_bid = 0;
				test_connection_established_info->current_bid = 0;
				test_connection_established_info->minimum_increment = 5;
				/*
				*/

				test_output_info->type = first_client_message->message_id;
				test_output_info->pid = pid1;//*process_id_1;
				test_output_info->info.start_info = *test_connection_established_info;

				print_output(test_output_info, 0); // 0 will be replaced by the client id assigned to the bidder


				test_server_message->message_id = 0;
				test_server_message->params = test_output_info->info;

				write(p1[0],test_server_message,sizeof(sm));
				//write(out_fd1[0],"1111",4);
				//close(p1[0]);


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

  				//printf("PID 02: %d \n", pid);


				print_input(test_input_info2, 1); // 0 will be replaced by the client id assigned to the bidder

				//cei test;

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


	char line2[256]; //test


	int fd1[2];
	int fd2[2];
	int out_fd1[2];
	int out_fd2[2];
	
	int pipe1 = PIPE(fd1); //socket creation 1
	int pipe2 = PIPE(fd2); //socket creation 2
	int pipe3 = PIPE(out_fd1); //socket creation 3
	int pipe4 = PIPE(out_fd2); //socket creation 4


	int file_desc_1 = open("child_1_output.txt",O_WRONLY | O_APPEND); 

	int file_desc_2 = open("child_2_output.txt",O_WRONLY | O_APPEND); 

	int file_desc_test = open("tricky.txt",O_WRONLY | O_APPEND); 

	////////////////////////////////////////
	///////////////////////////////////////
	
	int sockets[2], child;
   	char buf[1024];
   	if (socketpair(AF_UNIX, SOCK_STREAM, PF_UNIX, sockets) < 0) {
    	perror("opening stream socket pair");
    	exit(1);
   	}
	////////////////////////////////////////
	////////////////////////////////////////
	


	if (pipe1 == 0 && pipe2 == 0 && pipe3 == 0 && pipe4 == 0)
	{
		printf("Socket Pair is succesfully created.\n");

		int w; // parameter for wait()

		int fork_return1, fork_return2;

		if (fork_return2 = fork()) {
			if (fork_return1 = fork())
			{
				//printf("PARENT\n");  /* Parent process */
				//printf(" FORK RETURNS: %d %d \n", fork_return1, fork_return2 );

				/*
				int file_desc_test = open("tricky.txt",O_WRONLY | O_APPEND); 
				
				dup2(STDOUT_FILENO ,fd1[1]  ) ; //  STDOUT_FILENO = 1

				write(fd1[1],"\ntest1\n\n",8);
				*/

				//close(out_fd1[1]);

				//dup2(file_desc_test, out_fd1[0]) ; //
				
				//write(out_fd1[0],"2222",4);
				//close(out_fd1[0]);

				////////////////////////////////////////
				////////////////////////////////////////
				
				//close(sockets[0]);
				//if (read(sockets[1], buf, 1024) < 0)
				//	perror("reading stream message");
				//printf("-->%s\n", buf);
				//if (write(sockets[1], DATA2, sizeof(DATA2)) < 0)
				//	perror("writing stream message");
				//close(sockets[1]);
				///////////////////////////////////////
				////////////////////////////////////////


				//dup2(file_desc_test  , out_fd1[0]) ; //



				server(fd1,fd2,fork_return1,fork_return2, out_fd1);



				wait(&w);
				wait(&w);
			}else{												/* Child process 1*/
				printf("Child process: Bidder 1\n");

				char *args[3];
				args[0] = "Bidder";  /* Convention! Not required!! */
				args[1] = "1123";
				args[2] = NULL;


  				//close(out_fd1[0]);

				printf("STDOUT_FILENO: %d\n\n", STDOUT_FILENO);

				close(fd1[0]);
				
				dup2(fd1[1], STDOUT_FILENO) ; //  STDOUT_FILENO = 1
				dup2(fd1[1], STDIN_FILENO ) ; //
				
				//close(fd1[1]);

				


				//if (read(out_fd1[1], buf, 1024) < 0)
				//	perror("reading stream message");
				//printf("-->%s\n", buf);



				
				//close(out_fd1[0]);

				//dup2(file_desc_test  , out_fd1[1]) ; //
				//close(out_fd1[1]);
				
				//write(out_fd1[1],"9",1);


				execv("/home/furkan/Desktop/ceng334/HW1/bin/Bidder", args);


				//write(fd1[1],"\ntest1\n\n",8);

				//const char* input_to_child = "0000 0000 0000 0000 0000 0000 0000 0000 0500 0000";
				//char* input_to_child;
				//scanf("%s", &input_to_child);


				


			}
		} else {       /* Child process 2*/

			printf("Child process: Bidder 2\n");
			char *args[3];
			args[0] = "Bidder";  /* Convention! Not required!! */
			args[1] = "2222";
			args[2] = NULL; 

			//dup2(file_desc_2, STDOUT_FILENO) ; //  STDOUT_FILENO = 1


			printf("STDOUT_FILENO: %d\n", STDOUT_FILENO);

			////////////////////////////////////////
			////////////////////////////////////////
			
			//close(sockets[1]);
			//if (write(sockets[0], DATA1, sizeof(DATA1)) < 0)
			//	perror("writing stream message");
			//if (read(sockets[0], buf, 1024) < 0)
			//	perror("reading stream message");
			//printf("-->%s\n", buf);
			//close(sockets[0]);
			////////////////////////////////////////
			////////////////////////////////////////


			dup2(fd2[1], STDOUT_FILENO) ; //  STDOUT_FILENO = 1

			//dup2(file_desc_test, out_fd1[1]) ; //


			/////////////////WRITING TO A FILE////////////
			
			//out_fd1 is written in child_1 and 
			//the value is being written to the file for debugging here in child_2
			/*
			int n;
			for ( ; ; ) {
				// on the socket pair we cannot use printf, scanf, etc. 
				// For printf, scanf, etc. we need to have a pseudoterminal.  
				n = read(out_fd1[1], line2, 256-1);
				if (n < 0) break;
				line2[n] = '\0';
				write(file_desc_test, line2,n);
	  		}*/
			/////////////////WRITING TO A FILE////////////


			execv("/home/furkan/Desktop/ceng334/HW1/bin/Bidder", args);

		}



	}else{
		printf("Socket Pair error!.\n");

	}

	


	return 0;
}