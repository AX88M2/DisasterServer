#include <Lib.h>
#include <io/Threads.h>

int main(void)
{
	if (!disaster_init())
		return 1;

	return disaster_run();
}