#include "dllMain.h"

FREContext ctx;

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
	#include "win32.h"
#endif

void contextInitializer(void *extData, const uint8_t * ctxType, FREContext aneCtx, uint32_t * numFunctionsToSet, const FRENamedFunction ** functionsToSet)
{
	ctx = aneCtx;

	// Create mapping between function names and pointers in an array of FRENamedFunction.
	// These are the functions that you will call from ActionScript -
	// effectively the interface of your native library.
	// Each member of the array contains the following information:
	// { function name as it will be called from ActionScript,
	//   any data that should be passed to the function,
	//   a pointer to the implementation of the function in the native library }
	static FRENamedFunction extensionFunctions[] =
	{
		{}
		//{ (const uint8_t*)"coreInitialization", NULL, &coreInitialization },
	};

	// Tell AIR how many functions there are in the array:
	*numFunctionsToSet = sizeof(extensionFunctions) / sizeof(FRENamedFunction);

	// Set the output parameter to point to the array we filled in:
	*functionsToSet = extensionFunctions;
}

void contextFinalizer(FREContext ctx)
{
	return;
}

//extern "C" {

	void FractalServerExtensionInitializer(void** extData, FREContextInitializer* ctxInitializer, FREContextFinalizer* ctxFinalizer){
		*ctxInitializer = &contextInitializer; // The name of function that will intialize the extension context
		*ctxFinalizer = &contextFinalizer; // The name of function that will finalize the extension context
	}

	void FractalServerExtensionFinalizer(void* extData){
		return;
	}

//}

