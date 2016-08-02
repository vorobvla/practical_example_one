/*
 * File:   newtestclass.h
 * Author: Vladimir Vorobyev (vorobvla)
 *
 * Created on May 18, 2014, 6:25:53 PM
 */

#ifndef NEWTESTCLASS_H
#define	NEWTESTCLASS_H

#define CHARBUFSIZE 100
#define PORT 3000

#include <cppunit/extensions/HelperMacros.h>
#include <arpa/inet.h>
#include <fstream> 
#include <fcntl.h>

#include "robot.hpp"

class newtestclass : public CPPUNIT_NS::TestFixture {
    CPPUNIT_TEST_SUITE(newtestclass);

    CPPUNIT_TEST(lpcontrol_positive_test);
    CPPUNIT_TEST(lpcontrol_invalid_username_test);
    CPPUNIT_TEST(lpcontrol_passwd_invalid_val_test);
    CPPUNIT_TEST(lpcontrol_passwd_invalid_fmt_test);
    CPPUNIT_TEST(write_to_log_test);
    CPPUNIT_TEST(recv_message_test);
    CPPUNIT_TEST(recv_message_one_by_one_test);
    CPPUNIT_TEST(recv_message_long_test);
    CPPUNIT_TEST(recv_message_split_test);
    CPPUNIT_TEST(recv_message_buffer_test);
    CPPUNIT_TEST(check_buffer_command_positive_test);
    CPPUNIT_TEST(check_buffer_command_positive_start_test);
    CPPUNIT_TEST(check_buffer_command_invalid_fmt_test);
    CPPUNIT_TEST(check_buffer_command_invalid_command_test);
    CPPUNIT_TEST(recv_command_online_test);
    CPPUNIT_TEST(recv_command_buffer_test);
    CPPUNIT_TEST(recv_foto_positive_test);
    CPPUNIT_TEST(recv_foto_bad_checksum_test);
    CPPUNIT_TEST(recv_foto_wrong_fmt_test);
    CPPUNIT_TEST(send_message_positive_test);
    
    CPPUNIT_TEST_SUITE_END();

public:
    newtestclass();
    virtual ~newtestclass();
    void setUp();
    void tearDown();

private:
    char charbuf [CHARBUFSIZE];
    int * test_client_socket;
    std::fstream filestr;
    struct sockaddr_in sin;
    int test_client_send(string);
    int test_client_recv();

    void lpcontrol_positive_test();
    void lpcontrol_invalid_username_test();
    void lpcontrol_passwd_invalid_val_test();
    void lpcontrol_passwd_invalid_fmt_test();
    void write_to_log_test();
    void recv_message_test();
    void recv_message_one_by_one_test();
    void recv_message_long_test();
    void recv_message_split_test();
    void recv_message_buffer_test();
    void check_buffer_command_positive_test();
    void check_buffer_command_positive_start_test();
    void check_buffer_command_invalid_fmt_test();
    void check_buffer_command_invalid_command_test();
    void recv_command_online_test();
    void recv_command_buffer_test();
    void recv_foto_positive_test();
    void recv_foto_bad_checksum_test();
    void recv_foto_wrong_fmt_test();
    void send_message_positive_test();
};

#endif	/* NEWTESTCLASS_H */

