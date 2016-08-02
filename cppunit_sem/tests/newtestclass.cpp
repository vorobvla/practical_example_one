/*
 * File:   newtestclass.cpp
 * Author: Vladimir Vorobyev (vorobvla)
 *
 * Created on May 18, 2014, 6:25:53 PM
 */


#include "newtestclass.h"

CPPUNIT_TEST_SUITE_REGISTRATION(newtestclass);

newtestclass::newtestclass() {
}

newtestclass::~newtestclass() {
}

// *(test_client_socket) -- a client communicates with server on this socket 

void newtestclass::setUp() {
    //    cout << endl << "setUp: "<< getpid() << endl;
    bzero(charbuf, CHARBUFSIZE);
    cli.buffer = "";
    cli.data = "";

    //prepare shared mem and semaphores from main() to prevent segfault on some tests
    int shm_key = getpid(), shm_id;

    if ((shm_id = shmget(shm_key, sizeof (shared_data), IPC_CREAT | 0600)) < 0) {
        perror("setUp(): shmget");
    }
    if ((p_shared_data = (shared_data*) shmat(shm_id, NULL, 0)) == (void *) - 1) {
        perror("setUp(): shmat");
    }
    //allow the OS to cleanup the shared mem if it is not attached
    if (shmctl(shm_id, IPC_RMID, NULL) < 0) {
        perror("setUp(): shmctl");
    }
    //initialize shared variable needed for correct filename generating
    p_shared_data->f_cnt = 0;
    //initialize shared mutex needed for correct log appending
    if (sem_init(&(p_shared_data->log_mutex), 1, 1) < 0) {
        perror("setUp(): sem_init");
    }
    //initialize shared mutex needed for correct f_cnt update
    if (sem_init(&(p_shared_data->f_cnt_mutex), 1, 1) < 0) {
        perror("setUp(): sem_init");
    }

    //prepare shared mem -- need for a connect from other process
    int shm_key_conn = shm_key + 1234567, shm_id_conn;
    if ((shm_id_conn = shmget(shm_key_conn, sizeof (int), IPC_CREAT | 0600)) < 0) {
        perror("setUp(): shmget");
    }
    if ((test_client_socket = (int*) shmat(shm_id_conn, NULL, 0)) == (void *) - 1) {
        perror("setUp(): shmat");
    }
    //prepare socket for communication with server
    if ((*(test_client_socket) = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
        perror("setUp(): tester_socket = socket()");
        exit(1);
    }

    bzero(&sin, sizeof (sin));
    sin.sin_family = AF_INET;
    sin.sin_port = htons(PORT);
    sin.sin_addr.s_addr = inet_addr("127.0.0.1");


    //prepare server socket for connection
    struct sockaddr_in my_addr, rem_addr;
    unsigned int rem_addr_length;
    int yes = 1, sc_fd;
    if ((sc_fd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
        perror("setUp(): sd_fd = socket()");
    }
    bzero(&my_addr, sizeof (my_addr));
    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(PORT);
    //prepare attributes for accept()
    bzero(&rem_addr, sizeof (rem_addr));
    rem_addr_length = sizeof (rem_addr);
    //set option 'reuseaddr' on the server socket
    if (setsockopt(sc_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof (int)) == -1) {
        perror("setUp(): setsockopt()");
    }
    //bind the server socket and the port
    if (bind(sc_fd, (struct sockaddr *) &my_addr, sizeof (my_addr)) == -1) {
        perror("setUp(): bind()");
    }
    //set the length of the client queue
    if (listen(sc_fd, 5) == -1) {
        perror("setUp(): setsockopt()");
    } //fork a process to connect my socket to server's socket

    int test_client_pid = fork();
    if (test_client_pid == -1) {
        perror("setUp(): fork()");
        return;
    }
    if (test_client_pid == 0) {
        //test client
        //        cout << endl << "start connect process" << endl;
        sleep(1);

        if (connect(*(test_client_socket), (struct sockaddr *) &sin, sizeof (sin)) == -1) {
            perror("setUp(): connect()");
            close(*(test_client_socket));
            exit(1);
        }
        //        cout << endl << "end connect process" << endl;
        exit(0);
    }

    if ((cli.sc_fd = accept(sc_fd, (struct sockaddr *) &rem_addr, &rem_addr_length)) == -1) {
        perror("setUp(): accept()");
    }

    int status;
    waitpid(-1, &status, 0);
    close(sc_fd);

}

void newtestclass::tearDown() {
    close(cli.sc_fd);
    if (*(test_client_socket))
        close(*(test_client_socket));
    sem_close(&(p_shared_data->log_mutex));
    sem_close(&(p_shared_data->f_cnt_mutex));
    shmdt(p_shared_data);
    shmdt(test_client_socket);
}

int count_ascii_sum(string str) {
    int res = 0;
    for (int i = 0; i < str.size(); i++) {
        res += str[i];
    }
    return res;
}

int newtestclass::test_client_send(string msg) {
    return send(*(test_client_socket), msg.c_str(), msg.size(), 0);
}

int newtestclass::test_client_recv() {
    return recv(*(test_client_socket), charbuf, CHARBUFSIZE, 0);
}

//==============================================================
//                         unit tests
//==============================================================

//Test if the function lpcontrol recognizes valid username and password. 

void newtestclass::lpcontrol_positive_test() {
    string username("RobotTestbot");
    int ascii_sum = count_ascii_sum(username);
    sprintf(charbuf, "%d", ascii_sum);
    CPPUNIT_ASSERT(lpcontrol(username, string(charbuf)));
}


//Test if the function recognizes invalid username

void newtestclass::lpcontrol_invalid_username_test() {
    string username("robot");
    int ascii_sum = count_ascii_sum(username);
    sprintf(charbuf, "%d", ascii_sum);
    CPPUNIT_ASSERT(!lpcontrol(username, string(charbuf)));
}

//Test if the function recognizes invalid password value. 

void newtestclass::lpcontrol_passwd_invalid_val_test() {
    string username("RobotTestbot");
    int ascii_sum = count_ascii_sum(username);
    ascii_sum++;
    sprintf(charbuf, "%d", ascii_sum);
    CPPUNIT_ASSERT(!lpcontrol(username, string(charbuf)));
}

//Test if the function recognizes invalid password value

void newtestclass::lpcontrol_passwd_invalid_fmt_test() {
    string username("RobotTestbot");
    int ascii_sum = count_ascii_sum(username);
    ascii_sum++;
    sprintf(charbuf, "%da", ascii_sum);
    CPPUNIT_ASSERT(!lpcontrol(username, string(charbuf)));

}

//Test a write_to_log() function

void newtestclass::write_to_log_test() {
    string msgs[10];
    for (int i = 0; i < 10; i++) {
        sprintf(charbuf, "Test_msg_%d", i);
        msgs[i] = charbuf;
        write_to_log(msgs[i]);
        filestr.open("log", ios::in);
        filestr.seekg(0, ios_base::end);
        for (int j = i; j >= 0; j--) {

            filestr.seekg(-1 * (msgs[j].size() + 1), ios_base::cur);
            filestr.getline(charbuf, 100);
            filestr.seekg(-1 * (msgs[j].size() + 1), ios_base::cur);
            CPPUNIT_ASSERT(!strcmp(msgs[j].c_str(), charbuf)); //strcmp returns 0 when strings are ==
        }
        filestr.close();
        bzero(charbuf, 100);
    }
}

//Test if the recv_message() recognizes the messages correctly when all of them are sent at once

void newtestclass::recv_message_test() {
    string exp_cli_data_1("expexted message");
    string exp_cli_data_2("expe\nxted m\00e\x03\x84ss\rage");
    string exp_cli_data_3("expected\n\rmessage");
    test_client_send(exp_cli_data_1 + "\r\n" + exp_cli_data_2 + "\r\n" + exp_cli_data_3 + "\r\n");
    recv_message();
    CPPUNIT_ASSERT(exp_cli_data_1 == cli.data);
    recv_message();
    CPPUNIT_ASSERT(exp_cli_data_2 == cli.data);
    recv_message();
    CPPUNIT_ASSERT(exp_cli_data_3 == cli.data);
}

//Test if the recv_message() recognizes the messages correctly when are sent one by one

void newtestclass::recv_message_one_by_one_test() {
    string exp_cli_data_1("expexted message");
    string exp_cli_data_2("expexted msg \x02");
    string exp_cli_data_3("expected\n\rmessage");
    test_client_send(exp_cli_data_1 + "\r\n");
    recv_message();
    CPPUNIT_ASSERT(exp_cli_data_1 == cli.data);
    test_client_send(exp_cli_data_2 + "\r\n");
    recv_message();
    CPPUNIT_ASSERT(exp_cli_data_2 == cli.data);
    test_client_send(exp_cli_data_3 + "\r\n");
    recv_message();
    CPPUNIT_ASSERT(exp_cli_data_3 == cli.data);
}

//Test if the recv_message() is able to recieve a long message

void newtestclass::recv_message_long_test() {
    string exp_cli_data;
    for (int i = 0; i < 10000; i++) {
        exp_cli_data += "1";
    }
    test_client_send(exp_cli_data + "\r\n");
    recv_message();
    CPPUNIT_ASSERT(exp_cli_data == cli.data);
}

//Test if the recv_message() is working right when a part of msg 
//is alrady  in buffer and the rest is being sended by peer
void newtestclass::recv_message_split_test(){
    string exp_msg_half_1("firs\t hal\rf");
    string exp_msg_half_2("\x02\nd half");
    cli.buffer = exp_msg_half_1;
    test_client_send(exp_msg_half_2 + "\r\n");
    recv_message();
    CPPUNIT_ASSERT_EQUAL(exp_msg_half_1 + exp_msg_half_2, cli.data);
}

//Test if the recv_message_test() parses the buffer correctly

void newtestclass::recv_message_buffer_test() {

    string expt_cli_data_1("expexted\rstring\t1");
    string expt_cli_data_2("expexted\nstring\t2");
    string expt_cli_data_3("expexted\n\rstring\t3");
    cli.buffer = expt_cli_data_1 + "\r\n" + expt_cli_data_2 + "\r\n" + expt_cli_data_3 + "\r\n";
    recv_message();
    CPPUNIT_ASSERT(expt_cli_data_1 == cli.data);
    recv_message();
    CPPUNIT_ASSERT(expt_cli_data_2 == cli.data);
    recv_message();
    CPPUNIT_ASSERT(expt_cli_data_3 == cli.data);
}


//Test if the  check_buffer_command recognizes valid commands that are in the buffer

void newtestclass::check_buffer_command_positive_test() {

    cli.buffer = "INFO asdasdasd\r\n";
    CPPUNIT_ASSERT(check_buffer_command());
    cli.buffer = "FOTO asdasdasd\r\n";
    CPPUNIT_ASSERT(check_buffer_command());
}


//Test if the  check_buffer_command recognizes if the buffer contains rhe start of a valid command 

void newtestclass::check_buffer_command_positive_start_test() {

    cli.buffer = "I";
    CPPUNIT_ASSERT(check_buffer_command());
    cli.buffer = "IN";
    CPPUNIT_ASSERT(check_buffer_command());
    cli.buffer = "INF";
    CPPUNIT_ASSERT(check_buffer_command());
    cli.buffer = "INFO";
    CPPUNIT_ASSERT(check_buffer_command());
    cli.buffer = "F";
    CPPUNIT_ASSERT(check_buffer_command());
    cli.buffer = "FO";
    CPPUNIT_ASSERT(check_buffer_command());
    cli.buffer = "FOT";
    CPPUNIT_ASSERT(check_buffer_command());
    cli.buffer = "FOTO";
    CPPUNIT_ASSERT(check_buffer_command());
}


//Test if the  check_buffer_command recognizes invalid format of the commands that are in the buffer

void newtestclass::check_buffer_command_invalid_fmt_test() {

    cli.buffer = "INFOasdasdasd";
    CPPUNIT_ASSERT(!check_buffer_command());
    cli.buffer = "FOTO: asdasdasd";
    CPPUNIT_ASSERT(!check_buffer_command());
}


//Test if the  check_buffer_command recognizes invalid command of the commands that are in the buffer

void newtestclass::check_buffer_command_invalid_command_test() {

    cli.buffer = "TODO asdasdasd";
    CPPUNIT_ASSERT(!check_buffer_command());
    cli.buffer = "INTO asdasdasd";
    CPPUNIT_ASSERT(!check_buffer_command());
    cli.buffer = "info asdasdasd";
    CPPUNIT_ASSERT(!check_buffer_command());
    cli.buffer = "INFOFOTO asdasdasd";
    CPPUNIT_ASSERT(!check_buffer_command());
}

//Test if the recv_command() works right when hetting data from client
void newtestclass::recv_command_online_test(){
    string expt_data("VIII useful data");
    test_client_send("INFO " + expt_data);
    CPPUNIT_ASSERT_EQUAL(info, recv_command());
    CPPUNIT_ASSERT_EQUAL(expt_data, cli.buffer); 
    cli.buffer = "";
    test_client_send("FOTO " + expt_data);
    CPPUNIT_ASSERT_EQUAL(foto, recv_command());
    CPPUNIT_ASSERT_EQUAL(expt_data, cli.buffer);
    cli.buffer = "";     
    test_client_send(expt_data);
    CPPUNIT_ASSERT_EQUAL(unknown, recv_command());
    CPPUNIT_ASSERT_EQUAL(expt_data, cli.buffer);    
}

//Test if the recv_command() works right with valid and complete data in the buffer

void newtestclass::recv_command_buffer_test() {

    cli.buffer = "INFO useful data";
    CPPUNIT_ASSERT_EQUAL(info, recv_command());
    CPPUNIT_ASSERT(cli.buffer == "useful data");
    cli.buffer = "FOTO useful data";
    CPPUNIT_ASSERT_EQUAL(foto, recv_command());
    CPPUNIT_ASSERT(cli.buffer == "useful data");
    cli.buffer = "ABCD useful data";
    CPPUNIT_ASSERT_EQUAL(unknown, recv_command());
    CPPUNIT_ASSERT(cli.buffer == "ABCD useful data");
}


//Test if the recv_foto() works right when recieves a valid 'foto' message

void newtestclass::recv_foto_positive_test() {
    string exp_foto("ABCDEFGH"); //<<-- valid data are known from the task
    string chsum("\x00\x00\x02\x24", 4);
    test_client_send("8 " + exp_foto + chsum);
    recv_foto();
    char filename[20];
    sprintf(filename, "file%d.png", p_shared_data->f_cnt - 1);
    filestr.open(filename, ios::in);
    streampos fsize = filestr.tellg();
    filestr.seekg(0, ios::end);
    fsize = filestr.tellg() - fsize + 1;
    filestr.seekg(0, ios::beg);
    filestr.get(charbuf, fsize);
    filestr.close();
    CPPUNIT_ASSERT(!strcmp(exp_foto.c_str(), charbuf));
}

//Negative test of the recv_foto() with wrong checksum.  exception 700 expected

void newtestclass::recv_foto_bad_checksum_test() {
    string exp_foto("ABCDEFGH"); //<<-- valid data are known from the task
    string chsum("\x00\x10\x02\x24", 4);
    test_client_send("8 " + exp_foto + chsum);
    //    CPPUNIT_ASSERT_THROW_MESSAGE(700, recv_foto(), int);  
    //    <-- pokud bych mel v projektu normalni exeptiony pouzil bych toto makro 
    try {
        recv_foto();
    } catch (int ex) {
        CPPUNIT_ASSERT_EQUAL(700, ex);
        return;
    }
    CPPUNIT_ASSERT("exception 700 expected");
}

//Negative test of the recv_foto() with wrong format.  exception 600 expected

void newtestclass::recv_foto_wrong_fmt_test() {
    string exp_foto("ABCDEFGH"); //<<-- valid data are known from the task
    string chsum("\x00\x10\x02\x24", 4);
    test_client_send("8b " + exp_foto + chsum + "rubbish");
    //    CPPUNIT_ASSERT_THROW_MESSAGE(600, recv_foto(), int);  
    //    <-- pokud bych mnel v projektu normalni exeptiony pouzil bych toto makro 
    try {
        recv_foto();
    } catch (int ex) {
        CPPUNIT_ASSERT_EQUAL(600, ex);
        return;
    }
    CPPUNIT_FAIL("exception 600 expected");
}

//test a method send_message() that sands a message to a client

void newtestclass::send_message_positive_test() {
    char exp_msg[20];
    sprintf(exp_msg, "message for client");
    send_message(exp_msg);
    test_client_recv();
    CPPUNIT_ASSERT(!strcmp(exp_msg, charbuf));
}
