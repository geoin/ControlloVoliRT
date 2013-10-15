#ifndef TOOLS_EXPORTS_H
#define TOOLS_EXPORTS_H

#ifndef WIN32
#define TOOLS_EXPORTS
#else
#define TOOLS_EXPORTS __declspec( dllexport )
#endif

#endif