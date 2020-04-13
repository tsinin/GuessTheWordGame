#include <arpa/inet.h>
#include <fcntl.h>
#include <inttypes.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

const unsigned int LINES_NUM = 370104;
uint16_t ATTEMPTS = 12;

enum Status {
    Start = 0,
    ContinueLoop = 1,
    EndLoop = 2
};

volatile sig_atomic_t in_loop = 0;
volatile sig_atomic_t end = 0;
volatile sig_atomic_t accepting = 0;

void signal_handler(int sig_num)
{
    if (sig_num == SIGINT || sig_num == SIGTERM) {
        end = 1;
        if (!in_loop || accepting) {
            exit(0);
        }
    }
}

int main(int argc, char* argv[]) {
    struct sigaction sig_int_struct;
    struct sigaction sig_term_struct;

    sig_int_struct.sa_handler = signal_handler;
    sigemptyset(&sig_int_struct.sa_mask);
    sig_int_struct.sa_flags = 0;
    sig_term_struct.sa_handler = signal_handler;
    sigemptyset(&sig_term_struct.sa_mask);
    sig_term_struct.sa_flags = 0;

    sigaction(SIGINT, &sig_int_struct, NULL);
    sigaction(SIGTERM, &sig_term_struct, NULL);

    if (argc < 2) {
        printf("There must be at least 1 argument (port).\n");
        return 1;
    }
    if (argc == 3) {
        sscanf(argv[2], "%" SCNd16 "", &ATTEMPTS);
    }
    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd < 0) {
        printf("Error occured while creating socket.\n");
    }
    struct sockaddr_in serv_sock_addr;
    serv_sock_addr.sin_family = AF_INET;
    uint16_t port;
    sscanf(argv[1], "%" SCNd16 "", &port);
    serv_sock_addr.sin_port = htons(port);
    serv_sock_addr.sin_addr.s_addr = inet_addr("127.0.0.1");  // localhost

    if (bind(sock_fd, (struct sockaddr*)&serv_sock_addr, sizeof(serv_sock_addr)) < 0) {
        printf("Error occured while binding to the port.\n");
        return 1;
    }

    if (listen(sock_fd, SOMAXCONN) < 0) {
        printf("Error occured while starting listening the socket.\n");
        return 1;
    }

    FILE* words_file = fopen("words_database", "r");

    in_loop = 1;
    while (!end) {
        accepting = 1;
        int cl_sock_fd = accept(sock_fd, NULL, NULL);
        accepting = 0;
        if (cl_sock_fd < 0) {
            printf("Error occured while accepting the socket.\n");
            continue;
        }
        printf("accepted client...\n");
        enum Status status;
        int end_loop = 0;
        while (!end_loop) {

            // choosing randomly the word from file
            char word[40];
            unsigned int rand_line = rand() % LINES_NUM;
            fseek(words_file, 0, SEEK_SET);
            for (int i = 0; i < rand_line; ++i) {
                fscanf(words_file, "%s", word);
            }
            int word_len = strlen(word);
            printf("choosed the word: %s\n", word);

            // send to the client word length
            status = Start;
            write(cl_sock_fd, &status, 4);
            write(cl_sock_fd, &word_len, 4);
        
            // writing "***..."
            char client_word[word_len];
            memset(client_word, '*', word_len);
            write(cl_sock_fd, client_word, word_len);

            int attempts = 0;

            // loop
            while (1) {
                // read client's word
                // read exit=='#', restart=='!', answer=='?'
                char buf[7];
                int readed = read(cl_sock_fd, buf, 7);
                if (readed == 0) {      // client disconnected
                    end_loop = 1;
                    break;
                } else if (readed == 1) {
                    if (buf[0] == '#') {
                        end_loop = 1;
                        break;
                    } else if (buf[0] == '!') {
                        break;
                    } else if (buf[0] == '?') {
                        status = EndLoop;
                        write(cl_sock_fd, &status, 4);
                        write(cl_sock_fd, word, word_len);
                        break;
                    } else {
                        for (int i = 0; i < word_len; ++i) {
                            if (word[i] == buf[0]) {
                                client_word[i] = word[i];
                            }
                        }
                    }
                }
                int word_end = 1;
                for (int i = 0; i < word_len; ++i) {
                    if (client_word[i] == '*') {
                        word_end = 0;
                    }
                }
                attempts++;
                if (word_end || attempts == ATTEMPTS) {
                    status = EndLoop;
                    write(cl_sock_fd, &status, 4);
                    write(cl_sock_fd, word, word_len);
                    break;
                }
                status = ContinueLoop;
                write(cl_sock_fd, &status, 4);
                write(cl_sock_fd, client_word, word_len);
            }
        }

        // closing everything opened
        shutdown(cl_sock_fd, SHUT_RDWR);
        close(cl_sock_fd);
    }

    return 0;
}
