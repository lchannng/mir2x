/*
 * =====================================================================================
 *
 *       Filename: servertaodog.cpp
 *        Created: 04/10/2016 02:32:45
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

#include "mathf.hpp"
#include "pathf.hpp"
#include "raiitimer.hpp"
#include "servertaodog.hpp"

corof::eval_poller ServerTaoDog::updateCoroFunc()
{
    uint64_t targetUID = 0;
    std::optional<uint64_t> idleTime;

    while(m_sdHealth.HP > 0){
        if(targetUID && !m_actorPod->checkUIDValid(targetUID)){
            m_inViewCOList.erase(targetUID);
            targetUID = 0;
        }

        if(!targetUID){
            targetUID = co_await coro_pickTarget();
        }

        if(targetUID){
            idleTime.reset();
            setStandMode(true);
            co_await coro_trackAttackUID(targetUID);
        }

        else{
            if(!idleTime.has_value()){
                idleTime = hres_tstamp().to_sec();
            }
            else if(hres_tstamp().to_sec() - idleTime.value() > 30ULL){
                setStandMode(false);
            }

            if(m_actorPod->checkUIDValid(masterUID())){
                co_await coro_followMaster();
            }
            else{
                co_await corof::async_wait(200);
            }
        }
    }

    goDie();
    co_return true;
}

void ServerTaoDog::onAMAttack(const ActorMsgPack &mpk)
{
    const auto amAK = mpk.conv<AMAttack>();
    if(m_dead.get()){
        notifyDead(amAK.UID);
    }
    else{
        if(const auto &mr = DBCOM_MAGICRECORD(amAK.damage.magicID); !pathf::inDCCastRange(mr.castRange, X(), Y(), amAK.X, amAK.Y)){
            switch(uidf::getUIDType(amAK.UID)){
                case UID_MON:
                case UID_PLY:
                    {
                        AMMiss amM;
                        std::memset(&amM, 0, sizeof(amM));

                        amM.UID = amAK.UID;
                        m_actorPod->forward(amAK.UID, {AM_MISS, amM});
                        return;
                    }
                default:
                    {
                        return;
                    }
            }
        }

        if(amAK.UID != masterUID()){
            setStandMode(true);
            addOffenderDamage(amAK.UID, amAK.damage);
        }

        dispatchAction(ActionHitted
        {
            .x = X(),
            .y = Y(),
            .direction = Direction(),
            .extParam
            {
                .dog
                {
                    .standMode = m_standMode,
                }
            }
        });
        struckDamage(amAK.damage);
    }
}

DamageNode ServerTaoDog::getAttackDamage(int dc) const
{
    fflassert(to_u32(dc) == DBCOM_MAGICID(u8"神兽_喷火"));
    return MagicDamage
    {
        .magicID = dc,
        .damage = mathf::rand<int>(getMR().dc[0], getMR().dc[1]),
    };
}
