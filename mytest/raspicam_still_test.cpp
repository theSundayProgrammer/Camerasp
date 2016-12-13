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
        camera->open();
        cout<<"capture"<<endl;
        unsigned int length = camera->getImageBufferSize(); // Header + Image Data + Padding
        unsigned char * data = new unsigned char[length];
        console->info("Length = {0}", length);
        for(size_t i =0; i < 1000; ++i)
          if ( camera->takePicture( data,  length)) {
              cerr<<"Error in grab"<<endl;
          } else{
            char filename[32];
            sprintf(filename,"out%d.bmp",i%100); 
            ofstream ofs(filename,ios::binary);
            ofs.write((char*)data,length);
            cout << "done" << endl;
            usleep(500);
        }
        delete [] data;
	delete camera;
    }
   return 0;
}

