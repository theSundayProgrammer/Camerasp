## Camera Server

This project uses 
* asio  https://github.com/chriskohlhoff/asio
* Raspicam from https://github.com/cedricve/raspicam and
* via-httplib forked from https://github.com/kenba/via-httplib 

to convert a Raspberry Pi to a web server that returns images captured from the camera.

'asio' is a header only library. Hence move the contents of the include folder to your default include path.

Install g++-4.9 as described in 

https://solarianprogrammer.com/2015/01/13/raspberry-pi-raspbian-install-gcc-compile-cpp-14-programs/

Use cmake to build Raspicam.
Use the source code n src  to
 
make -f pi.mk 

This make file may need to be modified for local conditions such as include folder.   
 
