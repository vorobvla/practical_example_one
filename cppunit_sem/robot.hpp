/* 
 * File:   robot.hpp
 * Author: vorobvla
 *
 * Created on May 19, 2014, 6:49 PM
 * I need this header file mostly to implement unit tests
 */

#ifndef ROBOT_HPP
#define	ROBOT_HPP

#include <cstdlib>
#include <unistd.h>
#include <string.h>
#include <iostream>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <exception>
#include <stdio.h>
#include <sys/wait.h>
#include <semaphore.h>
#include <sys/shm.h>

#define M_200 "200 LOGIN\r\n"
#define M_201 "201 PASSWORD\r\n"
#define M_202 "202 OK\r\n"
#define M_300 "300 BAD CHECKSUM\r\n"
#define M_500 "500 LOGIN FAILED\r\n"
#define M_501 "501 SYNTAX ERROR\r\n"
#define M_502 "502 TIMEOUT\r\n"
#define BUFFSIZE 1000
#define LOGFILE "log"

#define RECV_FLAGS 0
#define TIMEOUT 45

#define PARALLEL

#define STRTOI(x) atoi(x.c_str())


using namespace std;

//structure to process the communication with client during connection
extern struct client {
    int sc_fd;
    string data;
    string buffer;
} cli;

extern struct shared_data {
    int f_cnt;
    sem_t log_mutex;
    sem_t f_cnt_mutex;
}*p_shared_data;

//command enumeration
enum command {
    unknown, none, info, foto
};

#endif	/* ROBOT_HPP */

//needed for an easier unit tests implementation;
client * get_cli(void);

//sum of ascii values of the symbols contained in the 'str' attribute
unsigned int asciisum(string str);

//check whether attribute 'str' contains only numbers
bool str_isdigit(string str);

//login & passwd check
bool lpcontrol(string login, string passwd);

//append attribute str to log file LOGFILE
void write_to_log(string str);

//save attribute str to file
void save_file(string str);

//send the message 'msg' to client with socket cli.sc_fd
void send_message(char* msg);

//receive the appropriate data to string cli.data
void recv_message();

//check whether cli.buffer starts with a valid command
bool check_buffer_command();

//recognize the command in the beginning of cli.buffer
command recv_command() ;

//receive and process a photo 
void recv_foto();

//correct finishing of a child process 
void reaper(int signum);

//actions when the connection is timeouted
void sig_handler(int signum);