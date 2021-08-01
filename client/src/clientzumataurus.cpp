#include "fflerror.hpp"
#include "processrun.hpp"
#include "clientzumataurus.hpp"

ClientZumaTaurus::ClientZumaTaurus(uint64_t uid, ProcessRun *proc, const ActionNode &action)
    : ClientStandMonster(uid, proc)
{
    fflassert(isMonster(u8"祖玛教主"));
    switch(action.type){
        case ACTION_SPAWN:
            {
                m_currMotion.reset(new MotionNode
                {
                    .type = MOTION_MON_STAND,
                    .direction = DIR_BEGIN,
                    .x = action.x,
                    .y = action.y,
                });

                m_standMode = false;
                break;
            }
        case ACTION_STAND:
            {
                if(action.extParam.stand.zumaTaurus.standMode){
                    m_currMotion.reset(new MotionNode
                    {
                        .type = MOTION_MON_STAND,
                        .direction = directionValid(action.direction) ? to_d(action.direction) : DIR_UP,
                        .x = action.x,
                        .y = action.y,
                    });
                    m_standMode = true;
                }
                else{
                    m_currMotion.reset(new MotionNode
                    {
                        .type = MOTION_MON_STAND,
                        .direction = DIR_BEGIN,
                        .x = action.x,
                        .y = action.y,
                    });
                    m_standMode = false;
                }
                break;
            }
        case ACTION_ATTACK:
            {
                m_currMotion.reset(new MotionNode
                {
                    .type = MOTION_MON_ATTACK0,
                    .direction = directionValid(action.direction) ? to_d(action.direction) : DIR_UP,
                    .x = action.x,
                    .y = action.y,
                });

                m_standMode = true;
                break;
            }
        case ACTION_MOVE:
            {
                m_currMotion.reset(new MotionNode
                {
                    .type = MOTION_MON_WALK,
                    .direction = directionValid(action.direction) ? to_d(action.direction) : DIR_UP,
                    .x = action.x,
                    .y = action.y,
                });

                m_standMode = true;
                break;
            }
        case ACTION_TRANSF:
            {
                m_currMotion.reset(new MotionNode
                {
                    .type = MOTION_MON_SPAWN,
                    .direction = DIR_BEGIN,
                    .x = action.x,
                    .y = action.y,
                });

                m_currMotion->addUpdate(false, [this](MotionNode *motionPtr) -> bool
                {
                    if(motionPtr->frame < 9){
                        return false;
                    }

                    m_forcedMotionQueue.push_back(std::unique_ptr<MotionNode>(new MotionNode
                    {
                        .type = MOTION_MON_STAND,
                        .direction = DIR_DOWNLEFT,
                        .x = motionPtr->x,
                        .y = motionPtr->y,
                    }));

                    m_processRun->addFixedLocMagic(std::unique_ptr<FixedLocMagic>(new ZumaTaurusFragmentEffect_RUN
                    {
                        motionPtr->x,
                        motionPtr->y,
                    }));
                    return true;
                });

                m_standMode = true;
                break;
            }
        case ACTION_HITTED:
            {
                m_currMotion.reset(new MotionNode
                {
                    .type = MOTION_MON_HITTED,
                    .direction = directionValid(action.direction) ? to_d(action.direction) : DIR_UP,
                    .x = action.x,
                    .y = action.y,
                });

                m_standMode = true; // can't be hitted if stay in the soil
                break;
            }
        default:
            {
                throw fflerror("invalid action: %s", actionName(action));
            }
    }
}

bool ClientZumaTaurus::onActionSpawn(const ActionNode &action)
{
    fflassert(m_forcedMotionQueue.empty());
    m_currMotion.reset(new MotionNode
    {
        .type = MOTION_MON_STAND,
        .direction = DIR_BEGIN,
        .x = action.x,
        .y = action.y,
    });

    m_standMode = false;
    return true;
}

bool ClientZumaTaurus::onActionStand(const ActionNode &action)
{
    if(finalStandMode() != (bool)(action.extParam.stand.zumaTaurus.standMode)){
        addActionTransf();
    }
    return true;
}

bool ClientZumaTaurus::onActionTransf(const ActionNode &action)
{
    const auto standReq = (bool)(action.extParam.transf.zumaTaurus.standModeReq);
    if(finalStandMode() != standReq){
        addActionTransf();
    }
    return true;
}

bool ClientZumaTaurus::onActionAttack(const ActionNode &action)
{
    if(!finalStandMode()){
        addActionTransf();
    }

    const auto [endX, endY, endDir] = motionEndGLoc().at(1);
    m_motionQueue = makeWalkMotionQueue(endX, endY, action.x, action.y, SYS_MAXSPEED);
    if(auto coPtr = m_processRun->findUID(action.aimUID)){
        m_motionQueue.push_back(std::unique_ptr<MotionNode>(new MotionNode
        {
            .type = MOTION_MON_ATTACK0,
            .direction = [&action, endDir, coPtr]() -> int
            {
                const auto nX = coPtr->x();
                const auto nY = coPtr->y();
                if(mathf::LDistance2<int>(nX, nY, action.x, action.y) == 0){
                    return endDir;
                }
                return PathFind::GetDirection(action.x, action.y, nX, nY);
            }(),
            .x = action.x,
            .y = action.y,
        }));

        m_motionQueue.back()->addUpdate(false, [this](MotionNode *motionPtr) -> bool
        {
            addAttachMagic(std::unique_ptr<AttachMagic>(new AttachMagic(u8"祖玛教主_施法", u8"启动", motionPtr->direction - DIR_BEGIN)));
            return true;
        });
    }
    return true;
}

void ClientZumaTaurus::addActionTransf()
{
    ClientStandMonster::addActionTransf();

    fflassert(!m_forcedMotionQueue.empty());
    fflassert(m_forcedMotionQueue.back()->type == MOTION_MON_SPAWN);

    m_forcedMotionQueue.back()->addUpdate(false, [this](MotionNode *motionPtr) -> bool
    {
        if(motionPtr->frame < 9){
            return false;
        }

        m_forcedMotionQueue.push_back(std::unique_ptr<MotionNode>(new MotionNode
        {
            .type = MOTION_MON_STAND,
            .direction = DIR_DOWNLEFT,
            .x = motionPtr->x,
            .y = motionPtr->y,
        }));

        m_processRun->addFixedLocMagic(std::unique_ptr<FixedLocMagic>(new ZumaTaurusFragmentEffect_RUN
        {
            motionPtr->x,
            motionPtr->y,
        }));
        return true;
    });
}