# SCTP Socket

The code is to provide a simple exmple of sctp connection establishment.
It supports establishing sctp server service, establishing sctp client connection, terminating sctp connection, dumping sctp information functions.
This sample code illustrates a simple sctp connection test:
* the code establishs a sctp server service and keep listening
* the code establishs a sctp client to connect to the server and keep listening
* the client sends a message to the server, and the server echo the message
* the client terminates the connection when receiving the echo message

## OS
ubuntu 18.04

## compile and run the code
* compile codes
	```	
	make
	```

* execute the program
	```
	./sctp_dev
	```

* clean useless things
	```
	make clean
	```

## configration
* NATIVE_SCTP_FUNC_FLAG
  - location: sctp_socket.h
  - difinition
    - true: use native sctp function to receive and send messages, i.e. sctp_recvmsg and sctp_sendmsg functions
    - false: use recvmsg and sendmsg functions to receive and send messages

* DUMP_INFO_FLAG
  - location: sctp_socket.h
  - difinition
    - true: print all sctp information
    - false: do not print all sctp information

