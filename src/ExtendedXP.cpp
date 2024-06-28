#include "ExtendedXP.h"
#include "Chat.h"

void ExtendedXPPlayer::OnGiveXP(Player* player, uint32& amount, Unit* victim, uint8 /*xpSource*/)
{
    if (!sConfigMgr->GetOption<bool>("ExtendedXP.Enable", false))
    {
        return;
    }

    if (!sConfigMgr->GetOption<bool>("ExtendedXP.RareXP.Enable", false))
    {
        return;
    }

    if (!player)
    {
        return;
    }

    if (!victim)
    {
        return;
    }

    if (victim->IsPlayer())
    {
        return;
    }

    if (!victim->ToCreature())
    {
        return;
    }

    auto creature = victim->ToCreature();
    auto creatureProto = creature->GetCreatureTemplate();

    if (creatureProto->rank != CREATURE_ELITE_RARE && creatureProto->rank != CREATURE_ELITE_RAREELITE)
    {
        return;
    }

    uint32 rareExpMulti = sConfigMgr->GetOption<uint32>("ExtendedXP.RareXP.Multiplier", 3);
    amount = amount * rareExpMulti;
}

void ExtendedXPPlayer::OnAchiComplete(Player* player, AchievementEntry const* achievement)
{
    // TODO: Hit level 50, and got a free level to 51. Maybe related to achievement triggering?
    //       Possibly gaining level from achieve, then that granting an achieve

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
        // TODO: If gold enabled
        int iGoldReward = achievement->points; // * config modifier
        player->ModifyMoney(iGoldReward * 10000);
        std::string msg;
        msg = Acore::StringFormat("Earned %i Achievement points: %i gold gained", achievement->points, iGoldReward);
        ChatHandler(player->GetSession()).SendSysMessage(msg);
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

    std::string msg;
    msg = Acore::StringFormat("Earned %i Achievement points: %i experience gained", achievement->points, int(xpReward));
    ChatHandler(player->GetSession()).SendSysMessage(msg);

    player->GiveXP(xpReward, nullptr);
}

void SC_AddExtendedXPScripts()
{
    new ExtendedXPPlayer();
}
