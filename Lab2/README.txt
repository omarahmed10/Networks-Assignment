Implementation of the Go-back-N protocol over UDP. This project builds both a client and server program.

Also included is my networking framework written to allow for easily swappable client/server input sources, application layer protocol, and transport layer protocol.

Client and Server specific code can be found in src/client/ and src/server/ respectively. Code found directly in src/ is used by both. Check the header files for class specific documentation. 

To build, run 'make' from the top level directory. Upon a successful buid, binaries will be placed in bin/. This project has been built successfully on GNU/Linux 3.2.0-23-generic x86_64.