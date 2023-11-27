#include <Lib.h>

int main(void)
{
	if (!disaster_init())
		return 1;

	return disaster_run();
}