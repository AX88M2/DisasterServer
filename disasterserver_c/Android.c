#include <Android.h>
#include <Lib.h>
#include <io/Dir.h>
#include <Log.h>

JNIEXPORT jboolean JNICALL Java_com_teamexeempire_disaster2d_Disaster_disaster_1start
(JNIEnv* env, jclass class)
{	
	if (!disaster_init())
		return false;
	
	Thread th;
	ThreadSpawn(th, disaster_run, NULL);
	return true;
}