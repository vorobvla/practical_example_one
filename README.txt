======================== README FOR ROBOT KAREL ==========================
============== (task for Networking and Automatic Testing) ===============
======================== by Vladimir Vorobyev ============================

This  folder contains my solution of two tasks, I have implemented while studying in Czech Technological University in Prague (CTU).

The first one is a C++ implementation of a server which supports communication with clients via  TCP-based text protocol. The clients can to log in and send text information or pictures in appropriate format, the server must process these actions and respond correctly. The detailed task is in the file 'task.txt' in this folder. The implementation is preformed in a text editor.

The second one is a suite of unit tests created with CPPUNIT library. For this, a Netbeans IDE project was created with the sources of the first solution. The tests can be ran in Netbeans (by clicking right button the project in the project browser in Netbeans and selecting 'Test' option). For correct performance, the 'C/C++' plug-in must be installed in Netbeans IDE and 'libcppunit-dev' package must be installed on the system (on Ubuntu 14.04).

The source code of the server is located in the root directory of Netbeans project ( ./cppunit_sem ). Unit test implementation and a report (a PDF file with description of each test) are located in ./cppunit_sem/tests 
