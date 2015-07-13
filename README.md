# javaHook
Extract the part of JavaHook in the Xposed. Usage: Inject one process to load libtest.so, then enter the Java world and hook the java function.

1、Choose a process that you want to inject.
    (jni/inject.c)
    target_pid = find_pid_of("system_server");
    if(inject_remote_process(target_pid, "/system/lib/libtest.so", "java_hook_test", "I'm parameter!", strlen("I'm parameter!")) == 0)
		  LOGV("inject success");
		
2、Attention the attribute of dex_out_path.
    (jni/libtest.cpp)
    For example, system_server can write in the folder such as "/data/dalvik-cache" because the owner is system.
  
3、Add the function you want to hook.
    (Hook.java)
    Bridge.hookAllMethods(Helpers.findClass("com.android.internal.policy.impl.PhoneWindowManager"), "interceptKeyBeforeQueueing");
  
4、Add the beforeHookedMethod and afterHookedMethod like Xposed.
    (Hook.java)
    static XC_MethodHook cb_interceptKeyBeforeQueueing = new XC_MethodHook() {
  		@Override
  		protected void beforeHookedMethod(MethodHookParam param)
  				throws Throwable {
  			Log.v("test", "[KeyPress]" + param.args[0] + " | " + param.args[1]
  					+ " | " + param.args[2]);
  		}
  	};

5、Call "before method" and "after method" in handleHookedMethod.
    (Bridge.java)
    if (method.getName().equals("interceptKeyBeforeQueueing"))
		  Hook.cb_interceptKeyBeforeQueueing.beforeHookedMethod(param);	
