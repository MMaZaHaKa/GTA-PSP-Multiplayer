/*
    Authors: MaZaHaKa, ASM95
    Do not delete this comment block. Respect others' work!
*/
#pragma once
#include "common.h"
#ifndef GTA_PSP
#include "multiplayer/net/emu/NetAdhocCommon.h"
#endif

// TODO: move in config.h
#ifdef GTA_LIBERTY

#if 1
    #define GTA_TITLE_ID "GTALCS00" // generic, MG Game room
#ifdef GTA_LIBERTY_EUROPEAN
    #define GTA_PRODUCT_ID "ULES00151" // MG Game ID
#else // AMERICAN
    #define GTA_PRODUCT_ID "ULUS10041" // MG Game ID
#endif
#else
// https://www.socom.cc/status.xml понты в aemu proAdhoc
#define GTA_TITLE_ID "GTARELCS"
#define GTA_PRODUCT_ID "ULUSRELCS"
#endif

#else // MIAMI

#if 1
    #define GTA_TITLE_ID "GTAVCS00" // generic, MG Game room
#ifdef GTA_MIAMI_EUROPEAN
    #define GTA_PRODUCT_ID "ULES00502" // MG Game ID
#else // AMERICAN
    #define GTA_PRODUCT_ID "ULUS10160" // MG Game ID
#endif
#else
// https://www.socom.cc/status.xml понты в aemu proAdhoc
#define GTA_TITLE_ID "GTAREVCS"
#define GTA_PRODUCT_ID "ULUSREVCS"
#endif

#endif

#ifdef ADHOC_PTP_PDP_CHAT_EMU
    #define GTA_RE_TITLE_ID "GTAREVCS"
    #define GTA_RE_PRODUCT_ID "ULUSREVCS"
#endif

#define EVENT_STACK_SZ           (50)
#define MULTI_TIME_OUT           (420)
#define MULTI_TIME_OUT_2         (300)

#ifdef GTA_LIBERTY
#define MULTI_TIME_OUT_3         (10000)
#else
#define MULTI_TIME_OUT_3         (25000)
#endif

#if defined(_DEBUG) && !defined(FINAL) && !defined(MASTER)
#define MULTI_TIME_OUT_4         (60.0f*60.0f) // sec
#else
#define MULTI_TIME_OUT_4         (15.0f) // sec
#endif

#ifdef GTA_PC
#define MULTI_CONNECT_WAIT_TIME  (200.0f) // TODO: figure out compat psp value 10.0f conn (pdp recv) // hle eat stuff? PC load faster and wait a lot psp
#else
#define MULTI_CONNECT_WAIT_TIME  (10.0f)
#endif
#define MULTI_WAIT_HEART_BEAT    (30)
#define MULTI_WAIT_HEART_BEAT_2  (60)

#if !defined(FINAL) && !defined(MASTER)
#define MULTI_ABORT_PLAYER_COUNT (1) // gDeveloperFlag allow start with 1 player (non in vanilla)
#define MULTI_RECEIVE_PLAYER_COUNT (0) // host+slave, count 1 >= 1 ok upd timer send and simsch
#else
#define MULTI_ABORT_PLAYER_COUNT (2)
#define MULTI_RECEIVE_PLAYER_COUNT (1)
#endif

#define MP_LOAD_FLAGS (STREAMFLAGS_DEPENDENCY | STREAMFLAGS_DONT_REMOVE) // 5
#define USE_COMPILED_LUA
#define MP_FE_MOUSE_IMPROVEMENTS
//#define MP_USE_CUSTOM_ALLOCATOR

#define DEBUG_MULTIGAME // some tmp debug stuff + log
//#define MULTIGAME_IMPROVEMENTS // more sync stuff, improvements, extras, etc, custom non re code
#ifdef MULTIGAME_IMPROVEMENTS
    #define MULTIGAME_SCM // non lua mode, scm
#endif

#ifdef MULTIGAME_SCM
#define IS_MULTIGAME_RUNNING (gIsMultiplayerGame && TheMPGame.GetGameType() != eGameType::SCM)
#else
#define IS_MULTIGAME_RUNNING (gIsMultiplayerGame)
#endif

//#ifdef GTA_NETWORK
//    MULTIGAME_DISABLE();
//#endif

#define MULTIGAME_DISABLE(...) \
    if (IS_MULTIGAME_RUNNING) \
        return  __VA_ARGS__;

#ifdef MP_FE_MOUSE_IMPROVEMENTS
#define MP_CHECK_HOVER(xs, ys, xe, ye) (FrontEndMenuManager->CheckHover(SCREEN_SCALE_X(xs), SCREEN_SCALE_X(xe), SCREEN_SCALE_Y(ys), SCREEN_SCALE_Y(ye)))
// not vanila control (allow click on only hovered row)
#if 1
    #define MP_IS_CLICK_LAST_Y() ((m_nLastMouseSelectedMenuOptionY > 0) && CPad::GetPad(0)->GetLeftMouseJustDown() && \
        MP_CHECK_HOVER(0, m_nLastMouseSelectedMenuOptionY, DEFAULT_SCREEN_WIDTH, m_nLastMouseSelectedMenuOptionY + 13))
#else
    #define MP_IS_CLICK_LAST_Y() (false)
#endif
#define DECLARE_MOUSE_RECT(row, xs, ys, xe, ye) \
    if (MP_CHECK_HOVER(xs, ys, xe, ye) && FrontEndMenuManager->m_bShowMouse) { \
        if(m_nSelectedMenuOptionIndex != row) { \
            DMAudio.PlayFrontEndSound(SOUND_FRONTEND_HIGHLIGHT_OPTION, 0); \
            m_nSelectedMenuOptionIndex = row; \
        } \
        m_nLastMouseSelectedMenuOptionY = ys; \
    }
#endif

#if 0 // need check ?
#define MULTIGAME_UNIMPLEMENTED() assert(0 && "todo me [MG]")
#else
#define MULTIGAME_UNIMPLEMENTED() assert(1 && "todo me [MG]")
#endif

// for adhoc bug fix when in release cAdhoc::StateIdle, cAdhoc::StateShutdown mixed into 1 empty function for all empty functions
#if 0
#define _KCF_CAT(a,b) a##b
#define _KCF_XCAT(a,b) _KCF_CAT(a,b)

#if defined(_MSC_VER) // MSVC (Windows)
#define KEEP_FUNC_ATTR __declspec(noinline)
#define KEEP_FUNC_BODY() \
    do { volatile int _KCF_XCAT(_kcf_mark_, __LINE__) = __LINE__; (void)_KCF_XCAT(_kcf_mark_, __LINE__); } while(0)
#elif defined(__GNUC__) || defined(__clang__) // GCC
#define KEEP_FUNC_ATTR __attribute__((noinline, used))
#define KEEP_FUNC_BODY() asm volatile("" ::: "memory")
#else
#define KEEP_FUNC_ATTR
#define KEEP_FUNC_BODY() ((void)0)
#endif
#endif

#if !defined(FINAL) && !defined(MASTER)
#define DONT_OPTIMIZE()    // TMP HIDDEN                                                                                                      \
#else
#define DONT_OPTIMIZE() ;
#endif

void mp_game_update_recv();
void mp_game_update_send();
void mp_game_draw_debug_net();

//// TODO: verify correct location for those variables (MultiGame.h)
//extern bool gbMP_DrawPauseScreen;
//extern bool gbMP_DrawPauseScreenNoBox;

//void DrawHudForMultiplayer(); // moved into CHud, psp lcs inlined in draw
//struct sPlayer;
//int16 GetPeerFromPlayerElement(sPlayer* player);
bool ProcessMultiGame();
