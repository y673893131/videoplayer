#ifndef APIDEF_H
#define APIDEF_H

#define C_API_DEFINE_BEGIN extern "C" {
#define C_APC_DEFINE_END }

#define C_API_MODULE_DEFINE(module) \
    static HMODULE module = nullptr;

#define C_API_DEFINE(ret, name, ...) \
    typedef ret (*name##_c)(__VA_ARGS__);\
    static name##_c name = nullptr;

#define C_API_MODULE_LOAD(space, name) \
    space::h = LoadLibraryA(#name);

#define C_API_MODULE_FREE(h) \
    if(h) {FreeLibrary(h); h = nullptr;}


#define C_API_LOAD_FUNC(name) \
    name = reinterpret_cast<name##_c>(::GetProcAddress(h, #name));

#define C_API_CALL_FUNC(errorRet, space, name, ...) \
    if(space::name) {return space::name(__VA_ARGS__); } else { return errorRet;}

#endif // APIDEF_H
