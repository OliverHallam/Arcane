// The following ifdef block is the standard way of creating macros which make exporting
// from a DLL simpler. All files within this DLL are compiled with the LIBRETRO_EXPORTS
// symbol defined on the command line. This symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see
// LIBRETRO_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef LIBRETRO_EXPORTS
#define LIBRETRO_API __declspec(dllexport)
#else
#define LIBRETRO_API __declspec(dllimport)
#endif

// This class is exported from the dll
class LIBRETRO_API CLibRetro {
public:
	CLibRetro(void);
	// TODO: add your methods here.
};

extern LIBRETRO_API int nLibRetro;

LIBRETRO_API int fnLibRetro(void);
