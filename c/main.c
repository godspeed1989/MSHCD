#include "mshcd.h"

int main(int argc, const char* argv[])
{
#ifdef WITH_OPENCV
	printf("With OpenCV support.\n");
#else
	printf("Without OpenCV support.\n");
#endif
	if(argc < 3)
	{
		printf("Usage: %s image cascade\n", argv[0]);
		return -1;
	}
	HAAR m;
	MSHCD(&m, argv[1], argv[2]);
	MSHCD_Cleanup(&m);
	return 0;
}

