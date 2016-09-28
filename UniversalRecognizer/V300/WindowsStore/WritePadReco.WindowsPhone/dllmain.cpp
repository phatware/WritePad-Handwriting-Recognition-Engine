#undef LANG_DAN
#undef LANG_ENGLISH
#undef LANG_FINNISH
#undef LANG_SPANISH
#undef LANG_FRENCH
#undef LANG_ITALIAN
#undef LANG_GERMAN
#undef LANG_DUTCH
#undef LANG_PORTUGUESE
#undef LANG_INDONESIAN

#include <windows.h>

BOOL APIENTRY DllMain(HMODULE /* hModule */, DWORD ul_reason_for_call, LPVOID /* lpReserved */)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}
