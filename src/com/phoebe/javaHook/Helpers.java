package com.phoebe.javaHook;

import org.apache.commons.lang3.ClassUtils;
import java.lang.reflect.Field;
import java.util.HashMap;
import android.util.Log;


public class Helpers {
	private static final HashMap<String, Field> fieldCache = new HashMap<String, Field>();
	public static ClassLoader classLoader = null;
	/**
	 * Look up a class with the specified class loader (or the boot class loader if
	 * <code>classLoader</code> is <code>null</code>).
	 * <p>Class names can be specified in different formats:
	 * <ul><li>java.lang.Integer
	 * <li>int
	 * <li>int[]
	 * <li>[I
	 * <li>java.lang.String[]
	 * <li>[Ljava.lang.String;
	 * <li>android.app.ActivityThread.ResourcesKey
	 * <li>android.app.ActivityThread$ResourcesKey
	 * <li>android.app.ActivityThread$ResourcesKey[]</ul>
	 * <p>A {@link ClassNotFoundError} is thrown in case the class was not found.
	 */
	public static Class<?> findClass(String className) {
		Log.v("test", className);
		if (classLoader == null)
			classLoader = Bridge.BOOTCLASSLOADER;
		try {
			return ClassUtils.getClass(classLoader, className, false);
		} catch (ClassNotFoundException e) {
			throw new ClassNotFoundError(e);
		}
	}
	public static class ClassNotFoundError extends Error {
		private static final long serialVersionUID = -1070936889459514628L;
		public ClassNotFoundError(Throwable cause) {
			super(cause);
		}
		public ClassNotFoundError(String detailMessage, Throwable cause) {
			super(detailMessage, cause);
		}
	}
	public static int getIntField(Object obj, String fieldName) {
		try {
			return findField(obj.getClass(), fieldName).getInt(obj);
		} catch (IllegalAccessException e) {
			// should not happen
			//XposedBridge.log(e);
			Log.v("test", e.getMessage());
			throw new IllegalAccessError(e.getMessage());
		} catch (IllegalArgumentException e) {
			throw e;
		}
	}
	/**
	 * Look up a field in a class and set it to accessible. The result is cached.
	 * If the field was not found, a {@link NoSuchFieldError} will be thrown.
	 */
	public static Field findField(Class<?> clazz, String fieldName) {
		StringBuilder sb = new StringBuilder(clazz.getName());
		sb.append('#');
		sb.append(fieldName);
		String fullFieldName = sb.toString();

		if (fieldCache.containsKey(fullFieldName)) {
			Field field = fieldCache.get(fullFieldName);
			if (field == null)
				throw new NoSuchFieldError(fullFieldName);
			return field;
		}

		try {
			Field field = findFieldRecursiveImpl(clazz, fieldName);
			field.setAccessible(true);
			fieldCache.put(fullFieldName, field);
			return field;
		} catch (NoSuchFieldException e) {
			fieldCache.put(fullFieldName, null);
			throw new NoSuchFieldError(fullFieldName);
		}
	}
	private static Field findFieldRecursiveImpl(Class<?> clazz, String fieldName) throws NoSuchFieldException {
		try {
			return clazz.getDeclaredField(fieldName);
		} catch (NoSuchFieldException e) {
			while (true) {
				clazz = clazz.getSuperclass();
				if (clazz == null || clazz.equals(Object.class))
					break;

				try {
					return clazz.getDeclaredField(fieldName);
				} catch (NoSuchFieldException ignored) {}
			}
			throw e;
		}
	}
	
	public static Object getObjectField(Object obj, String fieldName) {
		try {
			return findField(obj.getClass(), fieldName).get(obj);
		} catch (IllegalAccessException e) {
			// should not happen
			Log.v("test", e.getMessage());
			throw new IllegalAccessError(e.getMessage());
		} catch (IllegalArgumentException e) {
			throw e;
		}
	}
}
