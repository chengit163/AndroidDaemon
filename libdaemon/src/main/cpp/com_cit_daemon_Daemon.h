#ifndef COM_CIT_DAEMON_DAEMON_H
#define COM_CIT_DAEMON_DAEMON_H

#include <jni.h>

#ifdef __cplusplus
extern "C" {
#endif

JNIEXPORT void JNICALL Java_com_cit_daemon_Daemon_init(JNIEnv *, jobject, jstring);

#ifdef __cplusplus
}
#endif

#endif //COM_CIT_DAEMON_DAEMON_H
