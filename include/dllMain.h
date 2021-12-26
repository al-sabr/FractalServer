#pragma once

#include "cross-platform.h"
#include "FlashRuntimeExtensions.h"

#include <iostream>

#include <restinio/all.hpp>
#include <restinio/tls.hpp>



extern "C"
{
	EXPORTED void FractalServerExtensionInitializer(void** extData, FREContextInitializer* ctxInitializer, FREContextFinalizer* ctxFinalizer);

	EXPORTED void FractalServerExtensionFinalizer(void* extData);
}