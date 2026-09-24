#include "dvlnet/zerotier_lwip.h"

#include <cstdio>
#include <cstring>

#include <lwip/err.h>
#include <lwip/igmp.h>
#include <lwip/mld6.h>
#include <lwip/sockets.h>
#include <lwip/tcpip.h>

#include <SDL.h>

#ifdef USE_SDL1
#include "utils/sdl2_to_1_2_backports.h"
#else
#include "utils/sdl2_backports.h"
#endif

#include "dvlnet/zerotier_native.h"
#include "utils/log.hpp"

namespace devilution {
namespace net {

namespace {

void ZtMldDiag(const char *message)
{
	std::fprintf(stderr, "%s\n", message);
	std::fflush(stderr);
	if (FILE *file = std::fopen("zerotier_diagnostic.txt", "a")) {
		std::fprintf(file, "%s\n", message);
		std::fclose(file);
	}
}

const char *LwipErrorName(err_t error)
{
	switch (error) {
	case ERR_OK:
		return "ERR_OK";
	case ERR_MEM:
		return "ERR_MEM";
	case ERR_BUF:
		return "ERR_BUF";
	case ERR_TIMEOUT:
		return "ERR_TIMEOUT";
	case ERR_RTE:
		return "ERR_RTE";
	case ERR_INPROGRESS:
		return "ERR_INPROGRESS";
	case ERR_VAL:
		return "ERR_VAL";
	case ERR_WOULDBLOCK:
		return "ERR_WOULDBLOCK";
	case ERR_USE:
		return "ERR_USE";
	case ERR_ALREADY:
		return "ERR_ALREADY";
	case ERR_ISCONN:
		return "ERR_ISCONN";
	case ERR_CONN:
		return "ERR_CONN";
	case ERR_IF:
		return "ERR_IF";
	case ERR_ABRT:
		return "ERR_ABRT";
	case ERR_RST:
		return "ERR_RST";
	case ERR_CLSD:
		return "ERR_CLSD";
	case ERR_ARG:
		return "ERR_ARG";
	default:
		return "UNKNOWN";
	}
}

} // namespace

void print_ip6_addr(void *x)
{
	char ipstr[INET6_ADDRSTRLEN] = {};
	auto *in = static_cast<sockaddr_in6 *>(x);
	const char *result = lwip_inet_ntop(AF_INET6, &(in->sin6_addr), ipstr, INET6_ADDRSTRLEN);
	if (result != nullptr) {
		Log("ZeroTier: ZTS_EVENT_ADDR_NEW_IP6, addr={}", ipstr);
		char buffer[256];
		std::snprintf(buffer, sizeof(buffer), "[ZT EVENT] ADDR_ADDED_IP6 address=%s", ipstr);
		ZtMldDiag(buffer);
	} else {
		ZtMldDiag("[ZT EVENT] ADDR_ADDED_IP6 address=<inet_ntop failed>");
	}
}

void zt_ip6setup()
{
	ip6_addr_t mcaddr;
	std::memcpy(mcaddr.addr, dvl_multicast_addr, 16);
	mcaddr.zone = 0;

	char multicastAddress[INET6_ADDRSTRLEN] = {};
	ip6addr_ntoa_r(&mcaddr, multicastAddress, sizeof(multicastAddress));

	char startMessage[256];
	std::snprintf(startMessage, sizeof(startMessage),
	    "[ZT MLD] attempting mld6_joingroup(IP6_ADDR_ANY6, %s)", multicastAddress);
	ZtMldDiag(startMessage);

	LOCK_TCPIP_CORE();
	const err_t result = mld6_joingroup(IP6_ADDR_ANY6, &mcaddr);
	UNLOCK_TCPIP_CORE();

	char resultMessage[256];
	std::snprintf(resultMessage, sizeof(resultMessage),
	    "[ZT MLD] mld6_joingroup result=%d (%s)",
	    static_cast<int>(result), LwipErrorName(result));
	ZtMldDiag(resultMessage);
}

} // namespace net
} // namespace devilution
