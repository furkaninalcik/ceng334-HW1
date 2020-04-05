#include<sys/socket.h>
#include<sys/wait.h> 
#include <stdio.h>
#include <unistd.h>
#include<stdlib.h> 
#include<fcntl.h> 

#include <errno.h>

#include "lib/message.h"
#include "lib/logging.h"

//#include <cstring>

extern int errno;


#define PIPE(fd) socketpair(AF_UNIX, SOCK_STREAM, PF_UNIX, fd)

//////////////////////////////////////////////////////////////////////////////
///////////////////////////////hocanin codu//////////////////////////////////
bool socket_check(int*, int);
bool socket_check(int* socket_list, int number_of_bidders){

	bool result = true;

	for (int i = 0; i < number_of_bidders; ++i)
	{
		result = result && (socket_list[i] == 0);
	}

	return result;
}


bool isAllCompleted(bool*, int);
bool isAllCompleted(bool completionRecord[], int number_of_bidders){
	
	bool result = true;

	for (int i = 0; i < number_of_bidders; ++i)
	{
		result = result && completionRecord[i];			
	}

	return result;

}

bool activeBidders(int*, int);
bool activeBidders(int* bidderActivityList, int number_of_bidders){

	bool result = false;

	for (int i = 0; i < number_of_bidders; ++i)
	{
		result = result || bidderActivityList[i];	// This will be true if anyone of the bidders is active
	}

	return result;

}
int maxFileDescriptor(int**, int);
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
void server(int**, int*, int*, int, int, int );
void server(int** fd_list, int* pid_list, int* status_list, int starting_bid, int min_inc, int number_of_bidders)
{
	//float open2[2] = {1,1}; /* keep a flag for if pipe is still open */

	fd_set readset;
	//char mess[40];

    //int buf[8];



	
	
    int current_highest_bid = 0; //or starting_bid, but not stated clearly in the pdf
    int current_highest_bidder_id ;
    //int min_inc = 5 ;
    //int starting_bid = 0 ;

    int m, r;
	//int open[2] = {1,1}; /* keep a flag for if pipe is still open */
	int* open = new int[number_of_bidders]; /* keep a flag for if pipe is still open */


	//int c[1] = {9}; //CHANGE IT! -> THE LENGTH OF THE ARRAY SHOULD BE EQUAL TO THE NUMBER OF BIDDERS 
    //bidderCompletionRecord[0] = 0; //THIS WIL BE SET TO true WHEN THE BIDDER SENDS THE "CLIENT_FINISHED" MESSAGE
    //bidderCompletionRecord[1] = false; //THIS WIL BE SET TO true WHEN THE BIDDER SENDS THE "CLIENT_FINISHED" MESSAGE



    for (int i = 0; i < number_of_bidders; ++i)
    {
    	open[i] = 1; 
    	close(fd_list[i][1]);

    }


	//close(fd_list[0][1]); /* close unused ends */
	//close(fd_list[1][1]);

	//m = ((fd_list[0][0] > fd_list[1][0]) ? fd_list[0][0] : fd_list[1][0]) + 1; /* maximum of file descriptors */
	
    m = maxFileDescriptor(fd_list, number_of_bidders) + 1 ;


	//printf("CONTROL\n");

	//ii* the_input_info = new ii{};  // FREE()!

	//oi* the_output_info = new oi{}; // FREE()!


	//cei* the_connection_established_info = new cei{}; // FREE()!
	//bi* the_bid_info = new bi{}; // FREE()!
	//wi* the_winner_info = new wi{}; // FREE()!

	//sm* the_server_message  = new sm{};  //TRY NOT TO USE IT
	//sm* the_server_message = new sm{};  // CHANGE ITS NAME
	//sm* the_server_message = new sm{};  //TRY NOT TO USE IT

	//cm* the_client_message = new cm{};
	cm the_client_message ;




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

				r = read(fd_list[i][0], &the_client_message, sizeof(cm));
				
				if (r == 0)  {	/* EOF */
					//printf("EOF\n");
					open[i] = 0;
				}
				else{

					if (the_client_message.message_id == CLIENT_CONNECT)
					{
						ii the_input_info;
						the_input_info.type = CLIENT_CONNECT;
						the_input_info.pid = pid_list[i];
						the_input_info.info = the_client_message.params;
						
						//pid_t pid = getpid(); THIS DOES NOT WORK SINCE 
											  //THE RETURN VALUE IS THE PROCESS ID OF THE PARENT, NOT THE CHILD


						print_input(&the_input_info, i); // i is the client id assigned to the bidder
						
						cei the_connection_established_info;

						the_connection_established_info.client_id = i; 
						the_connection_established_info.starting_bid = starting_bid;
						the_connection_established_info.current_bid = current_highest_bid;
						the_connection_established_info.minimum_increment = min_inc;
		
						oi the_output_info;
						the_output_info.type = SERVER_CONNECTION_ESTABLISHED;
						the_output_info.pid = pid_list[i];
						the_output_info.info.start_info = the_connection_established_info;
						//the_output_info.info.result_info = NULL;
						//the_output_info.info.winner_info = NULL;

						
						sm the_server_message;

						the_server_message.message_id = SERVER_CONNECTION_ESTABLISHED; // change this to the_server_message  
						the_server_message.params.start_info = the_connection_established_info;

						write(fd_list[i][0],&the_server_message,sizeof(sm));
						print_output(&the_output_info, i); // i is the client id assigned to the bidder
						
						//printf("the_server_message ID %d\n", the_server_message.message_id );
						//write(out_fd1[0],"1111",4);
						//close(fd_list[i][0]);



					}else if (the_client_message.message_id == CLIENT_BID)
					{
						//printf("Message received!\n");
						ii the_input_info;

						the_input_info.type = CLIENT_BID;
						the_input_info.pid = pid_list[i];
						the_input_info.info = the_client_message.params;
						
						//printf("client bid: %d\n", the_client_message.params.bid );
						print_input(&the_input_info, i); // i is the client id assigned to the bidder



						/*
						if (the_client_message.params.bid >= current_highest_bid + min_inc ) 
						{}*/
					
						bi the_bid_info;
					
						if (the_client_message.params.bid < starting_bid) //BID_LOWER_THAN_STARTING_BID -> 1
						{
							//printf("11111111111111111111111111111111\n");
							the_bid_info.result      = BID_LOWER_THAN_STARTING_BID; //BID_LOWER_THAN_STARTING_BID -> 1
							the_bid_info.current_bid = current_highest_bid;

							

						} else if (the_client_message.params.bid < current_highest_bid) //BID_LOWER_THAN_CURRENT -> 2
						{
							//printf("22222222222222222222222222222222\n");

							the_bid_info.result      = BID_LOWER_THAN_CURRENT; //BID_LOWER_THAN_CURRENT -> 2
							the_bid_info.current_bid = current_highest_bid;

							

						} else if (the_client_message.params.bid - current_highest_bid < min_inc) //BID_INCREMENT_LOWER_THAN_MINIMUM ->3
						{
							//printf("33333333333333333333333333333333\n");
							the_bid_info.result      = BID_INCREMENT_LOWER_THAN_MINIMUM; //BID_INCREMENT_LOWER_THAN_MINIMUM -> 3
							the_bid_info.current_bid = current_highest_bid;

							
						} else{ //BID_ACCEPTED -> 0
							
							current_highest_bid = the_client_message.params.bid;
							current_highest_bidder_id = i; 					// BIDDER ID -> "i"

							the_bid_info.result      = BID_ACCEPTED; //BID_ACCEPTED -> 0
							the_bid_info.current_bid = current_highest_bid;
						
						}

						sm the_server_message;
						the_server_message.message_id = SERVER_BID_RESULT;
						the_server_message.params.result_info = the_bid_info;
						
						oi the_output_info;
						the_output_info.type = SERVER_BID_RESULT;
						the_output_info.pid = pid_list[i];
						//the_output_info.info.start_info = NULL;
						the_output_info.info.result_info = the_bid_info;
						//the_output_info.info.winner_info = NULL;

						

						write(fd_list[i][0],&the_server_message,sizeof(sm));
						print_output(&the_output_info, i); // i is the client id assigned to the bidder



					} else if (the_client_message.message_id == CLIENT_FINISHED) // replace with CLIENT_FINISHED!
					{
						ii the_input_info;

						the_input_info.type = CLIENT_FINISHED;
						the_input_info.pid = pid_list[i];
						the_input_info.info = the_client_message.params;

						print_input(&the_input_info, i); // i is the client id assigned to the bidder

						//printf("ID::: %d \n" , i);
						
						status_list[i] = the_client_message.params.status; // STATUS recieved from the client with id -> i
						//status_list[i] = i*9; // Check if the array is passed by reference
						
						open[i] = 0; //the filedescriptor of the bidder with id "i" will never be checked for reading data

						
					}

				}

			}

		}
			



	}
	delete[] open;
	//SENDING "SERVER_AUCTION_FINISHED" Message to All the Bidders		
	wi the_winner_info;
	the_winner_info.winner_id   = current_highest_bidder_id;
	the_winner_info.winning_bid = current_highest_bid;

	sm the_server_message;
	the_server_message.message_id = SERVER_AUCTION_FINISHED;
	the_server_message.params.winner_info = the_winner_info;

	print_server_finished(current_highest_bidder_id,current_highest_bid);
	
	for (int i = 0; i < number_of_bidders; ++i)
	{
		oi the_output_info;
		the_output_info.type = SERVER_AUCTION_FINISHED;
		the_output_info.pid = pid_list[i];
		//the_output_info.info.start_info = NULL;
		//the_output_info.info.result_info = NULL;
		the_output_info.info.winner_info = the_winner_info;

		write(fd_list[i][0],&the_server_message,sizeof(sm));
		print_output(&the_output_info, i); // i is the client id assigned to the bidder

	}





}
///////////////////////////////hocanin codu//////////////////////////////////
//////////////////////////////////////////////////////////////////////////////




int main()
{


	//////////////////////IO REDIRECTION///////////////////////
    //int file_desc = open("Test_Output.txt",O_WRONLY | O_APPEND); 
      

    //dup2(file_desc, STDOUT_FILENO) ; //  STDOUT_FILENO = 1


    int starting_bid, minimum_increment, number_of_bidders;

    
 	
  	int input_fds = open("./sample_to_work_on/inp_test1.txt", O_RDONLY); // TEST INPUT FILE

    if(dup2(input_fds, STDIN_FILENO) < 0) { // INPUT REDIRECTION FOR TESTING
    	//printf("ERROR");
    	
  	}
  	
  	scanf("%d %d %d", &starting_bid, &minimum_increment, &number_of_bidders);
 	
 
  	//printf("%d , %d , %d\n", starting_bid, minimum_increment , number_of_bidders);


	
	//int pipe1 = PIPE(fd1); //socket creation 1
	//int pipe2 = PIPE(fd2); //socket creation 2
	
	

   	//printf("Number of Bidders: %d \n", number_of_bidders );



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
		//printf("Socket Pair is created succesfully.\n");

		int w; // parameter for wait()

		int child_pid;


		for (int i = 0; i < number_of_bidders; ++i)
		{
			child_pid = fork(); //for parent process to save the child process id in the array pid_list

			if((pid_list[i] = child_pid) == 0)  // CHILD PROCESS
        	{ 

            	//printf("Child Bidder ID: %d \n",i); 
            	//printf("Child pid: %d from Parent pid: %d\n",getpid(),getppid()); 

            	int number_of_arguments;
            	char bidderExecutable[40]; //  40 is the maximum length for the bidderExecutable file name
    			                           //char* bidderExecutable = new char;

            	char bidderParameter[40];

            	scanf("%s %d", bidderExecutable, &number_of_arguments);

	           	char** args = new char*[number_of_arguments+2]; // Because args[0] = ExecutableName and args[number_of_arguments] = NULL
	           	
	           	for (int j = 0; j < number_of_arguments+2; ++j)
	           	{
	           		args[j] = new char[40];
	           	}

	           	//char args[number_of_arguments+2][40]; // Because args[0] = ExecutableName and args[number_of_arguments] = NULL
	           	//String args[6]; // Because args[0] = ExecutableName and args[number_of_arguments] = NULL



            	args[0] = bidderExecutable; //Convension
			
				printf("number_of_arguments: %d \n", number_of_arguments);
				printf("bidderExecutable:: %s \n", args[0]);
            	
            	for (int z = 1; z < number_of_arguments+1; ++z)
            	{
            		scanf("%s", args[z]);

            		//args[i] = bidderParameter;

            	}
            		//scanf("%s %s %s", args[1], args[2], args[3]);

            	args[number_of_arguments+1] = NULL; // Null terminated argument list

            	/*char* args3[6];
            	args3[0]= "./PaternBidder";
            	args3[1]= "100";
            	args3[2]= "0";
            	args3[3]= "1";
            	args3[4]= "2";
            	args3[5]= NULL;
				*/
            	for (int t = 0; t < number_of_arguments+2; ++t)
            	{
            		printf("ARG %d : %s\n", t ,args[t] );
            	}

				close(fd_list[i][0]);
				
		
				//STDIN REDIRECTION
				if (fd_list[i][1] != STDIN_FILENO) { 
					if (dup2(fd_list[i][1], STDIN_FILENO) != STDIN_FILENO) {
						perror("Error in stdin redirection!");
						exit(0);
					}
				}

				//STDOUT REDIRECTION
				if (fd_list[i][1] != STDOUT_FILENO) { 
					if (dup2(fd_list[i][1], STDOUT_FILENO) != STDOUT_FILENO) {
						perror("Error in stdout redirection!");
						exit(0);
					}
				}

				execv(bidderExecutable, args);

				
				for(int d = 0; d < number_of_arguments+2; d++)
				{
					delete[] args[d];
				}
				
				delete[] args;
				

            	//exit(5); //Should I exit or the bidder program has already exited   
        	
        	}
        	
		}

		int* status_list = new int[number_of_bidders];

		server(fd_list, pid_list, status_list, starting_bid, minimum_increment, number_of_bidders);


		int child_status;


		for (int i = 0; i < number_of_bidders; i++) {
			
			pid_t wPid = waitpid(pid_list[i], &child_status, 0);

			if (WIFEXITED(child_status)){
           		//printf("Child %d terminated with exit status %d\n", wPid, WEXITSTATUS(child_status));
           		
           		bool status_match = (WEXITSTATUS(child_status) == status_list[i]);

    			print_client_finished(i,WEXITSTATUS(child_status),status_match); 


			}
        	else{
	            printf("Child %d terminate abnormally\n", wPid);

        	}
    		
		
		} 

		delete[] pid_list;

		for(int i = 0; i < number_of_bidders; i++)
		{
			delete[] fd_list[i];
		}
		
		delete[] fd_list;
		
		delete[] socket_list;

		delete[] status_list;
		



	}else{
		//printf("ERROR -> Socket Pair Creation!.\n");

	}

	


	return 0;
}