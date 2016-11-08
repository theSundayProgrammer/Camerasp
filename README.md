## Camera Server

This project uses 
* asio  https://github.com/chriskohlhoff/asio (included)
* Raspicam from https://github.com/cedricve/raspicam and
* via-httplib forked from https://github.com/kenba/via-httplib (included)

to convert a Raspberry Pi to a web server that returns images captured from the camera.

'asio' is a header only library. Hence move the contents of the include folder to your default include path.

Install g++-x, the latest version of g++  as described below and modify the CXX variable in make files accordingly 

https://solarianprogrammer.com/2015/01/13/raspberry-pi-raspbian-install-gcc-compile-cpp-14-programs/

clone the raspicam repository and use cmake to build Raspicam.
To build run 'make' from the parent of 'src' directory.

 
