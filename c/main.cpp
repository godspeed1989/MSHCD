#include "mshcd.hpp"

int main()
{
#ifdef WITH_OPENCV
	MSHCD mshcd("../tools/gray_img.jpg", "../haar_alt.txt");
#else
	MSHCD mshcd("../tools/gray_img.raw", "../haar_alt.txt");
#endif
	return 0;
}

