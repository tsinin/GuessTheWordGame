#include <arpa/inet.h>
#include <inttypes.h>
#include <netinet/in.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>

enum Status {
    Start = 0,
    ContinueLoop = 1,
    EndLoop = 2
};

int main(int argc, char* argv[]) {
    if (argc != 3) {
        printf("There must be 2 arguments only\n");
        return 1;
    }
    struct sockaddr_in sockaddr;
    sockaddr.sin_family = AF_INET;
    uint16_t port;
    sscanf(argv[2], "%" SCNd16 "", &port);
    sockaddr.sin_port = htons(port);
    sockaddr.sin_addr.s_addr = inet_addr(argv[1]);

    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd < 0) {
        printf("Error caught while opening socket.\n");
        return 1;
    }
    if (connect(sock_fd, (const struct sockaddr*)&sockaddr, sizeof(sockaddr)) < 0) {
        printf("Connection error.\n");
        return 1;
    }

    printf("Connected to server %s (port = %s).\n", argv[1], argv[2]);
    printf("Here are the rules:\n"
           "You'll get the word with letters exchanged by '*'. "
           "You need to understand what is the word (open the hole word).\n"
           "Write by ONE letter and if this letter is in this word, then it'll be opened.\n"
           "Furthermore, you'll have only constant number of attempts to open the whole word.\n"
           "If you want to exit the game, write '#'.\n"
           "If you want to know what the word is, write '?'.\n"
           "If you want to get the new word instead of current, write '!'.\n"
           "Good game!\n");


    enum Status status;
    char word[40];
    int word_len;
    int end = 0;

    while (!end) {
        if (read(sock_fd, &status, 4) <= 0) {
            break;
        }

        if (status == Start) {
            read(sock_fd, &word_len, 4);
            word[word_len] = '\0';
            printf("Your new word is: ");
        } else if (status == ContinueLoop) {
            
        } else if (status == EndLoop) {
            read(sock_fd, word, word_len);
            printf("%s\n", word);
            printf("End of the game, starting new...\n");
            continue;
        }

        read(sock_fd, word, word_len);
        printf("%s\n", word);

        char query[100];
        if (scanf("%s", query) <= 0) {
            break;
        }
        if (write(sock_fd, query, strlen(query)) <= 0) {
            break;
        }
    }
    shutdown(sock_fd, SHUT_RDWR);
    close(sock_fd);
    return 0;
}
