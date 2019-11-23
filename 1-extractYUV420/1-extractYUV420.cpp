// 1-extractYUV420.cpp : Defines the entry point for the application.
//

#include "1-extractYUV420.h"

extern "C" {
#include "libavcodec/avcodec.h"
}

using namespace std;

int main()
{
	cout << avcodec_configuration() << endl;
	return 0;
}
