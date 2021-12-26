#include "includes.h"

extern "C"
{
	EXPORTED void FractalServerExtensionInitializer(void** extData, FREContextInitializer* ctxInitializer, FREContextFinalizer* ctxFinalizer);

	EXPORTED void FractalServerExtensionFinalizer(void* extData);
}