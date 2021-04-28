#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

#include <sys/types.h>   
#include <sys/socket.h>
#include <sys/un.h>
#include <arpa/inet.h>

#include <sys/select.h>

#define ISS_LOC     "/tmp/iss_sock.socket"
#define MAX_FDS     100


int update_port_fds(int sfds[], int usfds[], int curr, const uint16_t port_number){
    //open 2 fds, 1 for unix domain (for fd passing), and another sfd (for client to connect to)
    int usfd, sfd;

    usfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if(usfd<0){
        perror("socket() unix");
        return -1;
    }
    sfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sfd < 0){
        perror("socket()");
        return -1;
    }

    
    int on = 1;
    if(setsockopt(sfd, SOL_SOCKET, SO_REUSEPORT, &on, sizeof(on)) < 0){
        perror("setsockopt()");
        return -1;
    }
    if(setsockopt(usfd, SOL_SOCKET, SO_REUSEPORT, &on, sizeof(on)) < 0){
        perror("setsockopt() unix");
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

    

    addr.sin_family = AF_UNIX;
    if(bind(usfd, (struct sockaddr*) &addr, sizeof(addr))<0){
        perror("bind() unix");
        return -1;
    }



    if(listen(sfd, 5) < 0){
        perror("listen()");
        return -1;
    }

    if(listen(usfd, 1) < 0){
        perror("listen() usfd");
        return -1;
    }

    sfds[curr] = sfd;
    usfds[curr] = usfd;
}

int send_fd(int socket, int fd_to_send)
 {
  struct msghdr socket_message;
  struct iovec io_vector[1];
  struct cmsghdr *control_message = NULL;
  char message_buffer[1];
  /* storage space needed for an ancillary element with a paylod of length is CMSG_SPACE(sizeof(length)) */
  char ancillary_element_buffer[CMSG_SPACE(sizeof(int))];
  int available_ancillary_element_buffer_space;

  /* at least one vector of one byte must be sent */
  message_buffer[0] = 'F';
  io_vector[0].iov_base = message_buffer;
  io_vector[0].iov_len = 1;

  /* initialize socket message */
  memset(&socket_message, 0, sizeof(struct msghdr));
  socket_message.msg_iov = io_vector;
  socket_message.msg_iovlen = 1;

  /* provide space for the ancillary data */
  available_ancillary_element_buffer_space = CMSG_SPACE(sizeof(int));
  memset(ancillary_element_buffer, 0, available_ancillary_element_buffer_space);
  socket_message.msg_control = ancillary_element_buffer;
  socket_message.msg_controllen = available_ancillary_element_buffer_space;

  /* initialize a single ancillary data element for fd passing */
  control_message = CMSG_FIRSTHDR(&socket_message);
  control_message->cmsg_level = SOL_SOCKET;
  control_message->cmsg_type = SCM_RIGHTS;
  control_message->cmsg_len = CMSG_LEN(sizeof(int));
  *((int *) CMSG_DATA(control_message)) = fd_to_send;

  return sendmsg(socket, &socket_message, 0);
 }



int main(int argc, char *argv[]){

    signal(SIGPIPE, SIG_IGN);
    int iss_usfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if(iss_usfd < 0){
        perror("socket()");
        return -1;
    }

    struct sockaddr_un iss_addr;
    iss_addr.sun_family = AF_UNIX;
    strcpy(iss_addr.sun_path, ISS_LOC);
    if(bind(iss_usfd, (const struct sockaddr *) &iss_addr, sizeof(struct sockaddr)) < 0){
        perror("bind() iss");
        return -1;
    }
    unlink(ISS_LOC);
    
    if(listen(iss_usfd, 1) < 0){
        perror("listen() iss");
        return -1;
    }

    iss_usfd = accept(iss_usfd, NULL, 0);
    printf("linekd to tracer...\n");

    printf("listening...\n");


    fd_set rfds;
    FD_ZERO(&rfds);
    FD_SET(iss_usfd, &rfds);
    int max_fd = iss_usfd + 1;
    int sfds[MAX_FDS], usfds[MAX_FDS];
    int curr = 0;

    while(1){
        fd_set ofds = rfds;
        int status = select(max_fd, &rfds, NULL, NULL, NULL);

        if(status > 0){
            if(FD_ISSET(iss_usfd, &rfds)){
                if(curr < MAX_FDS){
                    char buff[5];
                    if(read(iss_usfd, buff, 5) < 0){
                        perror("read() port");
                    }
                    close(iss_usfd);
                    printf("new port received : %s\n", buff);
                    update_port_fds(sfds, usfds, curr, atoi(buff));
                    FD_SET(sfds[curr], &ofds);
                    max_fd = sfds[curr] + 1;
                    curr++;
                }else{
                    fprintf(stderr, "cant open more than 100 ports\n");
                }
            }else{
                //check for clients
                for(int i=0;i<curr;i++){
                    printf("client connected\n");
                    if(FD_ISSET(sfds[i], &rfds)){
                        int nsfd = accept(sfds[i], NULL, 0);
                        int unsfd = accept(usfds[i], NULL, 0);
                        //pass fd
                        send_fd(unsfd, nsfd);
                        close(nsfd);
                        close(unsfd);
                    }
                }
            }
        }

        rfds = ofds;
    }

}