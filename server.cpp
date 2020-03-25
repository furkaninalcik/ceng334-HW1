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
	
	
    int current_highest_bid = 0;
    int current_highest_bidder_id ;
    int min_inc = 5 ;
    int starting_bid = 0 ;


	int m, r;
	int open[2] = {1,1}; /* keep a flag for if pipe is still open */

	close(p1[1]); /* close unused ends */
	close(p2[1]);

	m = ((p1[0] > p2[0]) ? p1[0] : p2[0]) + 1; /* maximum of file descriptors */
	
	printf("CONTROL\n");

	ii* input_info = new ii{};
	ii* input_info2 = new ii{};

	oi* output_info = new oi{};


	cei* connection_established_info = new cei{};
	bi* bid_info = new bi{};

	sm* server_message_start  = new sm{};
	sm* server_message_result = new sm{};
	sm* server_message_winner = new sm{};



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
			cm* client_message;


			r = read(p1[0], client_message, sizeof(cm));
			
			if (r == 0)  	/* EOF */
				open[0] = 0;
			else{

				if (client_message->message_id == 1)
				{
					
					input_info->type = CLIENT_CONNECT;
					input_info->pid = pid1;//*process_id_1;
					input_info->info = client_message->params;
					
					//pid_t pid = getpid(); THIS DOES NOT WORK SINCE 
										  //THE RETURN VALUE IS THE PROCESS ID OF THE PARENT, NOT THE CHILD


						

					connection_established_info->client_id = 0; 
					connection_established_info->starting_bid = starting_bid;
					connection_established_info->current_bid = current_highest_bid;
					connection_established_info->minimum_increment = min_inc;
	

					output_info->type = SERVER_CONNECTION_ESTABLISHED;
					output_info->pid = pid1;//*process_id_1;
					output_info->info.start_info = *connection_established_info;
					//output_info->info.result_info = NULL;
					//output_info->info.winner_info = NULL;

					


					server_message_start->message_id = SERVER_CONNECTION_ESTABLISHED;
					server_message_start->params.start_info = *connection_established_info;

					write(p1[0],server_message_start,sizeof(sm));
					printf("server_message_start ID %d\n", server_message_start->message_id );
					//write(out_fd1[0],"1111",4);
					//close(p1[0]);



				}else if (client_message->message_id == 2)
				{
					printf("Message received!\n");
					input_info->type = CLIENT_BID;
					input_info->pid = pid1;//*process_id_1;
					input_info->info = client_message->params;
					
					
					if (client_message->params.bid >= current_highest_bid + min_inc ) //BID_ACCEPTED -> 0
					{

						current_highest_bid = client_message->params.bid;
						current_highest_bidder_id = 0; 					// BIDDER ID -> 0 STATIC JUST FOR NOW

						bid_info->result      = BID_ACCEPTED; //BID_ACCEPTED -> 0
						bid_info->current_bid = current_highest_bid;

						


					} else if (client_message->params.bid < starting_bid) //BID_LOWER_THAN_STARTING_BID -> 1
					{
						printf("11111111111111111111111111111111\n");

						bid_info->result      = BID_LOWER_THAN_STARTING_BID; //BID_LOWER_THAN_STARTING_BID -> 1
						bid_info->current_bid = current_highest_bid;

						

					} else if (client_message->params.bid < current_highest_bid) //BID_LOWER_THAN_CURRENT -> 2
					{
						printf("22222222222222222222222222222222\n");

						bid_info->result      = BID_LOWER_THAN_CURRENT; //BID_LOWER_THAN_CURRENT -> 2
						bid_info->current_bid = current_highest_bid;

						

					} else if (client_message->params.bid - current_highest_bid < min_inc) //BID_INCREMENT_LOWER_THAN_MINIMUM ->3
					{
						printf("33333333333333333333333333333333\n");
						bid_info->result      = BID_INCREMENT_LOWER_THAN_MINIMUM; //BID_INCREMENT_LOWER_THAN_MINIMUM -> 3
						bid_info->current_bid = current_highest_bid;

						
					} else{
						printf("ERROR !!!!!!!!!!!!!!!!!!!!!!\n");
					}

					server_message_result->message_id = SERVER_BID_RESULT;
					server_message_result->params.result_info = *bid_info;
					

					output_info->type = SERVER_BID_RESULT;
					output_info->pid = pid1;//*process_id_1;
					//output_info->info.start_info = NULL;
					output_info->info.result_info = *bid_info;
					//output_info->info.winner_info = NULL;

					

					write(p1[0],server_message_result,sizeof(sm));



				} else if (client_message->message_id == 3) // replace with CLIENT_FINISHED!
				{
					input_info->type = CLIENT_FINISHED;
					input_info->pid = pid1;//*process_id_1;
					input_info->info = client_message->params;
					


						

				}

				print_input(input_info, 0); // 0 will be replaced by the client id assigned to the bidder

				print_output(output_info, 0); // 0 will be replaced by the client id assigned to the bidder


					


			}

		}



		/* following if is not in else since both may have data to read */
		if (FD_ISSET(p2[0], &readset)) {  /* check if second pipe is in readset */

			cm* client_message;

			r = read(p2[0], client_message, sizeof(cm));
			//r = read(p2[0], buf, 8);


			int test;

		
			if ( r == 0 )  /* EOF */
				open[1] = 0;
			else{


				printf("Struct form child 2: %d %d \n", client_message->message_id, client_message->params.delay);


				
				//*input_info = {1, 9999, client_message->params};
				input_info2->type = client_message->message_id;
				input_info2->pid  = pid2;
				input_info2->info = client_message->params;
				
				pid_t pid = getpid();

  				//printf("PID 02: %d \n", pid);


				print_input(input_info2, 1); // 0 will be replaced by the client id assigned to the bidder

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
  	//scanf("%d %d", &bidder_executable, &minimum_increment);
 	
 
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
//	oi* test_output_info = new oi{};
//	cei* test_connection_established_info = new cei{};
//	sm* test_server_message = new sm{};


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
				
				//dup2(fd1[1], STDOUT_FILENO) ; //  STDOUT_FILENO = 1

				//dup2(fd1[1], STDIN_FILENO ) ; //
				

				if (fd1[1] != STDIN_FILENO) { /*Redirect standard input to socketpair*/
					if (dup2(fd1[1], STDIN_FILENO) != STDIN_FILENO) {
						perror("Cannot dup2 stdin");
						exit(0);
					}
				}
				printf("FD1 1: %d\n", fd1[1] );
				printf("STDIN: %d\n", STDIN_FILENO );

				if (fd1[1] != STDOUT_FILENO) { //Redirect standard output to socketpair
					if (dup2(fd1[1], STDOUT_FILENO) != STDOUT_FILENO) {
						perror("Cannot dup2 stdout");
						exit(0);
					}
				}

				/*
				test_connection_established_info->client_id = 0; 
				test_connection_established_info->starting_bid = 0;
				test_connection_established_info->current_bid = 0;
				test_connection_established_info->minimum_increment = 5;


				test_output_info->type = 0;
				test_output_info->pid = 8888;//*process_id_1;
				test_output_info->info.start_info = *test_connection_established_info;


				test_server_message->message_id = 0;
				test_server_message->params = test_output_info->info;
				*/
				
				//write(file_desc_test,test_server_message,sizeof(sm));


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
			dup2(fd2[1], STDIN_FILENO) ; //  STDIN_FILENO = 1

			//dup2(out_fd1[1], STDIN_FILENO) ; //


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
				write(file_desc_test, line2,n);break;
	  		}*/
			/////////////////WRITING TO A FILE////////////


			execv("/home/furkan/Desktop/ceng334/HW1/bin/Bidder", args);

		}



	}else{
		printf("Socket Pair error!.\n");

	}

	


	return 0;
}