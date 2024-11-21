#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>

#define MESSAGE_SIZE 256
#define HEARTBEAT_INTERVAL 10

struct message {
    long mtype;
    char mtext[MESSAGE_SIZE];
};

int msqid;
int client_id = -1;
char client_name[50];
int running = 1;

void* heartbeat_thread(void* arg) {
    while (running) {
        if (client_id > 0) {
            struct message msg = {.mtype = 3};
            snprintf(msg.mtext, sizeof(msg.mtext), "%d", client_id);
            if (msgsnd(msqid, &msg, strlen(msg.mtext) + 1, 0) < 0) {
                perror("Heartbeat failed");
            }
        }
        sleep(HEARTBEAT_INTERVAL);
    }
    return NULL;
}

void* receiver_thread(void* arg) {
    struct message msg;
    while (running) {
        if (msgrcv(msqid, &msg, MESSAGE_SIZE, client_id, IPC_NOWAIT) > 0) {
            printf("Message received: %s\n", msg.mtext);
        }
    }
    return NULL;
}

void cleanup(int sig) {
    running = 0;

    if (client_id > 0) {
        struct message msg = {.mtype = 4};
        snprintf(msg.mtext, sizeof(msg.mtext), "%d", client_id);
        msgsnd(msqid, &msg, strlen(msg.mtext) + 1, 0);
    }

    printf("\nExiting...\n");
    exit(0);
}

void register_client() {
    struct message msg;
    msg.mtype = 1;

    snprintf(msg.mtext, sizeof(msg.mtext), "%s", client_name);
    if (msgsnd(msqid, &msg, strlen(msg.mtext) + 1, 0) < 0) {
        perror("Failed to register client");
        exit(1);
    }

    struct message response;
    if (msgrcv(msqid, &response, MESSAGE_SIZE, 1, 0) < 0) {
        perror("Failed to receive client ID");
        exit(1);
    }

    client_id = atoi(response.mtext);
    printf("Registered as %s with ID: %d\n", client_name, client_id);
}

void send_message(const char* target_name, const char* message) {
    struct message msg = {.mtype = 2};
    snprintf(msg.mtext, sizeof(msg.mtext), "%s#%s", target_name, message);
    if (msgsnd(msqid, &msg, strlen(msg.mtext) + 1, 0) < 0) {
        perror("Failed to send message");
    }
}

int main() {
    signal(SIGINT, cleanup);

    printf("Enter your name: ");
    scanf("%49s", client_name);

    key_t key = ftok("key.txt", 0);
    msqid = msgget(key, 0666);
    if (msqid < 0) {
        perror("Failed to connect to message queue");
        exit(1);
    }

    register_client();

    pthread_t heartbeat_tid, receiver_tid;
    pthread_create(&heartbeat_tid, NULL, heartbeat_thread, NULL);
    pthread_create(&receiver_tid, NULL, receiver_thread, NULL);

    char input[MESSAGE_SIZE];
    while (running) {
        printf("Enter message (<Name>#<Message> or ## to exit): ");
        scanf(" %[^\n]", input);

        if (strcmp(input, "##") == 0) {
            cleanup(0);
        }

        char* target_name = strtok(input, "#");
        char* message = strtok(NULL, "#");

        if (target_name && message) {
            send_message(target_name, message);
        } else {
            printf("Invalid format. Use <Name>#<Message>\n");
        }
    }

    return 0;
}
