#include "gpk_tcpip.h"
#include "gpk_log.h"
#include "gpk_windows.h"
#include "gpk_stdsocket.h"

#include <WS2tcpip.h>

::gpk::error_t								gpk::tcpipInitialize					()																															{
#if defined(GPK_WINDOWS)
	::WSADATA										w												= {};	
	ree_if(::WSAStartup(0x0202, &w) != 0, "Could not open Windows sockets: 0x%X '%s'", WSAGetLastError(), ::gpk::getWindowsErrorAsString(WSAGetLastError()).begin());		
#endif
	info_printf("Network subsystem initialized.");
	return 0;
}

::gpk::error_t								gpk::tcpipShutdown						()																															{
#if defined(GPK_WINDOWS)
	ree_if(::WSACleanup() != 0, "Could not shut down Windows sockets: 0x%X '%s'", WSAGetLastError(), ::gpk::getWindowsErrorAsString(WSAGetLastError()).begin());		// Open windows connection
#endif
	info_printf("Network subsystem shut down.");
	return 0;
}

::gpk::error_t								gpk::tcpipAddressFromSockaddr				(const sockaddr_in& sockaddr_ipv4, uint8_t* a1, uint8_t* a2, uint8_t* a3, uint8_t* a4, uint16_t* port)	{
	safe_assign(a1	, (uint8_t)sockaddr_ipv4.sin_addr.S_un.S_un_b.s_b1);
	safe_assign(a2	, (uint8_t)sockaddr_ipv4.sin_addr.S_un.S_un_b.s_b2);
	safe_assign(a3	, (uint8_t)sockaddr_ipv4.sin_addr.S_un.S_un_b.s_b3);
	safe_assign(a4	, (uint8_t)sockaddr_ipv4.sin_addr.S_un.S_un_b.s_b4);
	safe_assign(port, (uint16_t)ntohs(sockaddr_ipv4.sin_port));
	return 0;
}

::gpk::error_t								gpk::tcpipAddressToSockaddr					(const uint8_t* a1, const uint8_t* a2, const uint8_t* a3, const uint8_t* a4, const uint16_t* port, sockaddr_in & sockaddr_ipv4)	{
	sockaddr_ipv4								= {AF_INET, port ? htons(*port) : (uint16_t)0};
	sockaddr_ipv4.sin_addr.S_un.S_un_b.s_b1		= a1 ? *a1 : 0;
	sockaddr_ipv4.sin_addr.S_un.S_un_b.s_b2		= a2 ? *a2 : 0;
	sockaddr_ipv4.sin_addr.S_un.S_un_b.s_b3		= a3 ? *a3 : 0;
	sockaddr_ipv4.sin_addr.S_un.S_un_b.s_b4		= a4 ? *a4 : 0;
	return 0;
}

::gpk::error_t								gpk::tcpipAddress							(SOCKET socket, uint8_t* a1, uint8_t* a2, uint8_t* a3, uint8_t* a4, uint16_t* port) {
	char											buf		[256]								= "";
	sockaddr_in										sockaddr_ipv4								= {};
	socklen_t										len											= sizeof(sockaddr_in);
	ree_if(getpeername(socket, (sockaddr*)&sockaddr_ipv4, &len) != 0, "getpeername failed.");
	return tcpipAddressFromSockaddr(sockaddr_ipv4, a1, a2, a3, a4, port);
}

::gpk::error_t								gpk::tcpipAddress							(uint16_t portRequested, uint32_t adapterIndex, TRANSPORT_PROTOCOL mode, uint8_t* a1, uint8_t* a2, uint8_t* a3, uint8_t* a4)										{
	char											host_name[257]								= {};
	gethostname(host_name, 256);
	return ::gpk::tcpipAddress(host_name, portRequested, adapterIndex, mode, a1, a2, a3, a4);
}
::gpk::error_t								gpk::tcpipAddress							(const char_t* szHostName, uint16_t portRequested, uint32_t adapterIndex, TRANSPORT_PROTOCOL mode, uint8_t* a1, uint8_t* a2, uint8_t* a3, uint8_t* a4)				{
	char_t											portString			[8]							= {};
	::sprintf_s(portString, "%u", portRequested);
	
	// Setup the hints address info structure which is passed to the getaddrinfo() function
	::addrinfo										hints											= {};
	hints.ai_family								= AF_INET;
	hints.ai_socktype							= (TRANSPORT_PROTOCOL_TCP == mode) ? SOCK_STREAM	: SOCK_DGRAM	;
	hints.ai_protocol							= (TRANSPORT_PROTOCOL_TCP == mode) ? IPPROTO_TCP	: IPPROTO_UDP	;

	const ::addrinfo								* createdAddrInfo								= 0;
	ree_if(errored(::getaddrinfo(szHostName, portString, &hints, (::addrinfo**)&createdAddrInfo)), "gettaddrinfo failed for host_name: %s, port: %s", szHostName, portString);
	//sockaddr_in6									* sockaddr_ipv6									= 0;

	uint32_t										iAddress										= 0;
	bool											addressFound									= false;
	for(const addrinfo* ptr = createdAddrInfo; ptr != NULL; ptr = ptr->ai_next)  {	// Retrieve each address and print out the hex bytes
		info_printf("getaddrinfo response at index %u.", iAddress);
		info_printf("Flags: 0x%x.", ptr->ai_flags);
		info_printf("Family: ");
		DWORD											ipbufferlength									= 46;
		char_t											ipstringbuffer	[46]							= {};
#if defined(GPK_WINDOWS)
		wchar_t											ipwstringbuffer	[46]							= {};
#endif
		::sockaddr										* sockaddr_ip									=  0;
		::sockaddr_in									* sockaddr_ipv4									=  0;
		::sockaddr_in6									* sockaddr_ipv6									=  0;
		//DWORD dwRetval;
		INT												iRetval;

		switch (ptr->ai_family)  {
		default			:	info_printf("Other %li.", ptr->ai_family	); break;
		case AF_NETBIOS	:	info_printf("%s", "AF_NETBIOS (NetBIOS)"	); break;
		case AF_UNSPEC	:	info_printf("%s", "Unspecified"				); break;
		case AF_INET	:
			info_printf("AF_INET (IPv4)");
			sockaddr_ipv4								= (::sockaddr_in *) ptr->ai_addr;
			ipbufferlength								= 46;
 			::inet_ntop(AF_INET, &sockaddr_ipv4->sin_addr, ipstringbuffer, ipbufferlength);
			info_printf("IPv4 address %s", ipstringbuffer);
			// Copy address
			if(adapterIndex == iAddress) {
				::gpk::tcpipAddressFromSockaddr(*sockaddr_ipv4, a1, a2, a3, a4, 0);
				//printf("\tIPv4 address %s\n", inet_ntoa(sockaddr_ipv4->sin_addr) );
				addressFound								= true;
			}
			break;
		case AF_INET6	:
			info_printf("AF_INET6 (IPv6)");
			// the InetNtop function is available on Windows Vista and later
			sockaddr_ipv6								= (struct ::sockaddr_in6 *) ptr->ai_addr;
			//info_printf("IPv6 address %s", InetNtop(AF_INET6, &sockaddr_ipv6->sin6_addr, ipwstringbuffer, 46) );

#if defined(GPK_WINDOWS)
			sockaddr_ip									= (LPSOCKADDR)ptr->ai_addr;
			ipbufferlength								= 46;	// The buffer length is changed by each call to WSAAddresstoString, so we need to set it for each iteration through the loop for safety
			iRetval										= ::WSAAddressToStringW(sockaddr_ip, (DWORD) ptr->ai_addrlen, NULL, ipwstringbuffer, &ipbufferlength );	// We use WSAAddressToString since it is supported on Windows XP and later
			if (iRetval) {
				warning_printf("WSAAddressToString failed with code 0x%X: '%s'.", ::WSAGetLastError(), ::gpk::getWindowsErrorAsString(::WSAGetLastError()).begin() );
			}
			else
				::wprintf(L"IPv6 address '%s'.\n", ipwstringbuffer);
#endif
			break;
		}

		info_printf("Socket type: ");
		switch (ptr->ai_socktype) {
		case 0				: info_printf("Unspecified"								);	break;
		case SOCK_STREAM	: info_printf("SOCK_STREAM (stream)"					);	break;
		case SOCK_DGRAM		: info_printf("SOCK_DGRAM (datagram)"					);	break;
		case SOCK_RAW		: info_printf("SOCK_RAW (raw)"							);	break;
		case SOCK_RDM		: info_printf("SOCK_RDM (reliable message datagram)"	);	break;
		case SOCK_SEQPACKET	: info_printf("SOCK_SEQPACKET (pseudo-stream packet)"	);	break;
		default:
			info_printf("Other %ld", ptr->ai_socktype);
			break;
		}
		info_printf("Protocol: ");
		switch (ptr->ai_protocol) {
		case 0				: info_printf("Unspecified"								);	break;
		case IPPROTO_TCP	: info_printf("IPPROTO_TCP (TCP)"						);	break;
		case IPPROTO_UDP	: info_printf("IPPROTO_UDP (UDP)"						);	break;
		default:
			info_printf("Other %ld", ptr->ai_protocol);
			break;
		}
		++iAddress;
		info_printf("Length of this sockaddr: %u", (uint32_t)ptr->ai_addrlen);
		info_printf("Canonical name: %s", ptr->ai_canonname);
		if(addressFound)
			break;
	}
	::freeaddrinfo((::addrinfo*)createdAddrInfo);
//#else
//	struct hostent									* hp		= gethostbyname(host_name);
//
//	ree_if(hp == NULL, "Could not get host by name.");
//
//	b1											= hp->h_addr_list[0][0];
//	b2											= hp->h_addr_list[0][1];
//	b3											= hp->h_addr_list[0][2];
//	b4											= hp->h_addr_list[0][3];
//#endif

	return iAddress;
}
