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

Ensure that the IJL Jpeg libray  is installed.
To build run 'make' from the parent of 'src' directory.

To install as a service on Raspian I used Jason Lewis' script except for the small change I suggested in https://github.com/jasonblewis/sample-service-script/issues/6.

Go Figure!

Copy options.txt to /srv/cameras/options.txt. This is the configuration file.  

