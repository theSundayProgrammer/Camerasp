#ifndef JPGRDWR_020615
#define JPGRDWR_020615
#include <cstddef>
#include <cstdio>
#ifndef RASPICAM_MOCK
#include <jpeglib.h>
#else
typedef unsigned char JSAMPLE;
typedef JSAMPLE* JSAMPROW;
#endif
#include <string>
#include <vector>
#include <iterator>
namespace Camerasp
{
    struct ImgInfo
    {
      std::vector<unsigned char> mutable buffer ;
      unsigned int image_height;
      unsigned int image_width;
      unsigned int row_stride;
      unsigned int quality;
      JSAMPROW get_scan_line(int scan_line,  int row_stride) const;
      void put_scan_line(JSAMPLE *,  int row_stride) const;
    };
    ImgInfo read_JPEG_file (std::string const& filename);
    void write_JPEG_file (std::string const& filename, ImgInfo const& img);
    std::vector<unsigned char> write_JPEG_dat (ImgInfo const& img);
}
  
#endif
