#pragma once
#ifdef __linux__

#define __fastcall
#define __declspec(x)
#define ASSEMBLY_BEGIN(x) static_assert(!"compiler not supported")
#define ASSEMBLY_END(x) ASSEMBLY_BEGIN(x)


#ifdef _MCS_VER
    #undef __fastcall
    #undef __declspec
#endif
#endif