
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <pthread.h>
#include <ctype.h>

#define NAME_SIZE 20
#define MSG_SIZE 100

char name[NAME_SIZE];
char msg[MSG_SIZE];

pthread_t send_tid;
pthread_t receive_tid;
int exitFlag;

char SERVER_IP[20];
char SERVER_PORT[6];

int client_sock;
void interrupt(int arg){
	printf("\nYou typped Ctrl + C\n");
	printf("Bye\n");

	pthread_cancel(send_tid);
	pthread_cancel(receive_tid);

	pthread_join(send_tid, 0);
	pthread_join(receive_tid, 0);

	close(client_sock);
	exit(1);
}
void removeEnterChar(char *buf){
	int len = strlen(buf);
	for (int i = len - 1; i >= 0; i--)	{
		if (buf[i] == '\n') {
			buf[i] = '\0';
			break;
		}
	}
}
void *sendMsg(){
	char buf[NAME_SIZE + MSG_SIZE + 1];

	while (!exitFlag){
		memset(buf, 0, NAME_SIZE + MSG_SIZE);
		
    	fgets(msg, MSG_SIZE, stdin);
        removeEnterChar(msg);
		if (!strcmp(msg, "exit")){
			exitFlag = 1;
			write(client_sock, msg, strlen(msg));
			break;
		}
		if (exitFlag) break;

        char beforeSpace[100];
        char afterSpace[100];
        memset(beforeSpace,0,100);
        memset(afterSpace,0,100);
        int i = 0;
        int foundSpace = 0;
        while (msg[i] != '\0') {
            if(msg[i] == ' '){
                foundSpace = 1;
                break;
            }
            beforeSpace[i] = msg[i];
            i++;
        }
        if (foundSpace) {
        strcpy(afterSpace, &msg[i + 1]);
        } else {
            // 공백이 없는 경우, afterSpace에 전체 문자열을 복사
            strcpy(afterSpace, msg);
            beforeSpace[0] = '\0'; // beforeSpace를 빈 문자열로 설정
        }
        int check = atoi(beforeSpace);
        if(check != 1 && check != 2){
            sprintf(buf, "%s %s", name, afterSpace);
        }
        else{
            int flag = 1;
            for(int i=0; i<strlen(afterSpace); i++){
                if(!isdigit(afterSpace[i])){
                    flag = 0;
                    break;
                }
            }

            if(flag == 1){
                if(check == 1){
                    sprintf(buf, "%s %d", name, atoi(afterSpace)*2);
                }
                else{
                    sprintf(buf, "%s %d", name, atoi(afterSpace)/2);
                }
            }
            else{
                for(int i=0; i<strlen(afterSpace); i++){
                    if(check == 1){
                        afterSpace[i] += 3;
                    }
                    else{
                        afterSpace[i] -= 3;
                    }
                }
                sprintf(buf, "%s %s", name, afterSpace);
            }

        }
		write(client_sock, buf, strlen(buf));
	}
}

void *receiveMsg(){
	char buf[NAME_SIZE + MSG_SIZE];
	while (!exitFlag){
		memset(buf, 0, NAME_SIZE + MSG_SIZE);
		int len = read(client_sock, buf, NAME_SIZE + MSG_SIZE - 1);
    		if (len == 0){
			printf("INFO :: Server Disconnected\n");
			kill(0, SIGINT);
			exitFlag = 1;
			break;
		}
		printf("%s\n", buf);
	}
}

int main(int argc, char *argv[]){
	if( argc<4 ){
		printf("ERROR Input [IP Addr] [Port Num] [User Name]\n");
		exit(1);
	}
	strcpy(SERVER_IP, argv[1]);
	strcpy(SERVER_PORT, argv[2]);
	sprintf(name, "[%s]", argv[3]);

	signal(SIGINT, interrupt);
	
	client_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (client_sock == -1){
		printf("ERROR :: 1_Socket Create Error\n");
		exit(1);
	}
	//printf("Socket Create!\n");
	
	struct sockaddr_in server_addr = {0};
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);
	server_addr.sin_port = htons(atoi(SERVER_PORT));
	socklen_t server_addr_len = sizeof(server_addr);
	
	if (connect(client_sock, (struct sockaddr *)&server_addr, server_addr_len) == -1){
		printf("ERROR :: 2_Connect Error\n");
		exit(1);
	}
	//printf("Connect Success!\n");
	
	pthread_create(&send_tid, NULL, sendMsg, NULL);
	pthread_create(&receive_tid, NULL, receiveMsg, NULL);

	pthread_join(send_tid, 0);
	pthread_join(receive_tid, 0);

	close(client_sock);
	return 0;
}
