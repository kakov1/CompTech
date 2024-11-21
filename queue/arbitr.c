#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>

#define MAX_CLIENTS 100
#define MESSAGE_SIZE 256
#define HEARTBEAT_INTERVAL 10

struct client {
	int	   id;
	char   name[50];
	time_t last_heartbeat;
};

struct message {
	long mtype;
	char mtext[MESSAGE_SIZE];
};

int			  msqid;
struct client clients[MAX_CLIENTS];
int			  client_count = 0;

int get_client_id(const char* name) {
	for (int i = 0; i < client_count; i++) {
		if (strcmp(clients[i].name, name) == 0) {
			return clients[i].id;
		}
	}

	return -1;
}

int register_client(const char* name) {
	if (client_count >= MAX_CLIENTS) {
		return -1;
	}

	int client_id = get_client_id(name);

	if (client_id != -1) {
		return client_id;
	}

	int id = client_count + 5;
	clients[client_count].id = id;
	strncpy(clients[client_count].name, name,
			sizeof(clients[client_count].name) - 1);
	clients[client_count].last_heartbeat = time(NULL);
	client_count++;

	printf("%i\n", id);

	return id;
}

void remove_inactive_clients() {
	time_t now = time(NULL);
	for (int i = 0; i < client_count;) {
		if (difftime(now, clients[i].last_heartbeat) > HEARTBEAT_INTERVAL) {
			printf("Removing inactive client: %s\n", clients[i].name);
			clients[i] = clients[client_count - 1];
			client_count--;
		}
		else {
			i++;
		}
	}
}

void process_messages() {
	struct message msg;
	while (1) {
		if (msgrcv(msqid, &msg, MESSAGE_SIZE, 0, IPC_NOWAIT) < 0) {
			break;
		}

		if (msg.mtype == 1) {
			char name[50];
			sscanf(msg.mtext, "%s", name);
			int id = register_client(name);

			struct message ans = {.mtype = 1};
			sprintf(ans.mtext, "%d", id);

			if (msgsnd(msqid, &ans, strlen(ans.mtext) + 1, 0) < 0) {
				perror("Failed to register client");
				exit(1);
			}
		}
		else if (msg.mtype == 2) {
			char* target = strtok(msg.mtext, "#");
			char* message = strtok(NULL, "#");
			int target_id = get_client_id(target);

			if (target_id > 0) {
				struct message out_msg = {.mtype = target_id};
				strncpy(out_msg.mtext, message, sizeof(out_msg.mtext) + 1);
				msgsnd(msqid, &out_msg, strlen(out_msg.mtext) + 1, 0);
				printf("Message sent to %s: %s\n", target, message);
			}
			else {
				printf("Target client not found: %s\n", target);
			}
		}
		else if (msg.mtype == 3) {
			int id = atoi(msg.mtext);
			for (int i = 0; i < client_count; i++) {
				if (clients[i].id == id) {
					clients[i].last_heartbeat = time(NULL);
					printf("Heartbeat received from client ID: %d\n", id);
					break;
				}
			}
		}
		else if (msg.mtype == 4) {
			int id = atoi(msg.mtext);
			for (int i = 0; i < client_count; i++) {
				if (clients[i].id == id) {
					printf("Client exited: %s\n", clients[i].name);
					clients[i] = clients[client_count - 1];
					client_count--;
					break;
				}
			}
		}
	}
}

void cleanup(int sig) {
	msgctl(msqid, IPC_RMID, NULL);
	printf("Queue removed. Exiting.\n");
	exit(0);
}

int main() {
	signal(SIGINT, cleanup);

	key_t key = ftok("key.txt", 0);
	msqid = msgget(key, 0666 | IPC_CREAT);
	if (msqid < 0) {
		perror("msgget");
		exit(1);
	}

	printf("Arbitrator started.\n");
	while (1) {
		process_messages();
		remove_inactive_clients();
	}

	return 0;
}
