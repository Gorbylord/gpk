#include "gpk_log.h"
#include "gpk_runtime.h"
#include "gpk_module.h"
#include "gpk_view_array.h"

#ifndef GPK_RUNTIME_MODULE_H_23084728934
#define GPK_RUNTIME_MODULE_H_23084728934

GPK_DECLARE_MODULE_FUNCTION(gpk_moduleTitle		, char_t* outputBuffer, uint8_t* outputBufferLenght					);
GPK_DECLARE_MODULE_FUNCTION(gpk_moduleVersion	, uint32_t* version													);
GPK_DECLARE_MODULE_FUNCTION(gpk_moduleCreate	, void** customApplication, ::gpk::SRuntimeValues* runtimeValues	);
GPK_DECLARE_MODULE_FUNCTION(gpk_moduleDelete	, void** customApplication											);
GPK_DECLARE_MODULE_FUNCTION(gpk_moduleSetup		, void*	 customApplication											);
GPK_DECLARE_MODULE_FUNCTION(gpk_moduleCleanup	, void*	 customApplication											);
GPK_DECLARE_MODULE_FUNCTION(gpk_moduleRender	, void*	 customApplication											);
GPK_DECLARE_MODULE_FUNCTION(gpk_moduleUpdate	, void*	 customApplication, bool requestedExit						);

namespace gpk 
{
#pragma pack(push, 1)
	struct SRuntimeModule : public SModuleBase {
		GPK_MODULE_FUNCTION_NAME(gpk_moduleTitle	)				Title						= 0;
		GPK_MODULE_FUNCTION_NAME(gpk_moduleVersion	)				Version						= 0;
		GPK_MODULE_FUNCTION_NAME(gpk_moduleCreate	)				Create						= 0;
		GPK_MODULE_FUNCTION_NAME(gpk_moduleDelete	)				Delete						= 0;
		GPK_MODULE_FUNCTION_NAME(gpk_moduleSetup	)				Setup						= 0;
		GPK_MODULE_FUNCTION_NAME(gpk_moduleCleanup	)				Cleanup						= 0;
		GPK_MODULE_FUNCTION_NAME(gpk_moduleRender	)				Render						= 0;
		GPK_MODULE_FUNCTION_NAME(gpk_moduleUpdate	)				Update						= 0;

		void														* Application;
	};
#pragma pack(pop)

	::gpk::error_t												loadRuntimeModule			(::gpk::SRuntimeModule& loadedModule, const ::gpk::view_const_string& moduleName);


#define	GPK_LOAD_MODULE(moduleName)								LoadLibraryA	(moduleName)
#define	GPK_LOAD_MODULE_FUNCTION(moduleHandle, symbolName)		((GPK_MODULE_FUNCTION_NAME(symbolName))GetProcAddress((HMODULE)moduleHandle, #symbolName))

#if defined GPK_WINDOWS
#	define GPK_MODULE_EXTENSION "dll"
#else
#	define GPK_MODULE_EXTENSION "os"
#endif
}

#endif // GPK_RUNTIME_MODULE_H_23084728934
