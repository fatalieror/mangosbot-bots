#pragma once
#include "../Trigger.h"
#include "../../PlayerbotAIConfig.h"
#include "../../ServerFacade.h"

namespace ai
{
    class EnemyTooCloseForSpellTrigger : public Trigger {
    public:
        EnemyTooCloseForSpellTrigger(PlayerbotAI* ai) : Trigger(ai, "enemy too close for spell") {}
        virtual bool IsActive()
        {
            Unit* target = AI_VALUE(Unit*, "current target");
            if (target)
            {
                if (target->GetTarget() == bot && !bot->GetGroup() && !target->IsRooted() && target->GetSpeedInMotion() > bot->GetSpeedInMotion() * 0.65)
                    return false;

                bool isBoss = false;
                bool isRaid = false;
                float combatReach = bot->GetCombinedCombatReach(target, false);
                float targetDistance = sServerFacade.GetDistance2d(bot, target) + combatReach;
                if (target->IsCreature())
                {
                    Creature* creature = ai->GetCreature(target->GetObjectGuid());
                        if (creature)
                        {
                            isBoss = creature->IsWorldBoss();
                        }
                }
                if (bot->GetMap() && bot->GetMap()->IsRaid())
                    isRaid = true;

                //if (isBoss || isRaid)
                //    return sServerFacade.IsDistanceLessThan(targetDistance, (ai->GetRange("spell") + combatReach) / 2);

                return sServerFacade.IsDistanceLessOrEqualThan(targetDistance, (ai->GetRange("spell") + combatReach) * 0.7f);
            }
            return false;
        }
    };

    class EnemyTooCloseForShootTrigger : public Trigger {
    public:
        EnemyTooCloseForShootTrigger(PlayerbotAI* ai) : Trigger(ai, "enemy too close for shoot") {}
        virtual bool IsActive()
        {
            Unit* target = AI_VALUE(Unit*, "current target");
            if (!target)
                return false;

            if (target->GetTarget() == bot && !bot->GetGroup() && !target->IsRooted() && target->GetSpeedInMotion() > bot->GetSpeedInMotion() * 0.65)
                return false;

            bool isBoss = false;
            bool isRaid = false;
            float combatReach = bot->GetCombinedCombatReach(target, false);
            float targetDistance = sServerFacade.GetDistance2d(bot, target) + combatReach;
            if (target->IsCreature())
            {
                Creature* creature = ai->GetCreature(target->GetObjectGuid());
                if (creature)
                {
                    isBoss = creature->IsWorldBoss();
                }
            }
            if (bot->GetMap() && bot->GetMap()->IsRaid())
                isRaid = true;

            //if (isBoss || isRaid)
            //    return sServerFacade.IsDistanceLessThan(targetDistance, ai->GetRange("shoot") + combatReach);

            return sServerFacade.IsDistanceLessOrEqualThan(targetDistance, (ai->GetRange("shoot") + combatReach) * 0.7f);
        }
    };

    class EnemyTooCloseForMeleeTrigger : public Trigger {
    public:
        EnemyTooCloseForMeleeTrigger(PlayerbotAI* ai) : Trigger(ai, "enemy too close for melee", 3) {}
        virtual bool IsActive()
        {
            Unit* target = AI_VALUE(Unit*, "current target");
            if (target && target->IsPlayer())
                return false;

            return target && AI_VALUE2(bool, "inside target", "current target");
        }
    };

    class EnemyIsCloseTrigger : public Trigger {
    public:
        EnemyIsCloseTrigger(PlayerbotAI* ai) : Trigger(ai, "enemy is close") {}
        virtual bool IsActive()
        {
            Unit* target = AI_VALUE(Unit*, "current target");
            return target &&
                sServerFacade.IsDistanceLessOrEqualThan(AI_VALUE2(float, "distance", "current target"), sPlayerbotAIConfig.tooCloseDistance);
        }
    };

    class OutOfRangeTrigger : public Trigger {
    public:
        OutOfRangeTrigger(PlayerbotAI* ai, string name, float distance) : Trigger(ai, name)
        {
            this->distance = distance;
        }
        virtual bool IsActive()
        {
            Unit* target = AI_VALUE(Unit*, GetTargetName());
            return target &&
                sServerFacade.IsDistanceGreaterThan(AI_VALUE2(float, "distance", GetTargetName()), distance);
        }
        virtual string GetTargetName() { return "current target"; }

    protected:
        float distance;
    };

    class EnemyOutOfMeleeTrigger : public OutOfRangeTrigger
    {
    public:
        EnemyOutOfMeleeTrigger(PlayerbotAI* ai) : OutOfRangeTrigger(ai, "enemy out of melee range", sPlayerbotAIConfig.meleeDistance) {}
        virtual bool IsActive()
        {
            Unit* target = AI_VALUE(Unit*, GetTargetName());
            if (!target)
                return false;

            return !bot->CanReachWithMeleeAttack(target) || !bot->IsWithinLOSInMap(target, true);
        }
    };

    class EnemyOutOfSpellRangeTrigger : public OutOfRangeTrigger
    {
    public:
        EnemyOutOfSpellRangeTrigger(PlayerbotAI* ai) : OutOfRangeTrigger(ai, "enemy out of spell range", ai->GetRange("spell")) {}
        virtual bool IsActive()
        {
            Unit* target = AI_VALUE(Unit*, GetTargetName());
            if (!target)
                return false;

            return target && (bot->GetDistance(target, true, DIST_CALC_COMBAT_REACH) > (distance - sPlayerbotAIConfig.contactDistance)) || !bot->IsWithinLOSInMap(target, true);
        }
    };

    class PartyMemberToHealOutOfSpellRangeTrigger : public OutOfRangeTrigger
    {
    public:
        PartyMemberToHealOutOfSpellRangeTrigger(PlayerbotAI* ai) : OutOfRangeTrigger(ai, "party member to heal out of spell range", ai->GetRange("heal")) {}
        virtual string GetTargetName() { return "party member to heal"; }
        virtual bool IsActive()
        {
            Unit* target = AI_VALUE(Unit*, GetTargetName());
            if (!target)
                return false;

            return target && (bot->GetDistance(target, true, DIST_CALC_COMBAT_REACH) > (distance - sPlayerbotAIConfig.contactDistance)) || !bot->IsWithinLOSInMap(target, true);
        }
    };

    class FarFromMasterTrigger : public Trigger {
    public:
        FarFromMasterTrigger(PlayerbotAI* ai, string name = "far from master", float distance = 12.0f, int checkInterval = 50) : Trigger(ai, name, checkInterval), distance(distance) {}

        virtual bool IsActive()
        {
            return sServerFacade.IsDistanceGreaterThan(AI_VALUE2(float, "distance", "master target"), distance);
        }

    private:
        float distance;
    };

    class OutOfReactRangeTrigger : public FarFromMasterTrigger
    {
    public:
        OutOfReactRangeTrigger(PlayerbotAI* ai) : FarFromMasterTrigger(ai, "out of react range", 50.0f, 5) {}
    };
}