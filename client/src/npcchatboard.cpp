/*
 * =====================================================================================
 *
 *       Filename: npcchatboard.cpp
 *        Created: 04/12/2020 19:03:35
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

#include <string_view>
#include "uidf.hpp"
#include "totype.hpp"
#include "sysconst.hpp"
#include "pngtexdb.hpp"
#include "sdldevice.hpp"
#include "processrun.hpp"
#include "npcchatboard.hpp"

extern PNGTexDB *g_progUseDB;
extern SDLDevice *g_sdlDevice;

NPCChatBoard::NPCChatBoard(ProcessRun *proc, Widget *pwidget, bool autoDelete)
    : Widget(DIR_UPLEFT, 0, 0, 386, 204, pwidget, autoDelete)
    , m_margin(20)
    , m_process(proc)
    , m_chatBoard
      {
          DIR_UPLEFT,
          m_margin,
          m_margin,
          386 - m_margin * 2,

          false,
          {0, 0, 0, 0},
          false,

          1,
          12,
          0,

          colorf::WHITE + colorf::A_SHF(255),
          0,

          LALIGN_LEFT,
          0,
          0,

          [this](const char *id, const char *arg, int oldEvent, int newEvent)
          {
              if(oldEvent == BEVENT_DOWN && newEvent == BEVENT_ON){
                  onClickEvent(id, arg);
              }
          },
          this,
      }
    , m_buttonClose
      {
          DIR_UPLEFT,
          100,
          100,
          {SYS_U32NIL, 0X0000001C, 0X0000001D},
          {
              SYS_U32NIL,
              SYS_U32NIL,
              0X01020000 + 105,
          },

          nullptr,
          nullptr,
          [this]()
          {
              show(false);
              m_process->sendNPCEvent(m_NPCUID, SYS_NPCDONE);
          },

          0,
          0,
          0,
          0,

          true,
          true,
          this,
      }
{
    if(m_margin < 0){
        throw fflerror("invalid margin: %d", m_margin);
    }

    auto fnAssertImage = [](uint32_t key, int w, int h)
    {
        if(auto texPtr = g_progUseDB->retrieve(key)){
            if(SDLDeviceHelper::getTextureSize(texPtr) == std::tuple<int, int>{w, h}){
                return;
            }
        }
        throw fflerror("image assertion failed: key = %llu, w = %d, h = %d", to_llu(key), w, h);
    };

    fnAssertImage(0X00000050, 386, 160);
    fnAssertImage(0X00000051, 386, 160);
    fnAssertImage(0X00000052, 386,  20);
    fnAssertImage(0X00000053, 386,  44);
    fnAssertImage(0X00000054, 386,  44);
    show(false);
}

void NPCChatBoard::drawWithNPCFace() const
{
    // |<------------386 ----------->|
    // +-----------------------------+ ---
    // | +----+ +------------------+ |  ^
    // | |    | |                  | |  |
    // | |    | |                  | |  160 + k * 20 + 44
    // | |    | |                  | |  |
    // | +----+ +------------------+ |  v
    // +-----------------------------+ ---

    auto facePtr = g_progUseDB->retrieve(getNPCFaceKey());
    if(!facePtr){
        throw fflerror("no valid NPC face image");
    }

    drawFrame();
    g_sdlDevice->drawTexture(facePtr, m_margin, (h() - SDLDeviceHelper::getTextureHeight(facePtr)) / 2);

    m_chatBoard.draw();
    m_buttonClose.draw();
}

void NPCChatBoard::drawPlain() const
{
    // |<------------386 ----------->|
    // +-----------------------------+ ---
    // | +-------------------------+ |  ^
    // | |                         | |  |
    // | |                         | |  160 + k * 20 + 44
    // | |                         | |  |
    // | +-------------------------+ |  v
    // +-----------------------------+ ---

    drawFrame();
    m_chatBoard.draw();
    m_buttonClose.draw();
}

void NPCChatBoard::drawEx(int, int, int, int, int, int) const
{
    if(g_progUseDB->retrieve(getNPCFaceKey())){
        drawWithNPCFace();
    }
    else{
        drawPlain();
    }
}

void NPCChatBoard::loadXML(uint64_t uid, const char *xmlString)
{
    if(uidf::getUIDType(uid) != UID_NPC){
        throw fflerror("invalid uid type: %s", uidf::getUIDTypeCStr(uid));
    }

    m_NPCUID = uid;
    m_chatBoard.clear();

    if(auto texPtr = g_progUseDB->retrieve(getNPCFaceKey())){
        m_chatBoard.setLineWidth(386 - m_margin * 3 - SDLDeviceHelper::getTextureWidth(texPtr));
    }
    else{
        m_chatBoard.setLineWidth(386 - m_margin * 2);
    }
    m_chatBoard.loadXML(xmlString);

    m_w = 386;
    m_h = 160 + 20 * getMiddleCount() + 44;

    m_buttonClose.moveTo(w() - 40, h() - 43);
    if(auto texPtr = g_progUseDB->retrieve(getNPCFaceKey())){
        m_chatBoard.moveTo(m_margin * 2 + SDLDeviceHelper::getTextureWidth(texPtr), (h() - m_chatBoard.h()) / 2);
    }
    else{
        m_chatBoard.moveTo(m_margin, (h() - m_chatBoard.h()) / 2);
    }
}

void NPCChatBoard::onClickEvent(const char *id, const char *arg)
{
    m_process->addCBLog(CBLOG_SYS, u8"clickEvent: id = %s, arg = %s", to_cstr(id), to_cstr(arg));

    fflassert(str_haschar(id));
    if(arg){
        m_process->sendNPCEvent(m_NPCUID, id, arg);
    }
    else{
        m_process->sendNPCEvent(m_NPCUID, id);
    }

    if(std::string_view(id).ends_with(SYS_NPCDONE)){
        show(false);
    }
}

int NPCChatBoard::getMiddleCount() const
{
    if(auto texPtr = g_progUseDB->retrieve(getNPCFaceKey())){
        const auto [faceW, faceH] = SDLDeviceHelper::getTextureSize(texPtr);
        if(faceW + m_margin * 3 + m_chatBoard.w() >= 386){
            throw fflerror("chat board size error");
        }

        const auto minBoardHeight = 160 + 44 - m_margin * 2;
        const auto maxHeight = std::max<int>(faceH, m_chatBoard.h());

        if(maxHeight < minBoardHeight){
            return 0;
        }
        return (maxHeight - minBoardHeight + 19) / 20;
    }

    // not using NPC face key
    // plain drawing

    const int minPlainHeight = 160 + 44 - m_margin * 2;
    if(m_chatBoard.h() < minPlainHeight){
        return 0;
    }
    return (m_chatBoard.h() - minPlainHeight + 19) / 20;
}

void NPCChatBoard::drawFrame() const
{
    const auto k = getMiddleCount();
    auto frameUp = g_progUseDB->retrieve(0X00000051);
    auto frameMid = g_progUseDB->retrieve(0X00000052);
    auto frameDown = g_progUseDB->retrieve(0X00000053);

    g_sdlDevice->drawTexture(frameUp, 0, 0);
    for(int i = 0; i < k; ++i){
        g_sdlDevice->drawTexture(frameMid, 0, 160 + i * 20);
    }
    g_sdlDevice->drawTexture(frameDown, 0, 160 + k * 20);
}
