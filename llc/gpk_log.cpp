#include "gpk_log.h"

#include <Windows.h>
#include <stdio.h>

		void														gpk::_base_debug_print							(const char* text, uint32_t textLen)									{
#if defined GPK_WINDOWS
	OutputDebugStringA(text); textLen;
#else
	printf("%s", text); textLen;
#endif 
}

static	::gpk::error_t												getSystemErrorAsString						(const uint64_t lastError, char* buffer, uint32_t bufferSize)			{	// Get the error message, if any.
#if defined GPK_WINDOWS
	if(nullptr == buffer)
		return -1;
	if(0 == lastError) 
		return 0;
	const DWORD																size										= ::FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, (DWORD)(lastError & 0xFFFFFFFF), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), buffer, bufferSize, NULL);
	return (::gpk::error_t)size;
#endif
}

		void														gpk::_gpk_print_system_errors				(const char* prefix, uint32_t prefixLen)								{
	char																	bufferError[4096]							= {};
	int64_t																	lastSystemError								= ::GetLastError() & 0xFFFFFFFFFFFFFFFFLL;
	if(lastSystemError) {
		::gpk::error_t															stringLength								= ::getSystemErrorAsString((uint64_t)lastSystemError, bufferError, ::gpk::size(bufferError));
		base_debug_print(prefix, prefixLen);
		base_debug_print(bufferError, (uint32_t)stringLength);
		base_debug_print("\n", 1);
	}
	lastSystemError														= errno;
	if(lastSystemError) {
		::strerror_s(bufferError, (int)lastSystemError);
		char																	bufferError2[4096]							= {};
		size_t																	stringLength								= ::sprintf_s(bufferError2, "Last system error: 0x%llX '%s'.", lastSystemError, bufferError);
		base_debug_print(prefix, prefixLen);
		base_debug_print(bufferError2, (uint32_t)stringLength);
		base_debug_print("\n", 1);
	}
}
