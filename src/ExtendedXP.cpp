#include "ExtendedXP.h"

void ExtendedXPPlayer::OnGiveXP(Player* player, uint32& amount, Unit* victim, uint8 /*xpSource*/)
{
    if (!sConfigMgr->GetOption<bool>("ExtendedXP.Enable", false))
    {
        return;
    }

    if (!player || !victim ||
        victim->IsPlayer() || !victim->ToCreature())
    {
        return;
    }

    if (sConfigMgr->GetOption<bool>("ExtendedXP.RareXP.Enable", false))
    {
        auto creature = victim->ToCreature();
        auto creatureProto = creature->GetCreatureTemplate();

        if (creatureProto->rank != CREATURE_ELITE_RARE && creatureProto->rank != CREATURE_ELITE_RAREELITE)
        {
            return;
        }

        float rareExpMulti = sConfigMgr->GetOption<float>("ExtendedXP.RareXP.Multiplier", 3);
        amount = amount * rareExpMulti;
    }

    if (sConfigMgr->GetOption<bool>("ExtendedXP.GroupXP.Enable", false))
    {
        auto group = player->GetGroup();
        if (!group)
        {
            return;
        }

        uint32 playersInRange = 0;
        for (auto it = group->GetFirstMember(); it != nullptr; it = it->next())
        {
            auto member = it->GetSource();
            if (!member)
            {
                continue;
            }

            if (!member->IsInMap(player) ||
                !member->IsWithinDist(player, member->GetSightRange(player), false))
            {
                continue;
            }

            playersInRange++;
        }

        float groupExpMulti = sConfigMgr->GetOption<float>("ExtendedXP.GroupXP.Multiplier", 0.33);
        uint32 groupExpCap = sConfigMgr->GetOption<uint32>("ExtendedXP.GroupXP.Cap", 5);

        if (playersInRange > groupExpCap)
        {
            playersInRange = groupExpCap;
        }

        auto bonusXP = (amount * groupExpMulti) * playersInRange;
        AwardXP(player, victim, bonusXP);
    }
}

void ExtendedXPPlayer::AwardXP(Player* player, Unit* victim, float xp)
{
    if (xp < 1)
    {
        return;
    }

    if (!player->IsAlive())
    {
        return;
    }

    if (player->HasPlayerFlag(PLAYER_FLAGS_NO_XP_GAIN))
    {
        return;
    }

    if (victim && victim->GetTypeId() == TYPEID_UNIT && !victim->ToCreature()->hasLootRecipient())
    {
        return;
    }

    uint8 level = player->GetLevel();
    if (level >= sWorld->getIntConfig(CONFIG_MAX_PLAYER_LEVEL))
    {
        return;
    }

    player->SendLogXPGain(xp, victim, 0, true);

    uint32 curXP = player->GetUInt32Value(PLAYER_XP);
    uint32 nextLvlXP = player->GetUInt32Value(PLAYER_NEXT_LEVEL_XP);
    uint32 newXP = xp;

    while (newXP >= nextLvlXP && level < sWorld->getIntConfig(CONFIG_MAX_PLAYER_LEVEL))
    {
        newXP -= nextLvlXP;

        if (level < sWorld->getIntConfig(CONFIG_MAX_PLAYER_LEVEL))
            player->GiveLevel(level + 1);

        level = player->GetLevel();
        nextLvlXP = player->GetUInt32Value(PLAYER_NEXT_LEVEL_XP);
    }

    player->SetUInt32Value(PLAYER_XP, newXP);
}

void ExtendedXPPlayer::OnAchiComplete(Player* player, AchievementEntry const* achievement)
{
    if (!sConfigMgr->GetOption<bool>("ExtendedXP.Enable", false))
    {
        return;
    }

    if (!sConfigMgr->GetOption<bool>("ExtendedXP.AchievementXP.Enable", false))
    {
        return;
    }

    if (!player)
    {
        return;
    }

    if (!achievement)
    {
        return;
    }

    auto pLevel = player->GetLevel();
    if (pLevel >= sWorld->getIntConfig(CONFIG_MAX_PLAYER_LEVEL))
    {
        return;
    }

    float expPercent = sConfigMgr->GetOption<float>("ExtendedXP.AchievementXP.Percent", 1.5f);
    float expMultiplier = (expPercent * achievement->points) / 100;

    if (sConfigMgr->GetOption<bool>("ExtendedXP.AchievementXP.ScaleLevel", false))
    {
        expMultiplier = ((expMultiplier * 100.0f) * (1.0f - (pLevel / 100.0f))) / 100.0f;
    }

    float xpMax = player->GetUInt32Value(PLAYER_NEXT_LEVEL_XP);
    float xpReward = xpMax * expMultiplier;

    player->GiveXP(xpReward, nullptr);
}

void SC_AddExtendedXPScripts()
{
    new ExtendedXPPlayer();
}
