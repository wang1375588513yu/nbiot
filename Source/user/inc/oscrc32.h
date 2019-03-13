

#ifndef   _OSCRC32_H_
#define   _OSCRC32_H_


#include <stdint.h>


/*
  Input Args  : arg0, signal byte pointer
                arg1, 4 bytes length
  Output Args : return crc32 result
  Purpose     : calculate crc32 from a stream of bytes.
*/
 unsigned int u4_oscrc32(unsigned int init, const unsigned char *buff, unsigned int len);



#endif
