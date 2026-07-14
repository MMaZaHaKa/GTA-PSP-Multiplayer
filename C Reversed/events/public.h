/*
    Authors: MaZaHaKa, ASM95
    Do not delete this comment block. Respect others' work!
*/
#pragma once
#include "multiplayer/net/packet.h"
#include "common.h"

//#define MULTIGAME_UNIMPLEMENTED_EVENT() assert(false && "UNIMPLEMENTED MULTIPLAYER PACKET")
#define MULTIGAME_UNIMPLEMENTED_EVENT()

void dump_packet_data(net::pckt_base& packet, const char* str, int32 colour); // debug
#define DUMP_PACKET(packet, str, colour) dump_packet_data(packet, str, colour)

#if !defined(FINAL) && !defined(MASTER)
void DebugMultigameTriggerError(); // spread error, stop all games, i guess
#endif

void MultigameKickPlayer(uint8 nID);
void MultigameRequestKickPlayer(uint8 nID);

// FireMessageHandler - NO ID, entry point to dispatcher  moved into cMultiGame

void on_recv_start_fire(net::pckt_start_fire& packet, int sender, uint16 time, bool bFromRing); // 0
//void on_recv_ack(net::pckt_ack& packet, int sender, uint16 time, bool bFromRing); // 1
//void on_recv_info(net::pckt_info& packet, int sender, uint16 time, bool bFromRing); // 2  cNetSession::ClientConnect()
//void on_recv_player_kill(net::pckt_player_kill& packet, int sender, uint16 time, bool bFromRing); // 3  moved into cMultiGame
void on_recv_kick_player(net::pckt_kick_player& packet, int sender, uint16 time, bool bFromRing); // 4
void on_recv_request_kick_player(net::pckt_request_kick_player& packet, int sender, uint16 time, bool bFromRing); // 5 non vcs
void on_recv_set_team_score(net::pckt_set_team_score& packet, int sender, uint16 time, bool bFromRing); // 6
void on_recv_send_game_event(net::pckt_send_game_event& packet, int sender, uint16 time, bool bFromRing); // 7
void on_recv_force_ped_from_vehicle(net::pckt_force_ped_from_vehicle& packet, int sender, uint16 time, bool bFromRing); // 8
void on_recv_set_vehicle_emergency_break_state(net::pckt_set_vehicle_emergency_break_state& packet, int sender, uint16 time, bool bFromRing); // 9
void on_recv_set_carlocked_state(net::pckt_set_carlocked_state& packet, int sender, uint16 time, bool bFromRing); // 10
void on_recv_repair_car(net::pckt_repair_car& packet, int sender, uint16 time, bool bFromRing); // 11
void on_recv_set_tyres_no_burst(net::pckt_set_tyres_no_burst& packet, int sender, uint16 time, bool bFromRing); // 12
void on_recv_delete_vehicle(net::pckt_delete_vehicle& packet, int sender, uint16 time, bool bFromRing); // 13
//void on_recv_game_state(net::pckt_game_state& packet, int sender, uint16 time, bool bFromRing); // 14  moved into cMultiGame
void on_recv_play_remote_sound(net::pckt_play_remote_sound& packet, int sender, uint16 time, bool bFromRing); // 15
//void on_recv_heart_beat(net::pckt_heart_beat& packet, int sender, uint16 time, bool bFromRing); // 16
//void on_recv_transfer_entity(net::pckt_transfer_entity& packet, int sender, uint16 time, bool bFromRing); // 17  moved into cMultiGame
//void on_recv_clock(net::pckt_clock& packet, int sender, uint16 time, bool bFromRing); // 18
void on_recv_game_time(net::pckt_game_time& packet, int sender, uint16 time, bool bFromRing); // 19
void on_recv_target_player(net::pckt_target_player& packet, int sender, uint16 time, bool bFromRing); // 20
void on_recv_set_fixed_camera(net::pckt_set_fixed_camera& packet, int sender, uint16 time, bool bFromRing); // 21
void on_recv_restore_camera(net::pckt_restore_camera& packet, int sender, uint16 time, bool bFromRing); // 22
void on_recv_pickup_collected(net::pckt_pickup_collected& packet, int sender, uint16 time, bool bFromRing); // 23
void on_recv_pickup_request(net::pckt_pickup_request& packet, int sender, uint16 time, bool bFromRing); // 24
void on_recv_powerup_collected(net::pckt_powerup_collected& packet, int sender, uint16 time, bool bFromRing); // 25
void on_recv_set_vehicle_infinite_mass(net::pckt_set_vehicle_infinite_mass& packet, int sender, uint16 time, bool bFromRing); // 26
void on_recv_fight_hit_ped(net::pckt_fight_hit_ped& packet, int sender, uint16 time, bool bFromRing); // 27
void on_recv_shot_ped(net::pckt_shot_ped& packet, int sender, uint16 time, bool bFromRing); // 28
void on_recv_fire_instant_hit(net::pckt_fire_instant_hit& packet, int sender, uint16 time, bool bFromRing); // 29
void on_recv_fire_sniper(net::pckt_fire_sniper& packet, int sender, uint16 time, bool bFromRing); // 30
void on_recv_fire_shotgun(net::pckt_fire_shotgun& packet, int sender, uint16 time, bool bFromRing); // 31
void on_recv_fire_projectile(net::pckt_fire_projectile& packet, int sender, uint16 time, bool bFromRing); // 32
void on_recv_use_detonator(net::pckt_use_detonator& packet, int sender, uint16 time, bool bFromRing); // 33
void on_recv_fire_area_effect(net::pckt_fire_area_effect& packet, int sender, uint16 time, bool bFromRing); // 34
void on_recv_shot_ped_from_car(net::pckt_shot_ped_from_car& packet, int sender, uint16 time, bool bFromRing); // 35
void on_recv_fire_instant_hit_car(net::pckt_fire_instant_hit_car& packet, int sender, uint16 time, bool bFromRing); // 36
void on_recv_kill_player_ped(net::pckt_kill_player_ped& packet, int sender, uint16 time, bool bFromRing); // 37
void on_recv_shot_vehicle(net::pckt_shot_vehicle& packet, int sender, uint16 time, bool bFromRing); // 38
void on_recv_melee(net::pckt_melee& packet, int sender, uint16 time, bool bFromRing); // 39
void on_recv_player_been_hit(net::pckt_player_been_hit& packet, int sender, uint16 time, bool bFromRing); // 40
void on_recv_player_control(net::pckt_player_control& packet, int sender, uint16 time, bool bFromRing); // 41
void on_recv_player_set_position(net::pckt_set_position& packet, int sender, uint16 time, bool bFromRing); // 42  on_recv_set_position
void on_recv_player_set_heading(net::pckt_set_heading& packet, int sender, uint16 time, bool bFromRing); // 43  on_recv_set_heading
void on_recv_add_3d_marker(net::pckt_add_3d_marker& packet, int sender, uint16 time, bool bFromRing); // 44
void on_recv_remove_3d_marker(net::pckt_remove_3d_marker& packet, int sender, uint16 time, bool bFromRing); // 45
void on_recv_enable_roads(net::pckt_enable_roads& packet, int sender, uint16 time, bool bFromRing); // 46
void on_recv_clear_area(net::pckt_clear_area& packet, int sender, uint16 time, bool bFromRing); // 47
void on_recv_set_player_blip_visible_state(net::pckt_set_player_blip_visible_state& packet, int sender, uint16 time, bool bFromRing); // 48
void on_recv_player_respawn(net::pckt_player_respawn& packet, int sender, uint16 time, bool bFromRing); // 49
void on_recv_add_explosion(net::pckt_add_explosion& packet, int sender, uint16 time, bool bFromRing); // 50
void on_recv_print_now(net::pckt_print_now& packet, int sender, uint16 time, bool bFromRing); // 51
//void on_recv_set_waypoint(void* pArg, net::pckt_set_waypoint& packet, int sender, uint16 time, bool bFromRing); // 52  moved into sWaypoint
//void on_recv_clear_waypoint(void* pArg, net::pckt_clear_waypoint& packet, int sender, uint16 time, bool bFromRing); // 53  moved into sWaypoint
//void on_recv_hit_waypoint(void* pArg, net::pckt_hit_waypoint& packet, int sender, uint16 time, bool bFromRing); // 54  moved into sWaypoint
void on_recv_spawn_car_debris(net::pckt_spawn_car_debris& packet, int sender, uint16 time, bool bFromRing); // 55
void on_recv_set_vehicle_health(net::pckt_set_vehicle_health& packet, int sender, uint16 time, bool bFromRing); // 56
void on_recv_set_vehicle_position(net::pckt_set_vehicle_position& packet, int sender, uint16 time, bool bFromRing); // 57
void on_recv_vehicle_impact(net::pckt_vehicle_impact& packet, int sender, uint16 time, bool bFromRing); // 58
void on_recv_msg_ready_for_cutscene(net::pckt_msg_ready_for_cutscene& packet, int sender, uint16 time, bool bFromRing); // 59  on_recv_sync_cutscene
#ifndef GTA_LIBERTY
void on_recv_ack_entity_create(net::pckt_ack_entity_create& packet, int sender, uint16 time, bool bFromRing); // 60
void on_recv_sync_peer_group(net::pckt_sync_peer_group& packet, int sender, uint16 time, bool bFromRing); // 61
void on_recv_debug_break(net::pckt_debug_break& packet, int sender, uint16 time, bool bFromRing); // 62
void on_recv_msg_blowup_vehicle(net::pckt_msg_blowup_vehicle& packet, int sender, uint16 time, bool bFromRing); // 63
void on_recv_msg_create_lua_object(net::pckt_msg_create_lua_object& packet, int sender, uint16 time, bool bFromRing); // 64
void on_recv_msg_server_ready_to_go(net::pckt_msg_server_ready_to_go& packet, int sender, uint16 time, bool bFromRing); // 65
#endif
