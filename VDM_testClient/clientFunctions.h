/*
 * clientFunctions.h
 *
 *  Created on: Apr 11, 2015
 *      Author: keinsword
 */

#ifndef CLIENTFUNCTIONS_H_
#define CLIENTFUNCTIONS_H_

int fdSetBlocking(int fd, int blocking);

int createClientSocket(const char *address, const char *port, const char *transport);

#endif /* CLIENTFUNCTIONS_H_ */
