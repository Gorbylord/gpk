#include "gpk_error.h"
#include "gpk_stdsocket.h"

#ifndef GPK_TCPIP_H_2874982374
#define GPK_TCPIP_H_2874982374

namespace gpk
{
	struct SIPv4 {
		ubyte_t											IP		[4]							;
		uint16_t										Port								;
//		int16_t											Adapter								;
	};

	::gpk::error_t									tcpipInitialize						();
	::gpk::error_t									tcpipShutdown						();

	enum TRANSPORT_PROTOCOL		
		{ TRANSPORT_PROTOCOL_UDP			= 0
		, TRANSPORT_PROTOCOL_TCP			
		};

	::gpk::error_t									tcpipAddress						(SOCKET socket, uint8_t* a1, uint8_t* a2, uint8_t* a3, uint8_t* a4, uint16_t* port);
	::gpk::error_t									tcpipAddress						(uint16_t portRequested, uint32_t adapterIndex, TRANSPORT_PROTOCOL mode, uint8_t* a1, uint8_t* a2, uint8_t* a3, uint8_t* a4);
	::gpk::error_t									tcpipAddress						(const char_t* hostName, uint16_t portRequested, uint32_t adapterIndex, TRANSPORT_PROTOCOL mode, uint8_t* a1, uint8_t* a2, uint8_t* a3, uint8_t* a4);
	static inline ::gpk::error_t					tcpipAddress						(SOCKET socket, ::gpk::SIPv4 & addr)																				{ 
		return tcpipAddress(socket, &addr.IP[0], &addr.IP[1], &addr.IP[2], &addr.IP[3], &addr.Port); 
	}
	static inline ::gpk::error_t					tcpipAddress						(uint16_t portRequested, uint32_t adapterIndex, TRANSPORT_PROTOCOL mode, SIPv4 & address)							{ 
		return tcpipAddress(portRequested, adapterIndex, mode, &address.IP[0], &address.IP[1], &address.IP[2], &address.IP[3]); 
	}
	static inline ::gpk::error_t					tcpipAddress						(const char_t* hostName, uint16_t portRequested, uint32_t adapterIndex, TRANSPORT_PROTOCOL mode, SIPv4 & address)	{ 
		return tcpipAddress(hostName, portRequested, adapterIndex, mode, &address.IP[0], &address.IP[1], &address.IP[2], &address.IP[3]); 
	}
} // namespace

#endif // GPK_TCPIP_H_2874982374
