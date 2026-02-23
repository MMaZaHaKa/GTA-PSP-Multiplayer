/*
    Authors: MaZaHaKa, ASM95
    Do not delete this comment block. Respect others' work!
*/
#pragma once
#ifndef PACKET_H
#define PACKET_H

#include "common.h"
#include "main.h"

#include "multiplayer/net/public.h"
//#include "multiplayer/elements/sSyncStream.h"

class sReadSyncStream;
class sWriteSyncStream;

#define MP_NET_MAX_INFO_ENTRY_SZ (16)
#ifdef GTA_LIBERTY
#define PS(lcssz, vcssz) lcssz
#else
#define PS(lcssz, vcssz) vcssz
#endif

#define STRU_PAD_PASTE(a,b) a##b
#define STRU_PAD(num, size) \
    uint8 STRU_PAD_PASTE(_pad, num)[size];
#define STRU_PAD_LINE(size) \
    uint8 STRU_PAD_PASTE(_pad, __LINE__)[size];
#if defined(__COUNTER__)
#define STRU_PAD_C(size) \
    uint8 STRU_PAD_PASTE(_pad, __COUNTER__)[size];
#else
#define STRU_PAD_C(size) STRU_PAD_LINE(size)
#endif

#define MP_TEST_PACKETS
#define MPZ (0) // psp unknown/undone/unused packet size dummy

namespace net {

#pragma pack(push, 1)
	struct pckt_def {
		const char* pckt_name; // def
		uint8 pckt_id; // def
		uint8 pckt_size; // def, struct size
	};

	struct pckt {
		uint16 pckt_size;

		inline uint32 CalcHash() { return fast_hash32(this, pckt_size); }
	};

	struct pckt_base : pckt {
		uint8 pckt_id;
	};

	// inherit with caution field names!
	struct pckt_with_element {
		uint32 owner;
		uint32 elem;
	};

	struct pckt_info_peer {
		uint16 port; // why not tListenAddr
		tMacAddr mac;
		int32 nRandom;
	};


	// Packets
	struct pckt_start_fire : pckt_base { // ID 0
		RwV3d pos;
		float strength;
		bool propagation;
		int16 source;
		int16 entity;
	};

	struct pckt_ack : pckt_base { // ID 1
		uint16 nFlags;
	};

	struct pckt_info : pckt_base { // ID 2
		STRU_PAD(3, 1);
		uint32 nMagic; // 'INFO' 0x494E464F
		uint16 nPeerA;
		uint16 nPeerB;
		pckt_info_peer aPeers[MP_NET_MAX_INFO_ENTRY_SZ];
	};

	struct pckt_player_kill : pckt_base { // ID 3
		uint8 assassin;
	};

	struct pckt_kick_player : pckt_base { // ID 4
		uint8 peer_id;
	};

	struct pckt_request_kick_player : pckt_base { // ID 5 (non vcs)
		uint8 peer_id;
	};

	struct pckt_set_team_score : pckt_base { // ID 6
		/* team id to which score has to be set */
		uint8 team_id;
		int32 score;
	};

	struct pckt_send_game_event : pckt_base { // ID 7
		/* id of event to be sent */
		int32 event;
	};

	struct pckt_force_ped_from_vehicle : pckt_base, pckt_with_element { // ID 8
	};

	struct pckt_set_vehicle_emergency_break_state : pckt_base, pckt_with_element { // ID 9
		bool enabled;
	};

	struct pckt_set_carlocked_state : pckt_base, pckt_with_element { // ID 10
		int32 state;
	};

	struct pckt_repair_car : pckt_base, pckt_with_element { // ID 11
	};

	struct pckt_set_tyres_no_burst : pckt_base, pckt_with_element { // ID 12
		bool enabled;
	};

	struct pckt_delete_vehicle : pckt_base, pckt_with_element { // ID 13
	};

	struct pckt_game_state : pckt_base { // ID 14
		int16 zone;
		uint8 sequence;
	};

	struct pckt_play_remote_sound : pckt_base { // ID 15
		uint32 sample;
		uint8 bank;
		uint32 counter;
		uint32 priority;
		uint32 frequency;
		uint8 volume;
		uint8 frames;
	};

	struct pckt_heart_beat : pckt_base { // ID 16
	};

	struct pckt_transfer_entity : pckt_base { // ID 17
		int8 src;
		int8 dest;
		int16 elem;
	};

	struct pckt_clock : pckt_base { // ID 18 (non vcs)
		TODO(); // from lcs
	};

	struct pckt_game_time : pckt_base { // ID 19
		struct {
			uint8 min;
			uint8 sec;
		} time;
		/* updated every 100 centiseconds */
		uint32 elapsedMs;
		int32 nTeamATime;
		int32 nTeamBTime;
	};

	struct pckt_target_player : pckt_base { // ID 20
		uint8 player;
	};

	struct pckt_set_fixed_camera : pckt_base { // ID 21
		RwV3d pos;
		RwV3d target;
	};

	struct pckt_restore_camera : pckt_base { // ID 22
		uint8 jumpcut : 1; // bit0 - IsJumpcut
	};

	struct pckt_pickup_collected : pckt_base { // ID 23
		uint16 elem;
		bool remove;
#ifndef GTA_LIBERTY
		int16 modelIndex;
#endif
	};

	struct pckt_pickup_request : pckt_base { // ID 24 old sMessagePickupRequest
		uint16 elem;
	};

	struct pckt_powerup_collected : pckt_base { // ID 25
		uint8 powerup_type; // ePowerupType
		uint8 amount;
		uint8 player;
	};

	struct pckt_set_vehicle_infinite_mass : pckt_base, pckt_with_element { // ID 26
		uint8 mass : 1; // bit0 - infinite mass
	};

	// todo new
	struct pckt_fight_hit_ped : pckt_base { // ID 27
		// Rechek + test
		TODO();
#ifndef GTA_LIBERTY
#endif
		uint16 victim_id;
		uint16 attacker_id;
		uint16 ped_piece; // u8? +pad
		uint8 damage_mult;
		uint8 local_dir;
		uint8 cur_fight_move;
		uint8 weapon;
		RwV3d blood_pos;
		RwV3d dir;
	};

	// todo new
	struct pckt_shot_ped : pckt_base { // ID 28
		// Rechek + test
		TODO();
		uint16 victim_id;
		uint16 shooter_id;
		uint16 ped_piece; // u8? +pad
		uint8 damage_mult;
		uint8 weapon;
		uint8 local_dir;
		RwV2d pos_offset;
		RwV3d point;
	};

	struct pckt_fire_instant_hit : pckt_base { // ID 29
		uint8 weapon_type;
		RwV3d fire_source;
		RwV3d target;
		RwV2d move_speed;
		// player ID who performed the shot
		uint16 shooter_id;
		uint16 firing_rate;
		uint8 changed_heading : 1; // bit0
		uint8 shooter_moving : 1; // bit1
	};

	struct pckt_fire_sniper : pckt_base { // ID 30
		uint8 weapon_type;
		RwV3d fire_source;
		RwV3d target; // direction
		// player ID who performed the shot
		uint16 shooter_id;
	};

	struct pckt_fire_shotgun : pckt_base { // ID 31
		uint8 weapon_type;
		RwV3d fire_source;
		float angle;
		// player ID who performed the shot
		uint16 shooter_id;
		RwV3d pos;
		bool has_attacker;
	};

	struct pckt_fire_projectile : pckt_base { // ID 32
		uint8 weapon_type;
		// no mattr w pad
		RwV3d matr_forward;
		RwV3d matr_right;
		RwV3d matr_up;
		RwV3d matr_pos;
		RwV3d velocity;
		bool gravity;
		int32 time;
		float elasticity;
		uint8 special_collision_response;
		// player ID who performed the shot
		uint16 shooter_id;
	};

	struct pckt_use_detonator : pckt_base { // ID 33
		// player ID who performed the shot
		uint16 shooter_id;
	};

	struct pckt_fire_area_effect : pckt_base { // ID 34
		uint8 weapon_type;
		RwV3d fire_source;
		RwV3d target;
		RwV3d direction;
		// player ID who performed the shot
		uint16 shooter_id;
	};

	struct pckt_shot_ped_from_car : pckt_base { // ID 35
		// player ID who got shot
		uint16 victim_id;
		// player ID who performed the shot
		uint16 shooter_id;
		uint16 ped_piece;
		// damage value of the weapon
		uint16 damage;
		// direction from the damage
		uint8 direction;
		// position to create particle effect
		RwV3d pos;
	};

	struct pckt_fire_instant_hit_car : pckt_base { // ID 36
		RwV3d fire_source;
		RwV3d target;
		RwV3d move_speed;
		// player ID who performed the shot
		uint16 shooter_id;
	};

	struct pckt_kill_player_ped : pckt_base { // ID 37
		uint32 player_id;
	};

	struct pckt_shot_vehicle : pckt_base { // ID 38
		// player ID who performed the shot
		uint16 shooter_id;
		// vehicle ID which received the damage
		uint16 vehicle_id;
		// damage value of the weapon
		uint16 damage;
		// weapon type
		uint8 type;
	};

	// todo new
	struct pckt_melee : pckt_base { // ID 39

	};

	struct pckt_player_been_hit : pckt_base { // ID 40
		uint8 player; // leeds bug! must be uint16
		uint8 impact;
	};

	struct pckt_player_control : pckt_base { // ID 41
		uint8 player_control_toggle_type : 1; // bit0 - toggle state
		uint8 player_control_toggle_value : 1; // bit1 - toggle value
	};

	struct pckt_set_position : pckt_base { // ID 42
		RwV3d pos;
	};

	struct pckt_set_heading : pckt_base { // ID 43
		float heading;
	};

	struct pckt_add_3d_marker : pckt_base, pckt_with_element { // ID 44
	};

	struct pckt_remove_3d_marker : pckt_base, pckt_with_element { // ID 45
	};

	struct pckt_enable_roads : pckt_base { // ID 46
		RwV3d posMin;
		RwV3d posMax;
		uint8 toggle : 1; // bit0
	};

	struct pckt_clear_area : pckt_base { // ID 47
		RwV3d pos;
		float radius;
	};

	struct pckt_set_player_blip_visible_state : pckt_base { // ID 48
		uint8 player_id;
		bool visible;
	};

	struct pckt_player_respawn : pckt_base { // ID 49
	};

	struct pckt_add_explosion : pckt_base { // ID 50
		RwV3d pos;
		uint32 lifetime;
		uint8 type;
		bool hasSound;
		int16 cause;
		int16 culprit;
	};

	struct pckt_print_now : pckt_base { // ID 51
		// base size by base + strlen() + '0'
		int32 time;
		int32 flag;
		char key[257];
	};

	struct pckt_set_waypoint : pckt_base { // ID 52
		uint16 nWaypointID;
		RwV3d vecPos;
		RwV3d vecLookAt; // direction
		RwV3d vecHitSize;
		float fMarkerSize;
		CRGBA colour;
		float fArrowHeight;
		bool bShowArrow;
#ifndef GTA_LIBERTY
		int32 nType;
#endif
	};

	struct pckt_clear_waypoint : pckt_base { // ID 53
		uint16 nWaypointID;
	};

	struct pckt_hit_waypoint : pckt_base { // ID 54 (valakas)
		uint16 nWaypointID;
	};

	struct pckt_spawn_car_debris : pckt_base, pckt_with_element { // ID 55
		int32 car_component;
		uint32 car_component_type;
		int32 unk1;
		RwV3d move_speed;
		RwV3d turn_speed;
		uint32 render_scorched : 1; // :/
	};

	struct pckt_set_vehicle_health : pckt_base, pckt_with_element { // ID 56
		float health;
	};

	struct pckt_set_vehicle_position : pckt_base, pckt_with_element { // ID 57
		RwV3d pos;
	};

	struct pckt_vehicle_impact : pckt_base { // ID 58
		int32 src;
		int32 dest;
		RwV3d vPoint;
		RwV3d vNormal;
		uint8 pieceB;
	};

	struct pckt_msg_ready_for_cutscene : pckt_base { // ID 59
	};

#ifndef GTA_LIBERTY
	struct pckt_ack_entity_create : pckt_base { // ID 60
		uint16 entityId;
	};

	struct pckt_sync_peer_group : pckt_base { // ID 61
		uint8 group;
		uint8 peer_mask;
	};

	struct pckt_debug_break : pckt_base { // ID 62
		// guessed empty
	};

	struct pckt_msg_blowup_vehicle : pckt_base, pckt_with_element { // ID 63
		int32 player_id;
	};

	struct pckt_msg_create_lua_object : pckt_base { // ID 64
		bool isDestroy;
		bool isCreate;
		RwV3d pos;
		int32 object_id;
	};

	struct pckt_msg_server_ready_to_go : pckt_base { // ID 65
		bool bIsServerReadyToGo;
	};
#endif
#pragma pack(pop)


#ifdef MP_TEST_PACKETS
#pragma warning(disable: 4305)
#pragma warning(disable: 4309)
#define MP_TEST_SIZE(pcktname, type, pspsize, id) \
	SetConsoleColor((int32)(sizeof(type) == pspsize)); \
	debug("[%s] sizeof(%.3d) == psp(%.3d) (%s) [ID: %d]\n", (sizeof(type) == pspsize) ? "PASS" : "FAIL", sizeof(type), pspsize, pcktname, id); \
	SetConsoleColor(6);
#else
#define MP_TEST_SIZE(pcktname, type, pspsize)
#endif

	struct packet_id_list_t {
		static pckt_def* aPacketsDefs[100]; // snPacketCount
		static int32 snPacketCount;

		pckt_def start_fire;                        // "STARTFIRE"  on_recv_start_fire  ID: 0
		pckt_def ack;                               // "ACK"  on_recv_ack  ID: 1
		pckt_def info;                              // "INFO"  on_recv_info  ID: 2
		pckt_def player_kill;                       // "PLAYERKILL"  on_recv_player_kill  ID: 3
		pckt_def kick_player;                       // "KICKPLAYER"  on_recv_kick_player  ID: 4
//#ifdef GTA_LIBERTY
		pckt_def request_kick_player;               // "REQUESTKICKPLAYER"  on_recv_request_kick_player  ID: 5
//#else
////		pckt_def empty_packet_id0;              // "" ID: 5
//		pckt_def request_kick_player;               // ""  on_recv_request_kick_player  ID: 5
//#endif
		pckt_def set_team_score;                    // "SETTEAMSCORE"  on_recv_set_team_score  ID: 6
		pckt_def send_game_event;                   // "SENDMPGAMEEVENT"  on_recv_send_game_event  ID: 7
		pckt_def force_ped_from_vehicle;            // "FORCEPEDFROMVEHICLE"  on_recv_force_ped_from_vehicle  ID: 8
		pckt_def set_vehicle_emergency_break_state; // "SETVEHICLEEMERGENCYBRAKESTATE"  on_recv_set_vehicle_emergency_break_state  ID: 9
		pckt_def set_carlocked_state;               // "SETCARLOCKEDSTATE"  on_recv_set_carlocked_state  ID: 10
		pckt_def repair_car;                        // "REPAIRCAR"  on_recv_repair_car  ID: 11
		pckt_def set_tyres_no_burst;                // "SETTYRESNOBURST"  on_recv_set_tyres_no_burst  ID: 12
		pckt_def delete_vehicle;                    // "DELETEVEHICLE"  on_recv_delete_vehicle  ID: 13
		pckt_def game_state;                        // "GAMESTATE"  on_recv_game_state  ID: 14
		pckt_def play_remote_sound;                 // "PLAYREMOTESOUND"  on_recv_play_remote_sound  ID: 15
		pckt_def heart_beat;                        // "HEARTBEAT"  on_recv_heart_beat  ID: 16
		pckt_def transfer_entity;                   // "TRANSFER"  on_recv_transfer_entity  ID: 17
//#ifdef GTA_LIBERTY
		pckt_def clock;                             // "CLOCK"  on_recv_clock  ID: 18
//#else
////		pckt_def empty_packet_id1;              // "" ID: 18
//		pckt_def clock;                             // ""  on_recv_clock  ID: 18
//#endif
		pckt_def game_time;                         // "GAMETIME"  on_recv_game_time  ID: 19
		pckt_def target_player;                     // "TARGETPLAYER"  on_recv_target_player  ID: 20
		pckt_def set_fixed_camera;                  // "SETFIXEDCAMERA"  on_recv_set_fixed_camera  ID: 21
		pckt_def restore_camera;                    // "RESTORECAMERA"  on_recv_restore_camera  ID: 22
		pckt_def pickup_collected;                  // "PICKUPCOLLECTED"  on_recv_pickup_collected  ID: 23
		pckt_def pickup_request;                    // "PICKUPREQUEST"  on_recv_pickup_request  ID: 24
		pckt_def powerup_collected;                 // "POWERUPCOLLECTED"  on_recv_powerup_collected  ID: 25
		pckt_def set_vehicle_infinite_mass;         // "SETVEHICLEINFINITEMASS"  on_recv_set_vehicle_infinite_mass  ID: 26
		pckt_def fight_hit_ped;                     // "FIGHTHITPED"  on_recv_fight_hit_ped  ID: 27
		pckt_def shot_ped;                          // "SHOTPED"  on_recv_shot_ped  ID: 28
		pckt_def fire_instant_hit;                  // "FIREINSTANTHIT"  on_recv_fire_instant_hit  ID: 29
		pckt_def fire_sniper;                       // "FIRESNIPER"  on_recv_fire_sniper  ID: 30
		pckt_def fire_shotgun;                      // "FIRESHOTGUN"  on_recv_fire_shotgun  ID: 31
		pckt_def fire_projectile;                   // "FIREPROJECTILE"  on_recv_fire_projectile  ID: 32
		pckt_def use_detonator;                     // "USEDETONATOR"  on_recv_use_detonator  ID: 33
		pckt_def fire_area_effect;                  // "FIREAREAEFFECT"  on_recv_fire_area_effect  ID: 34
		pckt_def shot_ped_from_car;                 // "SHOTPEDFROMCAR"  on_recv_shot_ped_from_car  ID: 35
		pckt_def fire_instant_hit_car;              // "FIREINSTANTHITCAR"  on_recv_fire_instant_hit_car  ID: 36
		pckt_def kill_player_ped;                   // "KILLPLAYERPED"  on_recv_kill_player_ped  ID: 37
		pckt_def shot_vehicle;                      // "SHOTVEHICLE"  on_recv_shot_vehicle  ID: 38
		pckt_def melee;                             // "MELEE"  on_recv_melee  ID: 39
		pckt_def player_been_hit;                   // "PLAYERBEENHIT"  on_recv_player_been_hit  ID: 40
		pckt_def player_control;                    // "PLAYERCONTROL"  mp_on_recv_player_control  ID: 41
		pckt_def set_position;                      // "SETPOSITION"  on_recv_set_position  on_recv_player_set_position  ID: 42
		pckt_def set_heading;                       // "SETHEADING"  on_recv_set_heading  on_recv_player_set_heading  ID: 43
		pckt_def add_3d_marker;                     // "SET_3D_MARKER"  on_recv_add_3d_marker  ID: 44
		pckt_def remove_3d_marker;                  // "REMOVE_3D_MARKER"  on_recv_remove_3d_marker  ID: 45
		pckt_def enable_roads;                      // "ENABLEROADS"  on_recv_enable_roads  ID: 46
		pckt_def clear_area;                        // "DELETE_VEHICLES_IN_AREA"  on_recv_clear_area  ID: 47
		pckt_def set_player_blip_visible_state;     // "SETPLAYERBLIPVISIBLESTATE"  on_recv_set_player_blip_visible_state  ID: 48
		pckt_def player_respawn;                    // "PLAYER_RESPAWN"  on_recv_player_respawn  ID: 49
		pckt_def add_explosion;                     // "ADDEXPLOSION"  on_recv_add_explosion  ID: 50
		pckt_def print_now;                         // "PRINTNOW"  on_recv_print_now  ID: 51
		pckt_def set_waypoint;                      // "SETWAYPOINT"  on_recv_set_waypoint  ID: 52
		pckt_def clear_waypoint;                    // "CLEARWAYPOINT"  on_recv_clear_waypoint  ID: 53
		pckt_def hit_waypoint;                      // "HITWAYPOINT"  on_recv_hit_waypoint  ID: 54
		pckt_def spawn_car_debris;                  // "SPAWNCARDEBRIS"  on_recv_spawn_car_debris  ID: 55
		pckt_def set_vehicle_health;                // "SET_VEHICLE_HEATH"  on_recv_set_vehicle_health  ID: 56
		pckt_def set_vehicle_position;              // "MSG_SET_VEHICLE_POSITION"  on_recv_set_vehicle_position  ID: 57
		pckt_def vehicle_impact;                    // "VEHICLEIMPACT"  on_recv_vehicle_impact  ID: 58
		pckt_def msg_ready_for_cutscene;            // "MSG_READYFORCUTSCENE"  on_recv_msg_ready_for_cutscene  on_recv_sync_cutscene  ID: 59  (cutscene_sync)
#ifndef GTA_LIBERTY
		pckt_def ack_entity_create;                 // "ACK_ENTITY_CREATE"  on_recv_ack_entity_create  ID: 60
		pckt_def sync_peer_group;                   // "SYNC_PEER_GROUP"  on_recv_sync_peer_group  ID: 61
		pckt_def debug_break;                       // "DEBUGBREAK"  on_recv_debug_break  ID: 62
		pckt_def msg_blowup_vehicle;                // "MSG_BLOWUP_VEHICLE"  on_recv_msg_blowup_vehicle  ID: 63
		pckt_def msg_create_lua_object;             // "MSG_CREATELUAOBJECT"  on_recv_msg_create_lua_object  ID: 64
		pckt_def msg_server_ready_to_go;            // "MSG_SERVER_READY_TO_GO"  on_recv_msg_server_ready_to_go  ID: 65
#endif

		// in lvcs it seems to be defined in each file, judging by the cppinit table
		packet_id_list_t()
		{
#define REG_PCKT(pcktname, var, type, pspsize)				\
	var.pckt_name = pcktname;								\
	var.pckt_size = sizeof(type);							\
	MP_TEST_SIZE(pcktname, type, pspsize, snPacketCount);	\
	var.pckt_id = snPacketCount;							\
	assert(ARRAY_SIZE(aPacketsDefs) > snPacketCount);	\
	aPacketsDefs[snPacketCount] = &var;					\
	snPacketCount++;
	//var.pckt_def_id = gnMP_PacketCount; \
	//gnMP_PacketCount++;

			snPacketCount = 0;

			REG_PCKT("STARTFIRE",                     start_fire,                        net::pckt_start_fire,                        PS(24, 24));   //ID 0
			REG_PCKT("ACK",                           ack,                               net::pckt_ack,                               PS(5, 5));     //ID 1
			REG_PCKT("INFO",                          info,                              net::pckt_info,                              PS(204, 204)); //ID 2
			REG_PCKT("PLAYERKILL",                    player_kill,                       net::pckt_player_kill,                       PS(4, 4));     //ID 3
			REG_PCKT("KICKPLAYER",                    kick_player,                       net::pckt_kick_player,                       PS(4, 4));     //ID 4
//#ifdef GTA_LIBERTY
			REG_PCKT("REQUESTKICKPLAYER",             request_kick_player,               net::pckt_request_kick_player,               PS(4, 4));     //ID 5 // unused, guessed size
//#else
//			REG_PCKT("",                              request_kick_player,               net::pckt_request_kick_player,               PS(MPZ, 0));   //ID 5
//#endif
			REG_PCKT("SETTEAMSCORE",                  set_team_score,                    net::pckt_set_team_score,                    PS(8, 8));     //ID 6
			REG_PCKT("SENDMPGAMEEVENT",               send_game_event,                   net::pckt_send_game_event,                   PS(7, 7));     //ID 7  // vcs ps2 8
			REG_PCKT("FORCEPEDFROMVEHICLE",           force_ped_from_vehicle,            net::pckt_force_ped_from_vehicle,            PS(11, 11));   //ID 8  // vcs ps2 12
			REG_PCKT("SETVEHICLEEMERGENCYBRAKESTATE", set_vehicle_emergency_break_state, net::pckt_set_vehicle_emergency_break_state, PS(12, 12));   //ID 9  // vcs ps2 16
			REG_PCKT("SETCARLOCKEDSTATE",             set_carlocked_state,               net::pckt_set_carlocked_state,               PS(15, 15));   //ID 10 // vcs ps2 16
			REG_PCKT("REPAIRCAR",                     repair_car,                        net::pckt_repair_car,                        PS(11, 11));   //ID 11 // vcs ps2 12
			REG_PCKT("SETTYRESNOBURST",               set_tyres_no_burst,                net::pckt_set_tyres_no_burst,                PS(12, 12));   //ID 12 // vcs ps2 16
			REG_PCKT("DELETEVEHICLE",                 delete_vehicle,                    net::pckt_delete_vehicle,                    PS(11, 11));   //ID 13 // vcs ps2 12
			REG_PCKT("GAMESTATE",                     game_state,                        net::pckt_game_state,                        PS(6, 6));     //ID 14 // vcs ps2 7
			REG_PCKT("PLAYREMOTESOUND",               play_remote_sound,                 net::pckt_play_remote_sound,                 PS(22, 22));   //ID 15
			REG_PCKT("HEARTBEAT",                     heart_beat,                        net::pckt_heart_beat,                        PS(3, 3));     //ID 16
			REG_PCKT("TRANSFER",                      transfer_entity,                   net::pckt_transfer_entity,                   PS(7, 7));     //ID 17
//#ifdef GTA_LIBERTY
			REG_PCKT("CLOCK",                         clock,                             net::pckt_clock,                             PS(MPZ, 3));   //ID 18 // unused, guessed size
//#else
//			REG_PCKT("",                              clock,                             net::pckt_clock,                             PS(MPZ, 0));   //ID 18
//#endif
			REG_PCKT("GAMETIME",                      game_time,                         net::pckt_game_time,                         PS(17, 17));   //ID 19
			REG_PCKT("TARGETPLAYER",                  target_player,                     net::pckt_target_player,                     PS(4, 4));     //ID 20
			REG_PCKT("SETFIXEDCAMERA",                set_fixed_camera,                  net::pckt_set_fixed_camera,                  PS(27, 27));   //ID 21 // vcs ps2 28
			REG_PCKT("RESTORECAMERA",                 restore_camera,                    net::pckt_restore_camera,                    PS(4, 4));     //ID 22 // vcs ps2 6
			REG_PCKT("PICKUPCOLLECTED",               pickup_collected,                  net::pckt_pickup_collected,                  PS(6, 8));     //ID 23 // vcs ps2 10
			REG_PCKT("PICKUPREQUEST",                 pickup_request,                    net::pckt_pickup_request,                    PS(5, 5));     //ID 24
			REG_PCKT("POWERUPCOLLECTED",              powerup_collected,                 net::pckt_powerup_collected,                 PS(6, 6));     //ID 25 // vcs ps2 8
			REG_PCKT("SETVEHICLEINFINITEMASS",        set_vehicle_infinite_mass,         net::pckt_set_vehicle_infinite_mass,         PS(12, 12));   //ID 26
			REG_PCKT("FIGHTHITPED",                   fight_hit_ped,                     net::pckt_fight_hit_ped,                     PS(37, 54));   //ID 27 // vcs ps2 56
			REG_PCKT("SHOTPED",                       shot_ped,                          net::pckt_shot_ped,                          PS(32, 32));   //ID 28 // vcs ps2 34
			REG_PCKT("FIREINSTANTHIT",                fire_instant_hit,                  net::pckt_fire_instant_hit,                  PS(41, 41));   //ID 29
			REG_PCKT("FIRESNIPER",                    fire_sniper,                       net::pckt_fire_sniper,                       PS(30, 30));   //ID 30
			REG_PCKT("FIRESHOTGUN",                   fire_shotgun,                      net::pckt_fire_shotgun,                      PS(35, 35));   //ID 31
			REG_PCKT("FIREPROJECTILE",                fire_projectile,                   net::pckt_fire_projectile,                   PS(76, 76));   //ID 32
			REG_PCKT("USEDETONATOR",                  use_detonator,                     net::pckt_use_detonator,                     PS(5, 5));     //ID 33
			REG_PCKT("FIREAREAEFFECT",                fire_area_effect,                  net::pckt_fire_area_effect,                  PS(42, 42));   //ID 34
			REG_PCKT("SHOTPEDFROMCAR",                shot_ped_from_car,                 net::pckt_shot_ped_from_car,                 PS(24, 24));   //ID 35 // vcs ps2 26
			REG_PCKT("FIREINSTANTHITCAR",             fire_instant_hit_car,              net::pckt_fire_instant_hit_car,              PS(41, 41));   //ID 36
			REG_PCKT("KILLPLAYERPED",                 kill_player_ped,                   net::pckt_kill_player_ped,                   PS(7, 7));     //ID 37
			REG_PCKT("SHOTVEHICLE",                   shot_vehicle,                      net::pckt_shot_vehicle,                      PS(10, 10));   //ID 38 // vcs ps2 12
			REG_PCKT("MELEE",                         melee,                             net::pckt_melee,                             PS(38, 38));   //ID 39 // vcs ps2 40
			REG_PCKT("PLAYERBEENHIT",                 player_been_hit,                   net::pckt_player_been_hit,                   PS(5, 5));     //ID 40
			REG_PCKT("PLAYERCONTROL",                 player_control,                    net::pckt_player_control,                    PS(4, 4));     //ID 41 // vcs ps2 6
			REG_PCKT("SETPOSITION",                   set_position,                      net::pckt_set_position,                      PS(15, 15));   //ID 42 // vcs ps2 16
			REG_PCKT("SETHEADING",                    set_heading,                       net::pckt_set_heading,                       PS(7, 7));     //ID 43 // vcs ps2 8
			REG_PCKT("SET_3D_MARKER",                 add_3d_marker,                     net::pckt_add_3d_marker,                     PS(11, 11));   //ID 44 // vcs ps2 12
			REG_PCKT("REMOVE_3D_MARKER",              remove_3d_marker,                  net::pckt_remove_3d_marker,                  PS(11, 11));   //ID 45 // vcs ps2 12
			REG_PCKT("ENABLEROADS",                   enable_roads,                      net::pckt_enable_roads,                      PS(28, 28));   //ID 46 // vcs ps2 30
			REG_PCKT("DELETE_VEHICLES_IN_AREA",       clear_area,                        net::pckt_clear_area,                        PS(19, 19));   //ID 47 // vcs ps2 20
			REG_PCKT("SETPLAYERBLIPVISIBLESTATE",     set_player_blip_visible_state,     net::pckt_set_player_blip_visible_state,     PS(5, 5));     //ID 48 // vcs ps2 6
			REG_PCKT("PLAYER_RESPAWN",                player_respawn,                    net::pckt_player_respawn,                    PS(3, 3));     //ID 49 // vcs ps2 4
			REG_PCKT("ADDEXPLOSION",                  add_explosion,                     net::pckt_add_explosion,                     PS(25, 25));   //ID 50
			REG_PCKT("PRINTNOW",                      print_now,                         net::pckt_print_now,                         PS(268, 268)); //ID 51 // vcs ps2 +13
			REG_PCKT("SETWAYPOINT",                   set_waypoint,                      net::pckt_set_waypoint,                      PS(54, 58));   //ID 52
			REG_PCKT("CLEARWAYPOINT",                 clear_waypoint,                    net::pckt_clear_waypoint,                    PS(5, 5));     //ID 53 // vcs ps2 6
			REG_PCKT("HITWAYPOINT",                   hit_waypoint,                      net::pckt_hit_waypoint,                      PS(5, 5));     //ID 54
			REG_PCKT("SPAWNCARDEBRIS",                spawn_car_debris,                  net::pckt_spawn_car_debris,                  PS(51, 51));   //ID 55
			REG_PCKT("SET_VEHICLE_HEATH",             set_vehicle_health,                net::pckt_set_vehicle_health,                PS(15, 15));   //ID 56 // vcs ps2 16
			REG_PCKT("MSG_SET_VEHICLE_POSITION",      set_vehicle_position,              net::pckt_set_vehicle_position,              PS(23, 23));   //ID 57 // vcs ps2 24
			REG_PCKT("VEHICLEIMPACT",                 vehicle_impact,                    net::pckt_vehicle_impact,                    PS(36, 36));   //ID 58 // vcs ps2 38
			REG_PCKT("MSG_READYFORCUTSCENE",          msg_ready_for_cutscene,            net::pckt_msg_ready_for_cutscene,            PS(3, 3));     //ID 59
#ifndef GTA_LIBERTY
			REG_PCKT("ACK_ENTITY_CREATE",             ack_entity_create,                 net::pckt_ack_entity_create,                 PS(0, 5));     //ID 60 // vcs ps2 6
			REG_PCKT("SYNC_PEER_GROUP",               sync_peer_group,                   net::pckt_sync_peer_group,                   PS(0, 5));     //ID 61
			REG_PCKT("DEBUGBREAK",                    debug_break,                       net::pckt_debug_break,                       PS(0, 3));     //ID 62 // unused, guessed size
			REG_PCKT("MSG_BLOWUP_VEHICLE",            msg_blowup_vehicle,                net::pckt_msg_blowup_vehicle,                PS(0, 15));    //ID 63 // vcs ps2 16
			REG_PCKT("MSG_CREATELUAOBJECT",           msg_create_lua_object,             net::pckt_msg_create_lua_object,             PS(0, 21));    //ID 64
			REG_PCKT("MSG_SERVER_READY_TO_GO",        msg_server_ready_to_go,            net::pckt_msg_server_ready_to_go,            PS(0, 4));     //ID 65
#endif

	        // assertation
#ifdef GTA_LIBERTY
			int32 pcktNum = 60;
			//assert(gnMP_PacketCount == pcktNum);
			assert(snPacketCount == pcktNum);
			assert(msg_ready_for_cutscene.pckt_id == (pcktNum - 1));
#else // miami
			int32 pcktNum = 66;
			//assert(gnMP_PacketCount == pcktNum);
			assert(snPacketCount == pcktNum);
			assert(msg_server_ready_to_go.pckt_id == (pcktNum - 1));
#endif
#undef REG_PCKT
		}

		void TestSizes()
		{
#define TEST_PCKT(pcktname, var, type, pspsize)				\
	MP_TEST_SIZE(pcktname, type, pspsize, snPacketCount);

			TEST_PCKT("STARTFIRE",                     start_fire,                        net::pckt_start_fire,                        PS(24, 24));   //ID 0
			TEST_PCKT("ACK",                           ack,                               net::pckt_ack,                               PS(5, 5));     //ID 1
			TEST_PCKT("INFO",                          info,                              net::pckt_info,                              PS(204, 204)); //ID 2
			TEST_PCKT("PLAYERKILL",                    player_kill,                       net::pckt_player_kill,                       PS(4, 4));     //ID 3
			TEST_PCKT("KICKPLAYER",                    kick_player,                       net::pckt_kick_player,                       PS(4, 4));     //ID 4
//#ifdef GTA_LIBERTY
			TEST_PCKT("REQUESTKICKPLAYER",             request_kick_player,               net::pckt_request_kick_player,               PS(4, 4));     //ID 5 // unused, guessed size
//#else
//			TEST_PCKT("",                              request_kick_player,               net::pckt_request_kick_player,               PS(MPZ, 0));   //ID 5
//#endif
			TEST_PCKT("SETTEAMSCORE",                  set_team_score,                    net::pckt_set_team_score,                    PS(8, 8));     //ID 6
			TEST_PCKT("SENDMPGAMEEVENT",               send_game_event,                   net::pckt_send_game_event,                   PS(7, 7));     //ID 7  // vcs ps2 8
			TEST_PCKT("FORCEPEDFROMVEHICLE",           force_ped_from_vehicle,            net::pckt_force_ped_from_vehicle,            PS(11, 11));   //ID 8  // vcs ps2 12
			TEST_PCKT("SETVEHICLEEMERGENCYBRAKESTATE", set_vehicle_emergency_break_state, net::pckt_set_vehicle_emergency_break_state, PS(12, 12));   //ID 9  // vcs ps2 16
			TEST_PCKT("SETCARLOCKEDSTATE",             set_carlocked_state,               net::pckt_set_carlocked_state,               PS(15, 15));   //ID 10 // vcs ps2 16
			TEST_PCKT("REPAIRCAR",                     repair_car,                        net::pckt_repair_car,                        PS(11, 11));   //ID 11 // vcs ps2 12
			TEST_PCKT("SETTYRESNOBURST",               set_tyres_no_burst,                net::pckt_set_tyres_no_burst,                PS(12, 12));   //ID 12 // vcs ps2 16
			TEST_PCKT("DELETEVEHICLE",                 delete_vehicle,                    net::pckt_delete_vehicle,                    PS(11, 11));   //ID 13 // vcs ps2 12
			TEST_PCKT("GAMESTATE",                     game_state,                        net::pckt_game_state,                        PS(6, 6));     //ID 14 // vcs ps2 7
			TEST_PCKT("PLAYREMOTESOUND",               play_remote_sound,                 net::pckt_play_remote_sound,                 PS(22, 22));   //ID 15
			TEST_PCKT("HEARTBEAT",                     heart_beat,                        net::pckt_heart_beat,                        PS(3, 3));     //ID 16
			TEST_PCKT("TRANSFER",                      transfer_entity,                   net::pckt_transfer_entity,                   PS(7, 7));     //ID 17
//#ifdef GTA_LIBERTY
			TEST_PCKT("CLOCK",                         clock,                             net::pckt_clock,                             PS(MPZ, 3));   //ID 18 // unused, guessed size
//#else
//			TEST_PCKT("",                              clock,                             net::pckt_clock,                             PS(MPZ, 0));   //ID 18
//#endif
			TEST_PCKT("GAMETIME",                      game_time,                         net::pckt_game_time,                         PS(17, 17));   //ID 19
			TEST_PCKT("TARGETPLAYER",                  target_player,                     net::pckt_target_player,                     PS(4, 4));     //ID 20
			TEST_PCKT("SETFIXEDCAMERA",                set_fixed_camera,                  net::pckt_set_fixed_camera,                  PS(27, 27));   //ID 21 // vcs ps2 28
			TEST_PCKT("RESTORECAMERA",                 restore_camera,                    net::pckt_restore_camera,                    PS(4, 4));     //ID 22 // vcs ps2 6
			TEST_PCKT("PICKUPCOLLECTED",               pickup_collected,                  net::pckt_pickup_collected,                  PS(6, 8));     //ID 23 // vcs ps2 10
			TEST_PCKT("PICKUPREQUEST",                 pickup_request,                    net::pckt_pickup_request,                    PS(5, 5));     //ID 24
			TEST_PCKT("POWERUPCOLLECTED",              powerup_collected,                 net::pckt_powerup_collected,                 PS(6, 6));     //ID 25 // vcs ps2 8
			TEST_PCKT("SETVEHICLEINFINITEMASS",        set_vehicle_infinite_mass,         net::pckt_set_vehicle_infinite_mass,         PS(12, 12));   //ID 26
			TEST_PCKT("FIGHTHITPED",                   fight_hit_ped,                     net::pckt_fight_hit_ped,                     PS(37, 54));   //ID 27 // vcs ps2 56
			TEST_PCKT("SHOTPED",                       shot_ped,                          net::pckt_shot_ped,                          PS(32, 32));   //ID 28 // vcs ps2 34
			TEST_PCKT("FIREINSTANTHIT",                fire_instant_hit,                  net::pckt_fire_instant_hit,                  PS(41, 41));   //ID 29
			TEST_PCKT("FIRESNIPER",                    fire_sniper,                       net::pckt_fire_sniper,                       PS(30, 30));   //ID 30
			TEST_PCKT("FIRESHOTGUN",                   fire_shotgun,                      net::pckt_fire_shotgun,                      PS(35, 35));   //ID 31
			TEST_PCKT("FIREPROJECTILE",                fire_projectile,                   net::pckt_fire_projectile,                   PS(76, 76));   //ID 32
			TEST_PCKT("USEDETONATOR",                  use_detonator,                     net::pckt_use_detonator,                     PS(5, 5));     //ID 33
			TEST_PCKT("FIREAREAEFFECT",                fire_area_effect,                  net::pckt_fire_area_effect,                  PS(42, 42));   //ID 34
			TEST_PCKT("SHOTPEDFROMCAR",                shot_ped_from_car,                 net::pckt_shot_ped_from_car,                 PS(24, 24));   //ID 35 // vcs ps2 26
			TEST_PCKT("FIREINSTANTHITCAR",             fire_instant_hit_car,              net::pckt_fire_instant_hit_car,              PS(41, 41));   //ID 36
			TEST_PCKT("KILLPLAYERPED",                 kill_player_ped,                   net::pckt_kill_player_ped,                   PS(7, 7));     //ID 37
			TEST_PCKT("SHOTVEHICLE",                   shot_vehicle,                      net::pckt_shot_vehicle,                      PS(10, 10));   //ID 38 // vcs ps2 12
			TEST_PCKT("MELEE",                         melee,                             net::pckt_melee,                             PS(38, 38));   //ID 39 // vcs ps2 40
			TEST_PCKT("PLAYERBEENHIT",                 player_been_hit,                   net::pckt_player_been_hit,                   PS(5, 5));     //ID 40
			TEST_PCKT("PLAYERCONTROL",                 player_control,                    net::pckt_player_control,                    PS(4, 4));     //ID 41 // vcs ps2 6
			TEST_PCKT("SETPOSITION",                   set_position,                      net::pckt_set_position,                      PS(15, 15));   //ID 42 // vcs ps2 16
			TEST_PCKT("SETHEADING",                    set_heading,                       net::pckt_set_heading,                       PS(7, 7));     //ID 43 // vcs ps2 8
			TEST_PCKT("SET_3D_MARKER",                 add_3d_marker,                     net::pckt_add_3d_marker,                     PS(11, 11));   //ID 44 // vcs ps2 12
			TEST_PCKT("REMOVE_3D_MARKER",              remove_3d_marker,                  net::pckt_remove_3d_marker,                  PS(11, 11));   //ID 45 // vcs ps2 12
			TEST_PCKT("ENABLEROADS",                   enable_roads,                      net::pckt_enable_roads,                      PS(28, 28));   //ID 46 // vcs ps2 30
			TEST_PCKT("DELETE_VEHICLES_IN_AREA",       clear_area,                        net::pckt_clear_area,                        PS(19, 19));   //ID 47 // vcs ps2 20
			TEST_PCKT("SETPLAYERBLIPVISIBLESTATE",     set_player_blip_visible_state,     net::pckt_set_player_blip_visible_state,     PS(5, 5));     //ID 48 // vcs ps2 6
			TEST_PCKT("PLAYER_RESPAWN",                player_respawn,                    net::pckt_player_respawn,                    PS(3, 3));     //ID 49 // vcs ps2 4
			TEST_PCKT("ADDEXPLOSION",                  add_explosion,                     net::pckt_add_explosion,                     PS(25, 25));   //ID 50
			TEST_PCKT("PRINTNOW",                      print_now,                         net::pckt_print_now,                         PS(268, 268)); //ID 51 // vcs ps2 +13
			TEST_PCKT("SETWAYPOINT",                   set_waypoint,                      net::pckt_set_waypoint,                      PS(54, 58));   //ID 52
			TEST_PCKT("CLEARWAYPOINT",                 clear_waypoint,                    net::pckt_clear_waypoint,                    PS(5, 5));     //ID 53 // vcs ps2 6
			TEST_PCKT("HITWAYPOINT",                   hit_waypoint,                      net::pckt_hit_waypoint,                      PS(5, 5));     //ID 54
			TEST_PCKT("SPAWNCARDEBRIS",                spawn_car_debris,                  net::pckt_spawn_car_debris,                  PS(51, 51));   //ID 55
			TEST_PCKT("SET_VEHICLE_HEATH",             set_vehicle_health,                net::pckt_set_vehicle_health,                PS(15, 15));   //ID 56 // vcs ps2 16
			TEST_PCKT("MSG_SET_VEHICLE_POSITION",      set_vehicle_position,              net::pckt_set_vehicle_position,              PS(23, 23));   //ID 57 // vcs ps2 24
			TEST_PCKT("VEHICLEIMPACT",                 vehicle_impact,                    net::pckt_vehicle_impact,                    PS(36, 36));   //ID 58 // vcs ps2 38
			TEST_PCKT("MSG_READYFORCUTSCENE",          msg_ready_for_cutscene,            net::pckt_msg_ready_for_cutscene,            PS(3, 3));     //ID 59
#ifndef GTA_LIBERTY
			TEST_PCKT("ACK_ENTITY_CREATE",             ack_entity_create,                 net::pckt_ack_entity_create,                 PS(0, 5));     //ID 60 // vcs ps2 6
			TEST_PCKT("SYNC_PEER_GROUP",               sync_peer_group,                   net::pckt_sync_peer_group,                   PS(0, 5));     //ID 61
			TEST_PCKT("DEBUGBREAK",                    debug_break,                       net::pckt_debug_break,                       PS(0, 3));     //ID 62 // unused, guessed size
			TEST_PCKT("MSG_BLOWUP_VEHICLE",            msg_blowup_vehicle,                net::pckt_msg_blowup_vehicle,                PS(0, 15));    //ID 63 // vcs ps2 16
			TEST_PCKT("MSG_CREATELUAOBJECT",           msg_create_lua_object,             net::pckt_msg_create_lua_object,             PS(0, 21));    //ID 64
			TEST_PCKT("MSG_SERVER_READY_TO_GO",        msg_server_ready_to_go,            net::pckt_msg_server_ready_to_go,            PS(0, 4));     //ID 65
#endif
#undef TEST_PCKT
		}

		inline const char* GetPacketName(int32 pckt_id) {
			return (pckt_id >= 0 && pckt_id < packet_id_list_t::snPacketCount) ? aPacketsDefs[pckt_id]->pckt_name : nil;
		}
	};
}

extern net::packet_id_list_t gtMP_PacketIDs;

#endif