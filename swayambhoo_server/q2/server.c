#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>   
#include <sys/socket.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <signal.h>

#define SERVICE(i) "SERVERICE FROM SERVER "i


 int recv_fd(int socket)
 {
  int sent_fd, available_ancillary_element_buffer_space;
  struct msghdr socket_message;
  struct iovec io_vector[1];
  struct cmsghdr *control_message = NULL;
  char message_buffer[1];
  char ancillary_element_buffer[CMSG_SPACE(sizeof(int))];

  memset(&socket_message, 0, sizeof(struct msghdr));
  memset(ancillary_element_buffer, 0, CMSG_SPACE(sizeof(int)));

  io_vector[0].iov_base = message_buffer;
  io_vector[0].iov_len = 1;
  socket_message.msg_iov = io_vector;
  socket_message.msg_iovlen = 1;

  socket_message.msg_control = ancillary_element_buffer;
  socket_message.msg_controllen = CMSG_SPACE(sizeof(int));

  if(recvmsg(socket, &socket_message, MSG_CMSG_CLOEXEC) < 0)
   return -1;

  if(message_buffer[0] != 'F')
  {
   return -1;
  }

  if((socket_message.msg_flags & MSG_CTRUNC) == MSG_CTRUNC)
  {
   return -1;
  }

   for(control_message = CMSG_FIRSTHDR(&socket_message);
       control_message != NULL;
       control_message = CMSG_NXTHDR(&socket_message, control_message))
  {
   if( (control_message->cmsg_level == SOL_SOCKET) &&
       (control_message->cmsg_type == SCM_RIGHTS) )
   {
    sent_fd = *((int *) CMSG_DATA(control_message));
    return sent_fd;
   }
  }

  return -1;
 }



int main(int argc, char *argv[]){
    int sfd;
    if(argc != 2){
        fprintf(stderr,"Usage : %s port_number",argv[0]);
        return -1;
    }
    const uint16_t port_number = atoi(argv[1]);

    sfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sfd < 0){
        perror("socket()");
        return -1;
    }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    addr.sin_port = htons(port_number);

    if(bind(sfd, (struct sockaddr*) &addr, sizeof(addr))<0){
        perror("bind()");
        return -1;
    }

    if(listen(sfd, 1) < 0){
        perror("listen()");
        return -1;
    }

    
    struct sockaddr_in caddr;
    socklen_t caddrlen = sizeof(caddr);
    printf("listening...\n");
    signal(SIGPIPE, SIG_IGN);
    int nsfd = accept(sfd, (struct sockaddr*) &caddr,&caddrlen);
    printf("First client connected...\n");
    send(sfd, SERVICE(" FIRST"), strlen(SERVICE(" FIRST")), 0);
    close(sfd);


    int usfd;
    usfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if(usfd<0){
        perror("socket() unix");
        return -1;
    }
    addr.sin_family = AF_UNIX;

    if(bind(sfd, (struct sockaddr*) &addr, sizeof(addr))<0){
        perror("bind()");
        return -1;
    }

    addr.sin_family = AF_UNIX;

    while(1){
        printf("listening...\n");
        printf("next client sent...\n");
        pid_t pid = fork();
        if(pid == 0){
            //handle client
            if(connect(usfd, (const struct sockaddr *)&addr, sizeof(addr))< 0){
                perror("connect()");
                return -1;
            }
            int pfd = recv_fd(usfd);
            send(pfd, SERVICE(" NEXT"), strlen(SERVICE(" NEXT")), 0);
            close(pfd);
            exit(0);
        }
        
    }

}