<pre name="code" class="html"># javaHook
Extract the part of JavaHook in the Xposed. Usage: Inject one process to load libtest.so, then enter the Java world and hook the java function.

1、Choose a process that you want to inject.
    (jni/inject.c)
    target_pid = find_pid_of(&quot;system_server&quot;);
    if(inject_remote_process(target_pid, &quot;/system/lib/libtest.so&quot;, &quot;java_hook_test&quot;, &quot;I'm parameter!&quot;, strlen(&quot;I'm parameter!&quot;)) == 0)
		  LOGV(&quot;inject success&quot;);
		
2、Attention the attribute of dex_out_path.
    (jni/libtest.cpp)
    For example, system_server can write in the folder such as &quot;/data/dalvik-cache&quot; because the owner is system.
  
3、Add the function you want to hook.
    (Hook.java)
    Bridge.hookAllMethods(Helpers.findClass(&quot;com.android.internal.policy.impl.PhoneWindowManager&quot;), &quot;interceptKeyBeforeQueueing&quot;);
  
4、Add the beforeHookedMethod and afterHookedMethod like Xposed.
    (Hook.java)
    static XC_MethodHook cb_interceptKeyBeforeQueueing = new XC_MethodHook() {
  		@Override
  		protected void beforeHookedMethod(MethodHookParam param)
  				throws Throwable {
  			Log.v(&quot;test&quot;, &quot;[KeyPress]&quot; + param.args[0] + &quot; | &quot; + param.args[1]
  					+ &quot; | &quot; + param.args[2]);
  		}
  	};

5、Call &quot;before method&quot; and &quot;after method&quot; in handleHookedMethod.
    (Bridge.java)
    if (method.getName().equals(&quot;interceptKeyBeforeQueueing&quot;))
		  Hook.cb_interceptKeyBeforeQueueing.beforeHookedMethod(param);	
</pre>
<br />
![image](https://github.com/phoebe1990/javaHook/blob/master/1.jpg)
