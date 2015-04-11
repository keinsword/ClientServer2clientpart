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
#include "clientFunctions.h"

extern errno;

void exchangeLoopTCP(connection *conn, int sockFD);

void exchangeLoopUDP(connection *conn, int sockFD, const char *address, const char *port);

int main(int argc, char *argv[]) {
	connection conn;										//структура, описывающая данные клиента и параметры соединения

	int sockFD;												//файловый дескриптор сокета клиента

    char tempBuffer[MSGSIZE];								//временный буфер для различных нужд
    //char buffer1[BUFFERSIZE];

	//инициализируем используемые строки и структуры нулями
	memset(&conn, 0, sizeof(conn));
	memset(&tempBuffer, 0, sizeof(tempBuffer));
	//memset(&buffer1, 0, sizeof(buffer1));

    //проверяем, получено ли необходимое количество аргументов
    if (argc == 4) {
        //вызываем функцию создания сокета и подключения к хосту
    	sockFD = createClientSocket(argv[1], argv[2], argv[3]);
    	//проверяем результат
        if(sockFD < 0) {
        	printf("Ошибка подключения к %s:%s!\n", argv[1], argv[2]);
        	return 0;
        }
        else if(strcmp(argv[3], "tcp") == 0)
        	printf("Вы подключены к %s:%s по протоколу TCP.\n\n", argv[1], argv[2]);
        else
        	printf("Программа работает по протоколу UDP.\n\n");

        //цикл получения никнейма и имени сервиса
        while(1) {
        	//создаем и обнуляем временный буфер
        	char cutBuffer[MSGSIZE];
        	memset(&cutBuffer, 0, sizeof(cutBuffer));

        	printf("Введите Ваш ник (от 4 до 15 символов): ");
        	fgets(tempBuffer, sizeof(tempBuffer), stdin);
        	//избавляемся от \n, который fgets помещает в конец строки
        	strncpy(cutBuffer, tempBuffer, strlen(tempBuffer)-1);
        	memset(&tempBuffer, 0, sizeof(tempBuffer));
        	//ник не должен быть короче 4 и длиннее 15 символов
        	if((strlen(cutBuffer) >= 4) && (strlen(cutBuffer) <=15)) {
        		//сохраняем ник
        		strncpy(conn.clientNickName, cutBuffer, strlen(cutBuffer));
        		memset(&cutBuffer, 0, sizeof(cutBuffer));
				printf("Введите имя сервиса: ");
				fgets(tempBuffer, sizeof(tempBuffer), stdin);
				//избавляемся от \n, который fgets помещает в конец строки
				strncpy(cutBuffer, tempBuffer, strlen(tempBuffer)-1);
				memset(&tempBuffer, 0, sizeof(tempBuffer));
				//сохраняем имя сервиса
				strncpy(conn.serviceName, cutBuffer, strlen(cutBuffer));
				memset(&cutBuffer, 0, sizeof(cutBuffer));
				break;
        	}
        	memset(&cutBuffer, 0, sizeof(cutBuffer));
        }

        if(strcmp(argv[3], "tcp") == 0)
        	exchangeLoopTCP(&conn, sockFD);
        else
        	exchangeLoopUDP(&conn, sockFD, argv[1], argv[2]);

        // закрываем файловый дескриптор сокета
    	close(sockFD);
    }
    else
    	//если введено неверное количество аргументов, выводим правильный формат запуска программы
    	printf("Использование: %s address port transport\n", argv[0]);

    return 0;
}

void exchangeLoopTCP(connection *conn, int sockFD) {
	char exitpr[] = "exitpr";								//строковая константа, по которой осуществляется выход из программы

    char buffer[BUFFERSIZE];								//буфер, принимающий ответ от сервера
    char tempBuffer[MSGSIZE];								//временный буфер для различных нужд

	//инициализируем используемые строки и структуры нулями
	memset(&buffer, 0, sizeof(buffer));
	memset(&tempBuffer, 0, sizeof(tempBuffer));

    //цикл обмена данных с сервером
    while(1) {
		int n;
		printf("Введите текст сообщения: ");
		fgets(tempBuffer, sizeof(tempBuffer), stdin);
		//избавляемся от \n, который fgets помещает в конец строки
		strncpy(conn->messageText, tempBuffer, strlen(tempBuffer)-1);
		//проверяем, не ввел ли клиент команду завершения программы
		if (strcmp(conn->messageText, exitpr) == 0) {
			printf("Клиент закрывается.\n\n");
			break;
		}

		strcpy(conn->protoName, PROTO_NAME);		//сохраняем имя нашего протокола
		strcpy(conn->protoVersion, PROTO_VER);	//сохраняем версию нашего протокола

		Serializer(conn, buffer);				//преобразуем структуру в единую строку

		//отправляем данные серверу и проверяем результат

		/*strncpy(buffer1, buffer, 20);
		n = write(sockFD, buffer1, strlen(buffer1));
		memset(&buffer1, 0, sizeof(buffer1));
		int i, j;
		for(i = 20, j = 0; i < 30; i++, j++)
			buffer1[j]=buffer[i];
		sleep(5);
		n = write(sockFD, buffer1, strlen(buffer1));
		memset(&buffer1, 0, sizeof(buffer1));
		for(i = 30, j = 0; i < strlen(buffer); i++, j++)
			buffer1[j]=buffer[i];
		sleep(5);
		n = write(sockFD, buffer1, strlen(buffer1));
		memset(&buffer1, 0, sizeof(buffer1));*/

		n = write(sockFD, buffer, strlen(buffer));
		if (n < 0)
			 printf("Ошибка при записи данных в сокет: %s.\n", strerror(errno));
		else
			printf("Серверу отправлено: %s\n", conn->messageText);
		memset(&buffer, 0, sizeof(buffer));

		//к этому моменту на стороне сервера наш сокет уже сделан неблокирующим, и, чтобы дождаться ответа
		//от сервера, нужно вернуть его в блокирующий режим
		fdSetBlocking(sockFD, 1);

		//чтение данных из сокета с проверкой результата
		n = read(sockFD, buffer, sizeof(buffer));
		if (n < 0)
			printf("Ошибка при чтении данных из сокета: %s.\n", strerror(errno));
		else
			printf("Ответ сервера: %s\n\n", buffer);

		//возвращаем сокет в блокирующий режим для корректной работы сервера
		fdSetBlocking(sockFD, 0);

		//проверяем, не прислал ли нам сервер сигнал о том, что на нем нет места для подключения
		if(strncmp(connStructOverflowNotification, buffer, strlen(buffer)+1) == 0) {
			printf("Клиент закрывается.\n\n");
			break;
		}

		if(strcmp(wrongSrvNotification, buffer) == 0)
			break;

		//обнуляем только ту часть структуры, которая содержит данные сообщения
		memset(&conn->messageText, 0, sizeof(conn->messageText));
		memset(&buffer, 0, sizeof(buffer));
		memset(&tempBuffer, 0, sizeof(tempBuffer));
	}
}

void exchangeLoopUDP(connection *conn, int sockFD, const char *address, const char *port) {
	struct sockaddr_in serverAddr;

	char exitpr[] = "exitpr";								//строковая константа, по которой осуществляется выход из программы

    char buffer[BUFFERSIZE];								//буфер, принимающий ответ от сервера
    char tempBuffer[MSGSIZE];								//временный буфер для различных нужд

	//инициализируем используемые строки и структуры нулями
	memset(&buffer, 0, sizeof(buffer));
	memset(&tempBuffer, 0, sizeof(tempBuffer));
	memset(&serverAddr, 0, sizeof(serverAddr));

    int portNum = atoi(port);
    serverAddr.sin_port = htons(portNum);
	serverAddr.sin_family = AF_INET;
	inet_pton(AF_INET, address, &serverAddr.sin_addr);
	socklen_t serverAddrSize = sizeof(serverAddr);

    //цикл обмена данных с сервером
    while(1) {
		int n;
		printf("Введите текст сообщения: ");
		fgets(tempBuffer, sizeof(tempBuffer), stdin);
		//избавляемся от \n, который fgets помещает в конец строки
		strncpy(conn->messageText, tempBuffer, strlen(tempBuffer)-1);
		//проверяем, не ввел ли клиент команду завершения программы
		if (strcmp(conn->messageText, exitpr) == 0) {
			printf("Клиент закрывается.\n\n");
			break;
		}

		strcpy(conn->protoName, PROTO_NAME);		//сохраняем имя нашего протокола
		strcpy(conn->protoVersion, PROTO_VER);	//сохраняем версию нашего протокола

		Serializer(conn, buffer);				//преобразуем структуру в единую строку

		//проверяем, превышает ли длина получившейся строки размер MTU
		int size = (int)strlen(buffer);
		if(size > (int)MTU) {
			//вызов функции сегментирования строки с проверкой результата
    		if(Divider(sockFD, buffer, &serverAddr, serverAddrSize) > 0)
    			printf("Серверу отправлено: %s\n", conn->messageText);
    		memset(&buffer, 0, sizeof(buffer));
		}
		else {
			n = sendto(sockFD, buffer, strlen(buffer), 0, &serverAddr, sizeof(serverAddr));
			if (n < 0)
				 printf("Ошибка при записи данных в сокет: %s.\n", strerror(errno));
			else
				printf("Серверу отправлено: %s\n", conn->messageText);
			memset(&buffer, 0, sizeof(buffer));

			//к этому моменту на стороне сервера наш сокет уже сделан неблокирующим, и, чтобы дождаться ответа
			//от сервера, нужно вернуть его в блокирующий режим
			n = fdSetBlocking(sockFD, 1);

			n = recvfrom(sockFD, buffer, sizeof(buffer), 0, (struct sockaddr *)&serverAddr, &serverAddrSize);
			if (n < 0)
				printf("Ошибка при чтении данных из сокета: %s.\n", strerror(errno));
			else
				printf("Ответ сервера: %s\n\n", buffer);

			//возвращаем сокет в блокирующий режим для корректной работы сервера
		//	fd_set_blocking(sockFD, 0);

			//проверяем, не прислал ли нам сервер сигнал о том, что на нем нет места для подключения
			if(strncmp(connStructOverflowNotification, buffer, strlen(buffer)+1) == 0) {
				printf("Клиент закрывается.\n\n");
				break;
			}

			if(strcmp(wrongSrvNotification, buffer) == 0)
				break;

			//обнуляем только ту часть структуры, которая содержит данные сообщения
			memset(&conn->messageText, 0, sizeof(conn->messageText));
			memset(&buffer, 0, sizeof(buffer));
			memset(&tempBuffer, 0, sizeof(tempBuffer));
		}
    }
}
