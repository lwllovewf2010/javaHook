package com.phoebe.javaHook;

import com.phoebe.javaHook.Bridge;
import com.phoebe.javaHook.Helpers;
import com.phoebe.javaHook.XC_MethodHook;
import android.util.Log;

public class Hook {
	
	static XC_MethodHook cb_interceptKeyBeforeQueueing = new XC_MethodHook() {

		@Override
		protected void beforeHookedMethod(MethodHookParam param)
				throws Throwable {
			Log.v("test", "[KeyPress]" + param.args[0] + " | " + param.args[1]
					+ " | " + param.args[2]);
		}
	};
	
	public static void hook(int i) {
		Log.v("test", "--------I am in hook--------");
		try {
			Bridge.hookAllMethods(Helpers.findClass("com.android.internal.policy.impl.PhoneWindowManager"), 
					"interceptKeyBeforeQueueing");
		} catch (Exception e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
			Log.v("test", e.getMessage());
		}
		Log.v("test", "--------hook success--------");
	}
}
