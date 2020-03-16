#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>

int main(int argc, char * argv[]){
	int n, i, j, nbprocessus, test, id, tmp_id, return_id;
	int tube_up_A[2], tube_up_B[2], tube_up_C[2], tube_A[2], tube_B[2], tube_C[2];
	n = atoi(*(argv+1));
	if(n<0){n *=-1;}
	nbprocessus = (n*(n+1))/2;
	printf("n = %d nbprocessus = %d \n", n, nbprocessus);
	
	//Initialisation des tubes de la racine
	pipe(&tube_A[0]); //lft
	pipe(&tube_B[0]); //rgt impair (i)
	pipe(&tube_up_A[0]); //up lft
	pipe(&tube_up_B[0]); //up rgt impair (i)
	
	//Création des processus
	test=1;
	i = 1;
	j = 0;
	while(test){
		//Random du id entre n*n
		srand(time(NULL)*getpid());
		id = rand()%(n*n)+1;
		return_id = id;
		
		printf("i=%d j=%d id=%d \n",i,j,id);
		test=0;
		
		if(j == 0 && i<n){
			if(fork() == 0){ //processus right
				i++;
				test=1;
				
				if(i < n){
					if(i%2 == 0){ // pair (i)
						pipe(&tube_A[0]); //lft
						pipe(&tube_C[0]); //rgt pair (i)
						pipe(&tube_up_A[0]); //up lft
						pipe(&tube_up_C[0]); //up rgt pair (i)
					}else{ //impair (i)
						pipe(&tube_A[0]); //lft
						pipe(&tube_B[0]); //rgt impair (i)
						pipe(&tube_up_A[0]); //up lft
						pipe(&tube_up_B[0]); //up rgt impair (i)
					}
				}
			}else{
				if(fork() == 0){ //processus left
					j++;
					test=1;
					
					if(j%2 == 0){ //pair (j)
						pipe(&tube_A[0]); //lft
						pipe(&tube_up_A[0]); //up lft
					}else{ //impair (j)
						pipe(&tube_C[0]); //lft
						pipe(&tube_up_C[0]); //up lft
					}				
				}
			}
		}else if(j < n-i){
			if(fork() == 0){ //processus left
				j++;
				test=1;
				
				if(j%2 == 0){ //pair (j)
					pipe(&tube_A[0]); //left
					pipe(&tube_up_A[0]); //up left
					
				}else{ //impair (j)
					pipe(&tube_C[0]); //left
					pipe(&tube_up_C[0]); //up left
				}
				
			}
		}
		
	}
	
	//processus (n,0)
	if(i == n && j == 0){
		//printf(" Ffin id=%d i=%d j=%d \n",id,i,j);
		if(i%2 == 0){ //pair (i)
			close(tube_up_B[0]);
			close(tube_B[1]);
			
			write(tube_up_B[1], &id, sizeof(int));
			
			//récupère le minimum
			read(tube_B[0], &return_id, sizeof(int));
		}else{
			close(tube_up_C[0]);
			close(tube_C[1]);
			
			write(tube_up_C[1], &id, sizeof(int));
			
			//récupère le minimum
			read(tube_C[0], &return_id, sizeof(int));
		}
	} //processus(i,n-i)
	else if(j == n-i){
		//printf(" fin id=%d i=%d j=%d \n",id,i,j);
		if(j%2 == 0){ //pair (j)
			close(tube_up_C[0]);
			close(tube_C[1]);
			
			write(tube_up_C[1], &id, sizeof(int));
			
			//récupère le minimum
			read(tube_C[0], &return_id, sizeof(int));
		}else{
			close(tube_up_A[0]);
			close(tube_A[1]);
			
			write(tube_up_A[1], &id, sizeof(int));
			
			//récupère le minimum
			read(tube_A[0], &return_id, sizeof(int));
		}
	} //processus(i,0)
	else if(j == 0){
		close(tube_up_A[1]);
		close(tube_A[0]);
		
		read(tube_up_A[0], &tmp_id, sizeof(int));
		//printf(" tmp_id=%d i=%d j=%d \n",tmp_id,i,j);
		
		if(return_id > tmp_id){return_id = tmp_id;}
		
		if(i%2 == 0){ //pair (i)
			close(tube_B[1]);
			close(tube_up_B[0]);
			close(tube_C[0]);
			close(tube_up_C[1]);
			
			read(tube_up_C[0], &tmp_id, sizeof(int));
		}else{ //impair (i)
			close(tube_B[0]);
			close(tube_up_B[1]);
			close(tube_C[1]);
			close(tube_up_C[0]);
			
			read(tube_up_B[0], &tmp_id, sizeof(int));
		}
		
		//printf(" tmp_id=%d i=%d j=%d \n",tmp_id,i,j);
		
		if(return_id > tmp_id){return_id = tmp_id;}
		
		if(i != 1){
			
			if(i%2 == 0){ //pair (i)
				write(tube_up_B[1], &return_id, sizeof(int));
				
				//récupère le minimum et l'envoie
				read(tube_B[0], &return_id, sizeof(int));
				write(tube_C[1], &return_id, sizeof(int));
			}else{ //impair (i)
				write(tube_up_C[1], &return_id, sizeof(int));
				
				//récupère le minimum et l'envoie
				read(tube_C[0], &return_id, sizeof(int));
				write(tube_B[1], &return_id, sizeof(int));
			}
			write(tube_A[1], &return_id, sizeof(int));
			
		}else{ //processus(1,0) affiche le minimun
			printf("Le minimun est %d \n", return_id);
			
			//envoie le minimum
			write(tube_B[1], &return_id, sizeof(int));
			write(tube_A[1], &return_id, sizeof(int));
		}
	} //processus(i,j) j!=0 et j!=n-i
	else{
		close(tube_B[0]);
		close(tube_B[1]);
		close(tube_up_B[0]);
		close(tube_up_B[1]);
			
		if(j%2 == 0){ //pair (j)
			close(tube_A[0]);
			close(tube_up_A[1]);
			close(tube_C[1]);
			close(tube_up_C[0]);
		
			read(tube_up_A[0], &tmp_id, sizeof(int));
			//printf(" tmp_id=%d i=%d j=%d \n",tmp_id,i,j);
			
			if(return_id > tmp_id){return_id = tmp_id;}
			
			write(tube_up_C[1], &return_id, sizeof(int));
			
			//récupère le minimum et l'envoie
			read(tube_C[0], &return_id, sizeof(int));
			write(tube_A[1], &return_id, sizeof(int));
			
			close(tube_A[1]);
			close(tube_up_A[0]);
			close(tube_C[0]);
			close(tube_up_C[1]);
			
		}else{ //impair (j)
			close(tube_A[1]);
			close(tube_up_A[0]);
			close(tube_C[0]);
			close(tube_up_C[1]);
			
			read(tube_up_C[0], &tmp_id, sizeof(int));
			//printf(" tmp_id=%d i=%d j=%d \n",tmp_id,i,j);
			
			if(return_id > tmp_id){return_id = tmp_id;}
			
			write(tube_up_A[1], &return_id, sizeof(int));
			
			//récupère le minimum et l'envoie
			read(tube_A[0], &return_id, sizeof(int));
			write(tube_C[1], &return_id, sizeof(int));
			
			close(tube_A[0]);
			close(tube_up_A[1]);
			close(tube_C[1]);
			close(tube_up_C[0]);
		}
	}
	
	//Affiche le ou les élu(s)
	if(return_id == id){printf("i=%d j=%d est elu ! \n",i,j);}
	exit(0);
}
