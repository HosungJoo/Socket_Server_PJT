#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <arpa/inet.h>

int server_sock;
int client_sock;

void interrupt(int arg){
    printf("\n You typed Ctrl + C\n");
    printf("Bye\n");

    close(client_sock);
    close(server_sock);

    exit(1);
}

void removeEnterChar(char *buf){
    int len = strlen(buf);
    for(int i=len-1; i>= 0; i--){
        if(buf[i] =='\n'){
            buf[i] ='\0';
            break;
        }
    }
}

int main(int argv, char* argc[]){

    signal(SIGINT, interrupt);   
    
    const char* PORT = argc[1];
    server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if(server_sock == -1){
        printf("ERROR :: 1_Socket Create Error\n");
        exit(1);
    }

    printf("Server On..\n");
    int optval = 1;
    setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, (void *)&optval, sizeof(optval));

    struct sockaddr_in server_addr = {0};
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(atoi(PORT));
    socklen_t server_addr_len = sizeof(server_addr);
    
    if(bind(server_sock, (struct sockaddr *)&server_addr, server_addr_len) == -1){
        printf("ERROR :: 2_bind Error \n");
        exit(1);
    }
    printf("Bind Success\n");

    if(listen(server_sock, 5) == -1){
        printf("ERROR ::3 _listen Error");
        exit(1);
    }

    printf("Wait Client....\n");

    client_sock = 0;
    struct sockaddr_in client_addr = {0};
    socklen_t client_addr_len = sizeof(client_addr);

    while(1){
        memset(&client_addr, 0, client_addr_len);

        /*
        server로부터 sockaddr을 받아와 client socket에 bind
        */
        client_sock = accept(server_sock,(struct sockaddr *)&client_addr, &client_addr_len);
        if(client_sock == -1){
            printf("ERROR ::4_Accept Error\n");
            break;
        }
        printf("Client Connect Success!\n");

        char buf[100];
        while(1){
            memset(buf,0,100);
            int len = read(client_sock, buf, 99);
            removeEnterChar(buf);
            if(len == 0){
                printf("INFO::Disconnect with client... BYE\n");
                break;
            }

            if(!strcmp(buf,"exit")){
                printf("INFO :: Client want close... BYE\n");
                break;
            }
            int flag = 1;
            for(int i=0; i<strlen(buf); i++){
                if(!isdigit(buf[i])){
                    flag = 0;
                    break;
                }
            }
            if(flag == 0){
                printf("%s\n",buf);
            }
            else{
                printf("%d\n",atoi(buf)*2);
            }
            memset(buf,0,100);
            scanf("%s",buf);
            write(client_sock, buf, 99);
        }
        close(client_sock);
        printf("Client Bye\n");
    } 
    close(server_sock);
    printf("Server Bye!");
    
    return 0;
}
