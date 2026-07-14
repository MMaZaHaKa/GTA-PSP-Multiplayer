/*
    Authors: MaZaHaKa, ASM95
    Do not delete this comment block. Respect others' work!
*/
#include "common.h"
#include "Font.h"
#include "Timer.h"
#include "Text.h"
#include "Radar.h"
#include "Hud.h"
#include "Frontend.h"
#include "Pad.h"
#include "PlayerPed.h"
#include "PlayerInfo.h"
#include "ModelInfo.h"
#include "VehicleModelInfo.h"
#include "Sprite.h"
#include "DMAudio.h"
#include "Script.h"

#include "leeds/base/stringt.h"
#include "leeds/psp_compat.h"

#include "multiplayer/public.h"
#include "multiplayer/net/public.h"
#include "multiplayer/MultiGame.h"
#include "multiplayer/net/Adhoc.h"
#include "multiplayer/net/public.h"
#include "multiplayer/elements/sTextSprite.h"
#include "multiplayer/Lobby.h"
#include "multiplayer/LobbyPed.h"
#if defined(ADHOCCTL_USE_CUSTOM_IDENT) && !defined(GTA_PSP)
#include "multiplayer/net/emu/proAdhoc.h"
#endif


#define UNPACK_COLOR(c) c->alpha, c->red, c->green, c->blue


#define OPTION_BOTTOM_BLOCK_START_X (13) // https://prnt.sc/mydJJAZCwinl
#define DEFAULT_SPACE_Y (13)
#ifdef GTA_LIBERTY
#define MP_FONT_SCALE() CFont::SetScale(SCALE_X(0.4048f), SCALE_Y(0.88f))

#define OPTION_LABEL_START_X (140)
#define OPTION_VALUE_START_X (145)
#define PLAYER_LIST_START_X (380)
#define MP_POS_Y_DEFAULT (58)
#define PLAYER_LIST_SPACE_X (13)
#else
#define MP_FONT_SCALE() CFont::SetScale(SCALE_X(MEDIUMTEXT_X_SCALE), SCALE_Y(MEDIUMTEXT_Y_SCALE))
//#define MP_FONT_SCALE() CFont::SetScale(SCALE_X(0.6f), SCALE_Y(0.55f)) // psp like (PSP 0.6f, 0.6f)

#define OPTION_LABEL_START_X (10)
#define OPTION_VALUE_START_X (185)
#define PLAYER_LIST_START_X (370) // https://prnt.sc/DUCfc2jEhDPC
#define MP_POS_Y_DEFAULT (74)     // element label, element data, player list
#define PLAYER_LIST_SPACE_X (5)
#endif

#define COLOR_BLUE 118, 176, 220
#define COLOR_WHITE 255, 255, 255 // (LCS HOST A GAME)
#define COLOR_YELLOW 255, 255, 0  // yellow (looking for hosts)
#define COLOR_GREEN 75, 151, 75
#define COLOR_ORANGE 217, 174, 87   // dark orange
#define COLOR_MUSTARD 255, 236, 147 // light orange (selected row)
#define COLOR_MUSTARD_INACTIVE 105, 86, 7
#define COLOR_CYAN 30, 255, 255 // light blue (menu tabs)
#define COLOR_CYAN_INACTIVE 0, 105, 105
#define COLOR_HEADER HEADER_COLOR.r, HEADER_COLOR.g, HEADER_COLOR.b // purple (menu page name) HEADER_COLOR

#ifdef GTA_LIBERTY
#define MP_TEXT_COLOUR COLOR_BLUE
#define MP_TEXT_COLOUR_SELECTED COLOR_WHITE
#define MP_HEADER_TEXT_COLOUR COLOR_ORANGE
#define SET_MP_TEXT_COLOUR_STYLE(reqRow, mode) SetTextColorStyle(m_nSelectedMenuOptionIndex == reqRow)
#else
#define MP_TEXT_COLOUR COLOR_CYAN
#define MP_TEXT_COLOUR_SELECTED COLOR_MUSTARD
#define MP_HEADER_TEXT_COLOUR COLOR_HEADER
#define SET_MP_TEXT_COLOUR_STYLE(reqRow, mode) SetTextColorStyle(m_nSelectedMenuOptionIndex == reqRow, mode)
#endif

#define SCALE_X(x) SCREEN_SCALE_X(x)
#define SCALE_Y(y) SCREEN_SCALE_Y(y)
#define MP_MENU_X(x) SCALE_X(x) // SCALE_AND_CENTER_X
#define SCREEN_W DEFAULT_SCREEN_WIDTH
#define SCREEN_H DEFAULT_SCREEN_HEIGHT
#define SCREEN_HALF_W (DEFAULT_SCREEN_WIDTH / 2)  // 240
#define SCREEN_HALF_H (DEFAULT_SCREEN_HEIGHT / 2) // 136
#define SCREEN_MP_MENU_ROW_Y(rowidx) (MP_POS_Y_DEFAULT + (rowidx * DEFAULT_SPACE_Y))

#define TEAM_1 (TheMPGame.GetPlayerTeamID(TheMPGame.LocalPlayerID()) == static_cast<uint32>(eGameTeam::TEAM_A)) // 0
#define TEAM_2 (TheMPGame.GetPlayerTeamID(TheMPGame.LocalPlayerID()) == static_cast<uint32>(eGameTeam::TEAM_B)) // 1

#ifdef MP_FE_MOUSE_IMPROVEMENTS
#define DECLARE_ROW_Y(y, row) DECLARE_MOUSE_RECT(row, 0, y, DEFAULT_SCREEN_WIDTH, y + DEFAULT_SPACE_Y)
#define DECLARE_ROW(row) DECLARE_ROW_Y(SCREEN_MP_MENU_ROW_Y(row), row)
#else
#define DECLARE_ROW_Y(y, row)
#define DECLARE_ROW(row)
#endif

static uint16 nBuddyIndex = 0;
static int nColorIndex = 0;
static int nProgressBarValue = 0;
static int nLastUpdate = 0;

CABGR abarColors[11] = {
    { 255, 223, 223, 255 },
    { 255, 191, 255, 191 },
    { 255, 255, 207, 207 },
    { 255, 255, 159, 159 },
    { 255, 0, 255, 207 },
    { 255, 127, 255, 127 },
    { 255, 127, 255, 255 },
    { 255, 127, 207, 255 },
    { 255, 255, 159, 255 },
    { 255, 207, 127, 255 },
    { 255, 0, 252, 181 }
};

void CHud::DrawMP_ProgressBox(float posX, float posY, int nValue, CABGR& color)
{
    CRGBA boxColor;
    boxColor.r = color.red;
    boxColor.g = color.green;
    boxColor.b = color.blue;
    boxColor.a = color.alpha;
    if (nValue == 0)
    {
        CSprite2d::Draw2DPolygon(posX - SCALE_X(3), posY + SCALE_Y(5), posX + SCALE_X(3), posY + SCALE_Y(5), posX, posY - SCALE_Y(2), posX - SCALE_X(3), posY + SCALE_Y(5), boxColor);
    }
    else if (nValue == 1)
    {
        CSprite2d::Draw2DPolygon(posX - SCALE_X(3), posY - SCALE_Y(2), posX + SCALE_X(3), posY - SCALE_Y(2), posX, posY + SCALE_Y(5), posX - SCALE_X(3), posY - SCALE_Y(2), boxColor);
    }
    else if (nValue == 2)
    {
        CRect box((posX - SCALE_X(2)), (posY - SCALE_Y(2)), (posX + SCALE_X(2)), (posY + SCALE_Y(2)));
        CSprite2d::DrawRect(box, boxColor);
    }
}

/**
 * TODO: VCS
    - change the function name
    - use PSP_SCREEN_SCALE_* macro to adjust fixed PSP coordinates
 */
void CHud::DrawHelpForMultiplayer()
{
    CABGR aColors[11];
    CRGBA* pColor = nil;
    float nYOffset = 0;

    memcpy(aColors, abarColors, sizeof(aColors));
    CFont::SetFontStyle(FONT_STANDARD);
#ifdef GTA_LIBERTY
    CFont::SetScale(0.38f, 0.75f);
#else
    MP_FONT_SCALE();
    CFont::SetWrapx(SCREEN_STRETCH_X(DEFAULT_SCREEN_WIDTH));
#endif
    CFont::SetColor(CRGBA(COLOR_WHITE, 255));
    CFont::SetRightJustifyOff();
    CFont::SetDropShadowPosition(0);
    CFont::SetPropOn();

    if ((CTimer::GetTimeInMilliseconds() - nLastUpdate) > 600)
    {
        if (++nColorIndex > 7)
            nColorIndex = 0;
        if (++nBuddyIndex >= TheMPGame.GetPlayersCount())
            nBuddyIndex = 0;
        nProgressBarValue = (nProgressBarValue == 2) ? 0 : nProgressBarValue++;
        nLastUpdate = CTimer::GetTimeInMilliseconds();
    }
    if (nBuddyIndex == TheMPGame.LocalPlayerID())
    {
        uint32 nPlayersCount = TheMPGame.GetPlayersCount();
        if (nPlayersCount >= 2)
            if (++nBuddyIndex >= nPlayersCount)
                nBuddyIndex = 0;
    }

#ifdef GTA_LIBERTY
    switch (TheMPGame.GetGameType())
    {
        case eGameType::DEATHMATCH:
        {
            TheRadar->DrawRadarSprite(RADAR_SPRITE_POWERUP, 255, COLOR_YELLOW, SCALE_X(120), SCALE_Y(152));
            CFont::PrintString(TheText.Get("MPI01"), 0, MP_MENU_X(130), SCALE_Y(146));
            CHud::DrawMP_ProgressBox(SCALE_X(120), SCALE_Y(164), nProgressBarValue, aColors[0]);
            CFont::PrintString(TheText.Get("MPI02"), 0, MP_MENU_X(130), SCALE_Y(158));
            CHud::DrawMP_ProgressBox(SCALE_X(120), SCALE_Y(176), nProgressBarValue, aColors[1]);
            CFont::PrintString(TheText.Get("MPI03"), 0, MP_MENU_X(130), SCALE_Y(170));
            CHud::DrawMP_ProgressBox(SCALE_X(240), SCALE_Y(152), nProgressBarValue, aColors[nColorIndex + 2]);
            CFont::PrintString(TheText.Get("MPI04"), 0, MP_MENU_X(250), SCALE_Y(146));
            if (TheMPGame.GetPlayerCount() >= 2)
            {
                if (GANG_MODE)
                {
                    pColor = TheMPGame.GetColor(TEAM_2);
                    TheRadar->DrawRadarSprite(RADAR_SPRITE_MP_PLAYER, UNPACK_COLOR(pColor), SCALE_X(240), SCALE_Y(164));
                    CFont::PrintString(TheText.Get("MPI05"), 0, MP_MENU_X(250), SCALE_Y(158));
                    pColor = TheMPGame.GetColor(TEAM_1);
                    TheRadar->DrawRadarSprite(RADAR_SPRITE_MP_PLAYER, UNPACK_COLOR(pColor), SCALE_X(240), SCALE_Y(176));
                    CFont::PrintString(TheText.Get("MPI06"), 0, MP_MENU_X(250), SCALE_Y(170));
                }
                else
                {
                    pColor = TheMPGame.GetPlayerColor(nBuddyIndex);
                    TheRadar->DrawRadarSprite(RADAR_SPRITE_MP_PLAYER, UNPACK_COLOR(pColor), SCALE_X(240), SCALE_Y(164));
                    CFont::PrintString(TheText.Get("MPI05"), 0, MP_MENU_X(250), SCALE_Y(158));
                }
            }
            break;
        }
        case eGameType::MULTIRACE:
        {
            TheRadar->DrawRadarSprite(RADAR_SPRITE_MP_CHECKPOINT, 255, COLOR_YELLOW, SCALE_X(120), SCALE_Y(152));
            CFont::PrintString(TheText.Get("MPI07"), 0, MP_MENU_X(130), SCALE_Y(146));
            TheRadar->DrawRadarSprite(RADAR_SPRITE_MP_CHECKPOINT, 255, COLOR_GREEN, SCALE_X(120), SCALE_Y(164));
            CFont::PrintString(TheText.Get("MPI08"), 0, MP_MENU_X(130), SCALE_Y(158));
            TheRadar->DrawRadarSprite(RADAR_SPRITE_MP_CHECKPOINT, 255, COLOR_YELLOW, SCALE_X(120), SCALE_Y(176));
            CFont::PrintString(TheText.Get("MPI09"), 0, MP_MENU_X(130), SCALE_Y(170));
            CHud::DrawMP_ProgressBox(SCALE_X(240), SCALE_Y(152), nProgressBarValue, aColors[10]);
            CFont::PrintString(TheText.Get("MPI01"), 0, MP_MENU_X(250), SCALE_Y(146));
            if (TheMPGame.GetPlayerCount() >= 2)
            {
                pColor = TheMPGame.GetPlayerColor(nBuddyIndex);
                TheRadar->DrawRadarSprite(RADAR_SPRITE_MP_PLAYER, UNPACK_COLOR(pColor), SCALE_X(240), SCALE_Y(164));
                CFont::PrintString(TheText.Get("MPI10"), 0, MP_MENU_X(250), SCALE_Y(158));
            }
            break;
        }
        case eGameType::DEFENDTHEBASE:
        {
            TheRadar->DrawRadarSprite(RADAR_SPRITE_POWERUP, 255, COLOR_YELLOW, SCALE_X(120), SCALE_Y(152));
            CFont::PrintString(TheText.Get("MPI01"), 0, MP_MENU_X(130), SCALE_Y(146));
            CHud::DrawMP_ProgressBox(SCALE_X(120), SCALE_Y(164), nProgressBarValue, aColors[0]);
            CFont::PrintString(TheText.Get("MPI02"), 0, MP_MENU_X(130), SCALE_Y(158));
            CHud::DrawMP_ProgressBox(SCALE_X(120), SCALE_Y(176), nProgressBarValue, aColors[1]);
            CFont::PrintString(TheText.Get("MPI03"), 0, MP_MENU_X(130), SCALE_Y(170));
            CHud::DrawMP_ProgressBox(SCALE_X(120), SCALE_Y(188), nProgressBarValue, aColors[nColorIndex + 2]);
            CFont::PrintString(TheText.Get("MPI04"), 0, MP_MENU_X(130), SCALE_Y(182));
            if (TheMPGame.GetPlayerCount() >= 2)
            {
                pColor = TheMPGame.GetColor(TEAM_2);
                TheRadar->DrawRadarSprite(RADAR_SPRITE_MP_PLAYER, UNPACK_COLOR(pColor), SCALE_X(240), SCALE_Y(152));
                CFont::PrintString(TheText.Get("MPI05"), 0, MP_MENU_X(250), SCALE_Y(146));
                pColor = TheMPGame.GetBlipColor(TheMPGame.GetDefendingTeamID() - 1);
                TheRadar->DrawRadarSprite(RADAR_SPRITE_MP_BASE, UNPACK_COLOR(pColor), SCALE_X(240), SCALE_Y(164));
                CFont::PrintString(TheText.Get("MPI11"), 0, MP_MENU_X(250), SCALE_Y(158));
                pColor = TheMPGame.GetColor(TEAM_1);
                TheRadar->DrawRadarSprite(RADAR_SPRITE_MP_PLAYER, UNPACK_COLOR(pColor), SCALE_X(240), SCALE_Y(176));
                CFont::PrintString(TheText.Get("MPI06"), 0, MP_MENU_X(250), SCALE_Y(170));
            }
            break;
        }
        case eGameType::CTF:
        {
            TheRadar->DrawRadarSprite(RADAR_SPRITE_POWERUP, 255, COLOR_YELLOW, SCALE_X(120), SCALE_Y(152));
            CFont::PrintString(TheText.Get("MPI01"), 0, MP_MENU_X(130), SCALE_Y(146));
            CHud::DrawMP_ProgressBox(SCALE_X(120), SCALE_Y(164), nProgressBarValue, aColors[0]);
            CFont::PrintString(TheText.Get("MPI02"), 0, MP_MENU_X(130), SCALE_Y(158));
            CHud::DrawMP_ProgressBox(SCALE_X(120), SCALE_Y(176), nProgressBarValue, aColors[1]);
            CFont::PrintString(TheText.Get("MPI03"), 0, MP_MENU_X(130), SCALE_Y(170));
            CHud::DrawMP_ProgressBox(SCALE_X(120), SCALE_Y(188), nProgressBarValue, aColors[nColorIndex + 2]);
            CFont::PrintString(TheText.Get("MPI04"), 0, MP_MENU_X(130), SCALE_Y(182));
            if (TheMPGame.GetPlayerCount() >= 2)
            {
                pColor = TheMPGame.GetColor(TEAM_2);
                TheRadar->DrawRadarSprite(RADAR_SPRITE_MP_PLAYER, UNPACK_COLOR(pColor), SCALE_X(240), SCALE_Y(152));
                CFont::PrintString(TheText.Get("MPI05"), 0, MP_MENU_X(250), SCALE_Y(146));
                pColor = TheMPGame.GetColor(TEAM_1);
                TheRadar->DrawRadarSprite(RADAR_SPRITE_MP_PLAYER, UNPACK_COLOR(pColor), SCALE_X(240), SCALE_Y(164));
                CFont::PrintString(TheText.Get("MPI06"), 0, MP_MENU_X(250), SCALE_Y(158));
                pColor = TheMPGame.GetBlipColor(TEAM_1);
                TheRadar->DrawRadarSprite(RADAR_SPRITE_MP_BASE, UNPACK_COLOR(pColor), SCALE_X(240), SCALE_Y(176));
                CFont::PrintString(TheText.Get("MPI11"), 0, MP_MENU_X(250), SCALE_Y(170));
                pColor = TheMPGame.GetBlipColor(TEAM_2);
                TheRadar->DrawRadarSprite(RADAR_SPRITE_MP_BASE, UNPACK_COLOR(pColor), SCALE_X(240), SCALE_Y(188));
                CFont::PrintString(TheText.Get("MPI12"), 0, MP_MENU_X(250), SCALE_Y(182));
            }
            break;
        }
        case eGameType::TANK:
        {
            TheRadar->DrawRadarSprite(RADAR_SPRITE_POWERUP, 255, COLOR_YELLOW, SCALE_X(120), SCALE_Y(152));
            CFont::PrintString(TheText.Get("MPI01"), 0, MP_MENU_X(130), SCALE_Y(146));
            CHud::DrawMP_ProgressBox(SCALE_X(120), SCALE_Y(164), nProgressBarValue, aColors[0]);
            CFont::PrintString(TheText.Get("MPI02"), 0, MP_MENU_X(130), SCALE_Y(158));
            CHud::DrawMP_ProgressBox(SCALE_X(120), SCALE_Y(176), nProgressBarValue, aColors[1]);
            CFont::PrintString(TheText.Get("MPI03"), 0, MP_MENU_X(130), SCALE_Y(170));
            CHud::DrawMP_ProgressBox(SCALE_X(240), SCALE_Y(152), nProgressBarValue, aColors[nColorIndex + 2]);
            CFont::PrintString(TheText.Get("MPI04"), 0, MP_MENU_X(250), SCALE_Y(182));
            nYOffset = 164;
            if (TheMPGame.GetPlayerCount() >= 2)
            {
                pColor = TheMPGame.GetPlayerColor(nBuddyIndex);
                TheRadar->DrawRadarSprite(RADAR_SPRITE_MP_PLAYER, UNPACK_COLOR(pColor), SCALE_X(240), SCALE_Y(nYOffset));
                CFont::PrintString(TheText.Get("MPI05"), 0, MP_MENU_X(250), SCALE_Y(158));
                nYOffset = 176;
            }
            TheRadar->DrawRadarSprite(RADAR_SPRITE_MP_TANK, 255, COLOR_GREEN, SCALE_X(240), SCALE_Y(nYOffset));
            CFont::PrintString(TheText.Get("MPI13"), 0, MP_MENU_X(250), SCALE_Y(nYOffset - 6));
            break;
        }
        case eGameType::HITPARADE:
        {
            TheRadar->DrawRadarSprite(RADAR_SPRITE_POWERUP, 255, COLOR_YELLOW, SCALE_X(120), SCALE_Y(152));
            CFont::PrintString(TheText.Get("MPI01"), 0, MP_MENU_X(130), SCALE_Y(146));
            CHud::DrawMP_ProgressBox(SCALE_X(120), SCALE_Y(164), nProgressBarValue, aColors[0]);
            CFont::PrintString(TheText.Get("MPI02"), 0, MP_MENU_X(130), SCALE_Y(158));
            CHud::DrawMP_ProgressBox(SCALE_X(120), SCALE_Y(176), nProgressBarValue, aColors[1]);
            CFont::PrintString(TheText.Get("MPI03"), 0, MP_MENU_X(130), SCALE_Y(170));
            CHud::DrawMP_ProgressBox(SCALE_X(240), SCALE_Y(152), nProgressBarValue, aColors[nColorIndex + 2]);
            CFont::PrintString(TheText.Get("MPI04"), 0, MP_MENU_X(250), SCALE_Y(182));
            if (TheMPGame.GetTargetPlayer() != TheMPGame.LocalPlayerID())
            {
                pColor = TheMPGame.GetPlayerColor(TheMPGame.GetTargetPlayer());
                TheRadar->DrawRadarSprite(RADAR_SPRITE_MP_TARGETPLAYER, UNPACK_COLOR(pColor), SCALE_X(240), SCALE_Y(164));
                CFont::PrintString(TheText.Get("MPI14"), 0, MP_MENU_X(250), SCALE_Y(158));
            }
            break;
        }
        case eGameType::SIXTYSECONDS:
        {
            TheRadar->DrawRadarSprite(RADAR_SPRITE_POWERUP, 255, COLOR_YELLOW, SCALE_X(120), SCALE_Y(152));
            CFont::PrintString(TheText.Get("MPI01"), 0, MP_MENU_X(130), SCALE_Y(146));
            CHud::DrawMP_ProgressBox(SCALE_X(120), SCALE_Y(164), nProgressBarValue, aColors[0]);
            CFont::PrintString(TheText.Get("MPI02"), 0, MP_MENU_X(130), SCALE_Y(158));
            CHud::DrawMP_ProgressBox(SCALE_X(120), SCALE_Y(176), nProgressBarValue, aColors[1]);
            CFont::PrintString(TheText.Get("MPI03"), 0, MP_MENU_X(130), SCALE_Y(170));
            CHud::DrawMP_ProgressBox(SCALE_X(120), SCALE_Y(188), nProgressBarValue, aColors[nColorIndex + 2]);
            CFont::PrintString(TheText.Get("MPI04"), 0, MP_MENU_X(130), SCALE_Y(182));
            nYOffset = 152;
            if (TheMPGame.GetPlayerCount() >= 2)
            {
                pColor = TheMPGame.GetPlayerColor(nBuddyIndex);
                TheRadar->DrawRadarSprite(RADAR_SPRITE_MP_PLAYER, UNPACK_COLOR(pColor), SCALE_X(240), SCALE_Y(152));
                CFont::PrintString(TheText.Get("MPI05"), 0, MP_MENU_X(250), SCALE_Y(146));
                nYOffset = 164;
            }
            TheRadar->DrawRadarSprite(RADAR_SPRITE_MP_CAR, 255, COLOR_GREEN, SCALE_X(240), SCALE_Y(nYOffset));
            CFont::PrintString(TheText.Get("MPI15"), 0, MP_MENU_X(250), SCALE_Y(nYOffset - 6));
            nYOffset += 12;
            TheRadar->DrawRadarSprite(RADAR_SPRITE_MP_CARLOCKUP, 255, COLOR_YELLOW, SCALE_X(240), SCALE_Y(nYOffset));
            CFont::PrintString(TheText.Get("MPI16"), 0, MP_MENU_X(250), SCALE_Y(nYOffset - 6));
            break;
        }
    }
#else
    switch (TheMPGame.GetGameType())
    {
        case eGameType::DEATHMATCH:
        {
            TheRadar->DrawRadarSprite(RADAR_SPRITE_POWERUP, 255, COLOR_YELLOW, SCALE_X(120), SCALE_Y(161));
            CFont::PrintString(TheText.Get("MPI01"), 0, MP_MENU_X(130), SCALE_Y(154));
            CHud::DrawMP_ProgressBox(SCALE_X(120), SCALE_Y(175), nProgressBarValue, aColors[0]);
            CFont::PrintString(TheText.Get("MPI02"), 0, MP_MENU_X(130), SCALE_Y(168));
            CHud::DrawMP_ProgressBox(SCALE_X(120), SCALE_Y(189), nProgressBarValue, aColors[1]);
            CFont::PrintString(TheText.Get("MPI03"), 0, MP_MENU_X(130), SCALE_Y(182));
            CHud::DrawMP_ProgressBox(SCALE_X(240), SCALE_Y(161), nProgressBarValue, aColors[nColorIndex + 2]);
            CFont::PrintString(TheText.Get("MPI04"), 0, MP_MENU_X(250), SCALE_Y(154));
            if (TheMPGame.GetPlayersCount() >= 2)
            {
                if (GANG_MODE)
                {
                    pColor = TheMPGame.GetColor(TEAM_2);
                    TheRadar->DrawRadarSprite(RADAR_SPRITE_MP_PLAYER, UNPACK_COLOR(pColor), SCALE_X(240), SCALE_Y(175));
                    CFont::PrintString(TheText.Get("MPI05"), 0, MP_MENU_X(250), SCALE_Y(168));
                    pColor = TheMPGame.GetColor(TEAM_1);
                    TheRadar->DrawRadarSprite(RADAR_SPRITE_MP_PLAYER, UNPACK_COLOR(pColor), SCALE_X(240), SCALE_Y(189));
                    CFont::PrintString(TheText.Get("MPI06"), 0, MP_MENU_X(250), SCALE_Y(182));
                }
                else
                {
                    pColor = TheMPGame.GetPlayerColor(nBuddyIndex);
                    TheRadar->DrawRadarSprite(RADAR_SPRITE_MP_PLAYER, UNPACK_COLOR(pColor), SCALE_X(240), SCALE_Y(175));
                    CFont::PrintString(TheText.Get("MPI05"), 0, MP_MENU_X(250), SCALE_Y(168));
                }
            }
            break;
        }
        case eGameType::MULTIRACE:
        {
            TheRadar->DrawRadarSprite(RADAR_SPRITE_MP_CHECKPOINT, 255, COLOR_YELLOW, SCALE_X(120), SCALE_Y(161));
            CFont::PrintString(TheText.Get("MPI07"), 0, MP_MENU_X(130), SCALE_Y(154));
            TheRadar->DrawRadarSprite(RADAR_SPRITE_MP_CHECKPOINT, 255, COLOR_GREEN, SCALE_X(120), SCALE_Y(175));
            CFont::PrintString(TheText.Get("MPI08"), 0, MP_MENU_X(130), SCALE_Y(168));
            TheRadar->DrawRadarSprite(RADAR_SPRITE_MP_CHECKPOINT, 255, COLOR_YELLOW, SCALE_X(120), SCALE_Y(189));
            CFont::PrintString(TheText.Get("MPI09"), 0, MP_MENU_X(130), SCALE_Y(182));
            CHud::DrawMP_ProgressBox(SCALE_X(240), SCALE_Y(152), nProgressBarValue, aColors[10]);
            CFont::PrintString(TheText.Get("MPI01"), 0, MP_MENU_X(250), SCALE_Y(154));
            if (TheMPGame.GetPlayersCount() >= 2)
            {
                pColor = TheMPGame.GetPlayerColor(nBuddyIndex);
                TheRadar->DrawRadarSprite(RADAR_SPRITE_MP_PLAYER, UNPACK_COLOR(pColor), SCALE_X(240), SCALE_Y(175));
                CFont::PrintString(TheText.Get("MPI10"), 0, MP_MENU_X(250), SCALE_Y(168));
            }
            break;
        }
        case eGameType::DEFENDTHEBASE:
        {
            TheRadar->DrawRadarSprite(RADAR_SPRITE_POWERUP, 255, COLOR_YELLOW, SCALE_X(120), SCALE_Y(161));
            CFont::PrintString(TheText.Get("MPI01"), 0, MP_MENU_X(130), SCALE_Y(154));
            CHud::DrawMP_ProgressBox(SCALE_X(120), SCALE_Y(175), nProgressBarValue, aColors[0]);
            CFont::PrintString(TheText.Get("MPI02"), 0, MP_MENU_X(130), SCALE_Y(168));
            CHud::DrawMP_ProgressBox(SCALE_X(120), SCALE_Y(189), nProgressBarValue, aColors[1]);
            CFont::PrintString(TheText.Get("MPI03"), 0, MP_MENU_X(130), SCALE_Y(182));
            CHud::DrawMP_ProgressBox(SCALE_X(120), SCALE_Y(203), nProgressBarValue, aColors[nColorIndex + 2]);
            CFont::PrintString(TheText.Get("MPI04"), 0, MP_MENU_X(130), SCALE_Y(196));
            if (TheMPGame.GetPlayersCount() >= 2)
            {
                pColor = TheMPGame.GetColor(TEAM_2);
                TheRadar->DrawRadarSprite(RADAR_SPRITE_MP_PLAYER, UNPACK_COLOR(pColor), SCALE_X(240), SCALE_Y(161));
                CFont::PrintString(TheText.Get("MPI05"), 0, MP_MENU_X(250), SCALE_Y(154));
                pColor = TheMPGame.GetBlipColor(TheMPGame.GetDefendingTeamID() - 1);
                TheRadar->DrawRadarSprite(RADAR_SPRITE_MP_BASE, UNPACK_COLOR(pColor), SCALE_X(240), SCALE_Y(175));
                CFont::PrintString(TheText.Get("MPI11"), 0, MP_MENU_X(250), SCALE_Y(168));
                pColor = TheMPGame.GetColor(TEAM_1);
                TheRadar->DrawRadarSprite(RADAR_SPRITE_MP_PLAYER, UNPACK_COLOR(pColor), SCALE_X(240), SCALE_Y(189));
                CFont::PrintString(TheText.Get("MPI06"), 0, MP_MENU_X(250), SCALE_Y(182));
            }
            break;
        }
        case eGameType::CTF:
        {
            TheRadar->DrawRadarSprite(RADAR_SPRITE_POWERUP, 255, COLOR_YELLOW, SCALE_X(120), SCALE_Y(161));
            CFont::PrintString(TheText.Get("MPI01"), 0, MP_MENU_X(130), SCALE_Y(154));
            CHud::DrawMP_ProgressBox(SCALE_X(120), SCALE_Y(175), nProgressBarValue, aColors[0]);
            CFont::PrintString(TheText.Get("MPI02"), 0, MP_MENU_X(130), SCALE_Y(168));
            CHud::DrawMP_ProgressBox(SCALE_X(120), SCALE_Y(189), nProgressBarValue, aColors[1]);
            CFont::PrintString(TheText.Get("MPI03"), 0, MP_MENU_X(130), SCALE_Y(182));
            CHud::DrawMP_ProgressBox(SCALE_X(120), SCALE_Y(203), nProgressBarValue, aColors[nColorIndex + 2]);
            CFont::PrintString(TheText.Get("MPI04"), 0, MP_MENU_X(130), SCALE_Y(196));
            if (TheMPGame.GetPlayersCount() >= 2)
            {
                pColor = TheMPGame.GetColor(TEAM_2);
                TheRadar->DrawRadarSprite(RADAR_SPRITE_MP_PLAYER, UNPACK_COLOR(pColor), SCALE_X(240), SCALE_Y(161));
                CFont::PrintString(TheText.Get("MPI05"), 0, MP_MENU_X(250), SCALE_Y(154));
                pColor = TheMPGame.GetColor(TEAM_1);
                TheRadar->DrawRadarSprite(RADAR_SPRITE_MP_PLAYER, UNPACK_COLOR(pColor), SCALE_X(240), SCALE_Y(175));
                CFont::PrintString(TheText.Get("MPI06"), 0, MP_MENU_X(250), SCALE_Y(168));
                pColor = TheMPGame.GetBlipColor(TEAM_1);
                TheRadar->DrawRadarSprite(RADAR_SPRITE_MP_BASE, UNPACK_COLOR(pColor), SCALE_X(240), SCALE_Y(189));
                CFont::PrintString(TheText.Get("MPI11"), 0, MP_MENU_X(250), SCALE_Y(182));
                pColor = TheMPGame.GetBlipColor(TEAM_2);
                TheRadar->DrawRadarSprite(RADAR_SPRITE_MP_BASE, UNPACK_COLOR(pColor), SCALE_X(240), SCALE_Y(203));
                CFont::PrintString(TheText.Get("MPI12"), 0, MP_MENU_X(250), SCALE_Y(196));
            }
            break;
        }
        case eGameType::TANK:
        case eGameType::HUNTERATTACK:
        {
            TheRadar->DrawRadarSprite(RADAR_SPRITE_POWERUP, 255, COLOR_YELLOW, SCALE_X(120), SCALE_Y(161));
            CFont::PrintString(TheText.Get("MPI01"), 0, MP_MENU_X(130), SCALE_Y(154));
            CHud::DrawMP_ProgressBox(SCALE_X(120), SCALE_Y(175), nProgressBarValue, aColors[0]);
            CFont::PrintString(TheText.Get("MPI02"), 0, MP_MENU_X(130), SCALE_Y(168));
            CHud::DrawMP_ProgressBox(SCALE_X(120), SCALE_Y(189), nProgressBarValue, aColors[1]);
            CFont::PrintString(TheText.Get("MPI03"), 0, MP_MENU_X(130), SCALE_Y(182));
            CHud::DrawMP_ProgressBox(SCALE_X(240), SCALE_Y(161), nProgressBarValue, aColors[nColorIndex + 2]);
            CFont::PrintString(TheText.Get("MPI04"), 0, MP_MENU_X(250), SCALE_Y(154));
            nYOffset = 175;
            int32 nYOffsetText = 168;
            if (TheMPGame.GetPlayersCount() >= 2)
            {
                pColor = TheMPGame.GetPlayerColor(nBuddyIndex);
                TheRadar->DrawRadarSprite(RADAR_SPRITE_MP_PLAYER, UNPACK_COLOR(pColor), SCALE_X(240), SCALE_Y(nYOffset));
                CFont::PrintString(TheText.Get("MPI05"), 0, MP_MENU_X(250), SCALE_Y(168));
                nYOffsetText = 182;
                nYOffset = 189;
            }
            if (TheMPGame.GetGameType() == eGameType::TANK)
            {
                TheRadar->DrawRadarSprite(RADAR_SPRITE_MP_TANK, 255, COLOR_GREEN, SCALE_X(240), SCALE_Y(nYOffset));
                CFont::PrintString(TheText.Get("MPI13"), 0, MP_MENU_X(250), SCALE_Y(nYOffsetText));
            }
            else // eGameType::HUNTERATTACK
            {
                TheRadar->DrawRadarSprite(RADAR_SPRITE_HELI, 255, COLOR_GREEN, SCALE_X(240), SCALE_Y(nYOffset));
                CFont::PrintString(TheText.Get("MPI17"), 0, MP_MENU_X(250), SCALE_Y(nYOffsetText));
            }
            break;
        }
        case eGameType::HITPARADE:
        {
            TheRadar->DrawRadarSprite(RADAR_SPRITE_POWERUP, 255, COLOR_YELLOW, SCALE_X(120), SCALE_Y(161));
            CFont::PrintString(TheText.Get("MPI01"), 0, MP_MENU_X(130), SCALE_Y(154));
            CHud::DrawMP_ProgressBox(SCALE_X(120), SCALE_Y(175), nProgressBarValue, aColors[0]);
            CFont::PrintString(TheText.Get("MPI02"), 0, MP_MENU_X(130), SCALE_Y(168));
            CHud::DrawMP_ProgressBox(SCALE_X(120), SCALE_Y(189), nProgressBarValue, aColors[1]);
            CFont::PrintString(TheText.Get("MPI03"), 0, MP_MENU_X(130), SCALE_Y(182));
            CHud::DrawMP_ProgressBox(SCALE_X(240), SCALE_Y(161), nProgressBarValue, aColors[nColorIndex + 2]);
            CFont::PrintString(TheText.Get("MPI04"), 0, MP_MENU_X(250), SCALE_Y(154));
            if (TheMPGame.GetTargetPlayer() != TheMPGame.LocalPlayerID())
            {
                pColor = TheMPGame.GetPlayerColor(TheMPGame.GetTargetPlayer());
                TheRadar->DrawRadarSprite(RADAR_SPRITE_MP_TARGETPLAYER, UNPACK_COLOR(pColor), SCALE_X(240), SCALE_Y(175));
                CFont::PrintString(TheText.Get("MPI14"), 0, MP_MENU_X(250), SCALE_Y(168));
                TheRadar->DrawRadarSprite(RADAR_SPRITE_MP_PLAYER, 255, 75, 151, 75, SCALE_X(240), SCALE_Y(189));
                CFont::PrintString(TheText.Get("MPI_ASS"), 0, MP_MENU_X(250), SCALE_Y(182));
            }
            break;
        }
        case eGameType::SIXTYSECONDS:
        {
            TheRadar->DrawRadarSprite(RADAR_SPRITE_POWERUP, 255, COLOR_YELLOW, SCALE_X(120), SCALE_Y(161));
            CFont::PrintString(TheText.Get("MPI01"), 0, MP_MENU_X(130), SCALE_Y(154));
            CHud::DrawMP_ProgressBox(SCALE_X(120), SCALE_Y(175), nProgressBarValue, aColors[0]);
            CFont::PrintString(TheText.Get("MPI02"), 0, MP_MENU_X(130), SCALE_Y(168));
            CHud::DrawMP_ProgressBox(SCALE_X(120), SCALE_Y(189), nProgressBarValue, aColors[1]);
            CFont::PrintString(TheText.Get("MPI03"), 0, MP_MENU_X(130), SCALE_Y(182));
            CHud::DrawMP_ProgressBox(SCALE_X(120), SCALE_Y(203), nProgressBarValue, aColors[nColorIndex + 2]);
            CFont::PrintString(TheText.Get("MPI04"), 0, MP_MENU_X(130), SCALE_Y(196));
            nYOffset = 161;
            if (TheMPGame.GetPlayersCount() >= 2)
            {
                pColor = TheMPGame.GetPlayerColor(nBuddyIndex);
                TheRadar->DrawRadarSprite(RADAR_SPRITE_MP_PLAYER, UNPACK_COLOR(pColor), SCALE_X(240), SCALE_Y(161));
                CFont::PrintString(TheText.Get("MPI05"), 0, MP_MENU_X(250), SCALE_Y(154));
                nYOffset = 175;
            }
            TheRadar->DrawRadarSprite(RADAR_SPRITE_MP_CAR, 255, COLOR_GREEN, SCALE_X(240), SCALE_Y(nYOffset));
            CFont::PrintString(TheText.Get("MPI15"), 0, MP_MENU_X(250), SCALE_Y(nYOffset - 7));
            nYOffset += 14;
            TheRadar->DrawRadarSprite(RADAR_SPRITE_MP_CARLOCKUP, 255, COLOR_YELLOW, SCALE_X(240), SCALE_Y(nYOffset));
            CFont::PrintString(TheText.Get("MPI16"), 0, MP_MENU_X(250), SCALE_Y(nYOffset - 7));
            break;
        }
        case eGameType::FLAGBALL:
        {
            TheRadar->DrawRadarSprite(RADAR_SPRITE_POWERUP, 255, COLOR_YELLOW, SCALE_X(120), SCALE_Y(161));
            CFont::PrintString(TheText.Get("MPI01"), 0, MP_MENU_X(130), SCALE_Y(154));
            CHud::DrawMP_ProgressBox(SCALE_X(120), SCALE_Y(175), nProgressBarValue, aColors[0]);
            CFont::PrintString(TheText.Get("MPI02"), 0, MP_MENU_X(130), SCALE_Y(168));
            CHud::DrawMP_ProgressBox(SCALE_X(120), SCALE_Y(189), nProgressBarValue, aColors[1]);
            CFont::PrintString(TheText.Get("MPI03"), 0, MP_MENU_X(130), SCALE_Y(182));
            CHud::DrawMP_ProgressBox(SCALE_X(120), SCALE_Y(203), nProgressBarValue, aColors[nColorIndex + 2]);
            CFont::PrintString(TheText.Get("MPI04"), 0, MP_MENU_X(130), SCALE_Y(196));

            // todo
            nYOffset = 147;
            if (TheMPGame.GetPlayersCount() >= 2)
            {
                pColor = TheMPGame.GetPlayerColor(TEAM_2);
                TheRadar->DrawRadarSprite(RADAR_SPRITE_MP_PLAYER, UNPACK_COLOR(pColor), SCALE_X(240), SCALE_Y(nYOffset));
                CFont::PrintString(TheText.Get("MPI05"), 0, MP_MENU_X(250), SCALE_Y(140));
                pColor = TheMPGame.GetColor(TEAM_1);
                TheRadar->DrawRadarSprite(RADAR_SPRITE_MP_PLAYER, UNPACK_COLOR(pColor), SCALE_X(240), SCALE_Y(161));
                CFont::PrintString(TheText.Get("MPI06"), 0, MP_MENU_X(250), SCALE_Y(154));
                pColor = TheMPGame.GetBlipColor(TEAM_2);
                TheRadar->DrawRadarSprite(RADAR_SPRITE_MP_BASE, UNPACK_COLOR(pColor), SCALE_X(240), SCALE_Y(182));
                CFont::PrintString(TheText.Get("MPI12"), 0, MP_MENU_X(250), SCALE_Y(196));

                nYOffset = 175;
            }
            TheRadar->DrawRadarSprite(RADAR_SPRITE_MP_BASE, 255, COLOR_GREEN, SCALE_X(240), SCALE_Y(nYOffset));
            CFont::PrintString(TheText.Get("MPI15"), 0, MP_MENU_X(250), SCALE_Y(nYOffset - 7));
            nYOffset += 14;
            TheRadar->DrawRadarSprite(RADAR_SPRITE_MPBOMB, 255, COLOR_YELLOW, SCALE_X(240), SCALE_Y(nYOffset));
            CFont::PrintString(TheText.Get("MPI16"), 0, MP_MENU_X(250), SCALE_Y(nYOffset - 7));

            MULTIGAME_UNIMPLEMENTED(); // todo
            break;
        }
        case eGameType::VIP:
        {
            if (TheMPGame.GetPlayersCount() >= 2)
            {

            }

            MULTIGAME_UNIMPLEMENTED(); // todo
            break;
        }
    }
#endif
}



/**
 * TODO:
    - change the function name
    - use PSP_SCREEN_SCALE_* macro to adjust fixed PSP coordinates
 */
void CHud::DrawStatusForMultiplayer() // inlined in VCS
{
#ifdef GTA_LIBERTY
    if (gbMP_DrawPauseScreen || !gbMP_RenderHudExtras)
#else
    if (gbMP_DrawPauseScreen || !gbMP_RenderHudExtras || !m_Wants_To_Draw_Hud || !gbMP_DrawHudCars)
#endif
        return;

    CRGBA color;

    switch (TheMPGame.GetGameType())
    {
#ifdef GTA_LIBERTY
        case eGameType::DEFENDTHEBASE:
            color = *TheMPGame.GetColorByTeamID(TheMPGame.getDefendingTeamID());
            TheRadar->RadarSprites[RADAR_SPRITE_MP_CAR]->Draw(SCREEN_SCALE_RECT(CRect(439, 105, 455, 121)), color);
            break;
        case eGameType::CTF:
            color = *TheMPGame.GetColorByTeamID(0);
            TheRadar->RadarSprites[RADAR_SPRITE_MP_CAR]->Draw(SCREEN_SCALE_RECT(CRect(418, 90, 434, 106)), color);
            color = *TheMPGame.GetColorByTeamID(1);
            TheRadar->RadarSprites[RADAR_SPRITE_MP_CAR]->Draw(SCREEN_SCALE_RECT(CRect(418, 134, 434, 150)), color);
            break;
#else
        case eGameType::DEFENDTHEBASE:
            color = *TheMPGame.GetColor(TheMPGame.GetDefendingTeamID() - 1);
            TheRadar->RadarSprites[RADAR_SPRITE_MP_CAR].Draw(SCREEN_SCALE_RECT(CRect(439, 105, 455, 121)), color);
            break;
        case eGameType::CTF:
            color = *TheMPGame.GetColor(TEAM_1);
            TheRadar->RadarSprites[RADAR_SPRITE_MP_CAR].Draw(SCREEN_SCALE_RECT(CRect(418, 90, 434, 106)), color);
            color = *TheMPGame.GetColor(TEAM_2);
            TheRadar->RadarSprites[RADAR_SPRITE_MP_CAR].Draw(SCREEN_SCALE_RECT(CRect(418, 134, 434, 150)), color);
            break;
#endif
    }
}

extern RwTexture* gpCrossHairTex; // TODO: this suggest us to move this func to CHud

void CHud::DrawHudForMultiplayer()
{
    if (gbMP_DrawPauseScreen)
    {
        CSprite2d sprite;
        sprite.Draw(0, 0, MP_MENU_X(SCREEN_W), SCALE_Y(SCREEN_H), CRGBA(0, 0, 0, 180));
        if (!gbMP_DrawPauseScreenNoBox)
            sprite.Draw(SCALE_X(105), SCALE_Y(50), SCALE_X(260), SCALE_Y(165), CRGBA(0, 0, 0, 220));
        RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, nil);
        RwRenderStateSet(rwRENDERSTATEZTESTENABLE, nil);
        RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)TRUE);
        RwRenderStateSet(rwRENDERSTATESRCBLEND, (void*)rwBLENDSRCALPHA);
        RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void*)rwBLENDINVSRCALPHA);
        RwRenderStateSet(rwRENDERSTATETEXTURERASTER, (void*)RwTextureGetRaster(gpCrossHairTex));
        int nPosX = 1000;
        int nPosY = (1000*0)+268;
        // TODO: strang clipping psp Y >272 ? or ps2 smth
#define OPTION_Y_START (230) // 268 idb
//#define OPTION_Y_SIZE (18) // psp idb
#define OPTION_Y_SIZE (15) // lua delta text print
        switch (gnMP_PauseScreenSelection)
        {
            case 0:
                nPosX = 195;
                nPosY = OPTION_Y_START + (OPTION_Y_SIZE * 0/*gnMP_PauseScreenSelection*/); // PC, todo mean clip
                //nPosY = 268; // psp delta 18, text print at 220
                break;
            case 1:
                nPosX = 195;
                nPosY = OPTION_Y_START + (OPTION_Y_SIZE * 1);
                //nPosY = 286; // psp, text print at 235
                break;
            case 2:
                nPosX = 195;
                nPosY = OPTION_Y_START + (OPTION_Y_SIZE * 2);
                //nPosY = 304; // psp  ://///
                break;
        }
#undef OPTION_Y_SIZE
#undef OPTION_Y_START
        CSprite::RenderOneXLUSprite_Rotate_Aspect(SCALE_X(nPosX), SCALE_Y(nPosY), 12.0f, SCALE_X(8.0f), SCALE_Y(8.0f), 255, 255, 255, 158, 1.0f, 0, 255);

        if (gbMP_HudShowHelp)
            TheHud->DrawHelpForMultiplayer();
    }
    TheHud->DrawStatusForMultiplayer();
#ifndef GTA_LIBERTY
#ifdef MULTIGAME_SCM
    if (gIsMultiplayerGame && FindPlayerPed() && TheMPGame.GetGameType() != eGameType::SCM)
#else
    if (gIsMultiplayerGame && FindPlayerPed()) // gIsMultiplayerGame usless
#endif
    {
        if (FindPlayerPed()->IsPlayerNotInActivityZone())
        {
            int32 nTime = (((5000.0f - FindPlayerPed()->GetInActivityZoneTime()) / 1000.0f) + 1.0f);
            nTime = Max(nTime, 0);
            CHud::PrintMPLeftActivityZone(nTime); // each frame add timed message, bug? no once?
        }
    }
#endif
    mp_update_sprites(); // vcs in hud, custom here, but ok
}

#ifndef GTA_LIBERTY
void CHud::PrintMPLeftActivityZone(int32 nTime)
{
    CMessages::AddBigMessage(TheText.Get("MPTURNB"), 20000, 7); // You left the play area!
}
#endif

#ifdef GTA_LIBERTY
void cLobby::SetTextColorStyle(bool selected)
#else
// mode 0 - cyan/orange, 1 - white/yellow(rst/start game)
void cLobby::SetTextColorStyle(bool selected, int32 mode)
#endif
{
    int nAlpha = FrontEndMenuManager->FadeIn(255);
    CFont::SetFontStyle(FONT_STANDARD);
#ifdef GTA_LIBERTY
    CFont::SetScale(0.364f, 0.792f); // lcs
#else
    MP_FONT_SCALE();
#endif
    CFont::SetDropShadowPosition(false);
#ifdef GTA_LIBERTY
    CFont::SetDropColor(CRGBA(0, 0, 0, nAlpha));
#endif
#ifdef GTA_LIBERTY
    CFont::SetColor(selected ? CRGBA(MP_TEXT_COLOUR_SELECTED, nAlpha) : CRGBA(MP_TEXT_COLOUR, nAlpha)); // lcs sel white, blue, vcs sel yellow, cyan
#else
    switch (mode)
    {
        case 0:
        {
            CFont::SetColor(selected ? CRGBA(MP_TEXT_COLOUR_SELECTED, nAlpha) : CRGBA(MP_TEXT_COLOUR, nAlpha)); // lcs sel white, blue, vcs sel yellow, cyan
            break;
        }
        case 1:
        {
            CFont::SetColor(selected ? CRGBA(COLOR_WHITE, nAlpha) : CRGBA(COLOR_YELLOW, nAlpha)); // vcs sel white, yellow (rst, start)
            break;
        }
    }
#endif
}

void cLobby::SetInactiveTextColorStyle(bool selected)
{
    int nAlpha = FrontEndMenuManager->FadeIn(255);
    CFont::SetFontStyle(FONT_STANDARD);
#ifdef GTA_LIBERTY
    CFont::SetScale(0.364f, 0.792f);
#else
    MP_FONT_SCALE();
#endif
    CFont::SetDropShadowPosition(false);
#ifdef GTA_LIBERTY
    CFont::SetDropColor(CRGBA(0, 0, 0, nAlpha));
#endif
    CFont::SetColor(selected ? CRGBA(COLOR_MUSTARD_INACTIVE, nAlpha) : CRGBA(COLOR_CYAN_INACTIVE, nAlpha));
}

void drawPlayerList(int nPosX, int nMaxTeams)
{
    cAdhoc& Adhoc = TheAdhoc;
    cMultiGame& Game = cMultiGame::Instance();
    int32 nOffsetY = DEFAULT_SPACE_Y + 5; // 5 is https://prnt.sc/LxcpAJ_iNYYp // 16?
    int32 nAlpha = FrontEndMenuManager->FadeIn(255);
    for (int32 nTeamID = 0; nTeamID < nMaxTeams; nTeamID++)
    {
        // draw red/blue gangs
        if (GANG_MODE) // https://prnt.sc/8ftyifWHn6aX
        {
            CFont::SetColor(*Game.GetColor(nTeamID));
            uint8 idx = nTeamID == static_cast<uint8>(eGameTeam::TEAM_A) ? Game.GetTeam1GangID() : Game.GetTeam2GangID();
            CFont::PrintString(MP_MENU_X(nPosX), SCALE_Y(MP_POS_Y_DEFAULT + nOffsetY), TheText.Get(gMPGangDefs[idx].name));
            nOffsetY += DEFAULT_SPACE_Y;
        }

        // from CStreaming::LoadPedbanksForMultiplayer()
        for (int32 peeridx = 0; peeridx < MP_NUM_PEERS; peeridx++)
        {
            tAdhocPeerData& peer = Adhoc.GetMatchingInfo(MP_HOST_INDEX)->m_nPeersConnInfo[peeridx];
            if (peer.macAddr.IsBroadcast())
                continue;

            if (peer.nTeamID == nTeamID || FREE_MODE)
            {
                CRGBA nameColor(MP_TEXT_COLOUR, nAlpha);
                bool bSelfPeer = (peer.macAddr == Adhoc.GetPlayerMacAddress());
                bool bIsHost = (Adhoc.GetMatchingInfo(MP_HOST_INDEX)->m_HostPeerData.peerAddr.mac == peer.macAddr);

                if (bSelfPeer)
                    nameColor = CRGBA(COLOR_YELLOW, nAlpha); // self yellow color
                else if (GANG_MODE)
                    nameColor = *Game.GetColor(nTeamID);

#if defined(ADHOCCTL_USE_CUSTOM_IDENT) && !defined(GTA_PSP)
                bool bLocal = false;
                SceNetAdhocctlPeerInfo* pProAdhocPeer = getPeerInfo(peer.macAddr.GetBytesProAdhoc(), &bLocal);
                bool bVanilaPeer = (pProAdhocPeer && ((pProAdhocPeer->flags & ADHOCCTL_CUSTOM_FLAG) == 0));

                CSprite2d* spr = nil;
                if (bVanilaPeer)
                    spr = &FrontEndMenuManager->m_aFrontEndSprites[MENUSPRITE_PSP];
                else
                    spr = &FrontEndMenuManager->m_aFrontEndSprites[MENUSPRITE_PC];
                    spr->Draw(MP_MENU_X(nPosX) - 32, SCALE_Y(MP_POS_Y_DEFAULT + nOffsetY), 20 * SCREEN_ASPECT_RATIO, 20 * SCREEN_ASPECT_RATIO, CRGBA(255, 255, 255, 255));
                    //spr->Draw(MP_MENU_X(nPosX) - 32, SCALE_Y(MP_POS_Y_DEFAULT + nOffsetY), 20 * SCREEN_ASPECT_RATIO, 20 * SCREEN_ASPECT_RATIO, nameColor); // nick colour
#endif

                CFont::SetColor(nameColor);
                base::string playerName;
                TheAdhoc.GetPlayerNameFromMacAddr(playerName, peer.macAddr);
                if (playerName)
                {
                    wchar buff[SCE_NET_ADHOCCTL_NICKNAME_LEN * 2];
#ifndef GTA_LIBERTY
                    char name[SCE_NET_ADHOCCTL_NICKNAME_LEN];
                    snprintf(name, sizeof(name), "%s%s", playerName.c_str(), bIsHost ? " *" : "");
                    AsciiToUnicode(name, buff);
                    CFont::PrintString(MP_MENU_X(nPosX + PLAYER_LIST_SPACE_X), SCALE_Y(MP_POS_Y_DEFAULT + nOffsetY), buff);
#else
                    AsciiToUnicode(playerName.c_str(), buff);
                    CFont::PrintString(MP_MENU_X(nPosX + PLAYER_LIST_SPACE_X), SCALE_Y(MP_POS_Y_DEFAULT + nOffsetY), buff);
                    if (bIsHost)
                    {
                        TODO();
                        TODO();
                        TODO();
                        TODO();
                        // todo m_NetStarSprite, temp radio 1
                        FrontEndMenuManager->m_aFrontEndSprites[MENUSPRITE_RADIO_1].Draw(MP_MENU_X(nPosX), SCALE_Y(MP_POS_Y_DEFAULT + nOffsetY), 11, 11, nameColor);
                    }
#endif
                }
                else
                {
                    CFont::PrintString(MP_MENU_X(nPosX + PLAYER_LIST_SPACE_X), SCALE_Y(MP_POS_Y_DEFAULT + nOffsetY), TheText.Get("NO_NAME"));
                }
                nOffsetY += DEFAULT_SPACE_Y;
            }
        }
    }
}

void drawStatusText(cLobby& pLobby, const char* textKey)
{
#ifdef GTA_LIBERTY
    pLobby.SetTextColorStyle(false);
#else
    pLobby.SetTextColorStyle(false, 1);
#endif
    CFont::SetCentreOn();
    CFont::PrintString(MP_MENU_X(SCREEN_HALF_W), SCALE_Y(210), TheText.Get(textKey));
}

void cLobby::RenderCharacterOption(int nRowIndex, int nPosY)
{
#ifdef GTA_LIBERTY
    m_lobbyPed.Render();
#endif
    SET_MP_TEXT_COLOUR_STYLE(nRowIndex, 0);
#ifdef MULTIGAME_SCM
    if (cMultiGame::Instance().GetGameType() == eGameType::SCM) // no choose skin at scm mode (visual)
        SetInactiveTextColorStyle(m_nSelectedMenuOptionIndex == nRowIndex);
#endif
    CFont::SetCentreOff();
#ifdef GTA_LIBERTY
    CFont::SetRightJustifyOn();
    CFont::PrintString(MP_MENU_X(OPTION_LABEL_START_X), SCALE_Y(nPosY), TheText.Get("FEMP_CS")); // character
    CFont::SetRightJustifyOff();
#else
    CFont::PrintString(MP_MENU_X(OPTION_LABEL_START_X), SCALE_Y(nPosY), TheText.Get("FEMP_CS")); // character
#endif
    CFont::PrintString(MP_MENU_X(OPTION_VALUE_START_X), SCALE_Y(nPosY), TheText.Get(ga_netModelList[cMultiGame::s_nPlayerModelIndex].key));
}

void cLobby::DrawMainGameScreen()
{
    DONT_OPTIMIZE();

    // Main 
    if (!TheAdhoc.IsNextStateNow(&cAdhoc::StateShutdown) && FrontEndMenuManager->IsMPPageActive())
    {
        int32 nYOffset = 60;
        CFont::SetCentreOn();
        SET_MP_TEXT_COLOUR_STYLE(0, 0);
        MP_FONT_SCALE();
#ifdef GTA_LIBERTY
        if (m_nSelectedMenuOptionIndex == 0)
        {
            CFont::SetScale(SCALE_X(0.48576f), SCALE_Y(1.056f));
            nYOffset = 58;
        }
#endif
        DECLARE_ROW_Y(nYOffset, 0);
        CFont::PrintString(TheText.Get("FEMP_JG"), 0, MP_MENU_X(SCREEN_HALF_W), SCALE_Y(nYOffset)); // join game
        SET_MP_TEXT_COLOUR_STYLE(1, 0);
        MP_FONT_SCALE();
        nYOffset = 80;
#ifdef GTA_LIBERTY
        if (m_nSelectedMenuOptionIndex == 1)
        {
            CFont::SetScale(SCALE_X(0.48576f), SCALE_Y(1.056f));
            nYOffset = 78;
        }
#endif
        DECLARE_ROW_Y(nYOffset, 1);
        CFont::PrintString(TheText.Get("FEMP_HG"), 0, MP_MENU_X(SCREEN_HALF_W), SCALE_Y(nYOffset)); // host game

        return;
    }

    // todo cleanup
    // Before main wanings
    CFont::SetColor(CRGBA(MP_TEXT_COLOUR, FrontEndMenuManager->FadeIn(255)));
    CFont::SetDropShadowPosition(0);
    CFont::SetDropColor(CRGBA(0, 0, 0, FrontEndMenuManager->FadeIn(255)));
    CFont::SetFontStyle(FONT_STANDARD);
    //CFont::ResetState(); // vcs todo, now its broke

    // Note: Big text (scale 1.0) [No WIFI, BadNick, Don't talk, CDERROR]
    //CFont::SetScale(SCREEN_SCALE_X(1.0f), SCREEN_SCALE_Y(1.0f)); // psp (so big)
    //CFont::SetScale(SCALE_X(0.6f), SCALE_Y(0.6f)); // custom PC, big feel
    MP_FONT_SCALE(); // default scale, like starting mp page

    CFont::SetWrapx(SCALE_X(467));
    CFont::SetRightJustifyWrap(13.0f);
    CFont::SetRightJustifyOff();
    CFont::SetCentreOn();
#ifdef GTA_LIBERTY
    CFont::SetCentreSize(SCALE_X(230));
#else
    CFont::SetCentreSize(SCALE_X(SCREEN_W - 125)); // 355 guessed todo font
#endif
    CFont::SetNewLineAdd(6);

    float fYOffset = 60;
    if (psp_is_umd_readable())
    {
        if (TheAdhoc.IsWifiSwitchOn())
        {
            if (FrontEndMenuManager->GetHasJoinedNetwork()) // when join/host menu, after yes no
            {
#ifdef GTA_LIBERTY
                // Don't talk with friends or strangers standing nearby, play Grand Theft Auto with them instead.
                CFont::PrintString(TheText.Get("FEMP_SM"), 0, MP_MENU_X(SCREEN_HALF_W), SCALE_Y(fYOffset));
#else
                if (!m_bBadCharWarn) {
                    // Don't talk with friends or strangers standing nearby, play Grand Theft Auto with them instead.
                    CFont::PrintString(TheText.Get("FEMP_SM"), 0, MP_MENU_X(SCREEN_HALF_W), SCALE_Y(fYOffset));
                }
                else {
                    // Your nickname contains unsupported characters. These characters will appear as ''
                    CFont::PrintString(TheText.Get("MP_MICM"), 0, MP_MENU_X(SCREEN_HALF_W), SCALE_Y(fYOffset));

                    fYOffset = 170;
                    //SET_MP_TEXT_COLOUR_STYLE(0, 0); // useless
                    DECLARE_ROW_Y(fYOffset, 0);
                    CFont::SetColor(CRGBA(COLOR_MUSTARD, 255));
                    CFont::PrintString(TheText.Get("GS_OK"), 0, MP_MENU_X(SCREEN_HALF_W), SCALE_Y(fYOffset));
                }
#endif
            }
            else // !m_bHasJoinedNetwork
            {
                MP_FONT_SCALE();

                // Starting a multiplayer game will cause all progress since the last save game to be lost.
                CFont::PrintString(TheText.Get("MP_WARN"), 0, MP_MENU_X(SCREEN_HALF_W), SCALE_Y(fYOffset));
                if (FrontEndMenuManager->IsMPPageActive())
                {
                    fYOffset = 120;
                    // Are you sure you want to start a multiplayer game?
                    CFont::PrintString(TheText.Get("FEMP_SQ"), 0, MP_MENU_X(SCREEN_HALF_W), SCALE_Y(fYOffset));
                    fYOffset = 170;
                    SET_MP_TEXT_COLOUR_STYLE(0, 0);
                    DECLARE_ROW_Y(fYOffset, 0);
#ifdef GTA_LIBERTY
                    if (m_nSelectedMenuOptionIndex == 0)
                    {
                        CFont::SetScale(SCALE_X(0.48576f), SCALE_Y(1.056f));
                        fYOffset = SCALE_X(168);
                    }
#endif
                    CFont::PrintString(TheText.Get("FEU_NO"), 0, MP_MENU_X(SCREEN_HALF_W), SCALE_Y(fYOffset));
                    fYOffset = 190;
                    SET_MP_TEXT_COLOUR_STYLE(1, 0);
                    DECLARE_ROW_Y(fYOffset, 1);
#ifdef GTA_LIBERTY
                    if (m_nSelectedMenuOptionIndex == 1)
                    {
                        CFont::SetScale(SCALE_X(0.48576f), SCALE_Y(1.056f));
                        fYOffset = SCALE_X(188);
                    }
#endif
                    CFont::PrintString(TheText.Get("FEU_YES"), 0, MP_MENU_X(SCREEN_HALF_W), SCALE_Y(fYOffset));
                }
            }
        }
        else // !wifi
        {
            // In order to play the multiplayer games you must first set the WLAN switch to ON
            CFont::PrintString(TheText.Get("NOWIFI1"), 0, MP_MENU_X(SCREEN_HALF_W), SCALE_Y(fYOffset));
        }
    }
    else // !psp_is_umd_readable()
    {
        // PSP -> Error reading the Grand Theft Auto: Vice City Stories UMD
        // PS2 -> Please wait...
        CFont::PrintString(TheText.Get("CDERROR"), 0, MP_MENU_X(SCREEN_HALF_W), SCALE_Y(fYOffset));
    }
    CFont::SetNewLineAdd(2);
    CFont::SetCentreOff();
    CFont::SetWrapx(SCALE_X((PSP_DEFAULT_SCREEN_WIDTH - 10)));
    CFont::SetRightJustifyWrap(10.0f);
}

void cLobby::DrawJoinGameScreen()
{
    DONT_OPTIMIZE();

#ifdef GTA_LIBERTY
    SetTextColorStyle(false);
#else
    SetTextColorStyle(false, 0);
#endif
    CFont::SetCentreOn();
    CFont::SetColor(CRGBA(MP_HEADER_TEXT_COLOUR, FrontEndMenuManager->FadeIn(255)));
    CFont::PrintString(TheText.Get("FEMP_GJ"), 0, MP_MENU_X(SCREEN_HALF_W), SCALE_Y(42)); // select game to join
    CFont::SetCentreOff();
#ifdef GTA_LIBERTY
    SetTextColorStyle(false);
#else
    SetTextColorStyle(false, 0);
#endif
    //int nPlayersHeaderX = 393;
    //int nPlayersCountOffset = 25;
    //if (TheLangSettings.language == 4) {
    //    nPlayersHeaderX = 385;
    //    nPlayersCountOffset = 38;
    //}
    CFont::PrintString(TheText.Get("FEMP_H"), 0, MP_MENU_X(13), SCALE_Y(MP_POS_Y_DEFAULT)); // host
#ifdef GTA_LIBERTY
    CFont::PrintString(TheText.Get("GS_TYPE"), 0, MP_MENU_X(103), SCALE_Y(MP_POS_Y_DEFAULT)); // scenario
#else
    CFont::PrintString(TheText.Get("GS_MODE"), 0, MP_MENU_X(103), SCALE_Y(MP_POS_Y_DEFAULT)); // game type
#endif
    CFont::PrintString(TheText.Get("GS_LOC2"), 0, MP_MENU_X(283), SCALE_Y(MP_POS_Y_DEFAULT)); // location
    CFont::PrintString(TheText.Get("FEMP_P"), 0, MP_MENU_X(393), SCALE_Y(MP_POS_Y_DEFAULT)); // number of players

    int nYOffset = 82;
    int nServerIndex = 0;
    int nOptIdx = m_nSelectedMenuOptionIndex;
    if (nOptIdx >= m_nNewHostIdx)
    {
        m_nSrvListMinIdx = 0;
        m_nSrvListMaxIdx = MAX_VISIBLE_ROOMS_ROWS;
    }
    else
    {
        if (m_nSrvListMaxIdx - 1 < nOptIdx)
        {
            m_nSrvListMaxIdx = nOptIdx + 1;
            m_nSrvListMinIdx = nOptIdx - (MAX_VISIBLE_ROOMS_ROWS - 1);
        }
        if (nOptIdx < m_nSrvListMinIdx)
        {
            m_nSrvListMinIdx = nOptIdx;
            m_nSrvListMaxIdx = m_nSrvListMinIdx + MAX_VISIBLE_ROOMS_ROWS;
        }
    }

    char sBuf[26];
    wchar sBuf_1[25];
#ifdef GTA_LIBERTY
    static char aLocations[][8] = {
        "IND_ZON", "COM_ZON", "SUB_ZON"
    };
#endif

    // from CStreaming::LoadPedbanksForMultiplayer()
    for (int32 i = 0; i < MP_NUM_MATCHING_GROUPS; i++)
    {
        tLobbyRemoteInfo* pData = TheAdhoc.GetMatchingInfo(i);
        if (pData == nil)
            continue;

        if (nServerIndex < m_nSrvListMinIdx || nServerIndex >= m_nSrvListMaxIdx)
        {
            nServerIndex++;
            continue;
        }

#ifdef GTA_LIBERTY
        ////SetTextColorStyle(nServerIndex == m_nSelectedMenuOptionIndex);
        CFont::SetCentreOff();
#endif
        base::string sPlayerName;
        TheAdhoc.GetPlayerNameFromMacAddr(sPlayerName, pData->m_HostPeerData.peerAddr.mac);
        if (sPlayerName.size() > 0)
            sprintf(sBuf, "%s", sPlayerName.c_str());
        else
            sprintf(sBuf, "%s", UnicodeToAscii(TheText.Get("NO_NAME")));
        AsciiToUnicode(sBuf, sBuf_1);
        SET_MP_TEXT_COLOUR_STYLE(nServerIndex, 0);
        int32 nPlayerCount = 0;
        for (int32 peerIdx = 0; peerIdx < MP_NUM_PEERS; peerIdx++)
        {
            tAdhocPeerData* pPeer = &pData->m_nPeersConnInfo[peerIdx];
            if (pPeer->macAddr.IsBroadcast())
                continue;
            nPlayerCount++;
        }

        nYOffset += 16;
        DECLARE_ROW_Y(nYOffset, nServerIndex);
        CFont::PrintString(sBuf_1, 0, MP_MENU_X(13), SCALE_Y(nYOffset)); // host player name

//#if defined(ADHOCCTL_USE_CUSTOM_IDENT) && !defined(GTA_PSP) // device type
//        bool bLocal = false;
//        SceNetAdhocctlPeerInfo* pProAdhocPeer = getPeerInfo(pData->m_HostPeerData.peerAddr.mac.GetBytesProAdhoc(), &bLocal);
//        bool bVanilaPeer = (pProAdhocPeer && ((pProAdhocPeer->flags & ADHOCCTL_CUSTOM_FLAG) == 0));
//
//        CSprite2d* spr = nil;
//        if (bVanilaPeer)
//            spr = &FrontEndMenuManager->m_aFrontEndSprites[MENUSPRITE_PSP];
//        else
//            spr = &FrontEndMenuManager->m_aFrontEndSprites[MENUSPRITE_PC];
//        spr->Draw(MP_MENU_X(103 - 32), SCALE_Y(nYOffset), 20 * SCREEN_ASPECT_RATIO, 20 * SCREEN_ASPECT_RATIO, CRGBA(255, 255, 255, 255));
//#endif

#ifdef GTA_LIBERTY
        strcpy(sBuf, "GS_G0"); // game type name (LIBETY CITY SURVIVOR,..)
        sBuf[4] = '1' + pData->m_GameType;
#else
        sprintf(sBuf, "GS_G%d", pData->m_GameType + 1); // game type name (VICE CITY SURVIVOR,..)
#endif
        CFont::PrintString(TheText.Get(sBuf), 0, MP_MENU_X(103), SCALE_Y(nYOffset));

#ifdef GTA_LIBERTY
        CFont::PrintString(TheText.Get(aLocations[pData->gameLocation]), 0, MP_MENU_X(283), SCALE_Y(nYOffset)); // location name
#else
        sprintf(sBuf, "MPLOC%d", pData->m_GameLocation); // location name
        CFont::PrintString(TheText.Get(sBuf), 0, MP_MENU_X(283), SCALE_Y(nYOffset));
#endif

        sprintf(sBuf, "%i", nPlayerCount);
        AsciiToUnicode(sBuf, sBuf_1);
        CFont::PrintString(sBuf_1, 0, MP_MENU_X(393), SCALE_Y(nYOffset)); // player count
        SetTextColorStyle(false, 0);
        nServerIndex++;
    }

    if (m_nNewHostIdx < nServerIndex)
        m_nSelectedMenuOptionIndex += nServerIndex - m_nNewHostIdx;
    m_nNewHostIdx = nServerIndex;
    if (nServerIndex == 0)
    {
        CFont::SetCentreOn();
        CFont::SetColor(CRGBA(COLOR_YELLOW, FrontEndMenuManager->FadeIn(255)));
        nYOffset += 16;
        CFont::PrintString(TheText.Get("NO_SVRS"), 0, MP_MENU_X(SCREEN_HALF_W), SCALE_Y(nYOffset)); // looking for hosts // yellow!!
        CFont::SetCentreOff();
#ifdef GTA_LIBERTY
        m_nSelectedMenuOptionIndex = 0;
#endif
    }

    SET_MP_TEXT_COLOUR_STYLE(nServerIndex, 0);
    CFont::SetCentreOn();
    nYOffset = 192;
    DECLARE_ROW_Y(nYOffset, nServerIndex);
    CFont::PrintString(TheText.Get("FEMP_HN"), 0, MP_MENU_X(SCREEN_HALF_W), SCALE_Y(nYOffset)); // host a new game
    CFont::SetCentreOff();
    // list up-down triangles
    TODO(); // SCALE
    static uint32 nLastTime = 0;
    uint32 nTimeDiff = CTimer::GetTimeInMillisecondsPauseMode() - nLastTime;
    if (m_nSrvListMinIdx > 0 && nTimeDiff > 600)
    {
        //CSprite2d::Draw2DPolygon(244, 94, 236, 94, 240, 86, 240, 86, CRGBA(MP_TEXT_COLOUR, FrontEndMenuManager->FadeIn(255)));
        CSprite2d::Draw2DPolygon(MP_MENU_X(244), SCALE_Y(94), MP_MENU_X(236), SCALE_Y(94), MP_MENU_X(240),
            SCALE_Y(86), MP_MENU_X(240), SCALE_Y(86), CRGBA(MP_TEXT_COLOUR, FrontEndMenuManager->FadeIn(255)));
        if (nTimeDiff > 1200)
            nLastTime = CTimer::GetTimeInMillisecondsPauseMode();
    }
    if (m_nSrvListMaxIdx < m_nNewHostIdx && nTimeDiff > 600)
    {
        //CSprite2d::Draw2DPolygon(244, 180, 236, 180, 240, 188, 240, 188, CRGBA(MP_TEXT_COLOUR, FrontEndMenuManager->FadeIn(255)));
        CSprite2d::Draw2DPolygon(MP_MENU_X(244), SCALE_Y(180), MP_MENU_X(236), SCALE_Y(180), MP_MENU_X(240),
            SCALE_Y(188), MP_MENU_X(240), SCALE_Y(188), CRGBA(MP_TEXT_COLOUR, FrontEndMenuManager->FadeIn(255)));
        if (nTimeDiff > 1200)
            nLastTime = CTimer::GetTimeInMillisecondsPauseMode();
    }
}

void cLobby::DrawHostGameScreen()
{
    DONT_OPTIMIZE();

    int32 nAlpha = FrontEndMenuManager->FadeIn(255);

    CFont::SetColor(CRGBA(MP_TEXT_COLOUR, nAlpha));
    CFont::SetDropShadowPosition(0);
    CFont::SetDropColor(CRGBA(0, 0, 0, nAlpha));
    CFont::SetFontStyle(FONT_STANDARD);
#ifdef GTA_LIBERTY
    CFont::SetScale(0.36431998f, 0.792f);
#else
    MP_FONT_SCALE();
#endif
    CFont::SetCentreOff();

#ifndef GTA_LIBERTY
    m_lobbyPed.Render();
#endif

    if (cAdhoc::Instance().bConnEvent) // ?
    {
        //CFont::SetColor(CRGBA(MP_TEXT_COLOUR, nAlpha));
#ifdef GTA_LIBERTY
        SetTextColorStyle(false);
#else
        SetTextColorStyle(false, 0);
#endif
        CFont::PrintString(MP_MENU_X(PLAYER_LIST_START_X), SCALE_Y(MP_POS_Y_DEFAULT), TheText.Get("SVRMBRS")); // player list
        int32 nMaxTeams = TheMPGame.eTDMStyle == eTDMStyle::FFA ? 1 : 2;
        drawPlayerList(PLAYER_LIST_START_X, nMaxTeams);
    }


    // 74 [0] // MGE_GAME_TYPE
    DECLARE_ROW(MGE_GAME_TYPE);
    SET_MP_TEXT_COLOUR_STYLE(MGE_GAME_TYPE, 0); // 0
#ifdef GTA_LIBERTY
    CFont::SetRightJustifyOn();
    CFont::PrintString(MP_MENU_X(OPTION_LABEL_START_X), SCALE_Y(SCREEN_MP_MENU_ROW_Y(MGE_GAME_TYPE)), TheText.Get("GS_TYPE")); // scenario [LABEL]
    CFont::SetRightJustifyOff();
#else
    CFont::PrintString(MP_MENU_X(OPTION_LABEL_START_X), SCALE_Y(SCREEN_MP_MENU_ROW_Y(MGE_GAME_TYPE)), TheText.Get("GS_MODE")); // game type [LABEL]
#endif
    eGameType nGameType = TheMPGame.GetGameType();
    char sKeyBuf[28];
    sprintf(sKeyBuf, "GS_G%d", static_cast<int>(nGameType) + 1); // option value game type [VALUE] https://prnt.sc/Ij6XfBV5258V
#ifdef MULTIGAME_SCM
    wchar sKeyBufW[56];
    if (nGameType == eGameType::SCM)
    {
        sprintf(sKeyBuf, "SCRIPT MODE (ReVCS)");
        AsciiToUnicode(sKeyBuf, sKeyBufW);
        CFont::PrintString(MP_MENU_X(OPTION_VALUE_START_X), SCALE_Y(SCREEN_MP_MENU_ROW_Y(MGE_GAME_TYPE)), sKeyBufW);
    }
    else
        CFont::PrintString(MP_MENU_X(OPTION_VALUE_START_X), SCALE_Y(SCREEN_MP_MENU_ROW_Y(MGE_GAME_TYPE)), TheText.Get(sKeyBuf));
#else
    CFont::PrintString(MP_MENU_X(OPTION_VALUE_START_X), SCALE_Y(SCREEN_MP_MENU_ROW_Y(MGE_GAME_TYPE)), TheText.Get(sKeyBuf));
#endif

    sprintf(sKeyBuf, "GS_D%d", static_cast<int>(nGameType) + 1); // game header (0 RE-LIVE) https://prnt.sc/EtTHEkIyeJJU
#ifdef GTA_LIBERTY
    SetTextColorStyle(false);
#else
    SetTextColorStyle(false, 0);
#endif
    CFont::SetCentreOn();
    CFont::SetColor(CRGBA(MP_HEADER_TEXT_COLOUR, nAlpha));
    CFont::SetWrapx(SCALE_X(460));
    CFont::SetCentreSize(SCALE_X(460));
    wchar* sGameDesc = TheText.Get(sKeyBuf);
#ifdef MULTIGAME_SCM
    if (nGameType == eGameType::SCM)
        sGameDesc = sKeyBufW;
#endif
#ifdef GTA_LIBERTY
    int nHeaderPosY = (CFont::GetStringWidth(sGameDesc, true) <= (460) ? 36 : 30);
#else
    int nHeaderPosY = 48;
    //CRect(0.0f, 90.0f, 480.0f, 20.0f) // ps2 huh? CRect(0.0f, 20.0f, 480.0f, 90.0f)
#endif
    CFont::PrintString(MP_MENU_X(SCREEN_HALF_W), SCALE_Y(nHeaderPosY), sGameDesc);
    CFont::SetCentreOff();


    // 87 [1] // MGE_GAME_LOCATION
    DECLARE_ROW(MGE_GAME_LOCATION);
    SET_MP_TEXT_COLOUR_STYLE(MGE_GAME_LOCATION, 0); // 1
#ifdef GTA_LIBERTY
    CFont::SetRightJustifyOn();
    CFont::PrintString(MP_MENU_X(OPTION_LABEL_START_X), SCALE_Y(SCREEN_MP_MENU_ROW_Y(MGE_GAME_LOCATION)), TheText.Get("GS_LOC")); // location
    CFont::SetRightJustifyOff();
#else
    CFont::PrintString(MP_MENU_X(OPTION_LABEL_START_X), SCALE_Y(SCREEN_MP_MENU_ROW_Y(MGE_GAME_LOCATION)), TheText.Get("GS_LOC")); // location
#endif
    eGameLocation nGameLoc = TheMPGame.GetGameLocation();
    char sLocKey[28];
    wchar sTextBuf[256];
#ifdef GTA_LIBERTY
    if (nGameLoc == eGameLocation::IND_ZON)
        strcpy(sLocKey, "IND_ZON");
    else if (nGameLoc == eGameLocation::COM_ZON)
        strcpy(sLocKey, "COM_ZON");
    else if (nGameLoc == eGameLocation::SUB_ZON)
        strcpy(sLocKey, "SUB_ZON");
#else
    sprintf(sLocKey, "MPLOC%d", nGameLoc);
#endif
#ifdef MULTIGAME_SCM
    if(nGameType == eGameType::SCM)
        SetInactiveTextColorStyle(m_nSelectedMenuOptionIndex == MGE_GAME_LOCATION); // no choose location at scm mode (visual)
#endif
    UnicodeMakeUpperCase(sTextBuf, TheText.Get(sLocKey));
    CFont::PrintString(MP_MENU_X(OPTION_VALUE_START_X), SCALE_Y(SCREEN_MP_MENU_ROW_Y(MGE_GAME_LOCATION)), sTextBuf);


#ifdef GTA_LIBERTY
    // 100 [2] // MGE_PLAY_CUTSCENE
    DECLARE_ROW(MGE_PLAY_CUTSCENE);
    SET_MP_TEXT_COLOUR_STYLE(MGE_PLAY_CUTSCENE, 0); // 2
#ifdef GTA_LIBERTY
    CFont::SetRightJustifyOn();
    CFont::PrintString(MP_MENU_X(OPTION_LABEL_START_X), SCALE_Y(SCREEN_MP_MENU_ROW_Y(MGE_PLAY_CUTSCENE)), TheText.Get("GS_SCUT")); // play cutscene
    CFont::SetRightJustifyOff();
#else
    CFont::PrintString(MP_MENU_X(OPTION_LABEL_START_X), SCALE_Y(SCREEN_MP_MENU_ROW_Y(MGE_PLAY_CUTSCENE)), TheText.Get("GS_SCUT")); // play cutscene
#endif
    eGameCutscenePlayback nCutscenePlaybackOpt = TheMPGame.m_playIntroCutscene;
    if (nCutscenePlaybackOpt == eGameCutscenePlayback::DONT_PLAY)
        UnicodeMakeUpperCase(sTextBuf, TheText.Get("FEM_NO"));
    else if (nCutscenePlaybackOpt == eGameCutscenePlayback::PLAY_ONCE)
        UnicodeMakeUpperCase(sTextBuf, TheText.Get("MPAUTO"));
    else if (nCutscenePlaybackOpt == eGameCutscenePlayback::ALWAYS_PLAY)
        UnicodeMakeUpperCase(sTextBuf, TheText.Get("FEM_YES"));
    CFont::PrintString(MP_MENU_X(OPTION_VALUE_START_X), SCALE_Y(SCREEN_MP_MENU_ROW_Y(MGE_PLAY_CUTSCENE)), sTextBuf);
#endif

    int nNextRowPos = MGE_AFTER_COMMON; // 3, 2

    char sBufText[11];
    wchar sWideBufText[11];
    switch (nGameType)
    {
        case eGameType::DEATHMATCH:
        {
            // 100 [3,2] // MGE_DEATHMATCH_GAME_STYLE
            DECLARE_ROW(MGE_DEATHMATCH_GAME_STYLE);
            SET_MP_TEXT_COLOUR_STYLE(MGE_DEATHMATCH_GAME_STYLE, 0);
#ifdef GTA_LIBERTY
            CFont::SetRightJustifyOn();
            CFont::PrintString(SCALE_X(OPTION_LABEL_START_X), SCALE_Y(SCREEN_MP_MENU_ROW_Y(MGE_DEATHMATCH_GAME_STYLE)), TheText.Get("GS_STYL")); // game style
            CFont::SetRightJustifyOff();
#else
            CFont::PrintString(MP_MENU_X(OPTION_LABEL_START_X), SCALE_Y(SCREEN_MP_MENU_ROW_Y(MGE_DEATHMATCH_GAME_STYLE)), TheText.Get("GS_STYL")); // game style
#endif
            CFont::PrintString(MP_MENU_X(OPTION_VALUE_START_X), SCALE_Y(SCREEN_MP_MENU_ROW_Y(MGE_DEATHMATCH_GAME_STYLE)), TheText.Get(GANG_MODE ? "GS_TEMS" : "GS_FFAL"));

            // 113 [4,3] // MGE_DEATHMATCH_KILL_LIMIT
            DECLARE_ROW(MGE_DEATHMATCH_KILL_LIMIT);
            SET_MP_TEXT_COLOUR_STYLE(MGE_DEATHMATCH_KILL_LIMIT, 0);
#ifdef GTA_LIBERTY
            CFont::SetRightJustifyOn();
            CFont::PrintString(MP_MENU_X(OPTION_LABEL_START_X), SCALE_Y(SCREEN_MP_MENU_ROW_Y(MGE_DEATHMATCH_KILL_LIMIT)), TheText.Get("GS_KLMT")); // kill limit
            CFont::SetRightJustifyOff();
#else
            CFont::PrintString(MP_MENU_X(OPTION_LABEL_START_X), SCALE_Y(SCREEN_MP_MENU_ROW_Y(MGE_DEATHMATCH_KILL_LIMIT)), TheText.Get("GS_KLMT")); // kill limit
#endif
            int nScoreLimit = TheMPGame.GetScoreLimit();
            if (nScoreLimit <= 0)
                CFont::PrintString(MP_MENU_X(OPTION_VALUE_START_X), SCALE_Y(SCREEN_MP_MENU_ROW_Y(MGE_DEATHMATCH_KILL_LIMIT)), TheText.Get("GS_ULMT")); // unlimited
            else
            {
                sprintf(sBufText, "%d", nScoreLimit);
                AsciiToUnicode(sBufText, sWideBufText);
                CFont::PrintString(MP_MENU_X(OPTION_VALUE_START_X), SCALE_Y(SCREEN_MP_MENU_ROW_Y(MGE_DEATHMATCH_KILL_LIMIT)), sWideBufText); // unlimited
            }

            // 126 [5,4] // MGE_DEATHMATCH_TIME_LIMIT
            DECLARE_ROW(MGE_DEATHMATCH_TIME_LIMIT);
            SET_MP_TEXT_COLOUR_STYLE(MGE_DEATHMATCH_TIME_LIMIT, 0);
#ifdef GTA_LIBERTY
            CFont::SetRightJustifyOn();
            CFont::PrintString(SCALE_X(OPTION_LABEL_START_X), SCALE_Y(SCREEN_MP_MENU_ROW_Y(MGE_DEATHMATCH_TIME_LIMIT)), TheText.Get("GS_TLMT")); // time limit
            CFont::SetRightJustifyOff();
#else
            CFont::PrintString(MP_MENU_X(OPTION_LABEL_START_X), SCALE_Y(SCREEN_MP_MENU_ROW_Y(MGE_DEATHMATCH_TIME_LIMIT)), TheText.Get("GS_TLMT")); // time limit
#endif
            int nTimeLimit = TheMPGame.GetTimeLimit();
            if (nTimeLimit <= 0)
                CFont::PrintString(MP_MENU_X(OPTION_VALUE_START_X), SCALE_Y(SCREEN_MP_MENU_ROW_Y(MGE_DEATHMATCH_TIME_LIMIT)), TheText.Get("GS_ULMT"));
            else
            {
                wchar* sText = TheText.Get(nTimeLimit == 1 ? "GS_MIN" : "GS_MINS");
                sprintf(sBufText, "%d %s", nTimeLimit, UnicodeToAscii(sText));
                AsciiToUnicode(sBufText, sWideBufText);
                CFont::PrintString(MP_MENU_X(OPTION_VALUE_START_X), SCALE_Y(SCREEN_MP_MENU_ROW_Y(MGE_DEATHMATCH_TIME_LIMIT)), sWideBufText);
            }

#ifdef GTA_LIBERTY
            // 139 [6,5] // MGE_DEATHMATCH_POWERUPS
            DECLARE_ROW(MGE_DEATHMATCH_POWERUPS);
            SET_MP_TEXT_COLOUR_STYLE(MGE_DEATHMATCH_POWERUPS, 0);
#ifdef GTA_LIBERTY
            CFont::SetRightJustifyOn();
            CFont::PrintString(MP_MENU_X(OPTION_LABEL_START_X), SCALE_Y(SCREEN_MP_MENU_ROW_Y(MGE_DEATHMATCH_POWERUPS)), TheText.Get("GS_PRUP")); // powerup
            CFont::SetRightJustifyOff();
#else
            CFont::PrintString(MP_MENU_X(OPTION_LABEL_START_X), SCALE_Y(SCREEN_MP_MENU_ROW_Y(MGE_DEATHMATCH_POWERUPS)), TheText.Get("GS_PRUP")); // powerup
#endif
            CFont::PrintString(MP_MENU_X(OPTION_VALUE_START_X), SCALE_Y(SCREEN_MP_MENU_ROW_Y(MGE_DEATHMATCH_POWERUPS)), TheText.Get(TheMPGame.m_bPowerUpOn ? "FEM_ON" : "FEM_OFF"));
#endif
            nNextRowPos = MGE_MAX_DEATHMATCH_DEFAULT; // 7,5
            if (GANG_MODE)
            {
                // 139 [7,5] // MGE_DEATHMATCH_SEL_GANG_1
                DECLARE_ROW(MGE_DEATHMATCH_SEL_GANG_1);
                SET_MP_TEXT_COLOUR_STYLE(MGE_DEATHMATCH_SEL_GANG_1, 0);
#ifdef GTA_LIBERTY
                CFont::SetRightJustifyOn();
                CFont::PrintString(SCALE_X(OPTION_LABEL_START_X), SCALE_Y(SCREEN_MP_MENU_ROW_Y(MGE_DEATHMATCH_SEL_GANG_1)), TheText.Get("MP_GNG1")); // selected gang 1
                CFont::SetRightJustifyOff();
#else
                CFont::PrintString(MP_MENU_X(OPTION_LABEL_START_X), SCALE_Y(SCREEN_MP_MENU_ROW_Y(MGE_DEATHMATCH_SEL_GANG_1)), TheText.Get("MP_GNG1")); // selected gang 1
#endif
                CFont::PrintString(MP_MENU_X(OPTION_VALUE_START_X), SCALE_Y(SCREEN_MP_MENU_ROW_Y(MGE_DEATHMATCH_SEL_GANG_1)), TheText.Get(gMPGangDefs[TheMPGame.GetTeam1GangID()].name));

                // 152 [8,6] // MGE_DEATHMATCH_SEL_GANG_2
                DECLARE_ROW(MGE_DEATHMATCH_SEL_GANG_2);
                SET_MP_TEXT_COLOUR_STYLE(MGE_DEATHMATCH_SEL_GANG_2, 0);
#ifdef GTA_LIBERTY
                CFont::SetRightJustifyOn();
                CFont::PrintString(SCALE_X(OPTION_LABEL_START_X), SCALE_Y(SCREEN_MP_MENU_ROW_Y(MGE_DEATHMATCH_SEL_GANG_2)), TheText.Get("MP_GNG2")); // selected gang 2
                CFont::SetRightJustifyOff();
#else
                CFont::PrintString(MP_MENU_X(OPTION_LABEL_START_X), SCALE_Y(SCREEN_MP_MENU_ROW_Y(MGE_DEATHMATCH_SEL_GANG_2)), TheText.Get("MP_GNG2")); // selected gang 2
#endif
                CFont::PrintString(MP_MENU_X(OPTION_VALUE_START_X), SCALE_Y(SCREEN_MP_MENU_ROW_Y(MGE_DEATHMATCH_SEL_GANG_2)), TheText.Get(gMPGangDefs[TheMPGame.GetTeam2GangID()].name));
                nNextRowPos = MGE_MAX_DEATHMATCH; // 9,7
            }
            break;
        }
        case eGameType::MULTIRACE:
        {
#ifndef GTA_LIBERTY
            // 100 [2] // MGE_MULTIRACE_GAME_STYLE
            DECLARE_ROW(MGE_MULTIRACE_GAME_STYLE);
            SET_MP_TEXT_COLOUR_STYLE(MGE_MULTIRACE_GAME_STYLE, 0);
            CFont::PrintString(MP_MENU_X(OPTION_LABEL_START_X), SCALE_Y(SCREEN_MP_MENU_ROW_Y(MGE_MULTIRACE_GAME_STYLE)), TheText.Get("GS_STYL")); // game style
            int32 nCTFScore = TheMPGame.GetCTFScoreLimit();
            if (nCTFScore == static_cast<int32>(eRaceStyle::RACE_QUADATHLON)) // 1
                sprintf(sBufText, "MRTYPEI");
            else if (nCTFScore == static_cast<int32>(eRaceStyle::RACE_JETSKI)) // 2
                sprintf(sBufText, "MRTYPEJ");
            else // (nCTFScore == static_cast<int32>(eRaceStyle::RACE_NORMAL)) // 0
                sprintf(sBufText, "MRTYPEN");
            CFont::PrintString(MP_MENU_X(OPTION_VALUE_START_X), SCALE_Y(SCREEN_MP_MENU_ROW_Y(MGE_MULTIRACE_GAME_STYLE)), TheText.Get(sBufText));
#endif


            // 113 [3] // MGE_MULTIRACE_RACE
            DECLARE_ROW(MGE_MULTIRACE_RACE);
            SET_MP_TEXT_COLOUR_STYLE(MGE_MULTIRACE_RACE, 0);
#ifdef GTA_LIBERTY
            CFont::SetRightJustifyOn();
            CFont::PrintString(MP_MENU_X(OPTION_LABEL_START_X), SCALE_Y(SCREEN_MP_MENU_ROW_Y(MGE_MULTIRACE_RACE)), TheText.Get("GS_RACE")); // race
            CFont::SetRightJustifyOff();
#else
            CFont::PrintString(MP_MENU_X(OPTION_LABEL_START_X), SCALE_Y(SCREEN_MP_MENU_ROW_Y(MGE_MULTIRACE_RACE)), TheText.Get("GS_RACE")); // race
#endif
            sprintf(sBufText, "TR_0%d", TheMPGame.m_nScenarioOrRaceTrackID);
            CFont::PrintString(MP_MENU_X(OPTION_VALUE_START_X), SCALE_Y(SCREEN_MP_MENU_ROW_Y(MGE_MULTIRACE_RACE)), TheText.Get(sBufText));


            // 126 [4] (VCS: can be inactive) // MGE_MULTIRACE_LAPS
            DECLARE_ROW(MGE_MULTIRACE_LAPS);
            SET_MP_TEXT_COLOUR_STYLE(MGE_MULTIRACE_LAPS, 0);
#ifdef GTA_LIBERTY
            CFont::SetRightJustifyOn();
            CFont::PrintString(SCALE_X(OPTION_LABEL_START_X), SCALE_Y(SCREEN_MP_MENU_ROW_Y(MGE_MULTIRACE_LAPS)), TheText.Get("GS_LAPS")); // laps
            CFont::SetRightJustifyOff();
#else
            if (nCTFScore == static_cast<int32>(eRaceStyle::RACE_QUADATHLON)) // 1
                SetInactiveTextColorStyle(m_nSelectedMenuOptionIndex == MGE_MULTIRACE_LAPS);
#endif
            int32 nScoreLimit = TheMPGame.GetScoreLimit();
            sprintf(sBufText, "%d", nScoreLimit);
            AsciiToUnicode(sBufText, sWideBufText);
            CFont::PrintString(MP_MENU_X(OPTION_VALUE_START_X), SCALE_Y(SCREEN_MP_MENU_ROW_Y(MGE_MULTIRACE_LAPS)), sWideBufText);
#ifndef GTA_LIBERTY
            CFont::PrintString(MP_MENU_X(OPTION_LABEL_START_X), SCALE_Y(SCREEN_MP_MENU_ROW_Y(MGE_MULTIRACE_LAPS)), TheText.Get("GS_LAPS")); // laps
#endif


            // 139 [5] (VCS: can be inactive) // MGE_MULTIRACE_VEHICLE
            DECLARE_ROW(MGE_MULTIRACE_VEHICLE);
            SET_MP_TEXT_COLOUR_STYLE(MGE_MULTIRACE_VEHICLE, 0);
#ifdef GTA_LIBERTY
            CFont::SetRightJustifyOn();
            CFont::PrintString(MP_MENU_X(OPTION_LABEL_START_X), SCALE_Y(SCREEN_MP_MENU_ROW_Y(MGE_MULTIRACE_VEHICLE)), TheText.Get("GS_SCAR")); // vehicle
            CFont::SetRightJustifyOff();
#else
            if (nCTFScore == static_cast<int32>(eRaceStyle::RACE_QUADATHLON)) // 1
                SetInactiveTextColorStyle(m_nSelectedMenuOptionIndex == MGE_MULTIRACE_VEHICLE);
            CFont::PrintString(MP_MENU_X(OPTION_LABEL_START_X), SCALE_Y(SCREEN_MP_MENU_ROW_Y(MGE_MULTIRACE_VEHICLE)), TheText.Get("GS_SCAR")); // vehicle
#endif
            CVehicleModelInfo* pVehInfo = (CVehicleModelInfo*)CModelInfo::GetModelInfo(TheMPGame.m_nRaceCarID);
#ifdef GTA_LIBERTY
            UnicodeMakeUpperCase(sTextBuf, TheText.Get(pVehInfo->m_gameName));
#else
            if (nCTFScore == static_cast<int32>(eRaceStyle::RACE_QUADATHLON)) // 1
                sprintf(sBufText, "MPVARI");
            else if (nCTFScore == static_cast<int32>(eRaceStyle::RACE_JETSKI)) // 2
                sprintf(sBufText, "MRTYPEJ");
            else // (nCTFScore == static_cast<int32>(eRaceStyle::RACE_NORMAL)) // 0
                sprintf(sBufText, pVehInfo->m_gameName);
            UnicodeMakeUpperCase(sTextBuf, TheText.Get(sBufText));
#endif
            CFont::PrintString(MP_MENU_X(OPTION_VALUE_START_X), SCALE_Y(SCREEN_MP_MENU_ROW_Y(MGE_MULTIRACE_VEHICLE)), sTextBuf);


#ifdef GTA_LIBERTY
            // 152 [6] // MGE_MULTIRACE_POWERUPS
            DECLARE_ROW(MGE_MULTIRACE_POWERUPS);
            SET_MP_TEXT_COLOUR_STYLE(MGE_MULTIRACE_POWERUPS, 0);
#ifdef GTA_LIBERTY
            CFont::SetRightJustifyOn();
            CFont::PrintString(MP_MENU_X(OPTION_LABEL_START_X), SCALE_Y(SCREEN_MP_MENU_ROW_Y(MGE_MULTIRACE_POWERUPS)), TheText.Get("GS_PRUP")); // powerups
            CFont::SetRightJustifyOff();
#else
            CFont::PrintString(MP_MENU_X(OPTION_LABEL_START_X), SCALE_Y(SCREEN_MP_MENU_ROW_Y(MGE_MULTIRACE_POWERUPS)), TheText.Get("GS_PRUP")); // powerups
#endif
            CFont::PrintString(MP_MENU_X(OPTION_VALUE_START_X), SCALE_Y(SCREEN_MP_MENU_ROW_Y(MGE_MULTIRACE_POWERUPS)), TheText.Get(TheMPGame.m_bRacePowerUpOn ? "FEM_ON" : "FEM_OFF"));
#endif
            nNextRowPos = MGE_MAX_MULTIRACE; //7,6
            break;
        }
        case eGameType::DEFENDTHEBASE:
        {
            // 100 [3,2] // MGE_DEFENDTHEBASE_POWERUPS/MGE_DEFENDTHEBASE_SCENARIO
#ifdef GTA_LIBERTY
            DECLARE_ROW(MGE_DEFENDTHEBASE_POWERUPS);
            SET_MP_TEXT_COLOUR_STYLE(MGE_DEFENDTHEBASE_POWERUPS, 0);
            CFont::SetRightJustifyOn();
            CFont::PrintString(MP_MENU_X(OPTION_LABEL_START_X), SCALE_Y(SCREEN_MP_MENU_ROW_Y(MGE_DEFENDTHEBASE_POWERUPS)), TheText.Get("GS_PRUP")); // POWERUPS
            CFont::SetRightJustifyOff();
            CFont::PrintString(MP_MENU_X(OPTION_VALUE_START_X), SCALE_Y(SCREEN_MP_MENU_ROW_Y(MGE_DEFENDTHEBASE_POWERUPS)), TheText.Get(TheMPGame.m_bPowerUpOn ? "FEM_ON" : "FEM_OFF"));
#else
            DECLARE_ROW(MGE_DEFENDTHEBASE_SCENARIO);
            SET_MP_TEXT_COLOUR_STYLE(MGE_DEFENDTHEBASE_SCENARIO, 0);
            sprintf(sBufText, "%d", TheMPGame.m_nScenarioOrRaceTrackID + 1);
            AsciiToUnicode(sBufText, sWideBufText);
            CFont::PrintString(MP_MENU_X(OPTION_LABEL_START_X), SCALE_Y(SCREEN_MP_MENU_ROW_Y(MGE_DEFENDTHEBASE_SCENARIO)), TheText.Get("GS_TYPE")); // SCENARIO
            CFont::PrintString(MP_MENU_X(OPTION_VALUE_START_X), SCALE_Y(SCREEN_MP_MENU_ROW_Y(MGE_DEFENDTHEBASE_SCENARIO)), sWideBufText);
#endif
            nNextRowPos = MGE_MAX_DEFENDTHEBASE;
            break;
        }
        case eGameType::CTF:
        {
#ifndef GTA_LIBERTY
            // 100 [3,2] // MGE_CTF_SCORE_SCENARIO
            DECLARE_ROW(MGE_CTF_SCORE_SCENARIO);
            SET_MP_TEXT_COLOUR_STYLE(MGE_CTF_SCORE_SCENARIO, 0);
            sprintf(sBufText, "%d", TheMPGame.m_nScenarioOrRaceTrackID + 1);
            AsciiToUnicode(sBufText, sWideBufText);
            CFont::PrintString(MP_MENU_X(OPTION_LABEL_START_X), SCALE_Y(SCREEN_MP_MENU_ROW_Y(MGE_CTF_SCORE_SCENARIO)), TheText.Get("GS_TYPE")); // SCENARIO
            CFont::PrintString(MP_MENU_X(OPTION_VALUE_START_X), SCALE_Y(SCREEN_MP_MENU_ROW_Y(MGE_CTF_SCORE_SCENARIO)), sWideBufText);
#endif

            // 113 [3] // MGE_CTF_SCORE_LIMIT
            DECLARE_ROW(MGE_CTF_SCORE_LIMIT);
            SET_MP_TEXT_COLOUR_STYLE(MGE_CTF_SCORE_LIMIT, 0);
#ifdef GTA_LIBERTY
            CFont::SetRightJustifyOn();
            CFont::PrintString(MP_MENU_X(OPTION_LABEL_START_X), SCALE_Y(SCREEN_MP_MENU_ROW_Y(MGE_CTF_SCORE_LIMIT)), TheText.Get("GS_SLMT")); // score limit
            CFont::SetRightJustifyOff();
#else
            CFont::PrintString(MP_MENU_X(OPTION_LABEL_START_X), SCALE_Y(SCREEN_MP_MENU_ROW_Y(MGE_CTF_SCORE_LIMIT)), TheText.Get("GS_SLMT")); // score limit
#endif
            int nScoreLimit = TheMPGame.GetCTFScoreLimit();
            if (nScoreLimit <= 0)
                CFont::PrintString(MP_MENU_X(OPTION_VALUE_START_X), SCALE_Y(SCREEN_MP_MENU_ROW_Y(MGE_CTF_SCORE_LIMIT)), TheText.Get("GS_ULMT")); // unlimited
            else
            {
                sprintf(sBufText, "%d", nScoreLimit);
                AsciiToUnicode(sBufText, sWideBufText);
                CFont::PrintString(MP_MENU_X(OPTION_VALUE_START_X), SCALE_Y(SCREEN_MP_MENU_ROW_Y(MGE_CTF_SCORE_LIMIT)), sWideBufText);
            }

#ifdef GTA_LIBERTY
            // 126 [4] // MGE_CTF_TIME_LIMIT
            DECLARE_ROW(MGE_CTF_TIME_LIMIT);
            SET_MP_TEXT_COLOUR_STYLE(MGE_CTF_TIME_LIMIT, 0);
#ifdef GTA_LIBERTY
            CFont::SetRightJustifyOn();
            CFont::PrintString(MP_MENU_X(OPTION_LABEL_START_X), SCALE_Y(SCREEN_MP_MENU_ROW_Y(MGE_CTF_TIME_LIMIT)), TheText.Get("GS_TLMT")); // time limit
            CFont::SetRightJustifyOff();
#else
            CFont::PrintString(MP_MENU_X(OPTION_LABEL_START_X), SCALE_Y(SCREEN_MP_MENU_ROW_Y(MGE_CTF_TIME_LIMIT)), TheText.Get("GS_TLMT")); // time limit
#endif
            int nTimeLimit = TheMPGame.GetTimeLimit();
            if (nTimeLimit <= 0)
                CFont::PrintString(MP_MENU_X(OPTION_VALUE_START_X), SCALE_Y(SCREEN_MP_MENU_ROW_Y(MGE_CTF_TIME_LIMIT)), TheText.Get("GS_ULMT")); // unlimited
            else
            {
                wchar* sText = TheText.Get(nTimeLimit == 1 ? "GS_MIN" : "GS_MINS");
                sprintf(sBufText, "%d %s", nTimeLimit, UnicodeToAscii(sText));
                AsciiToUnicode(sBufText, sWideBufText);
                CFont::PrintString(MP_MENU_X(OPTION_VALUE_START_X), SCALE_Y(SCREEN_MP_MENU_ROW_Y(MGE_CTF_TIME_LIMIT)), sWideBufText);
            }

            // 139 [5] // MGE_CTF_POWERUPS
            DECLARE_ROW(MGE_CTF_POWERUPS);
            SET_MP_TEXT_COLOUR_STYLE(MGE_CTF_POWERUPS, 0);
            CFont::SetRightJustifyOn();
            CFont::PrintString(MP_MENU_X(OPTION_LABEL_START_X), SCALE_Y(SCREEN_MP_MENU_ROW_Y(MGE_CTF_POWERUPS)), TheText.Get("GS_PRUP")); // powerups
            CFont::SetRightJustifyOff();
            CFont::PrintString(MP_MENU_X(OPTION_VALUE_START_X), SCALE_Y(SCREEN_MP_MENU_ROW_Y(MGE_CTF_POWERUPS)), TheText.Get(TheMPGame.m_bPowerUpOn ? "FEM_ON" : "FEM_OFF"));
#endif
            nNextRowPos = MGE_MAX_CTF; //6,4
            break;
        }
        case eGameType::TANK:
        {
            // 100 [2] // MGE_TANK_TANK_TIME
            DECLARE_ROW(MGE_TANK_TANK_TIME);
            SET_MP_TEXT_COLOUR_STYLE(MGE_TANK_TANK_TIME, 0);
#ifdef GTA_LIBERTY
            CFont::SetRightJustifyOn();
            CFont::PrintString(MP_MENU_X(OPTION_LABEL_START_X), SCALE_Y(SCREEN_MP_MENU_ROW_Y(MGE_TANK_TANK_TIME)), TheText.Get("GS_TTGT")); // tank time
            CFont::SetRightJustifyOff();
#else
            CFont::PrintString(MP_MENU_X(OPTION_LABEL_START_X), SCALE_Y(SCREEN_MP_MENU_ROW_Y(MGE_TANK_TANK_TIME)), TheText.Get("GS_TTGT")); // tank time
#endif
            int nTimeLimit = TheMPGame.GetScoreLimit();
            wchar* sText = TheText.Get(nTimeLimit == 1 ? "GS_MIN" : "GS_MINS");
            sprintf(sBufText, "%d %s", nTimeLimit, UnicodeToAscii(sText));
            AsciiToUnicode(sBufText, sWideBufText);
            CFont::PrintString(MP_MENU_X(OPTION_VALUE_START_X), SCALE_Y(SCREEN_MP_MENU_ROW_Y(MGE_TANK_TANK_TIME)), sWideBufText);
            nNextRowPos = MGE_MAX_TANK; //4,3
            break;
        }
        case eGameType::HITPARADE:
        {
            // 100 [2] // MGE_MAX_HITPARADE
#ifdef GTA_LIBERTY
            DECLARE_ROW(MGE_HITPARADE_POWERUPS);
            SET_MP_TEXT_COLOUR_STYLE(MGE_HITPARADE_POWERUPS, 0);
            CFont::SetRightJustifyOn();
            CFont::PrintString(MP_MENU_X(OPTION_LABEL_START_X), SCALE_Y(SCREEN_MP_MENU_ROW_Y(MGE_HITPARADE_POWERUPS)), TheText.Get("GS_PRUP"));
            CFont::SetRightJustifyOff();
            CFont::PrintString(MP_MENU_X(OPTION_VALUE_START_X), SCALE_Y(SCREEN_MP_MENU_ROW_Y(MGE_HITPARADE_POWERUPS)), TheText.Get(TheMPGame.m_bPowerUpOn ? "FEM_ON" : "FEM_OFF"));
#endif
            nNextRowPos = MGE_MAX_HITPARADE; //4,2 miami useless
            break;
        }
        case eGameType::SIXTYSECONDS:
        {
            // 100 [2] // MGE_SIXTYSECONDS_CASH_TARGET
            DECLARE_ROW(MGE_SIXTYSECONDS_CASH_TARGET);
            SET_MP_TEXT_COLOUR_STYLE(MGE_SIXTYSECONDS_CASH_TARGET, 0);
#ifdef GTA_LIBERTY
            CFont::SetRightJustifyOn();
            CFont::PrintString(MP_MENU_X(OPTION_LABEL_START_X), SCALE_Y(SCREEN_MP_MENU_ROW_Y(MGE_SIXTYSECONDS_CASH_TARGET)), TheText.Get("MPSSCT")); // cash target
            CFont::SetRightJustifyOff();
#else
            CFont::PrintString(MP_MENU_X(OPTION_LABEL_START_X), SCALE_Y(SCREEN_MP_MENU_ROW_Y(MGE_SIXTYSECONDS_CASH_TARGET)), TheText.Get("MPSSCT")); // cash target
#endif
            int nCashTarget = TheMPGame.GetCashTarget();
            sprintf(sBufText, "$%d", nCashTarget);
            AsciiToUnicode(sBufText, sWideBufText);
            CFont::PrintString(MP_MENU_X(OPTION_VALUE_START_X), SCALE_Y(SCREEN_MP_MENU_ROW_Y(MGE_SIXTYSECONDS_CASH_TARGET)), sWideBufText);
            nNextRowPos = MGE_MAX_SIXTYSECONDS; //4,3
            break;
        }
#ifndef GTA_LIBERTY
        case eGameType::HUNTERATTACK:
        {
            // 100 [2] // MGE_HUNTERATTACK_KILL_LIMIT
            DECLARE_ROW(MGE_HUNTERATTACK_KILL_LIMIT);
            SET_MP_TEXT_COLOUR_STYLE(MGE_HUNTERATTACK_KILL_LIMIT, 0);
#ifdef GTA_LIBERTY
            CFont::SetRightJustifyOn();
            CFont::PrintString(SCALE_X(OPTION_LABEL_START_X), SCALE_Y(SCREEN_MP_MENU_ROW_Y(MGE_HUNTERATTACK_KILL_LIMIT)), TheText.Get("GS_KLMT")); // kill limit
            CFont::SetRightJustifyOff();
#else
            CFont::PrintString(MP_MENU_X(OPTION_LABEL_START_X), SCALE_Y(SCREEN_MP_MENU_ROW_Y(MGE_HUNTERATTACK_KILL_LIMIT)), TheText.Get("GS_KLMT")); // kill limit
#endif
            int nScoreLimit = TheMPGame.GetScoreLimit();
            if (nScoreLimit <= 0)
                CFont::PrintString(MP_MENU_X(OPTION_VALUE_START_X), SCALE_Y(SCREEN_MP_MENU_ROW_Y(MGE_HUNTERATTACK_KILL_LIMIT)), TheText.Get("GS_ULMT")); // unlimited
            else
            {
                sprintf(sBufText, "%d", nScoreLimit);
                AsciiToUnicode(sBufText, sWideBufText);
                CFont::PrintString(MP_MENU_X(OPTION_VALUE_START_X), SCALE_Y(SCREEN_MP_MENU_ROW_Y(MGE_HUNTERATTACK_KILL_LIMIT)), sWideBufText); // unlimited
            }

            // 126 [3] // MGE_HUNTERATTACK_TIME_LIMIT
            DECLARE_ROW(MGE_HUNTERATTACK_TIME_LIMIT);
            SET_MP_TEXT_COLOUR_STYLE(MGE_HUNTERATTACK_TIME_LIMIT, 0);
#ifdef GTA_LIBERTY
            CFont::SetRightJustifyOn();
            CFont::PrintString(MP_MENU_X(OPTION_LABEL_START_X), SCALE_Y(SCREEN_MP_MENU_ROW_Y(MGE_HUNTERATTACK_TIME_LIMIT)), TheText.Get("GS_TLMT")); // time limit
            CFont::SetRightJustifyOff();
#else
            CFont::PrintString(MP_MENU_X(OPTION_LABEL_START_X), SCALE_Y(SCREEN_MP_MENU_ROW_Y(MGE_HUNTERATTACK_TIME_LIMIT)), TheText.Get("GS_TLMT")); // time limit
#endif
            int nTimeLimit = TheMPGame.GetTimeLimit();
            if (nTimeLimit <= 0)
                CFont::PrintString(MP_MENU_X(OPTION_VALUE_START_X), SCALE_Y(SCREEN_MP_MENU_ROW_Y(MGE_HUNTERATTACK_TIME_LIMIT)), TheText.Get("GS_ULMT"));
            else
            {
                wchar* sText = TheText.Get(nTimeLimit == 1 ? "GS_MIN" : "GS_MINS");
                sprintf(sBufText, "%d %s", nTimeLimit, UnicodeToAscii(sText));
                AsciiToUnicode(sBufText, sWideBufText);
                CFont::PrintString(MP_MENU_X(OPTION_VALUE_START_X), SCALE_Y(SCREEN_MP_MENU_ROW_Y(MGE_HUNTERATTACK_TIME_LIMIT)), sWideBufText);
            }
            nNextRowPos = MGE_MAX_HUNTERATTACK; //4
            break;
        }
        case eGameType::FLAGBALL:
        {
            // 100 [2] // MGE_FLAGBALL_SCENARIO
            DECLARE_ROW(MGE_FLAGBALL_SCENARIO);
            SET_MP_TEXT_COLOUR_STYLE(MGE_FLAGBALL_SCENARIO, 0);
            sprintf(sBufText, "%d", TheMPGame.m_nScenarioOrRaceTrackID + 1);
            AsciiToUnicode(sBufText, sWideBufText);
#ifdef GTA_LIBERTY
            CFont::SetRightJustifyOn();
            CFont::PrintString(MP_MENU_X(OPTION_LABEL_START_X), SCALE_Y(SCREEN_MP_MENU_ROW_Y(MGE_FLAGBALL_SCENARIO)), TheText.Get("GS_TYPE")); // SCENARIO
            CFont::SetRightJustifyOff();
#else
            CFont::PrintString(MP_MENU_X(OPTION_LABEL_START_X), SCALE_Y(SCREEN_MP_MENU_ROW_Y(MGE_FLAGBALL_SCENARIO)), TheText.Get("GS_TYPE")); // SCENARIO
#endif
            CFont::PrintString(MP_MENU_X(OPTION_VALUE_START_X), SCALE_Y(SCREEN_MP_MENU_ROW_Y(MGE_FLAGBALL_SCENARIO)), sWideBufText);
            nNextRowPos = MGE_MAX_FLAGBALL; //3
            break;
        }
        case eGameType::VIP:
        {
            // 100 [2] // MGE_VIP_VIP_TEAM
            DECLARE_ROW(MGE_VIP_VIP_TEAM);
            SET_MP_TEXT_COLOUR_STYLE(MGE_VIP_VIP_TEAM, 0);
#ifdef GTA_LIBERTY
            CFont::SetRightJustifyOn();
            CFont::PrintString(MP_MENU_X(OPTION_LABEL_START_X), SCALE_Y(SCREEN_MP_MENU_ROW_Y(MGE_VIP_VIP_TEAM)), TheText.Get("FEMP_VT")); // vip team
            CFont::SetRightJustifyOff();
#else
            CFont::PrintString(MP_MENU_X(OPTION_LABEL_START_X), SCALE_Y(SCREEN_MP_MENU_ROW_Y(MGE_VIP_VIP_TEAM)), TheText.Get("FEMP_VT")); // vip team
#endif
            uint8 nGangID = TheMPGame.bIsVipTeamTeam2 ? TheMPGame.GetTeam2GangID() : TheMPGame.GetTeam1GangID();
            CFont::PrintString(MP_MENU_X(OPTION_VALUE_START_X), SCALE_Y(SCREEN_MP_MENU_ROW_Y(MGE_VIP_VIP_TEAM)), TheText.Get(gMPGangDefs[nGangID].name));
            nNextRowPos = MGE_MAX_VIP; //3
            break;
        }
#endif
    }


    int nPosY = SCREEN_MP_MENU_ROW_Y(nNextRowPos);
    if (FREE_MODE)
    {
        DECLARE_ROW(nNextRowPos);
        RenderCharacterOption(nNextRowPos, nPosY);
        nNextRowPos++;
    }
    else if (GANG_MODE)
    {
#ifdef GTA_LIBERTY
        m_lobbyPed.Render();
#endif
        SET_MP_TEXT_COLOUR_STYLE(nNextRowPos, 0);
        DECLARE_ROW(nNextRowPos);
#ifdef GTA_LIBERTY
        CFont::SetRightJustifyOn();
        CFont::PrintString(MP_MENU_X(OPTION_LABEL_START_X), SCALE_Y(nPosY), TheText.Get("FEMP_TS")); // join gang
        CFont::SetRightJustifyOff();
#else
        CFont::PrintString(MP_MENU_X(OPTION_LABEL_START_X), SCALE_Y(nPosY), TheText.Get("FEMP_TS")); // join gang
#endif
        uint8 nGangID = cMultiGame::s_nSelectedTeam == eGameTeam::TEAM_A ? TheMPGame.GetTeam1GangID() : TheMPGame.GetTeam2GangID();
        CFont::PrintString(MP_MENU_X(OPTION_VALUE_START_X), SCALE_Y(nPosY), TheText.Get(gMPGangDefs[nGangID].name));
        nNextRowPos++;
    }

    SET_MP_TEXT_COLOUR_STYLE(nNextRowPos, 0);

    if (!TheAdhoc.IsHost())
    {
        drawStatusText(*this, "FEMP_WS"); // waiting host for start the game
        return;
    }

    // Bottom block [Reset To Default, Start Game]
    if (!TheAdhoc.IsNextStateNow(&cAdhoc::StateShutdown) &&
        !TheAdhoc.IsNextStateNow(&cAdhoc::StateInitialise) &&
        !TheAdhoc.IsNextStateNow(&cAdhoc::StateConnectLobbyGroup))
    {
        SET_MP_TEXT_COLOUR_STYLE(nNextRowPos, 1);
        nPosY = 192;
        //nPosY = SCREEN_MP_MENU_ROW_Y(nNextRowPos); // dynamic Y
        DECLARE_ROW_Y(nPosY, nNextRowPos);
        CFont::PrintString(MP_MENU_X(OPTION_BOTTOM_BLOCK_START_X), SCALE_Y(nPosY), TheText.Get("FEMP_R")); // reset to defaults
        nNextRowPos++;
        SET_MP_TEXT_COLOUR_STYLE(nNextRowPos, 1);
        nPosY = 208;
        //nPosY = SCREEN_MP_MENU_ROW_Y(nNextRowPos); // dynamic Y
        DECLARE_ROW_Y(nPosY, nNextRowPos);
        uint8 nConnPlayers = TheAdhoc.GetNumberOfConnectedPlayers();
        bool bGangsFilledUp = TheAdhoc.GetNumberOfNonEmptyGangs() >= 2;
        bool bCanJoinFFA = FREE_MODE && nConnPlayers >= 2;
        bool bCanJoinOthers = nConnPlayers >= 2 && bGangsFilledUp;
        if (gDeveloperFlag || bCanJoinFFA || bCanJoinOthers)
            CFont::PrintString(MP_MENU_X(OPTION_BOTTOM_BLOCK_START_X), SCALE_Y(nPosY), TheText.Get("FEMP_SG")); // start game
        else
        {
#ifdef GTA_LIBERTY
            CFont::SetColor(m_nSelectedMenuOptionIndex == nNextRowPos ? CRGBA(MP_TEXT_COLOUR_SELECTED, 160) : CRGBA(MP_TEXT_COLOUR, 160));
#else
            CFont::SetColor(m_nSelectedMenuOptionIndex == nNextRowPos ? CRGBA(COLOR_WHITE, 255) : CRGBA(COLOR_YELLOW, 255));
#endif
            if (!FREE_MODE && !bGangsFilledUp)
                CFont::PrintString(MP_MENU_X(OPTION_BOTTOM_BLOCK_START_X), SCALE_Y(nPosY), TheText.Get("FEMP_ST")); // 1 or more players per gang required
            else
                CFont::PrintString(MP_MENU_X(OPTION_BOTTOM_BLOCK_START_X), SCALE_Y(nPosY), TheText.Get("FEMP_SN")); // 2 or more players required
        }
        nNextRowPos++;
    }
    else {
        drawStatusText(*this, "FEMP_NC"); // Establishing network connections...Please Wait
        return;
    }
}
