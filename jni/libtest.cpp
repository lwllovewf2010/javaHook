
#include <android/log.h>
#include <stdio.h>
#include <string.h>
#include <dlfcn.h>
#include <jni.h>
#include <android_runtime/AndroidRuntime.h>
#include <pthread.h>
#include "common.h"
#include "libtest.h"

using namespace android;

#ifndef INJECT_JAR_PATH
#define INJECT_JAR_PATH "/system/framework/hook.jar"
#endif
#ifndef CLASS_JAVA
#define CLASS_JAVA "com.phoebe.javaHook.Hook"
#endif

static const char JSTRING[] = "Ljava/lang/String;";
static const char JCLASS_LOADER[] = "Ljava/lang/ClassLoader;";
static const char JCLASS[] = "Ljava/lang/Class;";
static char sig_buffer[512];

void java_hook_test(const char* s) {

	LOGI(s);
	pthread_t tid;
	pthread_create(&tid, NULL, _hookJava, NULL);

	LOGI("tid:%p\n",tid);

	pthread_detach(tid);

}
void* _hookJava(void*){
	HookJava();
	return NULL;
}
void HookJava()
{
	LOGI("start hook java");
	JavaVM* jvm = AndroidRuntime::getJavaVM();
	JNIEnv* env;
	jvm->AttachCurrentThread(&env, NULL);

	jstring apk_path = env->NewStringUTF(INJECT_JAR_PATH);
	jstring dex_out_path = env->NewStringUTF("/data/dalvik-cache");
	jclass dexloader_claxx = env->FindClass("dalvik/system/DexClassLoader");

	snprintf(sig_buffer, 512, "(%s%s%s%s)V", JSTRING, JSTRING, JSTRING, JCLASS_LOADER);
	jmethodID dexloader_init_method = env->GetMethodID(dexloader_claxx, "<init>", sig_buffer);

	snprintf(sig_buffer, 512, "(%s)%s", JSTRING, JCLASS);
	jmethodID loadClass_method = env->GetMethodID(dexloader_claxx, "loadClass", sig_buffer);
	jclass class_loader_claxx = env->FindClass("java/lang/ClassLoader");
	snprintf(sig_buffer, 512, "()%s", JCLASS_LOADER);
	jmethodID getSystemClassLoader_method = env->GetStaticMethodID(class_loader_claxx, "getSystemClassLoader", sig_buffer);
	jobject class_loader = env->CallStaticObjectMethod(class_loader_claxx, getSystemClassLoader_method);
	check_value(class_loader);

	jobject dex_loader_obj = env->NewObject(dexloader_claxx, dexloader_init_method, apk_path, dex_out_path, NULL, class_loader);
	jstring class_name = env->NewStringUTF(CLASS_JAVA);
	jclass class_jia = static_cast<jclass>(env->CallObjectMethod(dex_loader_obj, loadClass_method, class_name));
	check_value(class_jia);

	jmethodID invoke_method = env->GetStaticMethodID(class_jia, "hook", "(I)V");
	check_value(invoke_method);
	env->CallStaticObjectMethod(class_jia, invoke_method, 0);
	LOGI("end hook java");
	jvm->DetachCurrentThread();
}
