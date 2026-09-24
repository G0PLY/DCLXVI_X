#include "dvlnet/zerotier_native.h"

#include <SDL.h>
#include <atomic>
#include <cstdio>

#ifdef USE_SDL1
#include "utils/sdl2_to_1_2_backports.h"
#else
#include "utils/sdl2_backports.h"
#endif

#if (defined(_WIN64) || defined(_WIN32)) && !defined(NXDK)
#include "utils/stdcompat/filesystem.hpp"
#ifdef DVL_HAS_FILESYSTEM
#define DVL_ZT_SYMLINK
#endif
#endif

#ifdef DVL_ZT_SYMLINK
#include "utils/str_cat.hpp"
#include "utils/utf8.hpp"
#include <shlobj.h>
#include <sodium.h>
#endif

#include <ZeroTierSockets.h>
#include <cstdlib>

#include "dvlnet/zerotier_lwip.h"
#include "utils/log.hpp"
#include "utils/paths.h"

namespace devilution {
namespace net {

namespace {

void ZtNativeDiag(const char *message)
{
	std::fprintf(stderr, "%s\n", message);
	std::fflush(stderr);
	if (FILE *file = std::fopen("zerotier_diagnostic.txt", "a")) {
		std::fprintf(file, "%s\n", message);
		std::fclose(file);
	}
}

void ZtNativeDiagResult(const char *operation, int result)
{
	char buffer[256];
	std::snprintf(buffer, sizeof(buffer), "[ZT API] %s result=%d", operation, result);
	ZtNativeDiag(buffer);
}

#ifdef DVL_ZT_SYMLINK
bool HasMultiByteChars(string_view path)
{
	return std::any_of(path.begin(), path.end(), IsTrailUtf8CodeUnit);
}

std::string ComputeAlternateFolderName(string_view path)
{
	const size_t hashSize = crypto_generichash_BYTES;
	unsigned char hash[hashSize];
	const int status = crypto_generichash(hash, hashSize,
	    reinterpret_cast<const unsigned char *>(path.data()), path.size(), nullptr, 0);
	if (status != 0)
		return "";
	return fmt::format("{:02x}", fmt::join(hash, ""));
}

std::string ToZTCompliantPath(string_view configPath)
{
	if (!HasMultiByteChars(configPath))
		return std::string(configPath);

	char commonAppDataPath[MAX_PATH];
	if (!SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_COMMON_APPDATA, NULL, 0, commonAppDataPath))) {
		LogVerbose("Failed to retrieve common application data path");
		return std::string(configPath);
	}

	std::error_code err;
	std::string alternateConfigPath = StrCat(commonAppDataPath, "\\diasurgical\\devilution");
	std::filesystem::create_directories(alternateConfigPath, err);
	if (err) {
		LogVerbose("Failed to create directories in ZT-compliant config path");
		return std::string(configPath);
	}

	std::string alternateFolderName = ComputeAlternateFolderName(configPath);
	if (alternateFolderName == "") {
		LogVerbose("Failed to hash config path for ZT");
		return std::string(configPath);
	}

	std::string symlinkPath = StrCat(alternateConfigPath, "\\", alternateFolderName);
	bool symlinkExists = std::filesystem::exists(std::filesystem::u8path(symlinkPath), err);
	if (err) {
		LogVerbose("Failed to determine if symlink for ZT-compliant config path exists");
		return std::string(configPath);
	}

	if (!symlinkExists) {
		std::filesystem::create_directory_symlink(
		    std::filesystem::u8path(configPath),
		    std::filesystem::u8path(symlinkPath), err);
		if (err) {
			LogVerbose("Failed to create symlink for ZT-compliant config path");
			return std::string(configPath);
		}
	}

	return StrCat(symlinkPath, "\\");
}
#endif

} // namespace

// static constexpr uint64_t zt_earth = 0x8056c2e21c000001;
static constexpr uint64_t ZtNetwork = 0xa84ac5c10a7ebb5f;

static std::atomic_bool zt_network_ready(false);
static std::atomic_bool zt_node_online(false);
static std::atomic_bool zt_joined(false);

static void Callback(void *ptr)
{
	zts_event_msg_t *msg = reinterpret_cast<zts_event_msg_t *>(ptr);
	if (msg == nullptr) {
		ZtNativeDiag("[ZT EVENT] ERROR: callback received null message");
		return;
	}

	char eventMessage[256];
	std::snprintf(eventMessage, sizeof(eventMessage),
	    "[ZT EVENT] callback event_code=%d", static_cast<int>(msg->event_code));
	ZtNativeDiag(eventMessage);

	if (msg->event_code == ZTS_EVENT_NODE_ONLINE) {
		const unsigned long long nodeId = static_cast<unsigned long long>(msg->node->node_id);
		Log("ZeroTier: ZTS_EVENT_NODE_ONLINE, nodeId={:x}", nodeId);
		char buffer[256];
		std::snprintf(buffer, sizeof(buffer), "[ZT EVENT] NODE_ONLINE nodeId=%llx", nodeId);
		ZtNativeDiag(buffer);

		zt_node_online = true;
		if (!zt_joined) {
			char joinMessage[256];
			std::snprintf(joinMessage, sizeof(joinMessage),
			    "[ZT API] requesting network join networkId=%llx",
			    static_cast<unsigned long long>(ZtNetwork));
			ZtNativeDiag(joinMessage);
			const int result = zts_net_join(ZtNetwork);
			ZtNativeDiagResult("zts_net_join", result);
			zt_joined = true;
		}
	} else if (msg->event_code == ZTS_EVENT_NODE_OFFLINE) {
		Log("ZeroTier: ZTS_EVENT_NODE_OFFLINE");
		ZtNativeDiag("[ZT EVENT] NODE_OFFLINE");
		zt_node_online = false;
	} else if (msg->event_code == ZTS_EVENT_NETWORK_READY_IP6) {
		const unsigned long long networkId = static_cast<unsigned long long>(msg->network->net_id);
		Log("ZeroTier: ZTS_EVENT_NETWORK_READY_IP6, networkId={:x}", networkId);
		char buffer[256];
		std::snprintf(buffer, sizeof(buffer),
		    "[ZT EVENT] NETWORK_READY_IP6 networkId=%llx", networkId);
		ZtNativeDiag(buffer);

		ZtNativeDiag("[ZT EVENT] calling zt_ip6setup()");
		zt_ip6setup();
		ZtNativeDiag("[ZT EVENT] zt_ip6setup() returned");
		zt_network_ready = true;
		ZtNativeDiag("[ZT EVENT] network marked ready");
	} else if (msg->event_code == ZTS_EVENT_ADDR_ADDED_IP6) {
		ZtNativeDiag("[ZT EVENT] ADDR_ADDED_IP6 received");
		print_ip6_addr(&(msg->addr->addr));
	}
}

bool zerotier_network_ready()
{
	return zt_network_ready && zt_node_online;
}

void zerotier_network_start()
{
	std::string configPath = paths::ConfigPath();
#ifdef DVL_ZT_SYMLINK
	configPath = ToZTCompliantPath(configPath);
#endif
	std::string ztpath = configPath + "zerotier";

	char pathMessage[1024];
	std::snprintf(pathMessage, sizeof(pathMessage), "[ZT API] storage path=%s", ztpath.c_str());
	ZtNativeDiag(pathMessage);

	int result = zts_init_from_storage(ztpath.c_str());
	ZtNativeDiagResult("zts_init_from_storage", result);

	result = zts_init_set_event_handler(&Callback);
	ZtNativeDiagResult("zts_init_set_event_handler", result);

	result = zts_node_start();
	ZtNativeDiagResult("zts_node_start", result);
}

} // namespace net
} // namespace devilution
