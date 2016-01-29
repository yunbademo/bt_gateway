#include "yunba.h"
#include "cJSON.h"
//#include "MQTTClient.h"
#include "MQTTClientPersistence.h"

#include <stdio.h>
#include <signal.h>
#include <memory.h>

#if defined(WIN32)
#include <Windows.h>
#define sleep Sleep
#else
#include <time.h>
#include <stdlib.h>
#endif

#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>

REG_info my_reg_info;
int bt_fd;

volatile int toStop = 0;

void usage() {
	printf("Usage: bt_gateway topicname <options>, where options are:\n");
	printf("  --appkey <your-appkey>\n");
	printf("  --deviceid <your-device-id>\n");
	printf("  --alias <your-alias>\n");
	printf("  --bt_dev <your-bluetooth-device>\n");
	exit(-1);
}

void myconnect(MQTTClient* client, MQTTClient_connectOptions* opts) {
	printf("Connecting\n");
	if (MQTTClient_connect(*client, opts) != 0) {
		printf("Failed to connect\n");
		exit(-1);
	}
}

void cfinish(int sig) {
	signal(SIGINT, NULL);
	toStop = 1;
}

struct {
	char* delimiter;
	int maxdatalen;
	int qos;
	int retained;
	char *appkey;
	char *deviceid;
	char *alias;
	char *bt_dev;
	int verbose;
} opts = { "\n", 100, 0, 0, NULL, NULL, NULL, NULL, 0};

Presence_msg my_present;
MQTTClient client;
MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;

void getopts(int argc, char** argv);

int extendedCmdArrive(void *context, EXTED_CMD cmd, int status,
		int ret_string_len, char *ret_string) {
	char buf[1024];
	memset(buf, 0, 1024);
	memcpy(buf, ret_string, ret_string_len);
	printf("%s:%02x,%02x,%02x, %s\n", __func__, cmd, status, ret_string_len,
			buf);

}

int messageArrived(void* context, char* topicName, int topicLen,
		MQTTClient_message* m) {
	char action[30];
	char alias[60];
	int ret = -1;
	int i;
	char* payloadptr;

	my_present.action = action;
	my_present.alias = alias;
	ret = get_present_info(topicName, m, &my_present);
	if (ret == 0)
		printf("action:%s alias:%s\n", my_present.action, my_present.alias);
	time_t t;
	time(&t);
	printf("Message arrived, date:%s", ctime(&t));
	printf(" qos: %i\n", m->qos);
	printf(" messageid: %"PRIu64"\n", m->msgid);
	printf(" topic: %s\n", topicName);
	printf(" message: ");

	payloadptr = m->payload;
	for (i = 0; i < m->payloadlen; i++) {
		putchar(*payloadptr++);
	}
	putchar('\n');

	char *temp = (char *)calloc(m->payloadlen + 1, sizeof(char));
	memcpy(temp, m->payload, m->payloadlen);
	cJSON *root = cJSON_Parse(temp);
	if (root) {
		int ret_size = cJSON_GetArraySize(root);
		if (ret_size == 3) {
			char devid[30];
			char statu[10];
			char cmd[20];
			//{"cmd": "LED_set", "devid": "<smart-plug-device-id>", "status": <status>}
			strcpy(devid, cJSON_GetObjectItem(root, "devid")->valuestring);
			strcpy(statu, cJSON_GetObjectItem(root, "status")->valuestring);
			strcpy(cmd, cJSON_GetObjectItem(root, "cmd")->valuestring);
			printf("bt_gw:%s, %s, %s, %i\n", devid, statu, cmd, bt_fd);
			write(bt_fd, statu, sizeof(statu));
		}
		cJSON_Delete(root);
	}

	MQTTClient_freeMessage(&m);
	MQTTClient_free(topicName);
	free(temp);
	/* not expecting any messages */
	return 1;
}

void connectionLost(void *context, char *cause) {
	myconnect(&client, &conn_opts);
	printf("%s, %s, %s\r\n", __func__, context, cause);

}

int open_port(const char* bt_dev) {
	int fd = open(bt_dev, O_RDWR | O_NOCTTY | O_NDELAY);
	if (fd == -1) {
		/* Could not open the port. */
		perror("open_port: Unable to open  - ");
	} else
		fcntl(fd, F_SETFL, 0);

	return (fd);
}

int main(int argc, char** argv) {
	char* topic = NULL;
	char* buffer = NULL;
	int rc = 0;
	char url[100];

	if (argc < 2)
		usage();

	getopts(argc, argv);

	topic = argv[1];
	printf("Using topic %s\n", topic);

	bt_fd = open_port(opts.bt_dev);

	if (bt_fd < 0) {
		printf("can't open bt device\n");
		return -1;
	}

	int res = MQTTClient_setup_with_appkey_and_deviceid_v2(opts.appkey,
			opts.deviceid, &my_reg_info);
	if (res < 0) {
		printf("can't get reg info\n");
		return 0;
	}

	printf("Get reg info: client_id:%s,username:%s,password:%s, devide_id:%s\n",
			my_reg_info.client_id, my_reg_info.username, my_reg_info.password,
			my_reg_info.device_id);

	res = MQTTClient_get_host_v2(opts.appkey, url);
	if (res < 0) {
		printf("can't get host info\n");
		return 0;
	}
	printf("Get url info: %s\n", url);

	rc = MQTTClient_create(&client, url, my_reg_info.client_id,
	MQTTCLIENT_PERSISTENCE_NONE, NULL);

	signal(SIGINT, cfinish);
	signal(SIGTERM, cfinish);

	rc = MQTTClient_setCallbacks(client, NULL, connectionLost, messageArrived,
	NULL, extendedCmdArrive);

	conn_opts.keepAliveInterval = 300;
	conn_opts.reliable = 0;
	conn_opts.cleansession = 1;
	conn_opts.username = my_reg_info.username;
	conn_opts.password = my_reg_info.password;

	myconnect(&client, &conn_opts);

	buffer = malloc(opts.maxdatalen);

	rc = MQTTClient_subscribe(client, topic, 1);
	printf("subscribe topic:%s, %i\n", topic, rc);

	if (opts.alias != NULL) {
		printf("set alias: %s\n", opts.alias);
		MQTTClient_set_alias(client, opts.alias);
	}

	while (!toStop) {
		int data_len = 0;
		int delim_len = 0;

		delim_len = strlen(opts.delimiter);
		do {
			buffer[data_len++] = getchar();
			if (data_len > delim_len) {
				//printf("comparing %s %s\n", opts.delimiter, &buffer[data_len - delim_len]);
				if (strncmp(opts.delimiter, &buffer[data_len - delim_len],
						delim_len) == 0)
					break;
			}
		} while (data_len < opts.maxdatalen);

		if (opts.verbose)
			printf("Publishing data of length %d\n", data_len);
		rc = MQTTClient_publish(client, topic, data_len, buffer);
		if (rc != 0) {
			myconnect(&client, &conn_opts);
			rc = MQTTClient_publish(client, topic, data_len, buffer);
			printf("reconnect %i\n", rc);
		}
		if (opts.qos > 0)
			MQTTClient_yield();
	}

	printf("Stopping\n");

	free(buffer);

	MQTTClient_disconnect(client, 0);

	MQTTClient_destroy(&client);

	close(bt_fd);

	return 0;
}

void getopts(int argc, char** argv) {
	int count = 2;

	while (count < argc) {
		if (strcmp(argv[count], "--appkey") == 0) {
			if (++count < argc)
				opts.appkey = argv[count];
			else
				usage();
		} else if (strcmp(argv[count], "--deviceid") == 0) {
			if (++count < argc)
				opts.deviceid = argv[count];
		} else if (strcmp(argv[count], "--alias") == 0) {
			if (++count < argc)
				opts.alias = argv[count];
			else
				usage();
		} else if (strcmp(argv[count], "--bt_dev") == 0) {
			if (++count < argc)
				opts.bt_dev = argv[count];
			else
				usage();
		}
		count++;
	}
}
