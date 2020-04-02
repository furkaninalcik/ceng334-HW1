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

bool isAllCompleted(bool completionRecord[], int number_of_bidders){
	
	bool result = true;

	for (int i = 0; i < number_of_bidders; ++i)
	{
		result = result && completionRecord[i];			
	}

	return result;

}

void server(int fd_list[][2], int pid_list[2], int status_list[2])
{
	fd_set readset;
	char mess[40];

    int buf[8];

    int number_of_bidders = 2;


    int *p1 = fd_list[0];
    int *p2 = fd_list[1];

    int pid1 = pid_list[0];
    int pid2 = pid_list[1];
	
	
    int current_highest_bid = 0;
    int current_highest_bidder_id ;
    int min_inc = 5 ;
    int starting_bid = 0 ;


    bool bidderCompletionRecord[2]; //CHANGE IT! -> THE LENGTH OF THE ARRAY SHOULD BE EQUAL TO THE NUMBER OF BIDDERS 
    bidderCompletionRecord[0] = false; //THIS WIL BE SET TO true WHEN THE BIDDER SENDS THE "CLIENT_FINISHED" MESSAGE
    bidderCompletionRecord[1] = false; //THIS WIL BE SET TO true WHEN THE BIDDER SENDS THE "CLIENT_FINISHED" MESSAGE


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
	wi* the_winner_info = new wi{};

	sm* server_message_start  = new sm{};  //TRY NOT TO USE IT
	sm* server_message_result = new sm{};  // CHANGE ITS NAME
	sm* server_message_winner = new sm{};  //TRY NOT TO USE IT



	while (open[0] || open[1]) {
		/* following initializes a blocking set for select() */
		FD_ZERO(&readset);
		if (open[0]) FD_SET(fd_list[0][0],&readset);
		if (open[1]) FD_SET(fd_list[1][0],&readset);


		/* the following code will block until any of them have data to read */
		select(m, &readset, NULL,NULL,NULL);  /* no wset, eset, timeout */

		/* now readset contains the fd's with available data */

		for (int i = 0; i < 2; ++i)
		{
			if (FD_ISSET(fd_list[i][0], &readset)) {  /* check if pipe with index "i" is in readset */
				
				//r = read(fd_list[i][0], buf, 8);
				cm* client_message;


				r = read(fd_list[i][0], client_message, sizeof(cm));
				
				if (r == 0)  	/* EOF */
					open[i] = 0;
				else{

					if (client_message->message_id == 1)
					{
						
						input_info->type = CLIENT_CONNECT;
						input_info->pid = pid_list[i];//*process_id_1;
						input_info->info = client_message->params;
						
						//pid_t pid = getpid(); THIS DOES NOT WORK SINCE 
											  //THE RETURN VALUE IS THE PROCESS ID OF THE PARENT, NOT THE CHILD


						print_input(input_info, i); // i is the client id assigned to the bidder
						

						connection_established_info->client_id = i; 
						connection_established_info->starting_bid = starting_bid;
						connection_established_info->current_bid = current_highest_bid;
						connection_established_info->minimum_increment = min_inc;
		

						output_info->type = SERVER_CONNECTION_ESTABLISHED;
						output_info->pid = pid_list[i];//*process_id_1;
						output_info->info.start_info = *connection_established_info;
						//output_info->info.result_info = NULL;
						//output_info->info.winner_info = NULL;

						


						server_message_start->message_id = SERVER_CONNECTION_ESTABLISHED; // change this to server_message_result  
						server_message_start->params.start_info = *connection_established_info;

						write(fd_list[i][0],server_message_start,sizeof(sm));
						print_output(output_info, i); // i is the client id assigned to the bidder
						
						printf("server_message_start ID %d\n", server_message_start->message_id );
						//write(out_fd1[0],"1111",4);
						//close(fd_list[i][0]);



					}else if (client_message->message_id == 2)
					{
						//printf("Message received!\n");
						input_info->type = CLIENT_BID;
						input_info->pid = pid_list[i];//*process_id_1;
						input_info->info = client_message->params;
						
						
						print_input(input_info, i); // i is the client id assigned to the bidder


						if (client_message->params.bid >= current_highest_bid + min_inc ) //BID_ACCEPTED -> 0
						{

							current_highest_bid = client_message->params.bid;
							current_highest_bidder_id = i; 					// BIDDER ID -> "i"

							bid_info->result      = BID_ACCEPTED; //BID_ACCEPTED -> 0
							bid_info->current_bid = current_highest_bid;

							


						} else if (client_message->params.bid < starting_bid) //BID_LOWER_THAN_STARTING_BID -> 1
						{
							//printf("11111111111111111111111111111111\n");

							bid_info->result      = BID_LOWER_THAN_STARTING_BID; //BID_LOWER_THAN_STARTING_BID -> 1
							bid_info->current_bid = current_highest_bid;

							

						} else if (client_message->params.bid < current_highest_bid) //BID_LOWER_THAN_CURRENT -> 2
						{
							//printf("22222222222222222222222222222222\n");

							bid_info->result      = BID_LOWER_THAN_CURRENT; //BID_LOWER_THAN_CURRENT -> 2
							bid_info->current_bid = current_highest_bid;

							

						} else if (client_message->params.bid - current_highest_bid < min_inc) //BID_INCREMENT_LOWER_THAN_MINIMUM ->3
						{
							//printf("33333333333333333333333333333333\n");
							bid_info->result      = BID_INCREMENT_LOWER_THAN_MINIMUM; //BID_INCREMENT_LOWER_THAN_MINIMUM -> 3
							bid_info->current_bid = current_highest_bid;

							
						} else{
							printf("ERROR !!!!!!!!!!!!!!!!!!!!!!\n");
						}

						server_message_result->message_id = SERVER_BID_RESULT;
						server_message_result->params.result_info = *bid_info;
						

						output_info->type = SERVER_BID_RESULT;
						output_info->pid = pid_list[i];//*process_id_1;
						//output_info->info.start_info = NULL;
						output_info->info.result_info = *bid_info;
						//output_info->info.winner_info = NULL;

						

						write(fd_list[i][0],server_message_result,sizeof(sm));
						print_output(output_info, i); // i is the client id assigned to the bidder



					} else if (client_message->message_id == 3) // replace with CLIENT_FINISHED!
					{
						input_info->type = CLIENT_FINISHED;
						input_info->pid = pid_list[i];//*process_id_1;
						input_info->info = client_message->params;

						print_input(input_info, i); // i is the client id assigned to the bidder

						printf("ID::: %d \n" , i);
						
						status_list[i] = client_message->params.status; // STATUS recieved from the client with id -> i
						//status_list[i] = i+9; // Check if the array is passed by reference
						
						bidderCompletionRecord[i] = 1;

						if(isAllCompleted(bidderCompletionRecord, number_of_bidders)){
							
							the_winner_info->winner_id   = current_highest_bidder_id;
							the_winner_info->winning_bid = current_highest_bid;


							server_message_winner->message_id = SERVER_AUCTION_FINISHED;
							server_message_winner->params.winner_info = *the_winner_info;

							print_server_finished(current_highest_bidder_id,current_highest_bid);
							
							for (int i = 0; i < number_of_bidders; ++i)
							{
								output_info->type = SERVER_AUCTION_FINISHED;
								output_info->pid = pid_list[i];//*process_id_1;
								//output_info->info.start_info = NULL;
								//output_info->info.result_info = *bid_info;
								output_info->info.winner_info = *the_winner_info;

								write(fd_list[i][0],server_message_winner,sizeof(sm));
								print_output(output_info, i); // i is the client id assigned to the bidder

							}

						}

							

					}




						


				}

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
	
	//int pipe1 = PIPE(fd1); //socket creation 1
	//int pipe2 = PIPE(fd2); //socket creation 2
	




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
	


	int fd_list[2][2]; 	//list of file desctiptors
	int pipe1 = PIPE(fd_list[0]); //socket creation with list
	int pipe2 = PIPE(fd_list[1]); //socket creation with list

	


   	int pid_list[2];     //lists of child pids 

	if (pipe1 == 0 && pipe2 == 0 )
	{
		printf("Socket Pair is succesfully created.\n");

		int w; // parameter for wait()

		int fork_return1, fork_return2;
		int child_pid;


		for (int i = 0; i < 2; ++i)
		{
			child_pid = fork(); //for parent process to save the child process id in the array pid_list

			if(child_pid == 0) 
        	{ 

            	printf("Child Bidder ID: %d \n",i); 
            	printf("Child pid: %d from Parent pid: %d\n",getpid(),getppid()); 


				char *args[3];
				args[0] = "Bidder";  /* Convention! Not required!! */
				args[1] = "111";
				args[2] = NULL;

				if (i == 1)
				{
					args[1] = "222";
				}

				close(fd_list[i][0]);
				
		

				if (fd_list[i][1] != STDIN_FILENO) { /*Redirect standard input to socketpair*/
					if (dup2(fd_list[i][1], STDIN_FILENO) != STDIN_FILENO) {
						perror("Cannot dup2 stdin");
						exit(0);
					}
				}
				printf("FD1 1: %d\n", fd_list[i][1] );
				printf("STDIN: %d\n", STDIN_FILENO );

				if (fd_list[i][1] != STDOUT_FILENO) { //Redirect standard output to socketpair
					if (dup2(fd_list[i][1], STDOUT_FILENO) != STDOUT_FILENO) {
						perror("Cannot dup2 stdout");
						exit(0);
					}
				}

				execv("/home/furkan/Desktop/ceng334/HW1/bin/Bidder", args);


            	//exit(0); Should I exit or the bidder program has already exited   
        	
        	}
        	pid_list[i] = child_pid;
		}

		int status_list[2];

		server(fd_list, pid_list, status_list);

		printf("status_list[0]: %d \n", status_list[0] );
		printf("status_list[1]: %d \n", status_list[1] );


		for (int i = 0; i < 2; i++) {
    		pid_t childPid = wait(&w); 
    		print_client_finished(i,w,1); // status match ???
			printf("Child PID: %d\n",childPid );			
			printf("Exit Status: %d\n",w );			
		} 




	}else{
		printf("Socket Pair error!.\n");

	}

	


	return 0;
}