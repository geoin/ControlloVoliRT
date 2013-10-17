#ifndef COMMON_DEFS_H
#define COMMON_DEFS_H

#if defined( __APPLE__ )

#define __int64 __int64_t
#define stricmp strcasecmp
#define strnset memset
#define _fseeki64 fseek
#define _ftelli64 ftell

#endif

#endif