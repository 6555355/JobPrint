#ifndef _TEMP_HPP
#define _TEMP_HPP

#include <string>
#include <sstream>
#include <iomanip>
using namespace std;

template <typename T>
void LogBinArray(T * info, char * head, int base, int group)
{
//#ifdef _DEBUG

	ostringstream os;
	os << head << "\n";
	for (int j = 0; j < group; j++){
		for (int i = 0; i < base; i++)
			os << setw(5) <<info[i + j * base] << ", ";
		os << "\n";
	}

	LogfileStr(os.str().c_str());

//#endif
}

#endif
