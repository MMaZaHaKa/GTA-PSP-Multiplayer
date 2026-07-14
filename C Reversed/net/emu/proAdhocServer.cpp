/*
    Authors: MaZaHaKa, ASM95
    Do not delete this comment block. Respect others' work!
*/
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <signal.h>

#include "SocketCompat.h"
#include "NetAdhocCommon.h"
#ifdef WITH_UPNP
#include "PortManager.h"
#endif
#include "GameInstance.h"
#include "proAdhocServer.h"
#include "Utils.h"
#include "Resolve.h" // host check
#include "MultiGame.h" // config

#ifdef _WIN32
#undef errno
#define errno WSAGetLastError()
#endif

#define ON_CLOSE_SOCKET(fd) closesocket(fd)

// User Count
uint32_t _db_user_count = 0;

// User Database
SceNetAdhocctlUserNode * _db_user = NULL;

// Game Database
SceNetAdhocctlGameNode * _db_game = NULL;

// Server Status
std::atomic<bool> adhocServerRunning(false);
std::thread adhocServerThread;

// Crosslink database for cross region Adhoc play
std::vector<db_crosslink> crosslinks;
static const db_crosslink default_crosslinks[] = {
	{ "ULES00502", "ULUS10160" },
	{ "ULES00151", "ULUS10041" },
	{ "ULUX80146", "ULUS10041" },
};

std::vector<db_productid> productids;
static const db_productid default_productids[] = {
	{ "ULUS10160", "GTA Vice City Stories" },
	{ "ULUS10041", "GTA Liberty City Stories" },
};

// Function Prototypes
const char * strcpyxml(char * out, const char * in, uint32_t size);

// Function Prototypes
void interrupt(int sig);
void enable_address_reuse(int fd);
void enable_keepalive(int fd);
void change_nodelay_mode(int fd, int flag);
void change_blocking_mode(int fd, int nonblocking);
int create_listen_socket(uint16_t port);
int server_loop(int server);

void __AdhocServerInit() {
	// Database Product name will update if new game region played on my server to list possible crosslinks
	productids = std::vector<db_productid>(default_productids, default_productids + ARRAY_SIZE(default_productids));
	crosslinks = std::vector<db_crosslink>(default_crosslinks, default_crosslinks + ARRAY_SIZE(default_crosslinks));
}

/**
 * Login User into Database (Stream)
 * @param fd Socket
 * @param ip IP Address (Network Order)
 */
void login_user_stream(int fd, uint32_t ip)
{
	// Enough Space available
	if(_db_user_count < SERVER_USER_MAXIMUM)
	{
		// Check IP Duplication
		SceNetAdhocctlUserNode * u = _db_user;
		while(u != NULL && u->resolver.ip != ip) u = u->next;

#ifdef ADHOC_ANTISPOOF
		if (u != NULL) { // IP Already existed
			WARN_LOG(Log::sceNet, "AdhocServer: Already Existing IP: %s\n", ip2str(*(in_addr*)&u->resolver.ip).c_str());
		}

		// Unique IP Address
		else //if(u == NULL)
#endif
		{
			// Allocate User Node Memory
			SceNetAdhocctlUserNode * user = (SceNetAdhocctlUserNode *)malloc(sizeof(SceNetAdhocctlUserNode));

			// Allocated User Node Memory
			if(user != NULL)
			{
				// Clear Memory
				memset(user, 0, sizeof(SceNetAdhocctlUserNode));

				// Save Socket
				user->stream = fd;

				// Save IP
				user->resolver.ip = ip;

				// Link into User List
				user->next = _db_user;
				if(_db_user != NULL) _db_user->prev = user;
				_db_user = user;

				// Initialize Death Clock
				user->last_recv = time(NULL);

				// Notify User
				INFO_LOG(Log::sceNet, "AdhocServer: New Connection from %s", ip2str(*(in_addr*)&user->resolver.ip).c_str());

				// Fix User Counter
				_db_user_count++;

				// Update Status Log
				update_status();

				// Exit Function
				return;
			}
		}
	}

	// Duplicate IP, Allocation Error or not enough space - Close Stream
	ON_CLOSE_SOCKET(fd);
}

/**
 * Login User into Database (Login Data)
 * @param user User Node
 * @param data Login Packet
 */
void login_user_data(SceNetAdhocctlUserNode * user, SceNetAdhocctlLoginPacketC2S * data)
{
	// Product Code Check
	int valid_product_code = 1;

	// Iterate Characters
	int i = 0; for(; i < PRODUCT_CODE_LENGTH && valid_product_code == 1; i++)
	{
		// Valid Characters
		if(!((data->game.data[i] >= 'A' && data->game.data[i] <= 'Z') || (data->game.data[i] >= '0' && data->game.data[i] <= '9'))) valid_product_code = 0;
	}

	// Valid Packet Data
	if(valid_product_code == 1 && memcmp(&data->mac, "\xFF\xFF\xFF\xFF\xFF\xFF", sizeof(data->mac)) != 0 && memcmp(&data->mac, "\x00\x00\x00\x00\x00\x00", sizeof(data->mac)) != 0 && data->name.data[0] != 0)
	{
		// Check for duplicated MAC as most games identify Players by MAC
		SceNetAdhocctlUserNode* u = _db_user;
		while (u != NULL && !IsMatch(u->resolver.mac, data->mac)) u = u->next;

		if (u != NULL) { // MAC Already existed
			WARN_LOG(Log::sceNet, "AdhocServer: Already Existing MAC: %s [%s]\n", mac2str(&data->mac).c_str(), ip2str(*(in_addr*)&u->resolver.ip).c_str());
		}

		// Game Product Override
		game_product_override(&data->game);

		// Find existing Game
		SceNetAdhocctlGameNode * game = _db_game;
		while(game != NULL && strncmp(game->game.data, data->game.data, PRODUCT_CODE_LENGTH) != 0) game = game->next;

		// Game not found
		if(game == NULL)
		{
			// Allocate Game Node Memory
			game = (SceNetAdhocctlGameNode *)malloc(sizeof(SceNetAdhocctlGameNode));

			// Allocated Game Node Memory
			if(game != NULL)
			{
				// Clear Memory
				memset(game, 0, sizeof(SceNetAdhocctlGameNode));

				// Save Game Product ID
				game->game = data->game;

				// Link into Game List
				game->next = _db_game;
				if(_db_game != NULL) _db_game->prev = game;
				_db_game = game;
			}
		}

		// Game now available
		if(game != NULL)
		{
			// Save MAC
			user->resolver.mac = data->mac;

			// Save Nickname
			user->resolver.name = data->name;

			// Increase Player Count in Game Node
			game->playercount++;

			// Link Game to Player
			user->game = game;

			// Notify User
			char safegamestr[10];
			memset(safegamestr, 0, sizeof(safegamestr));
			strncpy(safegamestr, game->game.data, PRODUCT_CODE_LENGTH);
			INFO_LOG(Log::sceNet, "AdhocServer: %s (MAC: %s - IP: %s) started playing %s", (char *)user->resolver.name.data, mac2str(&user->resolver.mac).c_str(), ip2str(*(in_addr*)&user->resolver.ip).c_str(), safegamestr);

			// Update Status Log
			update_status();

			// Leave Function
			return;
		}
	}

	// Invalid Packet Data
	else
	{
		// Notify User
		WARN_LOG(Log::sceNet, "AdhocServer: Invalid Login Packet Contents from %s", ip2str(*(in_addr*)&user->resolver.ip).c_str());
	}

	// Logout User - Out of Memory or Invalid Arguments
	logout_user(user);
}

/**
 * Logout User from Database
 * @param user User Node
 */
void logout_user(SceNetAdhocctlUserNode * user)
{
	// Disconnect from Group
	if(user->group != NULL) disconnect_user(user);

	// Unlink Leftside (Beginning)
	if(user->prev == NULL) _db_user = user->next;

	// Unlink Leftside (Other)
	else user->prev->next = user->next;

	// Unlink Rightside
	if(user->next != NULL) user->next->prev = user->prev;

	// Close Stream
	ON_CLOSE_SOCKET(user->stream);

	// Playing User
	if(user->game != NULL)
	{
		// Notify User
		char safegamestr[10];
		memset(safegamestr, 0, sizeof(safegamestr));
		strncpy(safegamestr, user->game->game.data, PRODUCT_CODE_LENGTH);
		INFO_LOG(Log::sceNet, "AdhocServer: %s (MAC: %s - IP: %s) stopped playing %s", (char *)user->resolver.name.data, mac2str(&user->resolver.mac).c_str(), ip2str(*(in_addr*)&user->resolver.ip).c_str(), safegamestr);

		// Fix Game Player Count
		user->game->playercount--;

		// Empty Game Node
		if(user->game->playercount == 0)
		{
			// Unlink Leftside (Beginning)
			if(user->game->prev == NULL) _db_game = user->game->next;

			// Unlink Leftside (Other)
			else user->game->prev->next = user->game->next;

			// Unlink Rightside
			if(user->game->next != NULL) user->game->next->prev = user->game->prev;

			// Free Game Node Memory
			free(user->game);
		}
	}

	// Unidentified User
	else
	{
		// Notify User
		WARN_LOG(Log::sceNet, "AdhocServer: Dropped Connection to %s", ip2str(*(in_addr*)&user->resolver.ip).c_str());
	}

	// Free Memory
	free(user);

	// Fix User Counter
	_db_user_count--;

	// Update Status Log
	update_status();
}

/**
 * Free Database Memory
 */
void free_database()
{
	// There are users playing
	if(_db_user_count > 0)
	{
		// Send Shutdown Notice
		spread_chat_message(NULL, (std::string(ADHOC_SERVER_NAME) + ": " + SERVER_SHUTDOWN_MESSAGE).c_str());
	}

	// Iterate Users for Deletion
	SceNetAdhocctlUserNode * user = _db_user;
	while(user != NULL)
	{
		// Next User (for safe delete)
		SceNetAdhocctlUserNode * next = user->next;

		// Logout User
		logout_user(user);

		// Move Pointer
		user = next;
	}
}

/**
 * Connect User to Game Group
 * @param user User Node
 * @param group Group Name
 */
void connect_user(SceNetAdhocctlUserNode * user, SceNetAdhocctlGroupName * group)
{
	// Group Name Check
	int valid_group_name = 1;
	{
		// Iterate Characters
		int i = 0; for(; i < ADHOCCTL_GROUPNAME_LEN && valid_group_name == 1; i++)
		{
			// End of Name
			if(group->data[i] == 0) break;

			// A - Z
			if(group->data[i] >= 'A' && group->data[i] <= 'Z') continue;

			// a - z
			if(group->data[i] >= 'a' && group->data[i] <= 'z') continue;

			// 0 - 9
			if(group->data[i] >= '0' && group->data[i] <= '9') continue;

			// Invalid Symbol
			valid_group_name = 0;
		}
	}

	// Valid Group Name
	if(valid_group_name == 1)
	{
		// User is disconnected
		if(user->group == NULL)
		{
			// Find Group in Game Node
			SceNetAdhocctlGroupNode * g = user->game->group;
			while(g != NULL && strncmp((char *)g->group.data, (char *)group->data, ADHOCCTL_GROUPNAME_LEN) != 0) g = g->next;

			// BSSID Packet
			SceNetAdhocctlConnectBSSIDPacketS2C bssid;

			// Set BSSID Opcode
			bssid.base.opcode = OPCODE_CONNECT_BSSID;

			// Set Default BSSID
			bssid.mac = user->resolver.mac;

			// No Group found
			if(g == NULL)
			{
				// Allocate Group Memory
				g = (SceNetAdhocctlGroupNode *)malloc(sizeof(SceNetAdhocctlGroupNode));

				// Allocated Group Memory
				if(g != NULL)
				{
					// Clear Memory
					memset(g, 0, sizeof(SceNetAdhocctlGroupNode));

					// Link Game Node
					g->game = user->game;

					// Link Group Node
					g->next = g->game->group;
					if(g->game->group != NULL) g->game->group->prev = g;
					g->game->group = g;

					// Copy Group Name
					g->group = *group;

					// Increase Group Counter for Game
					g->game->groupcount++;
				}
			}

			// Group now available
			if(g != NULL)
			{
				// Iterate remaining Group Players
				SceNetAdhocctlUserNode * peer = g->player;
				while(peer != NULL)
				{
					// Connect Packet
					SceNetAdhocctlConnectPacketS2C packet;

					// Clear Memory
					// memset(&packet, 0, sizeof(packet));

					// Set Connect Opcode
					packet.base.opcode = OPCODE_CONNECT;

					// Set Player Name
					packet.name = user->resolver.name;

					// Set Player MAC
					packet.mac = user->resolver.mac;

					// Set Player IP
					packet.ip = user->resolver.ip;

					// Send Data
					int iResult = (int)send(peer->stream, (const char*)&packet, sizeof(packet), MSG_NOSIGNAL);
					if (iResult < 0) {
						ON_SOCKET_ERROR();
						ERROR_LOG(Log::sceNet, "AdhocServer: connect_user[send peer] (Socket error %d)", socket_errno);
					}

					// Set Player Name
					packet.name = peer->resolver.name;

					// Set Player MAC
					packet.mac = peer->resolver.mac;

					// Set Player IP
					packet.ip = peer->resolver.ip;

					// Send Data
					iResult = (int)send(user->stream, (const char*)&packet, sizeof(packet), MSG_NOSIGNAL);
					if (iResult < 0) {
						ON_SOCKET_ERROR();
						ERROR_LOG(Log::sceNet, "AdhocServer: connect_user[send user] (Socket error %d)", socket_errno);
					}

					// Set BSSID
					if(peer->group_next == NULL) bssid.mac = peer->resolver.mac;

					// Move Pointer
					peer = peer->group_next;
				}

				// Link User to Group
				user->group_next = g->player;
				if(g->player != NULL) g->player->group_prev = user;
				g->player = user;

				// Link Group to User
				user->group = g;

				// Increase Player Count
				g->playercount++;

				// Send Network BSSID to User
				int iResult = (int)send(user->stream, (const char*)&bssid, sizeof(bssid), MSG_NOSIGNAL);
				if (iResult < 0) {
					ON_SOCKET_ERROR();
					ERROR_LOG(Log::sceNet, "AdhocServer: connect_user[send user bssid] (Socket error %d)", socket_errno);
				}

				// Notify User
				char safegamestr[10];
				memset(safegamestr, 0, sizeof(safegamestr));
				strncpy(safegamestr, user->game->game.data, PRODUCT_CODE_LENGTH);
				char safegroupstr[9];
				memset(safegroupstr, 0, sizeof(safegroupstr));
				strncpy(safegroupstr, (char *)user->group->group.data, ADHOCCTL_GROUPNAME_LEN);
				INFO_LOG(Log::sceNet, "AdhocServer: %s (MAC: %s - IP: %s) joined %s group %s", (char *)user->resolver.name.data, mac2str(&user->resolver.mac).c_str(), ip2str(*(in_addr*)&user->resolver.ip).c_str(), safegamestr, safegroupstr);

				// Update Status Log
				update_status();

				// Exit Function
				return;
			}
		}

		// Already connected to another group
		else
		{
			// Notify User
			char safegamestr[10];
			memset(safegamestr, 0, sizeof(safegamestr));
			strncpy(safegamestr, user->game->game.data, PRODUCT_CODE_LENGTH);
			char safegroupstr[9];
			memset(safegroupstr, 0, sizeof(safegroupstr));
			strncpy(safegroupstr, (char *)group->data, ADHOCCTL_GROUPNAME_LEN);
			char safegroupstr2[9];
			memset(safegroupstr2, 0, sizeof(safegroupstr2));
			strncpy(safegroupstr2, (char *)user->group->group.data, ADHOCCTL_GROUPNAME_LEN);
			WARN_LOG(Log::sceNet, "AdhocServer: %s (MAC: %s - IP: %s) attempted to join %s group %s without disconnecting from %s first", (char *)user->resolver.name.data, mac2str(&user->resolver.mac).c_str(), ip2str(*(in_addr*)&user->resolver.ip).c_str(), safegamestr, safegroupstr, safegroupstr2);
		}
	}

	// Invalid Group Name
	else
	{
		// Notify User
		char safegamestr[10];
		memset(safegamestr, 0, sizeof(safegamestr));
		strncpy(safegamestr, user->game->game.data, PRODUCT_CODE_LENGTH);
		char safegroupstr[9];
		memset(safegroupstr, 0, sizeof(safegroupstr));
		strncpy(safegroupstr, (char *)group->data, ADHOCCTL_GROUPNAME_LEN);
		WARN_LOG(Log::sceNet, "AdhocServer: %s (MAC: %s - IP: %s) attempted to join invalid %s group %s", (char *)user->resolver.name.data, mac2str(&user->resolver.mac).c_str(), ip2str(*(in_addr*)&user->resolver.ip).c_str(), safegamestr, safegroupstr);
	}

	// Invalid State, Out of Memory or Invalid Group Name
	logout_user(user);
}

/**
 * Disconnect User from Game Group
 * @param user User Node
 */
void disconnect_user(SceNetAdhocctlUserNode * user)
{
	// User is connected
	if(user->group != NULL)
	{
		// Unlink Leftside (Beginning)
		if(user->group_prev == NULL) user->group->player = user->group_next;

		// Unlink Leftside (Other)
		else user->group_prev->group_next = user->group_next;

		// Unlink Rightside
		if(user->group_next != NULL) user->group_next->group_prev = user->group_prev;

		// Fix Player Count
		user->group->playercount--;

		// Iterate remaining Group Players
		SceNetAdhocctlUserNode * peer = user->group->player;
		while(peer != NULL)
		{
			// Disconnect Packet
			SceNetAdhocctlDisconnectPacketS2C packet;

			// Clear Memory
			// memset(&packet, 0, sizeof(packet));

			// Set Disconnect Opcode
			packet.base.opcode = OPCODE_DISCONNECT;

			// Set User IP
			packet.ip = user->resolver.ip;

			// Send Data
			int iResult = (int)send(peer->stream, (const char*)&packet, sizeof(packet), MSG_NOSIGNAL);
			if (iResult < 0) {
				ON_SOCKET_ERROR();
				ERROR_LOG(Log::sceNet, "AdhocServer: disconnect_user[send peer] (Socket error %d)", socket_errno);
			}

			// Move Pointer
			peer = peer->group_next;
		}

		// Notify User
		char safegamestr[10];
		memset(safegamestr, 0, sizeof(safegamestr));
		strncpy(safegamestr, user->game->game.data, PRODUCT_CODE_LENGTH);
		char safegroupstr[9];
		memset(safegroupstr, 0, sizeof(safegroupstr));
		strncpy(safegroupstr, (char *)user->group->group.data, ADHOCCTL_GROUPNAME_LEN);
		INFO_LOG(Log::sceNet, "AdhocServer: %s (MAC: %s - IP: %s) left %s group %s", (char *)user->resolver.name.data, mac2str(&user->resolver.mac).c_str(), ip2str(*(in_addr*)&user->resolver.ip).c_str(), safegamestr, safegroupstr);

		// Empty Group
		if(user->group->playercount == 0)
		{
			// Unlink Leftside (Beginning)
			if(user->group->prev == NULL) user->group->game->group = user->group->next;

			// Unlink Leftside (Other)
			else user->group->prev->next = user->group->next;

			// Unlink Rightside
			if(user->group->next != NULL) user->group->next->prev = user->group->prev;

			// Free Group Memory
			free(user->group);

			// Decrease Group Counter in Game Node
			user->game->groupcount--;
		}

		// Unlink from Group
		user->group = NULL;
		user->group_next = NULL;
		user->group_prev = NULL;

		// Update Status Log
		update_status();

		// Exit Function
		return;
	}

	// Not in a game group
	else
	{
		// Notify User
		char safegamestr[10];
		memset(safegamestr, 0, sizeof(safegamestr));
		strncpy(safegamestr, user->game->game.data, PRODUCT_CODE_LENGTH);
		WARN_LOG(Log::sceNet, "AdhocServer: %s (MAC: %s - IP: %s) attempted to leave %s group without joining one first", (char *)user->resolver.name.data, mac2str(&user->resolver.mac).c_str(), ip2str(*(in_addr*)&user->resolver.ip).c_str(), safegamestr);
	}

	// Delete User
	logout_user(user);
}

/**
 * Send Game Group List
 * @param user User Node
 */
void send_scan_results(SceNetAdhocctlUserNode * user)
{
	// User is disconnected
	if(user->group == NULL)
	{
		// Iterate Groups
		SceNetAdhocctlGroupNode * group = user->game->group;
		for(; group != NULL; group = group->next)
		{
			// Scan Result Packet
			SceNetAdhocctlScanPacketS2C packet;

			// Clear Memory
			// memset(&packet, 0, sizeof(packet));

			// Set Opcode
			packet.base.opcode = OPCODE_SCAN;

			// Set Group Name
			packet.group = group->group;

			// Iterate Players in Network Group
			SceNetAdhocctlUserNode * peer = group->player;
			for(; peer != NULL; peer = peer->group_next)
			{
				// Found Network Founder
				if(peer->group_next == NULL)
				{
					// Set Group Host MAC
					packet.mac = peer->resolver.mac;
				}
			}

			// Send Group Packet
			int iResult = (int)send(user->stream, (const char*)&packet, sizeof(packet), MSG_NOSIGNAL);
			if (iResult < 0) {
				ON_SOCKET_ERROR();
				ERROR_LOG(Log::sceNet, "AdhocServer: send_scan_result[send user] (Socket error %d)", socket_errno);
			}
		}

		// Notify Player of End of Scan
		uint8_t opcode = OPCODE_SCAN_COMPLETE;
		int iResult = (int)send(user->stream, (const char*)&opcode, 1, MSG_NOSIGNAL);
		if (iResult < 0) {
			ON_SOCKET_ERROR();
			ERROR_LOG(Log::sceNet, "AdhocServer: send_scan_result[send peer complete] (Socket error %d)", socket_errno);
		}

		// Notify User
		char safegamestr[10];
		memset(safegamestr, 0, sizeof(safegamestr));
		strncpy(safegamestr, user->game->game.data, PRODUCT_CODE_LENGTH);
		INFO_LOG(Log::sceNet, "AdhocServer: %s (MAC: %s - IP: %s) requested information on %d %s groups", (char *)user->resolver.name.data, mac2str(&user->resolver.mac).c_str(), ip2str(*(in_addr*)&user->resolver.ip).c_str(), user->game->groupcount, safegamestr);

		// Exit Function
		return;
	}

	// User in a game group
	else
	{
		// Notify User
		char safegamestr[10];
		memset(safegamestr, 0, sizeof(safegamestr));
		strncpy(safegamestr, user->game->game.data, PRODUCT_CODE_LENGTH);
		char safegroupstr[9];
		memset(safegroupstr, 0, sizeof(safegroupstr));
		strncpy(safegroupstr, (char *)user->group->group.data, ADHOCCTL_GROUPNAME_LEN);
		WARN_LOG(Log::sceNet, "AdhocServer: %s (MAC: %s - IP: %s) attempted to scan for %s groups without disconnecting from %s first", (char *)user->resolver.name.data, mac2str(&user->resolver.mac).c_str(), ip2str(*(in_addr*)&user->resolver.ip).c_str(), safegamestr, safegroupstr);
	}

	// Delete User
	logout_user(user);
}

/**
 * Spread Chat Message in P2P Network
 * @param user Sender User Node
 * @param message Chat Message
 */
void spread_chat_message(SceNetAdhocctlUserNode *user, const char *message)
{
	// Global Notice
	if(user == NULL)
	{
		// Iterate Players
		for(user = _db_user; user != NULL; user = user->next)
		{
			// Player has access to chat
			if(user->group != NULL)
			{
				// Chat Packet
				SceNetAdhocctlChatPacketS2C packet;

				// Clear Memory
				memset(&packet, 0, sizeof(packet));

				// Set Chat Opcode
				packet.base.base.opcode = OPCODE_CHAT;

				// Set Chat Message
				strcpy(packet.base.message, message);

				// Send Data
				int iResult = (int)send(user->stream, (const char*)&packet, sizeof(packet), MSG_NOSIGNAL);
				if (iResult < 0) {
					ON_SOCKET_ERROR();
					ERROR_LOG(Log::sceNet, "AdhocServer: spread_message[send user chat] (Socket error %d)", socket_errno);
				}
			}
		}

		// Prevent NULL Error
		return;
	}

	// User is connected
	else if(user->group != NULL)
	{
		// Broadcast Range Counter
		uint32_t counter = 0;

		// Iterate Group Players
		SceNetAdhocctlUserNode * peer = user->group->player;
		while(peer != NULL)
		{
			// Skip Self
			if(peer == user)
			{
				// Move Pointer
				peer = peer->group_next;

				// Continue Loop
				continue;
			}

			// Chat Packet
			SceNetAdhocctlChatPacketS2C packet{};

			// Set Chat Opcode
			packet.base.base.opcode = OPCODE_CHAT;

			// Set Chat Message
			strcpy(packet.base.message, message);

			// Set Sender Nickname
			packet.name = user->resolver.name;

			// Send Data
			int iResult = (int)send(peer->stream, (const char*)&packet, sizeof(packet), MSG_NOSIGNAL);
			if (iResult < 0) {
				ON_SOCKET_ERROR();
				ERROR_LOG(Log::sceNet, "AdhocServer: spread_message[send peer chat] (Socket error %d)", socket_errno);
			}

			// Move Pointer
			peer = peer->group_next;

			// Increase Broadcast Range Counter
			counter++;
		}

		// Message Sent
		if(counter > 0)
		{
			// Notify User
			char safegamestr[10];
			memset(safegamestr, 0, sizeof(safegamestr));
			strncpy(safegamestr, user->game->game.data, PRODUCT_CODE_LENGTH);
			char safegroupstr[9];
			memset(safegroupstr, 0, sizeof(safegroupstr));
			strncpy(safegroupstr, (char *)user->group->group.data, ADHOCCTL_GROUPNAME_LEN);
			INFO_LOG(Log::sceNet, "AdhocServer: %s (MAC: %s - IP: %s) sent \"%s\" to %d players in %s group %s", (char *)user->resolver.name.data, mac2str(&user->resolver.mac).c_str(), ip2str(*(in_addr*)&user->resolver.ip).c_str(), message, counter, safegamestr, safegroupstr);
		}

		// Exit Function
		return;
	}

	// User not in a game group
	else
	{
		// Notify User
		char safegamestr[10];
		memset(safegamestr, 0, sizeof(safegamestr));
		strncpy(safegamestr, user->game->game.data, PRODUCT_CODE_LENGTH);
		WARN_LOG(Log::sceNet, "AdhocServer: %s (MAC: %s - IP: %s) attempted to send a text message without joining a %s group first", (char *)user->resolver.name.data, mac2str(&user->resolver.mac).c_str(), ip2str(*(in_addr*)&user->resolver.ip).c_str(), safegamestr);
	}

	// Delete User
	logout_user(user);
}

/**
 * Get User State
 * @param user User Node
 */
int get_user_state(SceNetAdhocctlUserNode * user)
{
	// Timeout Status
	if((time(NULL) - user->last_recv) >= SERVER_USER_TIMEOUT) return USER_STATE_TIMED_OUT;

	// Waiting Status
	if(user->game == NULL) return USER_STATE_WAITING;

	// Logged-In Status
	return USER_STATE_LOGGED_IN;
}

/**
 * Clear RX Buffer
 * @param user User Node
 * @param clear Number of Bytes to clear (-1 for all)
 */
void clear_user_rxbuf(SceNetAdhocctlUserNode * user, int clear)
{
	// Fix Clear Length
	if(clear == -1 || clear > (int)user->rxpos) clear = user->rxpos;

	// Move Buffer
	memmove(user->rx, user->rx + clear, sizeof(user->rx) - clear);

	// Fix RX Buffer Pointer
	user->rxpos -= clear;
}

/**
 * Patch Game Product Code
 * @param product To-be-patched Product Code
 * @param from If the Product Code matches this...
 * @param to ... then change it to this one.
 */
void game_product_relink(SceNetAdhocctlProductCode * product, char * from, char * to)
{
	// Relink Region Code
	if(strncmp(product->data, from, PRODUCT_CODE_LENGTH) == 0) strncpy(product->data, to, PRODUCT_CODE_LENGTH);
}

/**
 * Game Product Override (used for mixing multi-region games)
 * @param product IN: Source Product OUT: Override Product
 */
void game_product_override(SceNetAdhocctlProductCode * product)
{
	// Safe Product Code
	char productid[PRODUCT_CODE_LENGTH + 1];

	// Prepare Safe Product Code
	strncpy(productid, product->data, PRODUCT_CODE_LENGTH);
	productid[PRODUCT_CODE_LENGTH] = 0;

	// Database Handle
	//sqlite3 * db = NULL;

	// Open Database
	//if(sqlite3_open(SERVER_DATABASE, &db) == SQLITE_OK)
	{
		// Crosslinked Flag
		int crosslinked = 0;

		// Exists Flag
		int exists = 0;

		// SQL Statements
		/*const char * sql = "SELECT id_to FROM crosslinks WHERE id_from=?;";
		const char * sql2 = "SELECT * FROM productids WHERE id=?;";
		const char * sql3 = "INSERT INTO productids(id, name) VALUES(?, ?);";

		// Prepared SQL Statement
		sqlite3_stmt * statement = NULL;

		// Prepare SQL Statement
		if(sqlite3_prepare_v2(db, sql, strlen(sql) + 1, &statement, NULL) == SQLITE_OK)
		{
			// Bind SQL Statement Data
			if(sqlite3_bind_text(statement, 1, productid, strlen(productid), SQLITE_STATIC) == SQLITE_OK)
			{
				// Found Matching Row
				if(sqlite3_step(statement) == SQLITE_ROW)
				{
					// Grab Crosslink ID
					const char * crosslink = (const char *)sqlite3_column_text(statement, 0);

					// Crosslink Product Code
					strncpy(product->data, crosslink, PRODUCT_CODE_LENGTH);

					// Log Crosslink
					INFO_LOG(Log::sceNet, "Crosslinked %s to %s", productid, crosslink);

					// Set Crosslinked Flag
					crosslinked = 1;
				}
			}

			// Destroy Prepared SQL Statement
			sqlite3_finalize(statement);
		}*/
		for (const auto &link : crosslinks) {
			if (IsMatch(link.id_from, productid)) {
				// Grab Crosslink ID
				char crosslink[PRODUCT_CODE_LENGTH + 1];
				strncpy(crosslink, link.id_to, PRODUCT_CODE_LENGTH);
				crosslink[PRODUCT_CODE_LENGTH] = 0; // null terminated

				// Crosslink Product Code
				strncpy(product->data, link.id_to, PRODUCT_CODE_LENGTH);

				// Log Crosslink
				INFO_LOG(Log::sceNet, "AdhocServer: Crosslinked %s to %s", productid, crosslink);

				// Set Crosslinked Flag
				crosslinked = 1;
				break;
			}
		}

		// Not Crosslinked
		if(!crosslinked)
		{
			// Prepare SQL Statement
			/*if(sqlite3_prepare_v2(db, sql2, strlen(sql2) + 1, &statement, NULL) == SQLITE_OK)
			{
				// Bind SQL Statement Data
				if(sqlite3_bind_text(statement, 1, productid, strlen(productid), SQLITE_STATIC) == SQLITE_OK)
				{
					// Found Matching Row
					if(sqlite3_step(statement) == SQLITE_ROW)
					{
						// Set Exists Flag
						exists = 1;
					}
				}

				// Destroy Prepare SQL Statement
				sqlite3_finalize(statement);
			}*/
			for (const auto &product : productids) {
				if (IsMatch(product.id, productid)) {
					// Set Exists Flag
					exists = 1;
					break;
				}
			}

			// Game doesn't exist in Database
			if(!exists)
			{
				// Prepare SQL Statement
				/*if(sqlite3_prepare_v2(db, sql3, strlen(sql3) + 1, &statement, NULL) == SQLITE_OK)
				{
					// Bind SQL Statement Data
					if(sqlite3_bind_text(statement, 1, productid, strlen(productid), SQLITE_STATIC) == SQLITE_OK && sqlite3_bind_text(statement, 2, productid, strlen(productid), SQLITE_STATIC) == SQLITE_OK)
					{
						// Save Product ID to Database
						if(sqlite3_step(statement) == SQLITE_DONE)
						{
							// Log Addition
							INFO_LOG(Log::sceNet, "Added Unknown Product ID %s to Database", productid);
						}
					}

					// Destroy Prepare SQL Statement
					sqlite3_finalize(statement);
				}*/
				db_productid unkproduct;
				strncpy(unkproduct.id, productid, sizeof(unkproduct.id));
				strncpy(unkproduct.name, productid, sizeof(unkproduct.name));
				productids.push_back(unkproduct); //productids[productids.size()] = unkproduct;
				// Log Addition
				INFO_LOG(Log::sceNet, "AdhocServer: Added Unknown Product ID %s to Database", productid);
			}
		}

		// Close Database
		//sqlite3_close(db);
	}
}

/**
 * Update Status Logfile
 */
void update_status()
{
	//// Open Logfile
	//FILE * log = File::OpenCFile(Path(SERVER_STATUS_XMLOUT), "w");

	//// Opened Logfile
	//if(log != NULL)
	//{
	//	// Write XML Header
	//	fprintf(log, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");

	//	// Write XSL Processor Information
	//	fprintf(log, "<?xml-stylesheet type=\"text/xsl\" href=\"status.xsl\"?>\n");

	//	// Output Root Tag + User Count
	//	fprintf(log, "<prometheus usercount=\"%u\">\n", _db_user_count);

	//	// Database Handle
	//	//sqlite3 * db = NULL;

	//	// Open Database
	//	//if(sqlite3_open(SERVER_DATABASE, &db) == SQLITE_OK)
	//	{
	//		// Iterate Games
	//		SceNetAdhocctlGameNode * game = _db_game; for(; game != NULL; game = game->next)
	//		{
	//			// Safe Product ID
	//			char productid[PRODUCT_CODE_LENGTH + 1];
	//			strncpy(productid, game->game.data, PRODUCT_CODE_LENGTH);
	//			productid[PRODUCT_CODE_LENGTH] = 0;

	//			// Display Name
	//			char displayname[128];
	//			memset(displayname, 0, sizeof(displayname));

	//			// SQL Statement
	//			//const char * sql = "SELECT name FROM productids WHERE id=?;";

	//			// Prepared SQL Statement
	//			//sqlite3_stmt * statement = NULL;

	//			// Prepare SQL Statement
	//			/*if(sqlite3_prepare_v2(db, sql, strlen(sql) + 1, &statement, NULL) == SQLITE_OK)
	//			{
	//				// Bind SQL Statement Data
	//				if(sqlite3_bind_text(statement, 1, productid, strlen(productid), SQLITE_STATIC) == SQLITE_OK)
	//				{
	//					// Found Matching Row
	//					if(sqlite3_step(statement) == SQLITE_ROW)
	//					{
	//						// Fetch Game Name from Database
	//						const char * gamename = (const char *)sqlite3_column_text(statement, 0);

	//						// Copy Game Name
	//						strcpyxml(displayname, gamename, sizeof(displayname));
	//					}

	//					// Game not in Database
	//					else
	//					{
	//						// Use Product Code as Name
	//						strcpyxml(displayname, productid, sizeof(displayname));
	//					}
	//				}

	//				// Destroy Prepared SQL Statement
	//				sqlite3_finalize(statement);
	//			}*/
	//			//db_productid *foundid = NULL;
	//			bool found = false;
	//			for (const auto &product : productids) {
	//				if (IsMatch(product.id, productid)) {
	//					// Copy Game Name
	//					strcpyxml(displayname, product.name, sizeof(displayname));
	//					found = true;
	//					break;
	//				}
	//			}

	//			if (!found) {
	//				// Use Product Code as Name
	//				strcpyxml(displayname, productid, sizeof(displayname));
	//			}

	//			// Output Game Tag + Game Name
	//			fprintf(log, "\t<game name=\"%s\" usercount=\"%u\">\n", displayname, game->playercount);

	//			// Activate User Count
	//			uint32_t activecount = 0;

	//			// Iterate Game Groups
	//			SceNetAdhocctlGroupNode * group = game->group; for(; group != NULL; group = group->next)
	//			{
	//				// Safe Group Name
	//				char groupname[ADHOCCTL_GROUPNAME_LEN + 1];
	//				strncpy(groupname, (const char *)group->group.data, ADHOCCTL_GROUPNAME_LEN);
	//				groupname[ADHOCCTL_GROUPNAME_LEN] = 0;

	//				// Output Group Tag + Group Name + User Count
	//				fprintf(log, "\t\t<group name=\"%s\" usercount=\"%u\">\n", strcpyxml(displayname, groupname, sizeof(displayname)), group->playercount);

	//				// Iterate Users
	//				SceNetAdhocctlUserNode * user = group->player; for(; user != NULL; user = user->group_next)
	//				{
	//					// Output User Tag + Username
	//					fprintf(log, "\t\t\t<user>%s</user>\n", strcpyxml(displayname, (const char *)user->resolver.name.data, sizeof(displayname)));
	//				}

	//				// Output Closing Group Tag
	//				fprintf(log, "\t\t</group>\n");

	//				// Increase Active Game User Count
	//				activecount += group->playercount;
	//			}

	//			// Output Idle Game Group
	//			if(game->playercount > activecount)
	//			{
	//				// Output Group Tag + Group Name + Idle User Count
	//				fprintf(log, "\t\t<group name=\"Groupless\" usercount=\"%u\" />\n", game->playercount - activecount);
	//			}

	//			// Output Closing Game Tag
	//			fprintf(log, "\t</game>\n");
	//		}

	//		// Close Database
	//		//sqlite3_close(db);
	//	}

	//	// Output Closing Root Tag
	//	fprintf(log, "</prometheus>");

	//	// Close Logfile
	//	fclose(log);
	//}
}

/**
 * Escape XML Sequences to avoid malformed XML files.
 * @param out Out Buffer
 * @param in In Buffer
 * @param size Size of Out Buffer
 * @return Reference to Out Buffer
 */
const char * strcpyxml(char * out, const char * in, uint32_t size)
{
	// Valid Arguments
	if(out != NULL && in != NULL && size > 0)
	{
		// Clear Memory
		memset(out, 0, size);

		// Written Size Pointer
		uint32_t written = 0;

		// Iterate In-Buffer Symbols
		uint32_t i = 0; for(; i < strlen(in); i++)
		{
			// " Symbol
			if(in[i] == '"')
			{
				// Enough Space in Out-Buffer (6B for &quot;)
				if((size - written) > 6)
				{
					// Write Escaped Sequence
					strcpy(out + written, "&quot;");

					// Move Pointer
					written += 6;
				}

				// Truncate required
				else break;
			}

			// < Symbol
			else if(in[i] == '<')
			{
				// Enough Space in Out-Buffer (4B for &lt;)
				if((size - written) > 4)
				{
					// Write Escaped Sequence
					strcpy(out + written, "&lt;");

					// Move Pointer
					written += 4;
				}

				// Truncate required
				else break;
			}

			// > Symbol
			else if(in[i] == '>')
			{
				// Enough Space in Out-Buffer (4B for &gt;)
				if((size - written) > 4)
				{
					// Write Escaped Sequence
					strcpy(out + written, "&gt;");

					// Move Pointer
					written += 4;
				}

				// Truncate required
				else break;
			}

			// & Symbol
			else if(in[i] == '&')
			{
				// Enough Space in Out-Buffer (5B for &amp;)
				if((size - written) > 5)
				{
					// Write Escaped Sequence
					strcpy(out + written, "&amp;");

					// Move Pointer
					written += 5;
				}

				// Truncate required
				else break;
			}

			// Normal Character
			else
			{
				// Enough Space in Out-Buffer (1B)
				if((size - written) > 1)
				{
					// Write Character
					out[written++] = in[i];
				}
			}
		}

		// Return Reference
		return out;
	}

	// Invalid Arguments
	return NULL;
}

/**
 * Server Entry Point
 * @param argc Number of Arguments
 * @param argv Arguments
 * @return OS Error Code
 */
int proAdhocServerThread(int port) // (int argc, char * argv[])
{
	SetCurrentThreadName("AdhocServer");
	// Result
	int result = 0;

#ifdef MAZAHAKA_PC // disable multiinstance shift + recheck addr
	if (net::HostPortExists(g_Config.sProAdhocServer, ADHOC_SERVER_PORT, 100, SOCK_STREAM)) { // TCP
		//INFO_LOG(Log::sceNet, "AdhocServer: Skipped starting because the server (%s) is already available", g_Config.sProAdhocServer.c_str());
		NOTICE_LOG(Log::sceNet, "AdhocServer: Skipped starting because the server (%s) is already available", g_Config.sProAdhocServer.c_str());
		return 0;
	}

	// Try to detect (resolve) server from cfg, if its bad (localhost3), didnt launch too
	else { // can't connect to sProAdhocServer, let's check is non isLoopbackIP, if isLoopbackIP - we can host it, else - unnecessary host thread

		//---------- localhost3 bad addr detect
		// cant connect to server from config, detect is need launch aemu by 
		//----------

		sockaddr_in addr_in;
		addrinfo* resolved = nullptr;
		std::string err;
		if (net::DNSResolve(g_Config.sProAdhocServer, "", &resolved, err) && resolved)
		{
			for (auto ptr = resolved; ptr != NULL; ptr = ptr->ai_next) {
				switch (ptr->ai_family) {
				case AF_INET:
					addr_in = *(sockaddr_in*)ptr->ai_addr;
					break;
				}
			}
			net::DNSResolveFree(resolved);
			addr_in.sin_port = htons(ADHOC_SERVER_PORT);

			// Resolve OK, it's can be nonstarted aemu in private net (LAN IP/LOOPBACK) or unavailable white public aemu host (socom, etc)

			////!!!!!!!!!!!!!!!!!!!!! + isPrivateIP(addr_in.sin_addr.s_addr)  192.168.0.152 isLoopbackIP(addr_in.sin_addr.s_addr)
			// if !loopback(we unavailable loopback, need start aemu) && (my local interface ip != addr_in.sin_addr.s_addr)
			bool bIsPublicAemuServerAddr = !isPrivateIP(addr_in.sin_addr.s_addr);
			if (bIsPublicAemuServerAddr) {
				// skip public
				// we can't connect to public server, skip starting local aemu hosting thread because we want public server in cfg
				NOTICE_LOG(Log::sceNet, "AdhocServer: Skipped starting because the public server (%s) is unavailable", g_Config.sProAdhocServer.c_str());
				return 0;
			}
			// smth for lan ip ?
		}
		else {
			ERROR_LOG(Log::sceNet, "AdhocServer: Skipped starting because: DNS Error Resolving from config %s\n", g_Config.sProAdhocServer.c_str());
			return 0; // didn't launch aemu, idk server from cfg, ex. localhost3
		}
	}
	// Unable to resolve DNS to determine if the host is public or sProAdhocServer is LoopbackIP (bEnableWlan + bEnableAdhocServer + loopback)
#endif
	NOTICE_LOG(Log::sceNet, "AdhocServer: Begin of AdhocServer (%s) Thread", g_Config.sProAdhocServer.c_str());

	// Create Signal Receiver for CTRL + C
	//signal(SIGINT, interrupt);

	// Create Signal Receiver for kill / killall
	//signal(SIGTERM, interrupt);

	// Create Listening Socket
	int server = create_listen_socket(port); // ADHOC_SERVER_PORT (proAdhoc.h)

	// Created Listening Socket
	if(server != SOCKET_ERROR)
	{
		// Notify User
		INFO_LOG(Log::sceNet, "AdhocServer: Listening for Connections on TCP Port %u", port); // ADHOC_SERVER_PORT

#ifdef WITH_UPNP
		// Port forward
		UPnP_Add(IP_PROTOCOL_TCP, port); // g_PortManager.Add(IP_PROTOCOL_TCP, port);
#endif

		// Enter Server Loop
		result = server_loop(server);

#ifdef WITH_UPNP
		// Remove Port mapping
		UPnP_Remove(IP_PROTOCOL_TCP, port); // g_PortManager.Remove(IP_PROTOCOL_TCP, port);
#endif

		// Notify User
		INFO_LOG(Log::sceNet, "AdhocServer: Shutdown complete");
	}

	//_status = 0;
	adhocServerRunning = false;

	INFO_LOG(Log::sceNet, "AdhocServer: End of AdhocServer Thread");

	// Return Result
	return result;
}

/**
 * Server Shutdown Request Handler
 * @param sig Captured Signal
 */
void interrupt(int sig)
{
	// Notify User
	INFO_LOG(Log::sceNet, "AdhocServer: Shutting down... please wait");

	// Trigger Shutdown
	//_status = 0;
	adhocServerRunning = false;
}

/**
 * Enable Address Reuse on Socket
 * @param fd Socket
 */
void enable_address_reuse(int fd)
{
	// Enable Port Reuse
	setSockReuseAddrPort(fd);
}

/**
 * Enable KeepAlive on Socket
 * @param fd Socket
 */
void enable_keepalive(int fd)
{
	// Enable Value
	int on = 1;

	// Enable Port Reuse
	setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, (const char*)&on, sizeof(on));
}

/**
 * Change TCP Socket TCP_NODELAY (Nagle Algo) mode
 * @param fd Socket
 * @param nonblocking 1 for Nonblocking, 0 for Blocking
 */
void change_nodelay_mode(int fd, int flag)
{
	int opt = flag;
	setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (char*)&opt, sizeof(opt));
}

/**
 * Change Socket Blocking Mode
 * @param fd Socket
 * @param nonblocking 1 for Nonblocking, 0 for Blocking
 */
void change_blocking_mode(int fd, int nonblocking)
{
	unsigned long on = 1;
	unsigned long off = 0;
#ifdef _WIN32
	if (nonblocking){
		// Change to Non-Blocking Mode
		ioctlsocket(fd, FIONBIO, &on);
	}
	else {
		// Change to Blocking Mode
		ioctlsocket(fd, FIONBIO, &off);
	}
#else
	// Change to Non-Blocking Mode
	if(nonblocking) fcntl(fd, F_SETFL, O_NONBLOCK);

	// Change to Blocking Mode
	else
	{
		// Get Flags
		int flags = fcntl(fd, F_GETFL);

		// Remove Non-Blocking Flag
		fcntl(fd, F_SETFL, flags & ~O_NONBLOCK);
	}
#endif
}

/**
 * Create Port-Bound Listening Socket
 * @param port TCP Port
 * @return Socket Descriptor
 */
int create_listen_socket(uint16_t port)
{
	// Create Socket
	int fd = (int)socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	// Created Socket
	if(fd != -1)
	{
		// Ignore SIGPIPE when supported (ie. BSD/MacOS)
		setSockNoSIGPIPE(fd, 1);

		// Enable KeepAlive
		enable_keepalive(fd);

		// Enable Address Reuse
		enable_address_reuse(fd); // Shouldn't Reuse the port for built-in AdhocServer to prevent conflict with Dedicated AdhocServer

		// Make Socket Nonblocking
		change_blocking_mode(fd, 1);

		// Make TCP Socket send immediately
		change_nodelay_mode(fd, 1);

		// Prepare Local Address Information
		struct sockaddr_in local;
		memset(&local, 0, sizeof(local));
		local.sin_family = AF_INET;
		local.sin_addr.s_addr = INADDR_ANY; // 0.0.0.0
		local.sin_port = htons(port);

		// Should only bind to specific IP for the 2nd or more instance of PPSSPP to prevent communication interference issue when sharing the same port. (ie. Capcom Classics Collection Remixed)
		// ppsspp bad logic
		// я переделал чтобы мультиинстансы не сдвигали локал а не запускали если есть коннект или запускали свой 0.1 на мультиинстансе (например крашнул хост)
#ifndef MAZAHAKA_PC // PPSSPP cant host in localhost in multiinstanse, need remake more hackly startup server
		if (GAME_ID > 1) {
			local.sin_addr = g_localhostIP.in.sin_addr;
		}
#endif

		// Bind Local Address to Socket
		int bindresult = bind(fd, (struct sockaddr *)&local, sizeof(local));

		// Bound Local Address to Socket
		if(bindresult != SOCKET_ERROR)
		{
			// Switch Socket into Listening Mode
			listen(fd, SERVER_LISTEN_BACKLOG);

			// Return Socket
			return fd;
		}

		// Notify User
		else {
			ON_SOCKET_ERROR();
			ERROR_LOG(Log::sceNet, "AdhocServer: Bind returned %i (Socket error %d)", bindresult, socket_errno);
		}

		// Close Socket
		ON_CLOSE_SOCKET(fd);
	} else {
		// Notify User
		ON_SOCKET_ERROR();
		ERROR_LOG(Log::sceNet, "AdhocServer: Socket returned %i (Socket error %d)", fd, socket_errno);
	}

	// Return Error
	return -1;
}

#ifdef DUMP_AEMU_SERVER_INFO
void print_status_console()
{
	static time_t lastTimeLog = 0;
	time_t now = time(NULL);
	if ( ! (lastTimeLog == 0 || (now - lastTimeLog) >= 5)) { // 5 sec   !invert allow rule
		return; // skip print
	}
	lastTimeLog = now;

	//void OpenConsole();
	//OpenConsole();
	//std::lock_guard<std::mutex> lock(g_db_mutex);

	printf("\n===== Adhoc Server Status (%s) =====\n", g_Config.sProAdhocServer.c_str());

	// Iterate games -> groups -> players
	SceNetAdhocctlGameNode* game = _db_game;
	if (game == NULL) {
		printf("No games currently.\n");
	}
	else {
		for (; game != NULL; game = game->next)
		{
			char safegamestr[PRODUCT_CODE_LENGTH + 1] = { 0 };
			strncpy(safegamestr, game->game.data, PRODUCT_CODE_LENGTH);
			safegamestr[PRODUCT_CODE_LENGTH] = 0;

			SceNetAdhocctlGroupNode* g = game->group;
			if (g == NULL) continue;

			for (; g != NULL; g = g->next)
			{
				char safegroupstr[ADHOCCTL_GROUPNAME_LEN + 1] = { 0 };
				strncpy(safegroupstr, (char*)g->group.data, ADHOCCTL_GROUPNAME_LEN);
				safegroupstr[ADHOCCTL_GROUPNAME_LEN] = 0;

				printf("Room %s (Game %s):\n", safegroupstr, safegamestr);

				SceNetAdhocctlUserNode* u = g->player;
				if (u == NULL) {
					printf("  (empty)\n");
				}
				for (; u != NULL; u = u->group_next)
				{
					const char* name = (const char*)u->resolver.name.data;
					if (name == NULL || name[0] == 0) name = "(unnamed)";
					printf("  %s\n", name);
				}
			}
		}
	}

	// Print users not in rooms
	printf("Users (not in rooms):\n");
	SceNetAdhocctlUserNode* user = _db_user;
	bool anyFree = false;
	for (; user != NULL; user = user->next)
	{
		if (user->group == NULL)
		{
			const char* name = (const char*)user->resolver.name.data;
			if (name == NULL || name[0] == 0) name = "(unnamed)";
			printf("  %s\n", name);
			anyFree = true;
		}
	}
	if (!anyFree) printf("  (none)\n");

	printf("===== End Status =====\n\n");
}
#endif

SceNetAdhocctlUserNode* find_user(SceNetEtherAddr* mac) {
	SceNetAdhocctlUserNode* user = _db_user;
	while (user != NULL) {
		if (memcmp(&user->resolver.mac, mac, sizeof(SceNetEtherAddr)) == 0) {
			return user;
		}
		user = user->next;
	}
	return NULL;
}

/**
 * Server Main Loop
 * @param server Server Listening Socket
 * @return OS Error Code
 */
int server_loop(int server)
{
	// Set Running Status
	//_status = 1;
	adhocServerRunning = true;

	// Create Empty Status Logfile
	update_status();

	// Handling Loop
	while (adhocServerRunning) //(_status == 1)
	{
#ifdef DUMP_AEMU_SERVER_INFO
		print_status_console(); // delay into
#endif

		// Login Block
		{
			// Login Result
			int loginresult = 0;

			// Login Processing Loop
			do
			{
				// Prepare Address Structure
				struct sockaddr_in addr;
				socklen_t addrlen = sizeof(addr);
				memset(&addr, 0, sizeof(addr));

				// Accept Login Requests
				// loginresult = accept4(server, (struct sockaddr *)&addr, &addrlen, SOCK_NONBLOCK);

				// Alternative Accept Approach (some Linux Kernel don't support the accept4 Syscall... wtf?)
				loginresult = accept(server, (struct sockaddr *)&addr, &addrlen);
				if(loginresult != -1)
				{
					// Switch Socket into Non-Blocking Mode
					change_blocking_mode(loginresult, 1);
				}

				// Login User (Stream)
				if (loginresult != -1) {
					uint32_t sip = addr.sin_addr.s_addr;
					/* // Replacing 127.0.0.x with Ethernet IP will cause issue with multiple-instance of localhost (127.0.0.x)
					if (sip == 0x0100007f) { //127.0.0.1 should be replaced with LAN/WAN IP whenever available
						char str[100];
						gethostname(str, 100);
						u8 *pip = (u8*)&sip;
						if (gethostbyname(str)->h_addrtype == AF_INET && gethostbyname(str)->h_addr_list[0] != NULL) pip = (u8*)gethostbyname(str)->h_addr_list[0];
						sip = *(u32_le*)pip;
						WARN_LOG(Log::sceNet, "AdhocServer: Replacing IP %s with %s", inet_ntoa(addr.sin_addr), inet_ntoa(*(in_addr*)&pip));
					}
					*/
					login_user_stream(loginresult, sip);
				}
			} while(loginresult != -1);
		}

		// Receive Data from Users
		SceNetAdhocctlUserNode * user = _db_user;
		while(user != NULL)
		{
			// Next User (for safe delete)
			SceNetAdhocctlUserNode * next = user->next;

			// Receive Data from User
			int recvresult = (int)recv(user->stream, (char*)user->rx + user->rxpos, sizeof(user->rx) - user->rxpos, MSG_NOSIGNAL);

			// Connection Closed or Timed Out
			if(recvresult == 0 || (recvresult == -1 && socket_errno != EAGAIN && socket_errno != EWOULDBLOCK) || get_user_state(user) == USER_STATE_TIMED_OUT)
			{
				// Logout User
				logout_user(user);
			}

			// Received Data (or leftovers in RX-Buffer)
			else if(recvresult > 0 || user->rxpos > 0)
			{
				// New Incoming Data
				if(recvresult > 0)
				{
					// Move RX Pointer
					user->rxpos += recvresult;

					// Update Death Clock
					user->last_recv = time(NULL);
				}

				// Waiting for Login Packet
				if(get_user_state(user) == USER_STATE_WAITING)
				{
					// Valid Opcode
					if(user->rx[0] == OPCODE_LOGIN)
					{
						// Enough Data available
						if(user->rxpos >= sizeof(SceNetAdhocctlLoginPacketC2S))
						{
							// Clone Packet
							SceNetAdhocctlLoginPacketC2S packet = *(SceNetAdhocctlLoginPacketC2S *)user->rx;

							// Remove Packet from RX Buffer
							clear_user_rxbuf(user, sizeof(SceNetAdhocctlLoginPacketC2S));

							// Login User (Data)
							login_user_data(user, &packet);
						}
					}

					// Invalid Opcode
					else
					{
						// Notify User
						WARN_LOG(Log::sceNet, "AdhocServer: Invalid Opcode 0x%02X in Waiting State from %s", user->rx[0], ip2str(*(in_addr*)&user->resolver.ip).c_str());

						// Logout User
						logout_user(user);
					}
				}

				// Logged-In User
				else if(get_user_state(user) == USER_STATE_LOGGED_IN)
				{
					// Ping Packet
					if(user->rx[0] == OPCODE_PING)
					{
						// Delete Packet from RX Buffer
						clear_user_rxbuf(user, 1);
					}

					// Group Connect Packet
					else if(user->rx[0] == OPCODE_CONNECT)
					{
						// Enough Data available
						if(user->rxpos >= sizeof(SceNetAdhocctlConnectPacketC2S))
						{
							// Cast Packet
							SceNetAdhocctlConnectPacketC2S * packet = (SceNetAdhocctlConnectPacketC2S *)user->rx;

							// Clone Group Name
							SceNetAdhocctlGroupName group = packet->group;

							// Remove Packet from RX Buffer
							clear_user_rxbuf(user, sizeof(SceNetAdhocctlConnectPacketC2S));

							// Change Game Group
							connect_user(user, &group);
						}
					}

					// Group Disconnect Packet
					else if(user->rx[0] == OPCODE_DISCONNECT)
					{
						// Remove Packet from RX Buffer
						clear_user_rxbuf(user, 1);

						// Leave Game Group
						disconnect_user(user);
					}

					// Network Scan Packet
					else if(user->rx[0] == OPCODE_SCAN)
					{
						// Remove Packet from RX Buffer
						clear_user_rxbuf(user, 1);

						// Send Network List
						send_scan_results(user);
					}

					// Chat Text Packet
					else if(user->rx[0] == OPCODE_CHAT)
					{
						// Enough Data available
						if(user->rxpos >= sizeof(SceNetAdhocctlChatPacketC2S))
						{
							// Cast Packet
							SceNetAdhocctlChatPacketC2S * packet = (SceNetAdhocctlChatPacketC2S *)user->rx;

							// Clone Buffer for Message
							char message[64];
							memset(message, 0, sizeof(message));
							strncpy(message, packet->message, sizeof(message) - 1);

							// Remove Packet from RX Buffer
							clear_user_rxbuf(user, sizeof(SceNetAdhocctlChatPacketC2S));

							// Spread Chat Message
							spread_chat_message(user, message);
						}
					}

#ifdef ADHOC_PTP_PDP_WRAPPERS
					// CSC Packet
					else if (user->rx[0] == OPCODE_MZHK)
					{
						// Enough Data for Header
						if (user->rxpos >= sizeof(SceNetAdhocctlPDPPTPPacketC2S))
						{
							// Cast Header
							SceNetAdhocctlPDPPTPPacketC2S* header = (SceNetAdhocctlPDPPTPPacketC2S*)user->rx;
							// Data Size
							uint16_t datasize = header->size;
							// Total Packet Size
							size_t packetsize = sizeof(SceNetAdhocctlPDPPTPPacketC2S) + datasize;
							// Enough Data for Full Packet
							if (user->rxpos >= packetsize)
							{
								// Find Target User by MAC
								SceNetAdhocctlUserNode* target = find_user(&header->target_mac);
								// Target Found
								if (target != NULL)
								{
									// Forward the Packet (S2C is same struct)
									int sent = (int)send(target->stream, (const char*)user->rx, packetsize, MSG_NOSIGNAL);
									// Check Send Result
									if (sent == packetsize)
									{
										NOTICE_LOG(Log::sceNet, "AdhocServer: Forwarded MZHK Packet (size=%u) from %s to %s", packetsize, mac2str(&user->resolver.mac).c_str(), mac2str(&target->resolver.mac).c_str());
									}
									else
									{
										ERROR_LOG(Log::sceNet, "AdhocServer: Failed to forward MZHK Packet to %s (sent=%d)", mac2str(&target->resolver.mac).c_str(), sent);
									}
								}
								else
								{
									WARN_LOG(Log::sceNet, "AdhocServer: MZHK Packet target MAC %s not found", mac2str(&header->target_mac).c_str());
								}
								// Remove Packet from RX Buffer
								clear_user_rxbuf(user, packetsize);
							}
						}
					}
#endif

					// Invalid Opcode
					else
					{
						// Notify User
						WARN_LOG(Log::sceNet, "AdhocServer: Invalid Opcode 0x%02X in Logged-In State from %s (MAC: %s - IP: %s)", user->rx[0], (char *)user->resolver.name.data, mac2str(&user->resolver.mac).c_str(), ip2str(*(in_addr*)&user->resolver.ip).c_str());

						// Logout User
						logout_user(user);
					}
				}
			}

			// Move Pointer
			user = next;
		}

		// Prevent needless CPU Overload (1ms Sleep)
		// !!!! COMMENT NOT REFLECTING REALITY
		sleep_ms(10, "pro-adhoc-poll");

		//// Don't do anything if it's paused, otherwise the log will be flooded
		//while (adhocServerRunning && Core_IsStepping() && coreState != CORE_POWERDOWN)
		//	sleep_ms(10, "pro-adhot-paused-poll");
	}

	// Free User Database Memory
	free_database();

	// Close Server Socket
	ON_CLOSE_SOCKET(server);

	// Return Success
	return 0;
}
