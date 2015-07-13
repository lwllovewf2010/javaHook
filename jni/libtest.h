
#ifdef __cplusplus
extern "C" {
#endif

void java_hook_test(const char* s);
void* _hookJava(void*);
void HookJava();

#ifdef __cplusplus
}
#endif
