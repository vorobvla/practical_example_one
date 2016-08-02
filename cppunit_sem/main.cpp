/* 
 * File:   main.cpp
 * Author: vorobvla
 *
 * Created on February 26, 2014, 11:15 AM
 */


#include "robot.hpp"

//#define DEBUG
#ifdef DEBUG
#define DBG(x) cout << x << " (" << __FILE__ << ":" << __LINE__ << ")" << endl
#else
#define DBG(x)
#endif

client cli;
shared_data *p_shared_data;

//sum of ascii values of the symbols contained in the 'str' attribute
unsigned int asciisum(string str) {
    unsigned int sum = 0;
    for (int i = 0, size = str.size(); i < size; i++) {
        sum += (unsigned char) str[i];
    };
    return sum;
}

//check whether attribute 'str' contains only numbers
bool str_isdigit(string str) {
    for (int i = 0, size = str.size(); i < size; i++) {
        if (!isdigit(str[i])) {
            return false;
        }
    }
    return true;
}

//login & passwd check
bool lpcontrol(string login, string passwd) {

    DBG(login << " / " << passwd);

    if (login.find("Robot") != 0) {
        return false;
    }

    if (!str_isdigit(passwd)) {
        return false;
    }

    if (asciisum(login) != STRTOI(passwd)) {
        return false;
    }

    return true;
}

//append attribute str to log file LOGFILE
void write_to_log(string str) {
    FILE* log;
    
    //critical section starts
//    sem_wait(&(p_shared_data->log_mutex));
    log = fopen(LOGFILE, "a");

    if (!log) {
//       sem_post(&(p_shared_data->log_mutex));
        throw 1000;
        //error while opening file, cs ends
    }

    if (!fprintf(log, "%s\n", str.c_str())) {
 //       sem_post(&(p_shared_data->log_mutex));
        throw 1000;
        //error while writing to file, cs ends
    }

    if (fclose(log) != 0) {
  //      sem_post(&(p_shared_data->log_mutex));
        throw 1000;
        //error while closing file, cs ends
    }
  //  sem_post(&(p_shared_data->log_mutex));
    //critical section ends
}

//save attribute str to file
void save_file(string str) {
    FILE* file;
    char name[20];
    int i;

    //critical section starts
    sem_wait(&(p_shared_data->f_cnt_mutex));
    if (p_shared_data->f_cnt == 1000) {
        p_shared_data->f_cnt = 0;
    }
    sprintf(name, "file%d.png", (p_shared_data->f_cnt)++);//safetly increment f_cnt
    sem_post(&(p_shared_data->f_cnt_mutex));
    //critical section ends

    file = fopen(name, "w");
    if (!file) {
        throw 2000;
    }
    for (i = 0; i < str.size(); i++) {
        if (!fprintf(file, "%c", str[i])) {
            throw 2000;
        }
    }
    if (fclose(file) != 0) {
        throw 2000;
    }
}

//send the message 'msg' to client with socket cli.sc_fd
void send_message(char* msg) {
    int len;
    len = send(cli.sc_fd, msg, strlen(msg), 0);
    if (len == -1) {
        throw 105;
    }
}

//receive the appropriate data to string cli.data
void recv_message() {
    int l, d;
    char buffer[BUFFSIZE];

    cli.data = "";

    while (1) {
        //if '\r\n' in cli.buffer found, copy the data before it to cli.data and
        //remove this data from cli.buffer
        DBG("cli.buffer:" << cli.buffer);
        if ((d = cli.buffer.find("\r\n")) != string::npos) {
            cli.data = cli.buffer.substr(0, d);
            cli.buffer.erase(0, d + 2);
            DBG(cli.data << endl << cli.buffer);
            break;
        } else {
            //otherwise receive more data form client and append it to cli.buffer
            l = recv(cli.sc_fd, buffer, BUFFSIZE, RECV_FLAGS);
            if (l <= 0) {
                perror("recv");
                throw 1106;
            }
            cli.buffer.append(buffer, l);
        }
    }

}

//check whether cli.buffer starts with a valid command
bool check_buffer_command() {

    const char* commands[2] = {"FOTO ", "INFO "};

    bool res = false;

    for (int i = 0; i < 2; i++) {
        int l = min(5, (int) cli.buffer.size());
        if (!cli.buffer.compare(0, l, commands[i], l)) {
            return true;
        }
    }

    return res;
}

//recognize the command in the beginning of cli.buffer
command recv_command() {
    command res;
    char buffer[BUFFSIZE];
    int l;

    res = unknown;
    
    //check whether the cli.buffer is not empty and starts with a valid command
    if ((cli.buffer.size() > 0) && (!check_buffer_command())) {
        return res;
    }

    //receive more data if needed
    while (cli.buffer.size() < 5) {
        l = recv(cli.sc_fd, buffer, BUFFSIZE, RECV_FLAGS);
        if (l < 0) {
            perror("recv");
            return res;
        } else if (l == 0) {
            res = none;
            return res;
        }
        cli.buffer.append(buffer, l);
        //check if the command valid
        if (!check_buffer_command()) {
            DBG("invalid command, buffer: " << cli.buffer);
            return res;
        }
    }
    //recognize the command
    if (cli.buffer.substr(0, 5) == "INFO ") {
        res = info;
    } else if (cli.buffer.substr(0, 5) == "FOTO ") {
        res = foto;
    }
    
    //remove redundant data
    cli.buffer.erase(0, 5);

    return res;
}

//receive and process a photo 
void recv_foto() {
    string data = "";
    string size;
    int foto_size, l, ctrl_sum;
    char buffer[BUFFSIZE];

    //receive, parse and control size of photo
    //receive data until ' ' not received
    while (cli.buffer.find(' ') == string::npos) {
        if (!str_isdigit(cli.buffer)) {
            throw 600;  //control if the data received are numeric
        }
        l = recv(cli.sc_fd, buffer, BUFFSIZE, RECV_FLAGS);
        if (l <= 0) {
            perror("recv");
            throw 9002;
        }
        cli.buffer.append(buffer, l);
    }
    //parse the size of photo
    l = cli.buffer.find(' ');
    size = cli.buffer.substr(0, l);
    cli.buffer.erase(0, l + 1);
    //check the size  
    if (!str_isdigit(size)) {
        throw 600;
    }
    //translate the size to int
    foto_size = STRTOI(size);

    if (foto_size < 1) {
        throw 600;
    }

    DBG("foto size: " << foto_size);
    DBG("buffer: '" << cli.buffer << "'");
    DBG("buffer len: " << cli.buffer.size());

    //receive the photo and 4 bytes of control sum
    while (cli.buffer.size() < foto_size + 4) {
        l = recv(cli.sc_fd, buffer, BUFFSIZE, RECV_FLAGS);
        if (l <= 0) {
            perror("recv");
            throw 9001;
        }
        cli.buffer.append(buffer, l);
    }
    //extract the processed data from cli.buffer
    data = cli.buffer.substr(0, foto_size + 4);
    cli.buffer.erase(0, foto_size + 4);
    
    //count the control sum
    ctrl_sum = (unsigned char) data[foto_size + 3] +
            (unsigned char) data[foto_size + 2] * 0x100 +
            (unsigned char) data[foto_size + 1] * 0x10000 +
            (unsigned char) data[foto_size] * 0x1000000;
    //remove the control sum from data 
    data.erase(foto_size, data.size() - foto_size);
    //check the control sum
    if (asciisum(data) != ctrl_sum) {
        throw 700;
    }
    //save the data to file
    save_file(data);
}

//correct finishing of a child process 
void reaper(int signum) {
    int status;
    while (waitpid(-1, &status, 0) > 0);
    signal(SIGCHLD, reaper);
}

//actions when the connection is timeouted
void sig_handler(int signum) {
    DBG("signal " << signum << " caught");
    if (signum == SIGALRM) {
        DBG("<< 502");
        send_message((char*) M_502);
        close(cli.sc_fd);
    }
    exit(signum);
}

int main(int argc, char** argv) {

    int sc_fd, shm_id, shm_key;
    struct sockaddr_in my_addr, rem_addr;
    unsigned int rem_addr_length;
    int yes = 1;
    string login, password;
    short port;
    
    
    //processing signals
    if ((signal(SIGCHLD, reaper) == SIG_ERR) || (signal(SIGINT, sig_handler) == SIG_ERR)) {
        perror("signal");
        exit(1);
    }

    try {
        //control and process arguments
        if (argc != 2) {
            port = 3000;
        } else {
            port = atoi(argv[1]);
        }

        if ((port > 3999) || (port < 3000)) {
            throw 2;
        }

        //create a server socket
        if ((sc_fd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
            throw 3;
        }
        //prepare attributes for bind()
        bzero(&my_addr, sizeof (my_addr));
        my_addr.sin_family = AF_INET;
        my_addr.sin_port = htons(port);
        //prepare attributes for accept()
        bzero(&rem_addr, sizeof (rem_addr));
        rem_addr_length = sizeof (rem_addr);
        //set option 'reuseaddr' on the server socket
        if (setsockopt(sc_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof (int)) == -1) {
            throw 100;
        }
        //bind the server socket and the port
        if (bind(sc_fd, (struct sockaddr *) &my_addr, sizeof (my_addr)) == -1) {
            throw 101;
        }
        //set the length of the client queue
        if (listen(sc_fd, 5) == -1) {
            throw 102;
        }

        //prepare and attach shared memory
        shm_key = getpid();

        if ((shm_id = shmget(shm_key, sizeof (shared_data), IPC_CREAT | 0600)) < 0) {
            perror("shmget");
            throw 103;
        }
        if ((p_shared_data = (shared_data*) shmat(shm_id, NULL, 0)) == (void *) - 1) {
            perror("shmat");
            throw 104;
        }
        //allow the OS to cleanup the shared mem if it is not attached
        if (shmctl(shm_id, IPC_RMID, NULL) < 0) {
            perror("shmctl");
            throw 115;
        }
        //initialize shared variable needed for correct filename generating
        p_shared_data->f_cnt = 0;
        //initialize shared mutex needed for correct log appending
        if (sem_init(&(p_shared_data->log_mutex), 1, 1) < 0) {
            perror("sem_init");
            return 1;
        }
        //initialize shared mutex needed for correct f_cnt update
        if (sem_init(&(p_shared_data->f_cnt_mutex), 1, 1) < 0) {
            perror("sem_init");
            return 1;
        }

        while (1) {
            command cmd;
            int cli_sc;
            //wait until a client connects
            //when it happened get the name of socket that is used for communication with client
            if ((cli_sc = accept(sc_fd, (struct sockaddr *) &rem_addr, &rem_addr_length)) == -1) {
                throw 106;
            }

#ifdef PARALLEL
            // when connection with a client started, create a child
            switch (fork()) {
                case 0:
                    DBG("* connection started");
//                    write_to_log("connection est");
                    //in child process
                    //close server socket
                    close(sc_fd);
                    //process a SIGALRM signal
                    if (signal(SIGALRM, sig_handler) == SIG_ERR) {
                        perror("signal");
                        exit(1);
                    }
                    //system will send ALRM signal after timeout
                    alarm(TIMEOUT);
#endif              //put the client socket number to a global structure    
                    cli.sc_fd = cli_sc;
                    //start communication to clientchild
                    try {
                        cli.buffer = "";
                        send_message((char*) M_200); // message U
                        DBG("<< 200");
                        recv_message();
                        DBG(">> " << cli.data);

                        login = cli.data;

                        send_message((char*) M_201); // message P
                        DBG("<< 201");
                        recv_message();
                        DBG(">> " << cli.data);
                    } catch (...) {
                        close(cli.sc_fd);
                        continue;
                    }
                    password = cli.data;
                    //got login&paswd, control them
                    if (!lpcontrol(login, password)) {
                        DBG("<< 500");
                        send_message((char*) M_500); // ERR
                        DBG("<close>");
                        close(cli.sc_fd);
                    } else {
                        send_message((char*) M_202); // OK                
                        while (1) {
                            //get command from client and process it
                            DBG("recv_command(), buffer: " << cli.buffer);
                            cmd = recv_command();
                            //processing 'INFO' command
                            if (cmd == info) {
                                try {
                                    recv_message();
                                } catch (int ex) {
                                    DBG(">> bad message: " << cli.data);
                                    if (ex == 1501) {
                                        send_message((char*) M_501);
                                    }
                                    break;
                                }
                                DBG(">> INFO command: " << cli.data);
                                try {
                                    write_to_log(cli.data);
                                } catch (int ex) {
                                    DBG("Error while writing to log, code: " << ex);
                                }
                                send_message((char*) M_202); // OK
                                DBG("<< 202");
                            } else if (cmd == foto) {
                                //processing 'FOTO' command
                                DBG(">> FOTO command");
                                try {
                                    recv_foto();
                                } catch (int ex) {
                                    if (ex == 600) {
                                        send_message((char*) M_501); // ERR
                                        DBG("<< 501");
                                        break;
                                    }
                                    if (ex == 700) {
                                        send_message((char*) M_300);
                                        DBG("<< 300");
                                        continue;
                                    }
                                    if (ex == 2000) {
                                        DBG("error while saving file");
                                    }
                                }
                                send_message((char*) M_202); // OK
                                DBG("<< 202");
                            } else if (cmd == none) {
                                DBG("connection closed by client");
                                break; //no command
                            } else {
                                DBG("unknown command");
                                send_message((char*) M_501); // ERR
                                DBG("<< 501");
                                break;
                            }
                        }

                        close(cli.sc_fd);
                        DBG("< close() >");
                    }

#ifdef PARALLEL
                    
                    //cleanup mutexes and detach shared mem
                    sem_close(&(p_shared_data->log_mutex));
                    sem_close(&(p_shared_data->f_cnt_mutex));
                    shmdt(p_shared_data);
                    exit(0);
                case -1:
                    perror("fork");
                    //cleanup mutexes and detach shared mem
                    sem_close(&(p_shared_data->log_mutex));
                    sem_close(&(p_shared_data->f_cnt_mutex));
                    shmdt(p_shared_data);
                    exit(errno);
                default:
                    //close client socket
                    close(cli_sc);
            }
#endif

        }
    } 
    //process exceptions
    catch (int ex) {
        if (ex > 100) {
            close(sc_fd);
        }
        cout << "An error occurred, code: " << ex << "\n";
        if (ex == 101) {
            perror("bind");
        }
        sem_close(&(p_shared_data->log_mutex));
        sem_close(&(p_shared_data->f_cnt_mutex));
        shmdt(p_shared_data);
        exit(1);
    }
    //cleanup mutexes and detach shared mem
    sem_close(&(p_shared_data->log_mutex));
    sem_close(&(p_shared_data->f_cnt_mutex));
    shmdt(p_shared_data);
    return 0;
}
