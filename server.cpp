#include<sys/socket.h>
#include <stdio.h>
#include <unistd.h>
#include<stdlib.h> 
#include<fcntl.h> 



#define PIPE(fd) socketpair(AF_UNIX, SOCK_STREAM, PF_UNIX, fd)




int main()
{


	//////////////////////IO REDIRECTION///////////////////////
    int file_desc = open("./sample_to_work_on/out1.txt",O_WRONLY | O_APPEND); 
      

    dup2(file_desc, STDOUT_FILENO) ; //  STDOUT_FILENO = 1


    int starting_bid, minimum_increment, number_of_bidders;
 
  	int input_fds = open("./sample_to_work_on/inp1.txt", O_RDONLY);

    if(dup2(input_fds, STDIN_FILENO) < 0) {
    	printf("ERROR");
    	
  	}
  	
  	scanf("%d %d %d", &starting_bid, &minimum_increment, &number_of_bidders);
  	//scanf("%d %d", &first_bidder_executable, &minimum_increment);
 	
 
  	printf("%d , %d , %d\n", starting_bid, minimum_increment , number_of_bidders);
    
	//////////////////////IO REDIRECTION///////////////////////


	int fd[2];
	int pipe = PIPE(fd); //socket creation
	if (pipe == 0)
	{
		printf("Socket Pair is succesfully created.\n");
	}else{
		printf("Socket Pair error!.\n");

	}

	


	return 0;
}