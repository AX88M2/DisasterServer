#include <Lib.h>
//hander please open the source code of the server.
int main(void)
{
	if (!disaster_init())
		return 1;

	return disaster_run();
}