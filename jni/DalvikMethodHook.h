#ifdef __cplusplus
extern "C" {
#endif

#include <jni.h>
#include <stddef.h>
#include <object.h>
#include <elf.h>

struct XposedHookInfo {
    struct {
        Method originalMethod;
        // copy a few bytes more than defined for Method in AOSP
        // to accomodate for (rare) extensions by the target ROM
        int dummyForRomExtensions[4];
    } originalMethodStruct;

    Object* reflectedMethod;
    Object* additionalInfo;
};

inline void setObjectArrayElement(const ArrayObject* obj, int index, Object* val);
void hookedMethodCallback(const u4* args, JValue* pResult, const Method* method, ::Thread* self);
inline bool isMethodHooked(const Method* method);
void Bridge_hookMethodNative(
		JNIEnv* env, jclass clazz, jobject reflectedMethodIndirect,
            jobject declaredClassIndirect, jint slot, jobject additionalInfoIndirect);
void Bridge_invokeOriginalMethodNative(const u4* args, JValue* pResult,
            const Method* method, ::Thread* self);
jboolean Bridge_initNative(JNIEnv* env, jclass clazz);

JNIEXPORT void JNICALL Java_com_example_jiaguanji_Bridge_hookMethodNative(JNIEnv* env, jclass clazz, jobject reflectedMethodIndirect,
            jobject declaredClassIndirect, jint slot, jobject additionalInfoIndirect);
JNIEXPORT jint JNICALL JNI_OnLoad
  (JavaVM* vm, void* reserved);

#ifdef __cplusplus
}
#endif
