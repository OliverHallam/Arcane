// LibRetro.cpp : Defines the exported functions for the DLL.
//

#include "pch.h"
#include "framework.h"
#include "LibRetro.h"


// This is an example of an exported variable
LIBRETRO_API int nLibRetro=0;

// This is an example of an exported function.
LIBRETRO_API int fnLibRetro(void)
{
    return 0;
}

// This is the constructor of a class that has been exported.
CLibRetro::CLibRetro()
{
    return;
}
