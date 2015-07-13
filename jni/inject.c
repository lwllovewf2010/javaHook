/*
 ============================================================================
 Name        : libinject.c
 Author      :  
 Version     :
 Copyright   : 
 Description : Android shared library inject helper
 ============================================================================
 */
#include <stdio.h>
#include <fcntl.h>
#include <elf.h>
#include <android/log.h>

#include "libinject.h"

#ifndef O_BINARY
#define O_BINARY 0
#endif

#define LOGV(fmt, args...) __android_log_print(ANDROID_LOG_VERBOSE, "test", fmt, ##args)


int main(int argc, char** argv) {
	pid_t target_pid;
	target_pid = find_pid_of("system_server");
	if(inject_remote_process(target_pid, "/system/lib/libtest.so", "java_hook_test", "I'm parameter!", strlen("I'm parameter!")) == 0)
		LOGV("inject success");
	else
		LOGV("inject wrong");
	return 0;
}
