/*
    Authors: MaZaHaKa, ASM95
    Do not delete this comment block. Respect others' work!
*/
#pragma once
#include "common.h"
#include "MultiGame.h"

#if /*defined(GTA_NETWORK) &&*/ defined(GTA_PC)
#include <string>
#include <vector>
#include <mutex>

class tChatMessage
{
public:
	std::string m_sNickName;
	std::string m_sChatMessage;
	float m_fTimestamp;
	int32 m_nAlpha;
};

typedef void (*FnChangeCallback_st)(std::string arg);
class ConCommandBase
{
public:

	bool m_bRegistered;
	std::string m_sName;
	std::string m_sHelpString;
	int32 m_nFlags;

	FnChangeCallback_st m_OnCall;

	inline ConCommandBase()	: m_bRegistered(false),	m_sName(""), m_sHelpString(""),	m_nFlags(0) { }
	inline ConCommandBase(const std::string& name, const std::string& helpString, FnChangeCallback_st onCall, int32 flags = 0) :
		m_bRegistered(false), m_sName(name),m_sHelpString(helpString), m_OnCall(onCall), m_nFlags(flags) { }
	inline ConCommandBase(const ConCommandBase& other) : m_bRegistered(other.m_bRegistered),
		m_sName(other.m_sName), m_sHelpString(other.m_sHelpString), m_nFlags(other.m_nFlags), m_OnCall(other.m_OnCall) { }

	inline bool IsRegistered() { return m_bRegistered; }
	inline std::string GetName() { return m_sName; }
};

//class CCommand
//{
//public:
//
//};
//
//class ConVar : public ConCommandBase
//{
//public:
//	// custom
//	std::string m_sStr;
//	int32 m_iNum;
//	float m_fFloat;
//	FnChangeCallback_st m_OnChange;
//};

class cGameChat
{
public:
	std::mutex mutex;

	bool m_bIsInitialised;
	bool m_bIsOpen;
	std::string m_sMyTextBuff; // when i am typing
	std::vector<tChatMessage> m_vChatHistory; // common
	std::vector<ConCommandBase> m_vCommands; // /game/server/game.cpp

	inline cGameChat() { m_bIsOpen = false; m_bIsInitialised = false; }
	void Initialise();

	void Open();
	void Close();

	void OnRender();
	void OnUpdate();
	void OnPressedChar(char ch);

	static void OnMessageRecv(std::string nickname, std::string message);
	void MessageSend(std::string message);
	void RegisterConCommandBase(ConCommandBase& command);

	inline bool IsOpen() { return m_bIsOpen; }
	inline bool CanUse() { return g_Config.bEnableWlan && g_Config.bEnableNetworkChat; }
	inline int32 ChatMode() { return gIsMultiplayerGame ? 2 : 1; }
};

extern cGameChat gChat;

#endif