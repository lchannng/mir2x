/*
 * =====================================================================================
 *
 *       Filename: followuidmagic.cpp
 *        Created: 12/07/2020 21:19:44
 *    Description:
 *
 *        Version: 1.0
 *       Revision: none
 *       Compiler: gcc
 *
 *         Author: ANHONG
 *          Email: anhonghe@gmail.com
 *   Organization: USTC
 *
 * =====================================================================================
 */

// Magic following UID has two offsets
// Location a is the <m_x, m_y>, offset (b - a) is where to draw frame gfxs
// Location c is the magic gfx arrow head (or fireball head) in the frame animation, which is used as target point
//
//     g_magicDB->retrieve(texID, offX, offY): ( offX,  offY) is (b - a)
//     FollowUIDMagic::targetOff()           : (txOff, tyOff) is (c - a)
//
//        b--------+
//       /|     /  |
//      / |    /   |
//     /  |   /    |
//    /   |  L c   |
//   a    |        |
//        +--------+
//
//  unfortunately the gfx library doesn't have this information
//  fortunately only follow uid magic needs this, mir2 also gives this hard-coded offset in src: Mir3EIClient/GameProcess/Magic.cpp

#include <cmath>
#include "pathf.hpp"
#include "dbcomid.hpp"
#include "sdldevice.hpp"
#include "processrun.hpp"
#include "dbcomrecord.hpp"
#include "attachmagic.hpp"
#include "pngtexoffdb.hpp"
#include "followuidmagic.hpp"
#include "clientargparser.hpp"

extern SDLDevice *g_sdlDevice;
extern PNGTexOffDB *g_magicDB;
extern ClientArgParser *g_clientArgParser;

FollowUIDMagic::FollowUIDMagic(
        const char8_t *magicName,
        const char8_t *magicStage,

        int x,
        int y,
        int gfxDirIndex,
        int flyDirIndex,
        int moveSpeed,

        uint64_t uid,
        ProcessRun *runPtr)
    : MagicBase(magicName, magicStage, gfxDirIndex)
    , m_x(x)
    , m_y(y)
    , m_flyDirIndex(flyDirIndex)
    , m_moveSpeed(moveSpeed)
    , m_uid(uid)
    , m_process(runPtr)
{
    fflassert(m_process);
    fflassert(flyDirIndex >= 0 && flyDirIndex < 16);
    fflassert(m_x >= 0 && m_y >= 0 && m_process->onMap(m_x / SYS_MAPGRIDXP, m_y / SYS_MAPGRIDYP));
}

uint32_t FollowUIDMagic::frameTexID() const
{
    if(m_gfxEntry.gfxID == SYS_TEXNIL){
        return SYS_TEXNIL;
    }

    switch(m_gfxEntry.gfxDirType){
        case  1: return m_gfxEntry.gfxID + frame();
        case  4:
        case  8:
        case 16:
        default: return m_gfxEntry.gfxID + frame() + m_gfxDirIndex * m_gfxEntry.gfxIDCount;
    }
}

std::tuple<int, int> FollowUIDMagic::targetOff() const
{
    // not from raw mir2 database
    // offset generated by FollowUIDMagicEditor

    switch(magicID()){
        case DBCOM_MAGICID(u8"火球术"):
        case DBCOM_MAGICID(u8"诺玛法老_火球术"):
            {
                switch(gfxDirIndex()){
                    case  0: return { 35, -52};
                    case  1: return { 53, -47};
                    case  2: return { 68, -39};
                    case  3: return { 77, -26};
                    case  4: return { 76, -11};
                    case  5: return { 68,   1};
                    case  6: return { 54,  11};
                    case  7: return { 33,  16};
                    case  8: return { 13,  14};
                    case  9: return { -6,  10};
                    case 10: return {-20,   1};
                    case 11: return {-29, -13};
                    case 12: return {-28, -24};
                    case 13: return {-22, -36};
                    case 14: return { -6, -48};
                    case 15: return { 15, -52};
                    default: throw bad_reach();
                }
            }
        case DBCOM_MAGICID(u8"幽灵盾"):
        case DBCOM_MAGICID(u8"神圣战甲术"):
        case DBCOM_MAGICID(u8"灵魂火符"):
        case DBCOM_MAGICID(u8"强魔震法"):
        case DBCOM_MAGICID(u8"猛虎强势"):
            {
                switch(gfxDirIndex()){
                    case  0: return { 31, -56};
                    case  1: return { 48, -52};
                    case  2: return { 62, -45};
                    case  3: return { 70, -34};
                    case  4: return { 70, -22};
                    case  5: return { 65, -10};
                    case  6: return { 53,  -1};
                    case  7: return { 35,   4};
                    case  8: return { 17,   4};
                    case  9: return {  1,  -1};
                    case 10: return {-14,  -8};
                    case 11: return {-19, -17};
                    case 12: return {-22, -31};
                    case 13: return {-15, -42};
                    case 14: return { -4, -51};
                    case 15: return { 12, -57};
                    default: throw bad_reach();
                }
            }
        case DBCOM_MAGICID(u8"大火球"):
            {
                switch(gfxDirIndex()){
                    case  0: return { 25, -51};
                    case  1: return { 44, -49};
                    case  2: return { 61, -41};
                    case  3: return { 72, -30};
                    case  4: return { 76, -17};
                    case  5: return { 72,  -2};
                    case  6: return { 61,   8};
                    case  7: return { 43,  16};
                    case  8: return { 24,  18};
                    case  9: return {  4,  16};
                    case 10: return {-13,   8};
                    case 11: return {-25,  -3};
                    case 12: return {-28, -17};
                    case 13: return {-23, -29};
                    case 14: return {-12, -41};
                    case 15: return {  4, -47};
                    default: throw bad_reach();
                }
            }
        case DBCOM_MAGICID(u8"冰月震天"):
            {
                switch(gfxDirIndex()){
                    case  0: return { 23, -22}; // 冰月震天 has only 1 gfxDirIndex but can have more flyDirIndex
                    default: throw bad_reach();

                }
            }
        case DBCOM_MAGICID(u8"冰月神掌"):
            {
                switch(gfxDirIndex()){
                    case  0: return { 24, -31};
                    case  1: return { 34, -30};
                    case  2: return { 42, -27};
                    case  3: return { 48, -22};
                    case  4: return { 49, -16};
                    case  5: return { 48,  -9};
                    case  6: return { 43,  -4};
                    case  7: return { 33,   0};
                    case  8: return { 23,   2};
                    case  9: return { 15,  -1};
                    case 10: return {  6,  -2};
                    case 11: return {  0,  -8};
                    case 12: return { -3, -14};
                    case 13: return {  0, -22};
                    case 14: return {  5, -28};
                    case 15: return { 14, -32};
                    default: throw bad_reach();
                }
            }
        case DBCOM_MAGICID(u8"祖玛弓箭手_射箭"):
            {
                switch(gfxDirIndex()){
                    case  0: return { 29, -70};
                    case  1: return { 48, -65};
                    case  2: return { 65, -59};
                    case  3: return { 73, -50};
                    case  4: return { 76, -38};
                    case  5: return { 71, -27};
                    case  6: return { 63, -16};
                    case  7: return { 48, -11};
                    case  8: return { 31,  -8};
                    case  9: return { 13, -11};
                    case 10: return {  1, -17};
                    case 11: return { -8, -26};
                    case 12: return {-13, -37};
                    case 13: return { -8, -47};
                    case 14: return {  2, -58};
                    case 15: return { 16, -63};
                    default: throw bad_reach();
                }
            }
        case DBCOM_MAGICID(u8"爆毒蚂蚁_喷毒"):
            {
                switch(gfxDirIndex()){
                    case  0: return { 22, -12};
                    default: throw bad_reach();
                }
            }
        case DBCOM_MAGICID(u8"霹雳掌"):
            {
                switch(gfxDirIndex()){
                    case  0: return { 23, -34};
                    case  1: return { 30, -33};
                    case  2: return { 38, -31};
                    case  3: return { 42, -26};
                    case  4: return { 44, -21};
                    case  5: return { 42, -17};
                    case  6: return { 38, -13};
                    case  7: return { 31, -11};
                    case  8: return { 25, -10};
                    case  9: return { 17, -10};
                    case 10: return { 11, -13};
                    case 11: return {  6, -16};
                    case 12: return {  6, -21};
                    case 13: return {  6, -26};
                    case 14: return { 11, -29};
                    case 15: return { 18, -32};
                    default: throw bad_reach();
                }
            }
        case DBCOM_MAGICID(u8"风掌"):
            {
                switch(gfxDirIndex()){
                    case  0: return { 29, -46};
                    case  1: return { 41, -45};
                    case  2: return { 53, -40};
                    case  3: return { 57, -33};
                    case  4: return { 57, -25};
                    case  5: return { 54, -15};
                    case  6: return { 31,  -7};
                    case  7: return { 31,  -8};
                    case  8: return { 17,  -6};
                    case  9: return {  3,  -8};
                    case 10: return {  0, -17};
                    case 11: return { -5, -21};
                    case 12: return {-12, -30};
                    case 13: return { -3, -39};
                    case 14: return {  4, -46};
                    case 15: return { 17, -48};
                    default: throw bad_reach();
                }
            }
        case DBCOM_MAGICID(u8"月魂断玉"):
            {
                switch(gfxDirIndex()){
                    case  0: return { 24, -14};
                    default: throw bad_reach();
                }
            }
        case DBCOM_MAGICID(u8"月魂灵波"):
            {
                switch(gfxDirIndex()){
                    case  0: return { 23, -18};
                    default: throw bad_reach();
                }
            }
        default:
            {
                return {0, 0};
            }
    }
}

bool FollowUIDMagic::update(double ms)
{
    const auto [dx, dy] = [this]() -> std::tuple<int, int>
    {
        if(const auto coPtr = m_process->findUID(m_uid)){
            if(const auto box = coPtr->getTargetBox()){
                const auto [srcTargetX, srcTargetY] = targetPLoc();
                const auto [dstTargetX, dstTargetY] = box.targetPLoc();

                const int xdiff = dstTargetX - srcTargetX;
                const int ydiff = dstTargetY - srcTargetY;

                m_lastLDistance2 = mathf::LDistance2(dstTargetX, dstTargetY, srcTargetX, srcTargetY);
                if(xdiff == 0 && ydiff == 0){
                    return {0, 0};
                }
                else{
                    return pathf::getDirOff(xdiff, ydiff, m_moveSpeed);
                }
            }
        }

        m_lastLDistance2 = INT_MAX;
        return m_lastFlyOff.value_or(pathf::getDir16Off(m_flyDirIndex, m_moveSpeed));
    }();

    m_x += dx;
    m_y += dy;

    m_lastFlyOff = {dx, dy};
    return MagicBase::update(ms);
}

void FollowUIDMagic::drawViewOff(int viewX, int viewY, uint32_t modColor) const
{
    if(const auto texID = frameTexID(); texID != SYS_TEXNIL){
        if(auto [texPtr, offX, offY] = g_magicDB->retrieve(texID); texPtr){
            SDLDeviceHelper::EnableTextureModColor enableModColor(texPtr, modColor);
            SDLDeviceHelper::EnableTextureBlendMode enableBlendMode(texPtr, SDL_BLENDMODE_BLEND);

            const int drawPX = (m_x + offX) - viewX;
            const int drawPY = (m_y + offY) - viewY;
            const auto [texW, texH] = SDLDeviceHelper::getTextureSize(texPtr);

            g_sdlDevice->drawTexture(texPtr, drawPX, drawPY);
            if(g_clientArgParser->drawMagicGrid){
                g_sdlDevice->drawRectangle(colorf::BLUE + colorf::A_SHF(200), drawPX, drawPY, texW, texH);
                g_sdlDevice->drawLine(colorf::RED + colorf::A_SHF(200), drawPX, drawPY, m_x - viewX, m_y - viewY);

                const auto [srcTargetX, srcTargetY] = targetPLoc();
                g_sdlDevice->drawLine(colorf::GREEN + colorf::A_SHF(200), m_x - viewX, m_y - viewY, srcTargetX - viewX, srcTargetY - viewY);

                if(const auto coPtr = m_process->findUID(m_uid)){
                    const auto [srcTargetX, srcTargetY] = targetPLoc();
                    const auto [dstTargetX, dstTargetY] = coPtr->getTargetBox().targetPLoc();
                    g_sdlDevice->drawLine(colorf::MAGENTA + colorf::A_SHF(200), srcTargetX - viewX, srcTargetY - viewY, dstTargetX - viewX, dstTargetY - viewY);
                }
            }
        }
    }
}

bool FollowUIDMagic::done() const
{
    if(const auto coPtr = m_process->findUID(m_uid)){
        const auto [srcTargetX, srcTargetY] = targetPLoc();
        const auto [dstTargetX, dstTargetY] = coPtr->getTargetBox().targetPLoc();
        return (std::abs(srcTargetX - dstTargetX) < 24 && std::abs(srcTargetY - dstTargetY) < 16) || (mathf::LDistance2<int>(srcTargetX, srcTargetY, dstTargetX, dstTargetY) > m_lastLDistance2);
    }
    return false;
}
