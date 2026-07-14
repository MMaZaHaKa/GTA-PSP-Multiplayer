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
//#define MP_USE_CUSTOM_ALLOCATOR

#define DEBUG_MULTIGAME // some tmp debug stuff + log

#define MP_FE_MOUSE_IMPROVEMENTS
//#define MULTIGAME_IMPROVEMENTS // more sync stuff, improvements, extras, etc, custom non re code
#define DEBUG_MULTIGAME_IMPROVEMENTS // additional logic for assertation + etc
#define MULTIGAME_ELEMENTS_IMPROVEMENTS // more code for entities (bike lights render, etc)
#ifdef MULTIGAME_IMPROVEMENTS
    #define MULTIGAME_SCM // non lua mode, scm
#endif

#if defined(MULTIGAME_ELEMENTS_IMPROVEMENTS) && defined(ADHOCCTL_USE_CUSTOM_IDENT) && !defined(GTA_PSP)
    //#define MULTIGAME_ELEMENTS_COMPAT_IMPROVEMENTS // todo mean how fast send sync directly for each peers (for controll delta + skip full custom(headers))
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

#define FLT_EPS (0.000001f)
#define FLT_EPS_EQ(a, b) (fabsf((a) - (b)) <= FLT_EPS)
#define FLT_EPS_NOT_EQ(a, b) (FLT_EPS_EQ(a, b) == false)

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
#define DONT_OPTIMIZE()                                                                                                         \
    do                                                                                                                          \
    {                                                                                                                           \
        static volatile uint32 _dont_optimize_seed = 777 + static_cast<uint32>(__TIME__[1]) + static_cast<uint32>(__TIME__[2]); \
        volatile uint32 a = __LINE__;                                                                                           \
        volatile uint32 b = a * 1664525u + 1013904223u;                                                                         \
        volatile uint32 c = (b >> 16) ^ (a << 8);                                                                               \
        volatile uint32 d = (c * 2654435761u) ^ 0xDEADBEEF;                                                                     \
        volatile uint32 e = d ^ (d >> 13);                                                                                      \
        volatile uint32 f = (e << 17) ^ (e >> 7);                                                                               \
        volatile uint32 g = (f * 1103515245u + 12345u);                                                                         \
        volatile uint32 h = (g >> 3) ^ (g << 11);                                                                               \
        volatile uint32 i = (h ^ 0xCAFEBABE) + (h >> 5);                                                                        \
        volatile uint32 j = (i * 31u) ^ (i >> 9);                                                                               \
        volatile uint32 k = (j ^ (j << 21)) + (j >> 19);                                                                        \
        volatile uint32 l = k ^ 0x13579BDFu;                                                                                    \
        volatile uint32 m = (l << 13) ^ (l >> 7);                                                                               \
        volatile uint32 n = (m * 97u) ^ (m >> 17);                                                                              \
        volatile uint32 o = (n ^ (n << 5)) + (n >> 23);                                                                         \
        volatile uint32 p = (o * 1664525u + 1013904223u);                                                                       \
        volatile uint32 q = (p ^ (p >> 27)) + (p << 7);                                                                         \
        volatile uint32 r = q ^ 0xAAAAAAAAu;                                                                                    \
        volatile uint32 s = (r << 9) ^ (r >> 11);                                                                               \
        volatile uint32 t = (s * 33u) ^ (s >> 15);                                                                              \
        _dont_optimize_seed = (_dont_optimize_seed ^ t ^ static_cast<uint32>(__LINE__));                                        \
        if (t == 0x12345678u)                                                                                                   \
        {                                                                                                                       \
            volatile int dummy = 0;                                                                                             \
            ++dummy;                                                                                                            \
        }                                                                                                                       \
    } while (false)
#else
#define DONT_OPTIMIZE() ;
#endif

void mp_game_update_recv();
void mp_game_update_send();
void mp_game_draw_debug_net();
void mp_game_draw_debug_zones();

//// TODO: verify correct location for those variables (MultiGame.h)
//extern bool gbMP_DrawPauseScreen;
//extern bool gbMP_DrawPauseScreenNoBox;

//void DrawHudForMultiplayer(); // moved into CHud, psp lcs inlined in draw
//struct sPlayer;
//int16 GetPeerFromPlayerElement(sPlayer* player);
bool ProcessMultiGame();
