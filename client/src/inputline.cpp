/*
 * =====================================================================================
 *
 *       Filename: inputline.cpp
 *        Created: 06/19/2017 11:29:06
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

#include <cmath>
#include "mathf.hpp"
#include "colorf.hpp"
#include "inputline.hpp"
#include "sdldevice.hpp"
#include "clientargparser.hpp"

extern SDLDevice *g_sdlDevice;
extern ClientArgParser *g_clientArgParser;

bool InputLine::processEvent(const SDL_Event &event, bool valid)
{
    if(!valid){
        return false;
    }

    switch(event.type){
        case SDL_KEYDOWN:
            {
                if(!focus()){
                    return false;
                }

                switch(event.key.keysym.sym){
                    case SDLK_TAB:
                        {
                            if(m_onTab){
                                m_onTab();
                            }
                            return true;
                        }
                    case SDLK_RETURN:
                        {
                            if(m_onCR){
                                m_onCR();
                            }
                            return true;
                        }
                    case SDLK_LEFT:
                        {
                            m_cursor = std::max<int>(0, m_cursor - 1);
                            m_cursorBlink = 0.0;
                            return true;
                        }
                    case SDLK_RIGHT:
                        {
                            if(m_tpset.empty()){
                                m_cursor = 0;
                            }
                            else{
                                m_cursor = std::min<int>(m_tpset.lineTokenCount(0), m_cursor + 1);
                            }
                            m_cursorBlink = 0.0;
                            return true;
                        }
                    case SDLK_BACKSPACE:
                        {
                            if(m_cursor > 0){
                                m_tpset.deleteToken(m_cursor - 1, 0, 1);
                                m_cursor--;
                            }
                            m_cursorBlink = 0.0;
                            return true;
                        }
                    case SDLK_ESCAPE:
                        {
                            focus(false);
                            return true;
                        }
                    default:
                        {
                            const char keyChar = SDLDeviceHelper::getKeyChar(event, true);
                            if(keyChar != '\0'){
                                const char text[] {keyChar, '\0'};
                                m_tpset.insertUTF8String(m_cursor++, 0, text);
                            }
                            m_cursorBlink = 0.0;
                            return true;
                        }
                }
            }
        case SDL_MOUSEBUTTONDOWN:
            {
                if(!in(event.button.x, event.button.y)){
                    focus(false);
                    return false;
                }

                const int eventX = event.button.x - x();
                const int eventY = event.button.y - y();

                const auto [cursorX, cursorY] = m_tpset.locCursor(eventX, eventY);
                if(cursorY != 0){
                    throw fflerror("cursor locates at wrong line");
                }

                m_cursor = cursorX;
                m_cursorBlink = 0.0;

                focus(true);
                return true;
            }
        default:
            {
                return false;
            }
    }
}

void InputLine::drawEx(int dstX, int dstY, int srcX, int srcY, int srcW, int srcH) const
{
    int srcCropX = srcX;
    int srcCropY = srcY;
    int srcCropW = srcW;
    int srcCropH = srcH;
    int dstCropX = dstX;
    int dstCropY = dstY;

    const int tpsetX = 0;
    const int tpsetY = 0 + (h() - (m_tpset.empty() ? m_tpset.getDefaultFontHeight() : m_tpset.ph())) / 2;

    const auto needDraw = mathf::ROICrop(
            &srcCropX, &srcCropY,
            &srcCropW, &srcCropH,
            &dstCropX, &dstCropY,

            w(),
            h(),

            tpsetX, tpsetY, m_tpset.pw(), m_tpset.ph());

    if(needDraw){
        m_tpset.drawEx(dstCropX, dstCropY, srcCropX - tpsetX, srcCropY - tpsetY, srcCropW, srcCropH);
    }

    if(std::fmod(m_cursorBlink, 1000.0) > 500.0){
        return;
    }

    if(!focus()){
        return;
    }

    int cursorY = y() + tpsetY;
    int cursorX = x() + tpsetX + [this]()
    {
        if(m_tpset.empty() || m_cursor == 0){
            return 0;
        }

        if(m_cursor == m_tpset.lineTokenCount(0)){
            return m_tpset.pw();
        }

        const auto pToken = m_tpset.getToken(m_cursor - 1, 0);
        return pToken->Box.State.W1 + pToken->Box.State.X + pToken->Box.Info.W;
    }();

    int cursorW = m_cursorWidth;
    int cursorH = std::max<int>(m_tpset.ph(), h());

    if(mathf::rectangleOverlapRegion(dstX, dstY, srcW, srcH, cursorX, cursorY, cursorW, cursorH)){
        g_sdlDevice->fillRectangle(m_cursorColor, cursorX, cursorY, cursorW, cursorH);
    }

    if(g_clientArgParser->debugDrawInputLine){
        g_sdlDevice->drawRectangle(colorf::BLUE + colorf::A_SHF(255), x(), y(), w(), h());
    }
}

void InputLine::deleteChar()
{
    m_tpset.deleteToken(m_cursor - 1, 0, 1);
    m_cursor--;
}

void InputLine::insertChar(char ch)
{
    const char rawString[]
    {
        ch, '\0',
    };

    m_tpset.insertUTF8String(m_cursor, 0, rawString);
    m_cursor++;
}
