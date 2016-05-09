/*
 * =====================================================================================
 *
 *       Filename: actorpod.hpp
 *        Created: 04/20/2016 21:49:14
 *  Last Modified: 05/08/2016 16:06:18
 *
 *    Description: why I made actor as a plug, because I want it to be a one to zero/one
 *                 mapping as ServerObject -> Actor
 *
 *                 Then a server object can plug to the actor system slot for concurrent
 *                 operation, but also some server objects don't need this functionality
 *
 *                 And what's more, if an object is both ServerObject and Actor, we need
 *                 MI, but I really don't want to use MI
 *
 *                 TODO & TBD
 *                 The most important member function of ActorPod is Send(), but I am not
 *                 sure how should I design this interface, two ways:
 *
 *                 0. bool Send(MessagePack, Address)
 *                 1. bool Send(MessagePack, Address, Respond)
 *                 2. bool Send(MessageType, MessageBuffer, BufferLen, Address, Respond)
 *
 *
 *                 for case-0 we put Respond inside MessagePack, make it as part of the
 *                 message, this is OK conceptually.
 *
 *                 Real issue is we support ``ID() / Respond()", the id is not allocated
 *                 by the caller, but by internal logic in ActorPod. However in the view
 *                 of the caller, it send a ``const" message, but if internal logic should
 *                 assign ID for the message, we can
 *                 1. append ID info to the message, then it's not ``const" conceptually
 *                 2. Make InternalMessagePack(MessagePack), this hurt performance
 *
 *                 so if we just send the information to the internal logic and let it
 *                 make the MessagePack, then we overcome the conceptual crisis, what to
 *                 supply:
 *                 0. this message is for responding?
 *                 1. message type
 *                 2. buffer of message body
 *                 3. buffer length
 *                 4. target actor address for sure
 *
 *
 *                 I decide to use Forward() instead of Send()
 *                 since for ActorPod, ``Forward()" is more close to its role
 *
 *                 and this helps to avoid the override of Theron::Actor::Send(), if you
 *                 use using Theron::Actor::Send, then any undefined ActorPod::Send()
 *                 will be redirect to Theron::Actor::Send() since it's a template
 *
 *                 Won't make Forward() virtual
 *
 *
 *                 TODO & TBD: to support trigger functionality.
 *                 every time when class state updated, we call the trigger to check what
 *                 should be done, for actor the only way to update state is to handle
 *                 message, then:
 *
 *                 if(response message){
 *                     check response pool;
 *                 }else{
 *                     call operation handler;
 *                 }
 *
 *                 if(trigger registered){
 *                     call trigger;
 *                 }
 *
 *                 so the best place to put trigger is inside class ActorPod, this means
 *                 for Transponder and ReactObject, we can't use trigger before activate
 *                 it (actiate means call ``new ActorPod"), so we design ActorPod ctor
 *                 as
 *
 *                 ActorPod(Trigger, Operation);
 *
 *                 to put the trigger here. Then for Transponder and ReactObject, we
 *                 provide method to install trigger handler:
 *
 *                 Transponder::Install("ClearQueue", fnClearQueue);
 *                 Transponder::Uninstall("ClearQueue");
 *
 *                 ReactObject::Install("ClearQueue", fnClearQueue);
 *                 ReactObject::Uninstall("ClearQueue");
 *
 *                 we need a map:
 *
 *                      std::unordered_map<std::string, std::function<void()>
 *
 *                 to store all handler, install/uninstall handlers. This map can only
 *                 be in Transponder/ReactObject, since if we put it in ActorPod, then
 *                 before Actiate(), Transponder::m_ActorPod is nullptr, how could we
 *                 put it in?
 *
 *                 After Activate(), all interaction between ActorPod *should* be via
 *                 message, then if we insist on putting this map inside ActorPod, we
 *                 have to call:
 *                      m_ActorPod->Install("ClearQueue", fnClearQueue);
 *                 this violate the rule.
 *
 *
 *                 So we follow the pattern how we define m_Operate, put the virtual
 *                 function invocation to m_Operate and m_Trigger, then Transponder,
 *                 ReactObject define how we handle message operation, event trigger
 *
 *                 Drawback is we need to copy the code in Transponder and ReactObject
 *                 to avoid MI.
 *
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
#pragma once

#include <functional>
#include <unordered_map>
#include <Theron/Theron.h>

#include "messagebuf.hpp"
#include "messagepack.hpp"

class ActorPod: public Theron::Actor
{
    private:
        using MessagePackOperation
            = std::function<void(const MessagePack&, const Theron::Address &)>;
        // no need to keep the message pack itself
        // since when registering response operation, we always have the message pack avaliable
        typedef struct _RespondMessageRecord {
            // MessagePack RespondMessagePack;
            MessagePackOperation    RespondOperation;

            // _RespondMessageRecord(const MessagePack & rstMPK,
            //         const MessagePackOperation &rstOperation)
            //     : RespondMessagePack(rstMPK)
            //     , RespondOperation(rstOperation)
            // {}
            _RespondMessageRecord(const MessagePackOperation &rstOperation)
                : RespondOperation(rstOperation)
            {}
        } RespondMessageRecord;

    protected:
        size_t m_ValidID;
        MessagePackOperation m_Operate;
        // TODO & TBD
        // trigger is only for state update, so it won't accept any parameters w.r.t
        // message or time or xxx
        // but it will be invoked every time when message handling finished, since
        // for actor, the only chance to update its state is via message driving.
        std::function<void()> m_Trigger;
        std::unordered_map<uint32_t, RespondMessageRecord> m_RespondMessageRecordM;

    public:
        // actor without trigger
        explicit ActorPod(Theron::Framework *pFramework,
                const std::function<void(const MessagePack &, const Theron::Address &)> &fnOperate)
            : Theron::Actor(*pFramework)
            , m_ValidID(0)
            , m_Operate(fnOperate)
        {
            RegisterHandler(this, &ActorPod::InnHandler);
        }

        // actor with trigger
        explicit ActorPod(Theron::Framework *pFramework, const std::function<void()> &fnTrigger,
                const std::function<void(const MessagePack &, const Theron::Address &)> &fnOperate)
            : Theron::Actor(*pFramework)
            , m_ValidID(0)
            , m_Operate(fnOperate)
            , m_Trigger(fnTrigger)
        {
            RegisterHandler(this, &ActorPod::InnHandler);
        }

        virtual ~ActorPod() = default;

    protected:
        void InnHandler(const MessagePack &, Theron::Address);

    public:
        // send a responding message without asking for reply
        bool Forward(const MessageBuf &rstMB, const Theron::Address &rstAddr, uint32_t nRespond = 0)
        {
            // TODO
            // do I need to put logic to avoid sending message to itself?
            return Theron::Actor::Send<MessagePack>({rstMB, 0, nRespond}, rstAddr);
        }

        // send a non-responding message and exptecting a reply
        bool Forward(const MessageBuf &rstMB, const Theron::Address &rstAddr,
                const std::function<void(const MessagePack&, const Theron::Address &)> &fnOPR)
        {
            return Forward(rstMB, rstAddr, 0, fnOPR);
        }

        // send a responding message and exptecting a reply
        bool Forward(const MessageBuf &, const Theron::Address &, uint32_t,
                const std::function<void(const MessagePack&, const Theron::Address &)> &);

    private:
        uint32_t ValidID();
};
