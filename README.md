## Camera Server

This project uses 
* asio  https://github.com/chriskohlhoff/asio (included)
* Raspicam from https://github.com/cedricve/raspicam and
* via-httplib forked from https://github.com/kenba/via-httplib (included)
* srcipt to install as a service from https://github.com/jasonblewis/sample-service-script

to convert a Raspberry Pi to a web server that returns images captured from the camera.

'asio' is a header only library. Hence move the contents of the include folder to your default include path.

Install g++-x, the latest version of g++  as described below and modify the CXX variable in make files accordingly 

https://solarianprogrammer.com/2015/01/13/raspberry-pi-raspbian-install-gcc-compile-cpp-14-programs/

clone the raspicam repository and use cmake to build Raspicam. Ensure that the IJL Jpeg libray  is insattled.

Use the source code in src/http to build the http library. Then build the project in src using

make -f pi.mk 

To install as a service on Raspian I used Jason Lewis' script except for the small change I suggested in https://github.com/jasonblewis/sample-service-script/issues/6
Another issue is with raspicam library. The application would not pick up the library even after the library was installed in  /usr/local/lib. So I had creat libraspicam.conf and save it in /etc/ld.so.conf.d/
Go Figure!

