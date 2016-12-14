#include <jpeg/jpgrdwr.h>
#define BMP_HEADER_SIZE 54
void camerasp::ImgInfo::put_scan_line(JSAMPLE *data, int row_stride) const
{
    std::copy(data,data+row_stride, std::back_inserter(buffer));
}

JSAMPROW camerasp::ImgInfo::get_scan_line(int scan_line,  int stride) const
{
  JSAMPROW retval= &buffer[BMP_HEADER_SIZE  +scan_line*row_stride];
  for (unsigned int i = 0; i < row_stride; i += 3)
  {
    JSAMPLE& r = *retval;
    JSAMPLE& g = *(retval + 1);
    JSAMPLE& b = *(retval + 2);
    JSAMPLE t = r;
    r = b;
    b = t;
    retval += 3;
  }
  return  &buffer[BMP_HEADER_SIZE  +scan_line*row_stride];
}

