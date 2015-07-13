#ifndef COMMON_H_
#define COMMON_H_

#include <cutils/log.h>
#include <stdlib.h>

#ifdef LOG_TAG
#undef LOG_TAG
#define LOG_TAG "test"
#endif

#ifdef DEBUG
#define LOGI(...) ALOGI(__VA_ARGS__)
#define LOGE(...) ALOGE(__VA_ARGS__)
#define LOGW(...) ALOGW(__VA_ARGS__)
#else
#define LOGI(...) while(0){}
#define LOGE(...) while(0){}
#define LOGW(...) while(0){}
#endif

#define check_value(x) if (NULL == x) {LOGE("NULL =="#x);}

#define CHECK_VALID(V) 				\
	if(V == NULL){					\
		LOGE("%s is null.", #V);	\
		exit(-1);					\
	}else{							\
		LOGI("%s is %p.", #V, V);	\
	}								\

#endif /* COMMON_H_ */
