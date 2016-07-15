#ifndef _FILEUTIL_H
#define _FILEUTIL_H

#include "GlobalDef.h"

#include <alljoyn/Status.h>

#include <string.h>

QStatus FU_CopyFile(const char* src, const char* dest);
QStatus FU_RemoveFile(const char* path);

void FU_RemoveLastNewLine(char *str);

#endif // _FILEUTIL_H
