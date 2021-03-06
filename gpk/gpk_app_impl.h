/// Copyright 2017-2018 - asm128
#include "gpk_runtime.h"
#include "gpk_ptr.h"
#include "gpk_module.h"

#if defined GPK_AVOID_LOCAL_APPLICATION_MODULE_MODEL_EXECUTABLE_RUNTIME
#	define GPK_DEFINE_APPLICATION_ENTRY_POINT(_mainClass, _moduleTitle)																																																																							\
		::gpk::error_t																				setup										(_mainClass& applicationInstance);																																												\
		::gpk::error_t																				cleanup										(_mainClass& applicationInstance);																																												\
		::gpk::error_t																				update										(_mainClass& applicationInstance, bool systemRequestedExit);																																					\
		::gpk::error_t																				draw										(_mainClass& applicationInstance);																																												\
		::gpk::error_t	GPK_STDCALL																	gpk_moduleVersion							()															{ return 1; }																																		\
		::gpk::error_t	GPK_STDCALL																	gpk_moduleCreate							(void**	instanceApp, ::gpk::SRuntimeValues* runtimeValues)	{ try { *instanceApp = new _mainClass{*runtimeValues};												return 0;		} catch(...) {} return -1; }	\
		::gpk::error_t	GPK_STDCALL																	gpk_moduleDelete							(void**	instanceApp)										{ try { delete ((_mainClass*)*instanceApp);	*instanceApp = 0;										return 0;		} catch(...) {} return -1; }	\
		::gpk::error_t	GPK_STDCALL																	gpk_moduleSetup								(void*	instanceApp)										{ try { const ::gpk::error_t result = setup		(*(_mainClass*)instanceApp);						return result;	} catch(...) {} return -1; }	\
		::gpk::error_t	GPK_STDCALL																	gpk_moduleCleanup							(void*	instanceApp)										{ try { const ::gpk::error_t result = cleanup	(*(_mainClass*)instanceApp);						return result;	} catch(...) {} return -1; }	\
		::gpk::error_t	GPK_STDCALL																	gpk_moduleUpdate							(void*	instanceApp, bool systemRequestedExit)				{ try { const ::gpk::error_t result = update	(*(_mainClass*)instanceApp, systemRequestedExit);	return result;	} catch(...) {} return -1; }	\
		::gpk::error_t	GPK_STDCALL																	gpk_moduleRender							(void*	instanceApp)										{ try { const ::gpk::error_t result = draw		(*(_mainClass*)instanceApp);						return result;	} catch(...) {} return -1; }	\
		::gpk::error_t	GPK_STDCALL																	gpk_moduleTitle								(char* out_title, uint32_t *maxCount)						{																																					\
	static constexpr const char mylmoduleTitle[] = _moduleTitle;									\
	if(0 == out_title) 																				\
		return maxCount ? (*maxCount = ::gpk::size(mylmoduleTitle)) : ::gpk::size(mylmoduleTitle);	\
	memcpy(out_title, mylmoduleTitle, ::gpk::min(::gpk::size(mylmoduleTitle), *maxCount));			\
	return 0;																						\
}	
#else

#define GPK_DEFINE_APPLICATION_ENTRY_POINT(_mainClass, _moduleTitle)																																									\
		::gpk::error_t																				setup										(_mainClass& applicationInstance);																																												\
		::gpk::error_t																				cleanup										(_mainClass& applicationInstance);																																												\
		::gpk::error_t																				update										(_mainClass& applicationInstance, bool systemRequestedExit);																																					\
		::gpk::error_t																				draw										(_mainClass& applicationInstance);																																												\
		::gpk::error_t	GPK_STDCALL																	gpk_moduleVersion							()															{ return 1; }																																		\
		::gpk::error_t	GPK_STDCALL																	gpk_moduleCreate							(void**	instanceApp, ::gpk::SRuntimeValues* runtimeValues)	{ try { *instanceApp = new _mainClass{*runtimeValues};												return 0;		} catch(...) {} return -1; }	\
		::gpk::error_t	GPK_STDCALL																	gpk_moduleDelete							(void**	instanceApp)										{ try { delete ((_mainClass*)*instanceApp);	*instanceApp = 0;										return 0;		} catch(...) {} return -1; }	\
		::gpk::error_t	GPK_STDCALL																	gpk_moduleSetup								(void*	instanceApp)										{ try { const ::gpk::error_t result = setup		(*(_mainClass*)instanceApp);						return result;	} catch(...) {} return -1; }	\
		::gpk::error_t	GPK_STDCALL																	gpk_moduleCleanup							(void*	instanceApp)										{ try { const ::gpk::error_t result = cleanup	(*(_mainClass*)instanceApp);						return result;	} catch(...) {} return -1; }	\
		::gpk::error_t	GPK_STDCALL																	gpk_moduleUpdate							(void*	instanceApp, bool systemRequestedExit)				{ try { const ::gpk::error_t result = update	(*(_mainClass*)instanceApp, systemRequestedExit);	return result;	} catch(...) {} return -1; }	\
		::gpk::error_t	GPK_STDCALL																	gpk_moduleRender							(void*	instanceApp)										{ try { const ::gpk::error_t result = draw		(*(_mainClass*)instanceApp);						return result;	} catch(...) {} return -1; }	\
		::gpk::error_t	GPK_STDCALL																	gpk_moduleTitle								(char* out_title, uint32_t *maxCount)						{																																					\
	static constexpr const char mylmoduleTitle[] = _moduleTitle;									\
	if(0 == out_title) 																				\
		return maxCount ? (*maxCount = ::gpk::size(mylmoduleTitle)) : ::gpk::size(mylmoduleTitle);	\
	memcpy(out_title, mylmoduleTitle, ::gpk::min(::gpk::size(mylmoduleTitle), *maxCount));			\
	return 0;																						\
}	\
static	::gpk::error_t																				rtMain										(::gpk::SRuntimeValues& runtimeValues);													\
		int																							main										(int argc, char *argv[], char *envp[])												{	\
	::gpk::SRuntimeValues																					runtimeValues								= {};																			\
	runtimeValues.PlatformDetail.EntryPointArgsWin														= {GetModuleHandle(NULL), 0, 0, SW_SHOW};																						\
	runtimeValues.PlatformDetail.EntryPointArgsStd														= {argc, argv, envp};																											\
	return ::gpk::failed(::rtMain(runtimeValues)) ? EXIT_FAILURE : EXIT_SUCCESS;																																						\
}																																																										\
																																																										\
		int	WINAPI																					WinMain																																\
	(	_In_		::HINSTANCE		hInstance																																															\
	,	_In_opt_	::HINSTANCE		hPrevInstance																																														\
	,	_In_		::LPSTR			lpCmdLine																																															\
	,	_In_		::INT			nShowCmd																																															\
	)																																																									\
{																																																										\
	::gpk::SRuntimeValues																					runtimeValues					= {};																						\
	runtimeValues.PlatformDetail.EntryPointArgsWin.hInstance											= hInstance		;																												\
	runtimeValues.PlatformDetail.EntryPointArgsWin.hPrevInstance										= hPrevInstance	;																												\
	runtimeValues.PlatformDetail.EntryPointArgsWin.lpCmdLine											= lpCmdLine		;																												\
	runtimeValues.PlatformDetail.EntryPointArgsWin.nShowCmd												= nShowCmd		;																												\
	runtimeValues.PlatformDetail.EntryPointArgsStd.argc													= __argc;																														\
	runtimeValues.PlatformDetail.EntryPointArgsStd.argv													= __argv;																														\
	runtimeValues.PlatformDetail.EntryPointArgsStd.envp													= _environ;																														\
	return ::gpk::failed(::rtMain(runtimeValues)) ? EXIT_FAILURE : EXIT_SUCCESS;																																						\
}																																																										\
																																																										\
		::gpk::error_t																				setup										(_mainClass& applicationInstance);														\
		::gpk::error_t																				cleanup										(_mainClass& applicationInstance);														\
		::gpk::error_t																				update										(_mainClass& applicationInstance, bool systemRequestedExit);							\
		::gpk::error_t																				draw										(_mainClass& applicationInstance);														\
		::gpk::error_t																				rtMain										(::gpk::SRuntimeValues& runtimeValues)												{	\
	{																																																									\
		_CrtSetDbgFlag(_CRTDBG_LEAK_CHECK_DF | _CRTDBG_ALLOC_MEM_DF);																																									\
		::gpk::ptr_obj<_mainClass>																			applicationInstance							= {};																			\
		reterr_error_if(0 == applicationInstance.create(runtimeValues), "Failed to create application instance. Out of memory?");																										\
		info_printf("Initializing application instance.");																																												\
		gpk_necall(::setup(*applicationInstance), "User reported error. Execution stopped.");																																			\
		info_printf("Application instance initialized successfully. Executing main loop...");																																			\
		while(true) {																																																					\
			::gpk::error_t																						updateResult								= ::update(*applicationInstance, ::GetAsyncKeyState(VK_ESCAPE) != 0);		\
			break_info_if(1 == updateResult, "Application requested termination.");																																						\
			break_error_if(errored(updateResult), "update() returned error.");																																							\
			error_if(::draw(*applicationInstance), "Why would this ever happen?");																																						\
		}																																																								\
		info_printf("Cleaning up application instance...");																																												\
		::cleanup(*applicationInstance);																																																\
		info_printf("Application instance destroyed.");																																													\
	}																																																									\
	return 0;																																																							\
}
#endif // GPK_AVOID_LOCAL_APPLICATION_MODULE_MODEL_EXECUTABLE_RUNTIME
