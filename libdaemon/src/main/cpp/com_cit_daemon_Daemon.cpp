#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/inotify.h>
#include <sys/system_properties.h>
#include <android/log.h>

#include "com_cit_daemon_Daemon.h"

#define LOG_TAG "daemon"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)

/**
 *  get the android version code
 */
int get_version()
{
    char value[8] = "";
    __system_property_get("ro.build.version.sdk", value);
    return atoi(value);
}

jobject getContext(JNIEnv *env)
{
    jobject context = NULL;
    jclass activityThread_class = env->FindClass("android/app/ActivityThread");
    jmethodID currentActivityThread_methodID = env->GetStaticMethodID(activityThread_class,
                                                                      "currentActivityThread",
                                                                      "()Landroid/app/ActivityThread;");
    jobject at = env->CallStaticObjectMethod(activityThread_class, currentActivityThread_methodID);
    if (at)
    {
        jmethodID getApplication_methodID = env->GetMethodID(activityThread_class, "getApplication",
                                                             "()Landroid/app/Application;");
        context = env->CallObjectMethod(at, getApplication_methodID);
        env->DeleteLocalRef(at);
    }
    return context;
}

void getAppDataData(JNIEnv *env, char *appDataData)
{
    jobject context = getContext(env);
    if (context)
    {
        jclass context_class = env->GetObjectClass(context);
        jmethodID getPackageName_methodID = env->GetMethodID(context_class, "getPackageName",
                                                             "()Ljava/lang/String;");
        jstring str_packageName = (jstring) env->CallObjectMethod(context,
                                                                  getPackageName_methodID);
        if (str_packageName)
        {
            const char *c_packageName = env->GetStringUTFChars(str_packageName, NULL);
            sprintf(appDataData, "/data/data/%s", c_packageName);
            env->ReleaseStringUTFChars(str_packageName, c_packageName);
        }
        env->DeleteLocalRef(context);
    }
}


JNIEXPORT void JNICALL Java_com_cit_daemon_Daemon_init(JNIEnv *env, jobject thiz, jstring url)
{
    //初始化log
    pid_t pid = fork();
    if (pid < 0)
    {
        LOGD("fork failed...");
    } else if (pid == 0)
    {
        char appDataData[128] = {0};
        memset(appDataData, 0, sizeof(appDataData));
        getAppDataData(env, appDataData);
        //
        int fileDescriptor = inotify_init();
        if (fileDescriptor < 0)
        {
            LOGD("inotify_init failed...");
            exit(1);
        }
        int watchDescriptor = inotify_add_watch(fileDescriptor, appDataData, IN_DELETE);
        if (watchDescriptor < 0)
        {
            LOGD("inotify_add_watch failed...");
            exit(1);
        }
        //分配缓存，以便读取event，缓存大小=一个struct inotify_event的大小，这样一次处理一个event
        void *p_buf = malloc(sizeof(struct inotify_event));
        if (p_buf == NULL)
        {
            LOGD("malloc failed...");
            exit(1);
        }
        //开始监听
        LOGD("observer: %s", appDataData);
        size_t readBytes = read(fileDescriptor, p_buf, sizeof(struct inotify_event));
        //read会阻塞进程，走到这里说明收到目录被删除的事件，注销监听器
        free(p_buf);
        inotify_rm_watch(fileDescriptor, IN_DELETE);
        // 目录被删除
        LOGD("uninstall: %s", appDataData);
        const char *c_url = env->GetStringUTFChars(url, JNI_FALSE);
        int sdkVersion = get_version();
        if (sdkVersion >= 17)
        {
            // Android4.2系统之后支持多用户操作，所以得指定用户
            execlp("am", "am", "start", "--user", "0", "-a", "android.intent.action.VIEW", "-d",
                   c_url, (char *) NULL);
        } else
        {
            // Android4.2以前的版本无需指定用户
            execlp("am", "am", "start", "-a", "android.intent.action.VIEW", "-d", c_url,
                   (char *) NULL);
        }
        env->ReleaseStringUTFChars(url, c_url);
    } else
    {
        //父进程直接退出，使子进程被init进程领养，以避免子进程僵死
    }
}
