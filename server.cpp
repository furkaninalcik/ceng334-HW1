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


bool activeBidders(int* bidderActivityList, int number_of_bidders){

	bool result = false;

	for (int i = 0; i < number_of_bidders; ++i)
	{
		result = result || bidderActivityList[i];	// This will be true if anyone of the bidders is active
	}

	return result;

}

int maxFileDescriptor(int** fd_list, int number_of_bidders){

	int max = 0;
	for (int i = 0; i < number_of_bidders; ++i)
	{
		if (fd_list[i][0] > max){
			max = fd_list[i][0];
		}
	}
	return max;


}

void server(int** fd_list, int* pid_list, int* status_list, int starting_bid, int min_inc, int number_of_bidders)
{
	//float open2[2] = {1,1}; /* keep a flag for if pipe is still open */

	fd_set readset;
	//char mess[40];

    //int buf[8];



	
	
    int current_highest_bid = 0;
    int current_highest_bidder_id ;
    //int min_inc = 5 ;
    //int starting_bid = 0 ;

    int m, r;
	//int open[2] = {1,1}; /* keep a flag for if pipe is still open */
	int* open = new int[number_of_bidders]; /* keep a flag for if pipe is still open */


	//int c[1] = {9}; //CHANGE IT! -> THE LENGTH OF THE ARRAY SHOULD BE EQUAL TO THE NUMBER OF BIDDERS 
    //bidderCompletionRecord[0] = 0; //THIS WIL BE SET TO true WHEN THE BIDDER SENDS THE "CLIENT_FINISHED" MESSAGE
    //bidderCompletionRecord[1] = false; //THIS WIL BE SET TO true WHEN THE BIDDER SENDS THE "CLIENT_FINISHED" MESSAGE




    bool* bidderCompletionRecord = new bool[number_of_bidders];

    for (int i = 0; i < number_of_bidders; ++i)
    {
    	bidderCompletionRecord[i] = false;
    	open[i] = 1; 
    	close(fd_list[i][1]);

    }


	//close(fd_list[0][1]); /* close unused ends */
	//close(fd_list[1][1]);

	//m = ((fd_list[0][0] > fd_list[1][0]) ? fd_list[0][0] : fd_list[1][0]) + 1; /* maximum of file descriptors */
	
    m = maxFileDescriptor(fd_list, number_of_bidders) + 1 ;


	printf("CONTROL\n");

	ii* input_info = new ii{};  // FREE()!

	oi* output_info = new oi{}; // FREE()!


	cei* connection_established_info = new cei{}; // FREE()!
	bi* bid_info = new bi{}; // FREE()!
	wi* the_winner_info = new wi{}; // FREE()!

	sm* server_message  = new sm{};  //TRY NOT TO USE IT
	//sm* server_message = new sm{};  // CHANGE ITS NAME
	//sm* server_message = new sm{};  //TRY NOT TO USE IT

	cm* client_message = new cm{};




	while (activeBidders(open, number_of_bidders)) {
		/* following initializes a blocking set for select() */
		FD_ZERO(&readset);
		//if (open[0]) FD_SET(fd_list[0][0],&readset);
		//if (open[1]) FD_SET(fd_list[1][0],&readset);

		for (int i = 0; i < number_of_bidders; ++i)
		{
			if (open[i]) FD_SET(fd_list[i][0],&readset);
		}


		/* the following code will block until any of them have data to read */
		select(m, &readset, NULL,NULL,NULL);  /* no wset, eset, timeout */

		/* now readset contains the fd's with available data */

		for (int i = 0; i < number_of_bidders; ++i)
		{
			if (FD_ISSET(fd_list[i][0], &readset)) {  /* check if pipe with index "i" is in readset */
				
				//r = read(fd_list[i][0], buf, 8);

				r = read(fd_list[i][0], client_message, sizeof(cm));
				
				if (r == 0)  {	/* EOF */
					printf("EOF\n");
					open[i] = 0;
				}
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

						


						server_message->message_id = SERVER_CONNECTION_ESTABLISHED; // change this to server_message  
						server_message->params.start_info = *connection_established_info;

						write(fd_list[i][0],server_message,sizeof(sm));
						print_output(output_info, i); // i is the client id assigned to the bidder
						
						printf("server_message ID %d\n", server_message->message_id );
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

						server_message->message_id = SERVER_BID_RESULT;
						server_message->params.result_info = *bid_info;
						

						output_info->type = SERVER_BID_RESULT;
						output_info->pid = pid_list[i];//*process_id_1;
						//output_info->info.start_info = NULL;
						output_info->info.result_info = *bid_info;
						//output_info->info.winner_info = NULL;

						

						write(fd_list[i][0],server_message,sizeof(sm));
						print_output(output_info, i); // i is the client id assigned to the bidder



					} else if (client_message->message_id == 3) // replace with CLIENT_FINISHED!
					{
						input_info->type = CLIENT_FINISHED;
						input_info->pid = pid_list[i];//*process_id_1;
						input_info->info = client_message->params;

						print_input(input_info, i); // i is the client id assigned to the bidder

						//printf("ID::: %d \n" , i);
						
						status_list[i] = client_message->params.status; // STATUS recieved from the client with id -> i
						//status_list[i] = i*9; // Check if the array is passed by reference
						
						bidderCompletionRecord[i] = true;

						if(isAllCompleted(bidderCompletionRecord, number_of_bidders)){
							
							the_winner_info->winner_id   = current_highest_bidder_id;
							the_winner_info->winning_bid = current_highest_bid;


							server_message->message_id = SERVER_AUCTION_FINISHED;
							server_message->params.winner_info = *the_winner_info;

							print_server_finished(current_highest_bidder_id,current_highest_bid);
							
							for (int i = 0; i < number_of_bidders; ++i)
							{
								output_info->type = SERVER_AUCTION_FINISHED;
								output_info->pid = pid_list[i];//*process_id_1;
								//output_info->info.start_info = NULL;
								//output_info->info.result_info = *bid_info;
								output_info->info.winner_info = *the_winner_info;

								write(fd_list[i][0],server_message,sizeof(sm));
								print_output(output_info, i); // i is the client id assigned to the bidder

							}
							free(bidderCompletionRecord);

						}

							

					}




						


				}

			}

		}
			



	}

}
///////////////////////////////onursehitoglu//////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


bool socket_check(int* socket_list, int number_of_bidders){

	bool result = true;

	for (int i = 0; i < number_of_bidders; ++i)
	{
		result = result && (socket_list[i] == 0);
	}

	return result;
}

int main()
{


	//////////////////////IO REDIRECTION///////////////////////
    //int file_desc = open("Test_Output.txt",O_WRONLY | O_APPEND); 
      

    //dup2(file_desc, STDOUT_FILENO) ; //  STDOUT_FILENO = 1


    int starting_bid, minimum_increment, number_of_bidders, number_of_arguments;

    
 	
  	int input_fds = open("./sample_to_work_on/inp1.txt", O_RDONLY); // TEST INPUT FILE

    if(dup2(input_fds, STDIN_FILENO) < 0) { // INPUT REDIRECTION FOR TESTING
    	printf("ERROR");
    	
  	}
  	
  	scanf("%d %d %d", &starting_bid, &minimum_increment, &number_of_bidders);
 	
 
  	printf("%d , %d , %d\n", starting_bid, minimum_increment , number_of_bidders);



	
	//int pipe1 = PIPE(fd1); //socket creation 1
	//int pipe2 = PIPE(fd2); //socket creation 2
	
	

   	printf("Number of Bidders: %d \n", number_of_bidders );



   	int** fd_list = new int*[number_of_bidders]; // FREE() when we are done!!!

	for(int i = 0; i < number_of_bidders; ++i){
    	fd_list[i] = new int[2];
	}

	//^
	//|
	//int fd_list[2][2]; 	//list of file desctiptors


	int* socket_list = new int[number_of_bidders]; // FREE() when we are done!!!

	for (int i = 0; i < number_of_bidders; ++i)
	{
		socket_list[i] = PIPE(fd_list[i]);
	}

	//int pipe1 = PIPE(fd_list[0]); //socket creation with list
	//int pipe2 = PIPE(fd_list[1]); //socket creation with list

	
	pid_t* pid_list = new pid_t[number_of_bidders]; // FREE() when we are done!!!

   	//pid_t pid_list[2];     //lists of child pids 


	if (socket_check(socket_list, number_of_bidders) )
	{
		printf("Socket Pair is succesfully created.\n");

		int w; // parameter for wait()

		int child_pid;


		for (int i = 0; i < number_of_bidders; ++i)
		{
			child_pid = fork(); //for parent process to save the child process id in the array pid_list

			if((pid_list[i] = child_pid) == 0)  // CHILD PROCESS
        	{ 

            	printf("Child Bidder ID: %d \n",i); 
            	printf("Child pid: %d from Parent pid: %d\n",getpid(),getppid()); 

            	int number_of_arguments;
            	char bidderExecutable[40]; //  40 is the maximum length for the bidderExecutable file name
    			                           //char* bidderExecutable = new char;

            	char bidderParameter[40];

            	scanf("%s %d", bidderExecutable, &number_of_arguments);

            	char** args2 = new char*[number_of_arguments+2];

            	args2[0] = bidderExecutable;
			
				printf("number_of_arguments: %d \n", number_of_arguments);
				printf("bidderExecutable:: %s \n", bidderExecutable);
            	
            	for (int i = 0; i < number_of_arguments; ++i)
            	{
            		scanf("%s", bidderParameter);
            		args2[i+1] = bidderParameter;
            		printf("TEST: %s\n", args2[i+1] );

            	}
            	args2[number_of_arguments+1] = NULL;


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
					printf("TEST1\n");
					if (dup2(fd_list[i][1], STDOUT_FILENO) != STDOUT_FILENO) {
						perror("Cannot dup2 stdout");
						exit(0);
					}
				}

				execv(bidderExecutable, args2);


            	//exit(5); //Should I exit or the bidder program has already exited   
        	
        	}
        	
		}

		int* status_list = new int[number_of_bidders];

		server(fd_list, pid_list, status_list, starting_bid, minimum_increment, number_of_bidders);

		//printf("status_list[0]: %d \n", status_list[0] );
		//printf("status_list[1]: %d \n", status_list[1] );

		int child_status;


		for (int i = 0; i < number_of_bidders; i++) {
			
			pid_t wPid = waitpid(pid_list[i], &child_status, 0);

			if (WIFEXITED(child_status)){
           		printf("Child %d terminated with exit status %d\n", wPid, WEXITSTATUS(child_status));
    			print_client_finished(i,WEXITSTATUS(child_status),WEXITSTATUS(child_status) == status_list[i]); // status match ???


			}
        	else{
	            printf("Child %d terminate abnormally\n", wPid);

        	}
    		
    		//pid_t childPid = wait(&w); 
    		//print_client_finished(i,w,1); // status match ???
			//printf("Child PID: %d\n",wPid );			
			//printf("Exit Status: %d\n",w );			
		} 

		free(pid_list);

		for(int i = 0; i < number_of_bidders; i++)
		{
			free(fd_list[i]);
		}
		
		free(fd_list);
		
		free(socket_list);
		



	}else{
		printf("Socket Pair error!.\n");

	}

	


	return 0;
}