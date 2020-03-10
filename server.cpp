#include<sys/socket.h>
#include<sys/wait.h> 
#include <stdio.h>
#include <unistd.h>
#include<stdlib.h> 
#include<fcntl.h> 

#include <errno.h>

extern int errno;


#define PIPE(fd) socketpair(AF_UNIX, SOCK_STREAM, PF_UNIX, fd)

//////////////////////////////////////////////////////////////////////////////
///////////////////////////////onursehitoglu//////////////////////////////////
void server(int p1[], int p2[])
{
	fd_set readset;
	char mess[40];
	int m, r;
	int open[2] = {1,1}; /* keep a flag for if pipe is still open */

	close(p1[1]); /* close unused ends */
	close(p2[1]);

	m = ((p1[0] > p2[0]) ? p1[0] : p2[0]) + 1; /* maximum of file descriptors */

	while (open[0] || open[1]) {
		/* following initializes a blocking set for select() */
		FD_ZERO(&readset);
		if (open[0]) FD_SET(p1[0],&readset);
		if (open[1]) FD_SET(p2[0],&readset);

		/* the following code will block until any of them have data to read */
		select(m, &readset, NULL,NULL,NULL);  /* no wset, eset, timeout */

		/* now readset contains the fd's with available data */
		if (FD_ISSET(p1[0], &readset)) {  /* check if first pipe is in readset */
			r = read(p1[0], mess, 40);
			if (r == 0)  	/* EOF */
				open[0] = 0;
			else
				printf("%d: %s\n", 0, mess);

		}

		/* following if is not in else since both may have data to read */
		if (FD_ISSET(p2[0], &readset)) {  /* check if second pipe is in readset */
			r = read(p2[0], mess, 40);
			if ( r == 0 )  /* EOF */
				open[1] = 0;
			else
				printf("%d: %s\n", 1, mess);
		}

	}

}
///////////////////////////////onursehitoglu//////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

int main()
{


	//////////////////////IO REDIRECTION///////////////////////
    int file_desc = open("./sample_to_work_on/out1.txt",O_WRONLY | O_APPEND); 
      

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

	if (pipe1 == 0 && pipe2 == 0)
	{
		printf("Socket Pair is succesfully created.\n");

		int w; // parameter for wait()

		if (fork()) {
			if (fork())
			{
				printf("PARENT\n");  /* Parent process */
				server(fd1,fd2);
				wait(&w);
				wait(&w);
			}else{												/* Child process 1*/
				printf("Child process: Bidder 1\n");

				char *newargs[3];
				newargs[0] = "Bidder";  /* Convention! Not required!! */
				newargs[1] = "50";
				newargs[2] = NULL; 
				execv("/home/furkan/Desktop/ceng334/HW1/bin/Bidder", newargs);

			}
		} else {       /* Child process 2*/

			printf("Child process: Bidder 2\n");
			char *newargs[3];
			newargs[0] = "Bidder";  /* Convention! Not required!! */
			newargs[1] = "50";
			newargs[2] = NULL; 
			execv("/home/furkan/Desktop/ceng334/HW1/bin/Bidder", newargs);

		}



	}else{
		printf("Socket Pair error!.\n");

	}

	


	return 0;
}