#ifndef JPGRDWR_020615
#define JPGRDWR_020615
#include <jpeglib.h>
#include <string>
#include <vector>
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
  
inline void Camerasp::ImgInfo::put_scan_line(JSAMPLE *data, int row_stride) const
{
    std::copy(data,data+row_stride, std::back_inserter(buffer));
}

inline JSAMPROW Camerasp::ImgInfo::get_scan_line(int scan_line,  int stride) const
{
    return &buffer[scan_line*row_stride];
}

#endif