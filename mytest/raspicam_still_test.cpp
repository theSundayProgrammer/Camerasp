/****************************************************************/
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <camerasp/cam_still.hpp>
#include <unistd.h>
#include <camerasp/parseCmd.hpp>
using namespace std;
namespace spd = spdlog;
std::shared_ptr<spd::logger> console;
std::atomic_flag saved=ATOMIC_FLAG_INIT;
//Returns the value of a param. If not present, returns the defvalue
float getParamVal ( string id,int argc,char **argv,float defvalue ) {
    for ( int i=0; i<argc; i++ )
        if ( id== argv[i] )
            return atof ( argv[i+1] );
    return defvalue;
}


//prints program command line usage
void usage() {
    cout<<"-w val : sets my image width (2592 default)"<<endl;
    cout<<"-h val : sets image height (1944 default)"<<endl;
     cout<<"-iso val: set iso [100,800] (400 default)"<<endl;
}
void  image_taken(unsigned char * data, int error, unsigned int length) {
  if (error == 0){
  ofstream file("picture.bmp", ios::binary);
  file.write((char*)data, length);
  }
  saved.clear();
}

int main ( int argc, char *argv[] ) {
    usage();

    int width = getParamVal ( "-w",argc,argv,2592 );
    int height =getParamVal ( "-h",argc,argv,1944 );
    int iso=getParamVal ( "-iso",argc,argv,400);

    console = spd::stdout_color_mt("mytest"); 
    cout << "Initializing ..."<<width<<"x"<<height<<endl;
    {
        camerasp::cam_still *camera=new camerasp::cam_still();
        camera->setWidth ( 2*width );
        camera->setHeight ( 2*height );
        camera->setISO(iso);
        camera->setEncoding ( raspicam::RASPICAM_ENCODING_BMP );
        camera->open();
        cout<<"capture"<<endl;
        unsigned int length = camera->getImageBufferSize(); // Header + Image Data + Padding
        unsigned char * data = new unsigned char[length];
        saved.test_and_set();
       console->info("Length = {0}", length);
       if ( camera->startCapture(image_taken, data, 0, length)) {
            cerr<<"Error in grab"<<endl;
            return -1;
        }
        cout << "saving picture.bmp" << endl;
        while (saved.test_and_set()) {}
        camera->stopCapture();
        cout << "done" << endl;
        camera->setWidth ( width );
        camera->setHeight ( height );
        camera->setISO(iso);
        camera->setEncoding ( raspicam::RASPICAM_ENCODING_BMP );
        cout<<"capture"<<endl;
        length = camera->getImageBufferSize(); // Header + Image Data + Padding

       console->info("Length = {0}", length);
       if ( camera->startCapture(image_taken, data, 0, length)) {
            cerr<<"Error in grab"<<endl;
            return -1;
        }
        cout << "saving picture.bmp" << endl;
        while (saved.test_and_set()) {}
        cout << "saved picture.bmp" << endl;
        camera->stopCapture();
        cout << "done" << endl;
        delete [] data;
	delete camera;
    }
   return 0;
}

