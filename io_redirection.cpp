#include<stdlib.h> 
#include<unistd.h> 
#include<stdio.h> 
#include<fcntl.h> 
  
int main() 
{ 
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
    


  	



    
      
return 0; 
}