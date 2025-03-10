/*
 * =====================================================================================
 *
 *       Filename: monsterco.cpp
 *        Created: 03/19/2019 06:43:21
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

#include "uidf.hpp"
#include "pathf.hpp"
#include "corof.hpp"
#include "monster.hpp"
#include "monoserver.hpp"

corof::eval_awaiter<bool> Monster::coro_followMaster()
{
    const auto fnwait = +[](Monster *p) -> corof::eval_poller
    {
        corof::async_variable<bool> done;
        p->followMaster([&done](){ done.assign(true); }, [&done](){ done.assign(false); });

        if(co_await done){
            co_await corof::async_wait(p->getMR().walkWait);
            co_return true;
        }
        else{
            co_await corof::async_wait(20);
            co_return false;
        }
    };
    return fnwait(this).to_awaiter<bool>();
}

corof::eval_awaiter<bool> Monster::coro_randomMove()
{
    const auto fnwait = +[](Monster *p) -> corof::eval_poller
    {
        if(std::rand() % 10 < 2){
            if(p->randomTurn()){
                co_await corof::async_wait(p->getMR().walkWait);
            }
            else{
                co_await corof::async_wait(200);
            }
        }

        else{
            if(co_await p->coro_moveForward()){
                co_await corof::async_wait(p->getMR().walkWait);
            }
            else{
                co_await corof::async_wait(200);
            }
        }
        co_return true;
    };
    return fnwait(this).to_awaiter<bool>();
}

corof::eval_awaiter<bool> Monster::coro_moveForward()
{
    const auto fnwait = +[](Monster *p) -> corof::eval_poller
    {
        const auto reachRes = p->oneStepReach(p->Direction(), 1);
        if(!reachRes.has_value()){
            co_return false;
        }

        const auto [nextX, nextY, reachDist] = reachRes.value();
        if(reachDist != 1){
            co_return false;
        }

        corof::async_variable<bool> done;
        p->requestMove(nextX, nextY, p->moveSpeed(), false, false, [&done](){ done.assign(true); }, [&done](){ done.assign(false); });
        co_return (co_await done);
    };
    return fnwait(this).to_awaiter<bool>();
}

corof::eval_awaiter<uint64_t> Monster::coro_pickTarget()
{
    const auto fnwait = +[](Monster *p) -> corof::eval_poller
    {
        corof::async_variable<uint64_t> targetUID;
        p->pickTarget([&targetUID](uint64_t uid){ targetUID.assign(uid); });
        co_return (co_await targetUID);
    };
    return fnwait(this).to_awaiter<uint64_t>();
}

corof::eval_awaiter<uint64_t> Monster::coro_pickHealTarget()
{
    const auto fnwait = +[](Monster *p) -> corof::eval_poller
    {
        const auto fnNeedHeal = +[](Monster *p, uint64_t uid) -> corof::eval_poller
        {
            if(uid && p->m_inViewCOList.count(uid)){
                switch(uidf::getUIDType(uid)){
                    case UID_MON:
                    case UID_PLY:
                        {
                            const auto health = co_await p->coro_queryHealth(uid);
                            if(health.has_value() && health.value().hp < health.value().maxHP){
                                co_return true;
                            }
                            break;
                        }
                    default:
                        {
                            break;
                        }
                }
            }
            co_return false;
        };

        if(p->masterUID() && (co_await fnNeedHeal(p, p->masterUID()).to_awaiter<bool>())){
            co_return p->masterUID();
        }

        // coroutine bug, a reference before and after a coroutine switch can become dangling
        // following code is bad:
        //
        //     for(const auto &[uid, coLoc]: p->m_inViewCOList){
        //         if((uid != p->masterUID()) && (co_await fnNeedHeal(p, uid).to_awaiter<bool>())){
        //             co_return uid;
        //         }
        //     }
        //
        // in this code either the reference to [uid, coLoc] can be released because of co_await
        // or the p->m_inViewCOList itself can be change which makes the range-based-loop to be invalid

        for(const auto uid: p->getInViewUIDList()){
            if((uid != p->masterUID()) && (co_await fnNeedHeal(p, uid).to_awaiter<bool>())){
                co_return uid;
            }
        }
        co_return to_u64(0);
    };
    return fnwait(this).to_awaiter<uint64_t>();
}

corof::eval_awaiter<int> Monster::coro_checkFriend(uint64_t uid)
{
    const auto fnwait = +[](Monster *p, uint64_t uid) -> corof::eval_poller
    {
        corof::async_variable<int> friendType;
        p->checkFriend(uid, [&friendType](uint64_t type){ friendType.assign(type); });
        co_return (co_await friendType);
    };
    return fnwait(this, uid).to_awaiter<int>();
}

corof::eval_awaiter<bool> Monster::coro_trackAttackUID(uint64_t targetUID)
{
    const auto fnwait = +[](Monster *p, uint64_t targetUID) -> corof::eval_poller
    {
        corof::async_variable<bool> done;
        p->trackAttackUID(targetUID, [&done]{ done.assign(true); }, [&done]{ done.assign(false); });

        if(co_await done){
            co_await corof::async_wait(p->getMR().attackWait);
            co_return true;
        }
        else{
            co_await corof::async_wait(20);
            co_return false;
        }
    };
    return fnwait(this, targetUID).to_awaiter<bool>();
}

corof::eval_awaiter<bool> Monster::coro_trackUID(uint64_t targetUID, DCCastRange r)
{
    const auto fnwait = +[](Monster *p, uint64_t targetUID, DCCastRange r) -> corof::eval_poller
    {
        corof::async_variable<bool> done;
        p->trackUID(targetUID, r, [&done]{ done.assign(true); }, [&done]{ done.assign(false); });
        co_return (co_await done);
    };
    return fnwait(this, targetUID, r).to_awaiter<bool>();
}

corof::eval_awaiter<bool> Monster::coro_attackUID(uint64_t targetUID, int dcType)
{
    const auto fnwait = +[](Monster *p, uint64_t targetUID, int dcType) -> corof::eval_poller
    {
        corof::async_variable<bool> done;
        p->attackUID(targetUID, dcType, [&done]{ done.assign(true); }, [&done]{ done.assign(false); });
        co_return (co_await done);
    };
    return fnwait(this, targetUID, dcType).to_awaiter<bool>();
}

corof::eval_awaiter<bool> Monster::coro_jumpGLoc(int dstX, int dstY, int newDir)
{
    const auto fnwait = +[](Monster *p, int dstX, int dstY, int newDir) -> corof::eval_poller
    {
        corof::async_variable<bool> done;
        p->requestJump(dstX, dstY, newDir, [&done]{ done.assign(true); }, [&done]{ done.assign(false); });
        co_return (co_await done);
    };
    return fnwait(this, dstX, dstY, newDir).to_awaiter<bool>();
}

corof::eval_awaiter<bool> Monster::coro_jumpUID(uint64_t targetUID)
{
    const auto fnwait = +[](Monster *p, uint64_t targetUID) -> corof::eval_poller
    {
        corof::async_variable<bool> done;
        p->jumpUID(targetUID, [&done]{ done.assign(true); }, [&done]{ done.assign(false); });
        co_return (co_await done);
    };
    return fnwait(this, targetUID).to_awaiter<bool>();
}

corof::eval_awaiter<bool> Monster::coro_jumpAttackUID(uint64_t targetUID)
{
    const auto fnwait = +[](Monster *p, uint64_t targetUID) -> corof::eval_poller
    {
        corof::async_variable<bool> done;
        p->jumpAttackUID(targetUID, [&done]{ done.assign(true); }, [&done]{ done.assign(false); });

        if(co_await done){
            co_await corof::async_wait(p->getMR().attackWait);
            co_return true;
        }
        else{
            co_await corof::async_wait(20);
            co_return false;
        }
    };
    return fnwait(this, targetUID).to_awaiter<bool>();
}

corof::eval_awaiter<bool> Monster::coro_inDCCastRange(uint64_t targetUID, DCCastRange r)
{
    fflassert(targetUID);
    fflassert(r);

    const auto fnwait = +[](Monster *p, uint64_t targetUID, DCCastRange r) -> corof::eval_poller
    {
        corof::async_variable<bool> done;
        p->getCOLocation(targetUID, [p, r, &done](const COLocation &coLoc)
        {
            if(p->m_map->in(coLoc.mapID, coLoc.x, coLoc.y)){
                done.assign(pathf::inDCCastRange(r, p->X(), p->Y(), coLoc.x, coLoc.y));
            }
            else{
                done.assign(false);
            }
        },

        [&done]()
        {
            done.assign(false);
        });

        co_return (co_await done);
    };
    return fnwait(this, targetUID, r).to_awaiter<bool>();
}

corof::eval_awaiter<std::optional<SDHealth>> Monster::coro_queryHealth(uint64_t uid)
{
    const auto fnwait = +[](Monster *p, uint64_t uid) -> corof::eval_poller
    {
        corof::async_variable<std::optional<SDHealth>> health;
        p->queryHealth(uid, [uid, &health](uint64_t argUID, SDHealth argHealth)
        {
            if(argUID){
                fflassert(uid == argUID);
                health.assign(std::move(argHealth));
            }
            else{
                health.assign({});
            }
        });
        co_return (co_await health);
    };
    return fnwait(this, uid).to_awaiter<std::optional<SDHealth>>();
}

corof::eval_awaiter<std::tuple<uint32_t, int, int>> Monster::coro_getCOGLoc(uint64_t targetUID)
{
    const auto fnwait = +[](Monster *p, uint64_t targetUID) -> corof::eval_poller
    {
        int x = -1;
        int y = -1;
        uint32_t mapID = 0;
        corof::async_variable<bool> done;

        p->getCOLocation(targetUID, [&x, &y, &mapID, &done](const COLocation &coLoc)
        {
            x = coLoc.x;
            y = coLoc.y;
            mapID = coLoc.mapID;
            done.assign(true);
        },

        [&done]
        {
            done.assign(false);
        });

        if(co_await done){
            co_return std::tuple<uint32_t, int, int>(mapID, x, y);
        }
        else{
            co_return std::tuple<uint32_t, int, int>(0, -1, -1);
        }
    };
    return fnwait(this, targetUID).to_awaiter<std::tuple<uint32_t, int, int>>();
}

corof::eval_awaiter<bool> Monster::coro_validTarget(uint64_t targetUID)
{
    const auto fnwait = +[](Monster *p, uint64_t targetUID) -> corof::eval_poller
    {
        switch(uidf::getUIDType(targetUID)){
            case UID_MON:
            case UID_PLY:
                {
                    break;
                }
            default:
                {
                    co_return false;
                }
        }

        if(!p->m_actorPod->checkUIDValid(targetUID)){
            co_return false;
        }

        const auto [locMapID, locX, locY] = co_await p->coro_getCOGLoc(targetUID);
        if(!p->inView(locMapID, locX, locY)){
            co_return false;
        }

        const auto viewDistance = p->getMR().view;
        if(viewDistance <= 0){
            co_return false;
        }

        if(mathf::LDistance2<int>(p->X(), p->Y(), locX, locY) > viewDistance * viewDistance){
            co_return false;
        }
        co_return true;
    };
    return fnwait(this, targetUID).to_awaiter<bool>();
}
