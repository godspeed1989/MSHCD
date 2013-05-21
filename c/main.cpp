#include "mshcd.hpp"

int main(int argc, const char* argv[])
{
	if(argc < 3)
	{
		printf("Usage: %s image cascade\n", argv[0]);
		return -1;
	}
	MSHCD mshcd(argv[1], argv[2]);
#ifdef WITH_OPENCV
	mshcd.ShowDetectionResult(argv[1]);
#endif
	return 0;
}

