#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define MESSAGE_LENGTH 255

int main(int argc, char* argv[]) {
	if (argc != 3) {
		fprintf(stderr, "Usage: %s <receive_type> <send_type>\n", argv[0]);
		exit(1);
	}

	long receive_type = atol(argv[1]);
	long send_type = atol(argv[2]);

	if (receive_type <= 0 || send_type <= 0) {
		fprintf(stderr, "Message types must be positive integers.\n");
		exit(1);
	}

	int	  msqid;
	char  pathname[] = "key.txt";
	key_t key;
	int	  len, maxlen;

	struct mymsgbuf {
		long mtype;
		char mtext[MESSAGE_LENGTH];
	} mybuf;

	key = ftok(pathname, 0);

	if ((msqid = msgget(key, 0666 | IPC_CREAT)) < 0) {
		printf("Can\'t get msqid\n");
		exit(-1);
	}

	pid_t pid = fork();

	if (pid == -1) {
		printf("Error fork\n");
	}

	if (pid == 0) {
		while (1) {
			maxlen = 81;
			if ((len = msgrcv(msqid, (struct msgbuf*)&mybuf, maxlen, 0, 0)) <
				0) {
				printf("Can\'t receive message from queue\n");
				exit(-1);
			}

			if (mybuf.mtype == MESSAGE_LENGTH) {
				msgctl(msqid, IPC_RMID, (struct msqid_ds*)NULL);
				exit(0);
			}

			if (mybuf.mtype != send_type) {
				printf("%ld: %s", send_type, mybuf.mtext);
			}

			else {
				if (msgsnd(msqid, (struct msgbuf*)&mybuf, len, 0) < 0) {
					printf("Can\'t send message to queue\n");
					msgctl(msqid, IPC_RMID, (struct msqid_ds*)NULL);
					exit(-1);
				}
			}
		}
	}

	else {
		while (1) {
			char buffer[256];

			if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
				printf("Incorrect string.\n");
			}

			mybuf.mtype = send_type;
			strcpy(mybuf.mtext, buffer);
			len = strlen(mybuf.mtext) + 1;

			if (msgsnd(msqid, (struct msgbuf*)&mybuf, len, 0) < 0) {
				printf("Can\'t send message to queue\n");
				msgctl(msqid, IPC_RMID, (struct msqid_ds*)NULL);
				exit(-1);
			}
		}
	}
}
