#include "common.h"
#include "dvm.h"
#include "DalvikMethodHook.h"

#ifndef CLASS_BRIDGE
#define CLASS_BRIDGE "com.phoebe.javaHook.Bridge"
#endif
#ifndef NATIVE_METHOD
#define NATIVE_METHOD(className, functionName, signature) \
  { #functionName, signature, reinterpret_cast<void*>(className ## _ ## functionName) }
#endif

#define NELEM(x) ((int) (sizeof(x) / sizeof((x)[0])))

static Method* HandleHookedMethod = NULL;
static ClassObject* objectArrayClass = NULL;
static size_t arrayContentsOffset = 0;
static jclass classBridge = NULL;

/** Portable clone of dvmSetObjectArrayElement() */
inline void setObjectArrayElement(const ArrayObject* obj, int index, Object* val)
{
	uintptr_t arrayContents = (uintptr_t)obj + arrayContentsOffset;
    ((Object **)arrayContents)[index] = val;
    dvmWriteBarrierArray(obj, index, index + 1);
}

/** This is called when a hooked method is executed. */
void hookedMethodCallback(const u4* args, JValue* pResult, const Method* method, ::Thread* self)
{
	//LOGI("hookedMethodCallback");
    XposedHookInfo* hookInfo = (XposedHookInfo*) method->insns;
    Method* original = (Method*) hookInfo;
    Object* originalReflected = hookInfo->reflectedMethod;
    Object* additionalInfo = hookInfo->additionalInfo;

    // convert/box arguments
    const char* desc = &method->shorty[1]; // [0] is the return type.
    Object* thisObject = NULL;
    size_t srcIndex = 0;
    size_t dstIndex = 0;

    // for non-static methods determine the "this" pointer
    if (!dvmIsStaticMethod(original)) {
        thisObject = (Object*) args[0];
        srcIndex++;
    }

    ArrayObject* argsArray = dvmAllocArrayByClass(objectArrayClass, strlen(method->shorty) - 1, ALLOC_DEFAULT);
    if (argsArray == NULL) {
        return;
    }

    while (*desc != '\0') {
        char descChar = *(desc++);
        JValue value;
        Object* obj;

        switch (descChar) {
        case 'Z':
        case 'C':
        case 'F':
        case 'B':
        case 'S':
        case 'I':
            value.i = args[srcIndex++];
            obj = (Object*) dvmBoxPrimitive(value, dvmFindPrimitiveClass(descChar));
            dvmReleaseTrackedAlloc(obj, self);
            break;
        case 'D':
        case 'J':
            value.j = dvmGetArgLong(args, srcIndex);
            srcIndex += 2;
            obj = (Object*) dvmBoxPrimitive(value, dvmFindPrimitiveClass(descChar));
            dvmReleaseTrackedAlloc(obj, self);
            break;
        case '[':
        case 'L':
            obj  = (Object*) args[srcIndex++];
            break;
        default:
            ALOGE("Unknown method signature description character: %c", descChar);
            obj = NULL;
            srcIndex++;
        }
        setObjectArrayElement(argsArray, dstIndex++, obj);
    }

    // call the Java handler function
    JValue result;
    dvmCallMethod(self, HandleHookedMethod, NULL, &result,
        originalReflected, (int) original, additionalInfo, thisObject, argsArray);

    dvmReleaseTrackedAlloc(argsArray, self);

    // exceptions are thrown to the caller
    if (dvmCheckException(self)) {
        return;
    }

    // return result with proper type
    ClassObject* returnType = dvmGetBoxedReturnType(method);
    if (returnType->primitiveType == PRIM_VOID) {
        // ignored
    } else if (result.l == NULL) {
        if (dvmIsPrimitiveClass(returnType)) {
            LOGE("null result when primitive expected");
        }
        pResult->l = NULL;
    } else {
        if (!dvmUnboxPrimitive((Object*)result.l, returnType, pResult)) {
            //dvmThrowClassCastException(result.l->clazz, returnType);
            LOGE("result.l cannot be cast to %s", returnType);
        }
    }
}
/** Check whether a method is already hooked. */
inline bool isMethodHooked(const Method* method) {
    return (method->nativeFunc == &hookedMethodCallback);
}
void Bridge_hookMethodNative(
		JNIEnv* env, jclass clazz, jobject reflectedMethodIndirect,
            jobject declaredClassIndirect, jint slot, jobject additionalInfoIndirect)
{
	//LOGI("Bridge_hookMethodNative");
    // Usage errors?
    if (declaredClassIndirect == NULL || reflectedMethodIndirect == NULL) {
    	LOGE("method and declaredClass must not be null");
        return;
    }

    // Find the internal representation of the method
    ClassObject* declaredClass = (ClassObject*) dvmDecodeIndirectRef(dvmThreadSelf(), declaredClassIndirect);
    Method* method = dvmSlotToMethod(declaredClass, slot);
    if (method == NULL) {
    	LOGE("Could not get internal representation for method");
        return;
    }
/*
    if (isMethodHooked(method)) {
        // already hooked
        return;
    }*/
    if(method->nativeFunc == hookedMethodCallback){
    	LOGE("[*] method had been hooked");
    	return;
    }

    // Save a copy of the original method and other hook info
    XposedHookInfo* hookInfo = (XposedHookInfo*) calloc(1, sizeof(XposedHookInfo));
    memcpy(hookInfo, method, sizeof(hookInfo->originalMethodStruct));
    hookInfo->reflectedMethod = dvmDecodeIndirectRef(dvmThreadSelf(), env->NewGlobalRef(reflectedMethodIndirect));
    hookInfo->additionalInfo = dvmDecodeIndirectRef(dvmThreadSelf(), env->NewGlobalRef(additionalInfoIndirect));

    // Replace method with our own code
    SET_METHOD_FLAG(method, ACC_NATIVE);
    method->insns = (const u2*) hookInfo;
    method->registersSize = method->insSize;
    method->outsSize = 0;
    method->nativeFunc = &hookedMethodCallback;

    LOGI("[+] method was hooked");
/*
    if (PTR_gDvmJit != NULL) {
        // reset JIT cache
        char currentValue = *((char*)PTR_gDvmJit + MEMBER_OFFSET_VAR(DvmJitGlobals,codeCacheFull));
        if (currentValue == 0 || currentValue == 1) {
            MEMBER_VAL(PTR_gDvmJit, DvmJitGlobals, codeCacheFull) = true;
        } else {
            ALOGE("Unexpected current value for codeCacheFull: %d", currentValue);
        }
    }*/
}

/**
 * Simplified copy of Method.invokeNative(), but calls the original (non-hooked) method
 * and has no access checks. Used to call the real implementation of hooked methods.
 */
void Bridge_invokeOriginalMethodNative(const u4* args, JValue* pResult,
            const Method* method, ::Thread* self)
{
	//LOGI("Bridge_invokeOriginalMethodNative");
    Method* meth = (Method*) args[1];
    if (meth == NULL) {
        meth = dvmGetMethodFromReflectObj((Object*) args[0]);
        if (isMethodHooked(meth)) {
            meth = (Method*) meth->insns;
        }
    }
    ArrayObject* params = (ArrayObject*) args[2];
    ClassObject* returnType = (ClassObject*) args[3];
    Object* thisObject = (Object*) args[4]; // null for static methods
    ArrayObject* argList = (ArrayObject*) args[5];

    // invoke the method
    pResult->l = dvmInvokeMethod(thisObject, meth, argList, params, returnType, true);
    return;
}
jboolean Bridge_initNative(JNIEnv* env, jclass clazz)
{
	LOGI("Bridge_initNative");
	HandleHookedMethod = (Method*) env->GetStaticMethodID(classBridge, "handleHookedMethod",
		"(Ljava/lang/reflect/Member;ILjava/lang/Object;Ljava/lang/Object;[Ljava/lang/Object;)Ljava/lang/Object;");
	if (HandleHookedMethod == NULL) {
		LOGE("ERROR: could not find method %s.handleHookedMethod(Member, int, Object, Object, Object[])", CLASS_BRIDGE);
		env->ExceptionClear();
		return false;
	}

    Method* InvokeOriginalMethodNative = (Method*) env->GetStaticMethodID(classBridge, "invokeOriginalMethodNative",
        "(Ljava/lang/reflect/Member;I[Ljava/lang/Class;Ljava/lang/Class;Ljava/lang/Object;[Ljava/lang/Object;)Ljava/lang/Object;");
    if (InvokeOriginalMethodNative == NULL) {
        LOGE("ERROR: could not find method %s.invokeOriginalMethodNative(Member, int, Class[], Class, Object, Object[])", CLASS_BRIDGE);
        env->ExceptionClear();
        return false;
    }
    dvmSetNativeFunc(InvokeOriginalMethodNative, Bridge_invokeOriginalMethodNative, NULL);

    objectArrayClass = dvmFindArrayClass("[Ljava/lang/Object;", NULL);
    if (objectArrayClass == NULL) {
        LOGE("Error while loading Object[] class");
        env->ExceptionClear();
        return false;
    }

    return true;
}

JNIEXPORT jint JNICALL JNI_OnLoad
  (JavaVM* vm, void* reserved)
{
	LOGI("JNI_OnLoad");

	JNIEnv *env = NULL;
	if(vm->GetEnv((void **)&env, JNI_VERSION_1_4)!= JNI_OK)
		return -1;

	// detect offset of ArrayObject->contents
	jintArray dummyArray = env->NewIntArray(1);
	if (dummyArray == NULL) {
		LOGE("Could allocate int array for testing");
		env->ExceptionClear();
	    //return;
	}
	jint* dummyArrayElements = env->GetIntArrayElements(dummyArray, NULL);
	arrayContentsOffset = (size_t)dummyArrayElements - (size_t)dvmDecodeIndirectRef(dvmThreadSelf(), dummyArray);
	env->ReleaseIntArrayElements(dummyArray,dummyArrayElements, 0);
	env->DeleteLocalRef(dummyArray);

	if (arrayContentsOffset < 12 || arrayContentsOffset > 128) {
	    LOGE("Detected strange offset %d of ArrayObject->contents", arrayContentsOffset);
	    //return;
	}

	classBridge = env->FindClass(CLASS_BRIDGE);
	classBridge = reinterpret_cast<jclass>(env->NewGlobalRef(classBridge));

	if (classBridge == NULL) {
		LOGE("Error while loading Xposed class '%s':", CLASS_BRIDGE);
	    env->ExceptionClear();
	}
	LOGI("Found class '%s', now initializing", CLASS_BRIDGE);
	const JNINativeMethod methods[] = {
	        NATIVE_METHOD(Bridge, initNative, "()Z"),
	        NATIVE_METHOD(Bridge, hookMethodNative, "(Ljava/lang/reflect/Member;Ljava/lang/Class;ILjava/lang/Object;)V"),
//	#ifdef ART_TARGET
	        NATIVE_METHOD(Bridge, invokeOriginalMethodNative,
	            "(Ljava/lang/reflect/Member;I[Ljava/lang/Class;Ljava/lang/Class;Ljava/lang/Object;[Ljava/lang/Object;)Ljava/lang/Object;"),
//	#endif
	};
	if (env->RegisterNatives(classBridge, methods, NELEM(methods))!= JNI_OK)
	{
		LOGE("Could not register natives for '%s'", CLASS_BRIDGE);
	    env->ExceptionClear();
	}
	return JNI_VERSION_1_4;
}
