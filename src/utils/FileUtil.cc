#include "utils/FileUtil.h"
#include "utils/Log.h"

#include <fstream>

#define TAG "FileUtil"

using namespace std;

void FU_RemoveLastNewLine(char *str)
{
	int last = strlen(str)-1;
	if (str[last] == '\n') {
		str[last] = '\0';
	}
}

QStatus FU_CopyFile(const char* src, const char* dest)
{
	ifstream ifs(src, fstream::binary);
	if (ifs) {
		ofstream ofs(dest, fstream::trunc | fstream::binary);
		if (ofs) {
			ofs << ifs.rdbuf();
		}
		else {
			LOGD(TAG, "File %s open fail.", dest);
			return ER_FAIL;
		}
	}
	else {
		LOGD(TAG, "File %s open fail.", src);
		return ER_FAIL;
	}

	LOGD(TAG, "File copy %s to %s ok.", src, dest);
	return ER_OK;
}

QStatus FU_RemoveFile(const char* path)
{
	if (remove(path) != 0) {
		LOGD(TAG, "Remove file %s fail.", path);
		return ER_FAIL;
	}
	LOGD(TAG, "Remove file %s ok.", path);
	return ER_OK;
}

