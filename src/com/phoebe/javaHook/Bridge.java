package com.phoebe.javaHook;

import java.lang.reflect.Constructor;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Member;
import java.lang.reflect.Method;
import java.lang.reflect.Modifier;

import com.phoebe.javaHook.XC_MethodHook.MethodHookParam;

import android.util.Log;

public class Bridge {
	public static final ClassLoader BOOTCLASSLOADER = ClassLoader.getSystemClassLoader();
	static {
		// º”‘ÿ∂ØÃ¨ø‚
		try {
			System.load("/system/lib/libtest.so");		
			Log.v("test", "Bridge load libtest.so");
			if(!initNative())
				Log.v("test", "Bridge init Native failed");
		} catch (Throwable e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
			Log.v("test", e.getMessage());
		}
	}
	
	public static void hookAllMethods(Class<?> hookClass, String methodName) {		
		
		for (Member method : hookClass.getDeclaredMethods())
		{
			//Log.v("test", hookClass.getName() + "." + method.getName());
			if (method.getName().equals(methodName)){
				Log.v("test", "hook:" + methodName);
				hookMethod(method);
			}
		}
	}
	/**
	 * Hook any method with the specified callback
	 *
	 * @param hookMethod The method to be hooked
	 * @param callback
	 */
	public static void hookMethod(Member hookMethod) {
		Log.v("test", "enter hookMethod");
		if (!(hookMethod instanceof Method) && !(hookMethod instanceof Constructor<?>)) {
			//throw new IllegalArgumentException("Only methods and constructors can be hooked: " + hookMethod.toString());
			Log.v("test", "Only methods and constructors can be hooked: " + hookMethod.toString());
		} else if (hookMethod.getDeclaringClass().isInterface()) {
			//throw new IllegalArgumentException("Cannot hook interfaces: " + hookMethod.toString());
			Log.v("test", "Cannot hook interfaces: " + hookMethod.toString());
		} else if (Modifier.isAbstract(hookMethod.getModifiers())) {
			//throw new IllegalArgumentException("Cannot hook abstract methods: " + hookMethod.toString());
			Log.v("test", "Cannot hook abstract methods: " + hookMethod.toString());
		}


		Class<?> declaringClass = hookMethod.getDeclaringClass();
		int slot = (int) Helpers.getIntField(hookMethod, "slot");

		Class<?>[] parameterTypes;
		Class<?> returnType;
		if (hookMethod instanceof Method) {
			parameterTypes = ((Method) hookMethod).getParameterTypes();
			returnType = ((Method) hookMethod).getReturnType();
		} else {
			parameterTypes = ((Constructor<?>) hookMethod).getParameterTypes();
			returnType = null;
		}

		AdditionalHookInfo additionalInfo = new AdditionalHookInfo(parameterTypes, returnType);
		hookMethodNative(hookMethod, declaringClass, slot, additionalInfo);
	}
	/**
	 * This method is called as a replacement for hooked methods.
	 */
	private static Object handleHookedMethod(Member method, int originalMethodId, Object additionalInfoObj,
			Object thisObject, Object[] args) throws Throwable  {
		//Log.v("test", "enter handleHookedMethod");
		
		AdditionalHookInfo additionalInfo = (AdditionalHookInfo) additionalInfoObj;

		MethodHookParam param = new MethodHookParam();
		param.method = method;
		param.thisObject = thisObject;
		param.args = args;

		// call "before method" callbacks
		try {
			if (method.getName().equals("interceptKeyBeforeQueueing"))
				Hook.cb_interceptKeyBeforeQueueing.beforeHookedMethod(param);	
		} catch (Throwable t) {
			Log.v("test", t.getMessage());
			// reset result (ignoring what the unexpectedly exiting callback did)
			param.setResult(null);
			param.returnEarly = false;
		}

		// call original method if not requested otherwise
		if (!param.returnEarly) {
			try {
				param.setResult(invokeOriginalMethodNative(method, originalMethodId,
					additionalInfo.parameterTypes, additionalInfo.returnType, param.thisObject, param.args));
			} catch (InvocationTargetException e) {
				param.setThrowable(e.getCause());
			}
		}

		// call "after method" callbacks

		Object lastResult =  param.getResult();
		Throwable lastThrowable = param.getThrowable();

		try {
		//	if (method.getName().equals("init"))
		//		Hook.cb_init.afterHookedMethod(param);
		} catch (Throwable t) {
			Log.v("test", t.getMessage());
			// reset to last result (ignoring what the unexpectedly exiting callback did)
			if (lastThrowable == null)
				param.setResult(lastResult);
			else
				param.setThrowable(lastThrowable);
		}

		// return
		if (param.hasThrowable())
			throw param.getThrowable();
		else
			return param.getResult();
	}
	private static class AdditionalHookInfo {
		final Class<?>[] parameterTypes;
		final Class<?> returnType;

		private AdditionalHookInfo(Class<?>[] parameterTypes, Class<?> returnType) {
			this.parameterTypes = parameterTypes;
			this.returnType = returnType;
		}
	}
	
	private native static boolean initNative();
	
	/**
	 * Intercept every call to the specified method and call a handler function instead.
	 * @param method The method to intercept
	 */
	private native synchronized static void hookMethodNative(Member method, Class<?> declaringClass, int slot, Object additionalInfo);
	
	private native static Object invokeOriginalMethodNative(Member method, int methodId,
			Class<?>[] parameterTypes, Class<?> returnType, Object thisObject, Object[] args)
			throws IllegalAccessException, IllegalArgumentException, InvocationTargetException;

}
