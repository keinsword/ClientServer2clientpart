/*
 * clientFunctions.c
 *
 *  Created on: Apr 11, 2015
 *      Author: keinsword
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>
#include <arpa/inet.h>
#include <math.h>
#include <fcntl.h>
#include "crc.h"
#include "protocol.h"

//реализация функции установки блокирующего/неблокирующего режима работы с файловым дескриптором
//аргументы:
//fd - файловый дескриптор сокета
//blocking - режим работы (0 - неблокирующий, 1 - блокирующий)
int fdSetBlocking(int fd, int blocking) {
    //сохраняем текущие флаги
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1)
        return 0;

    //устанавливаем режим работы
    if (blocking)
        flags &= ~O_NONBLOCK;
    else
        flags |= O_NONBLOCK;
    return fcntl(fd, F_SETFL, flags) != -1;
}

//реализация функции создания сокета и подключения к хосту
//аргументы:
//address - адрес хоста
//port - порт хоста
//transport - имя транспортного протокола
int createClientSocket(const char *address, const char *port, const char *transport) {
	int sockFD;								//файловый дескриптор сокета
	int portNum;							//номер порта в целочисленном формате
	int type, proto;						//тип транспортного протокола

	struct sockaddr_in serverAddr;			//структура, содержащая информацию об адресе

	memset(&serverAddr, 0, sizeof(serverAddr));

	//используем имя протокола для определения типа сокета
	if(strcmp(transport, "udp") == 0) {
		type = SOCK_DGRAM;
		proto = IPPROTO_UDP;
	}
	else if(strcmp(transport, "tcp") == 0) {
		type = SOCK_STREAM;
		proto = IPPROTO_TCP;
	}
	else {
		printf("Неверное имя транспротного протокола: %s.\n", strerror(errno));
		return -1;
	}

	//вызываем функцию создания сокета с проверкой результата
	sockFD = socket(PF_INET, type, proto);
	if (sockFD < 0) {
		printf("Ошибка создания сокета: %s.\n", strerror(errno));
		return -1;
	}

	portNum = atoi(port);					//преобразовываем номер порта из строкового формата в целочисленный
	serverAddr.sin_port = htons(portNum);	//конвертируем номер порта из пользовательского порядка байт в сетевой
	serverAddr.sin_family = AF_INET;		//указываем тип адреса

	//конвертируем адрес в бинарный формат
	inet_pton(AF_INET, address, &serverAddr.sin_addr);

	if(type == SOCK_STREAM)
		//вызываем функцию подключения к хосту с проверкой результата
		if(connect(sockFD, (struct sockaddr *) &serverAddr, sizeof(serverAddr)) < 0) {
			printf("Ошибка подключения к %s: %s (%s)!\n", address, port, strerror(errno));
			return -1;
		}

	return sockFD;							//возвращаем файловый дескриптор созданного сокета
}


