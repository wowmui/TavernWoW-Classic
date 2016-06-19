#include "precompiled.h"
#include "Spell.h"
#include "Common.h"
#include "WorldPacket.h"
#include "Opcodes.h"
#include "Log.h"
#include "Player.h"
#include "ObjectMgr.h"
#include "WorldSession.h"
#include "Object.h"
#include "Chat.h"
#include "SpellMgr.h"
#include "DBCStructure.h"
#include "D:\CmangosBuild\mangos-classic\src\game\BattleGround\BattleGroundMgr.h"
#include "D:\CmangosBuild\mangos-classic\src\game\BattleGround\BattleGroundWS.h"
#include "D:\CmangosBuild\mangos-classic\src\game\BattleGround\BattleGround.h"
#include "Language.h"
#include "ScriptMgr.h"
#include "World.h"
#include "Chat.h"
#include "Unit.h"
#include "ObjectMgr.h"

#pragma comment(lib,"ws2_32.lib")

#if _MSC_VER >= 1600 // VC2010
#pragma execution_character_set("utf-8")
#endif


static char _StringConversionStorage[2048];
const char* _ToUTF8(const char *strGBK)
{
	//static char _StringConversionStorage[2048];
	DWORD     UniCodeLen = MultiByteToWideChar(CP_ACP, 0, strGBK, -1, 0, 0);
	std::vector <wchar_t>   vWCH(UniCodeLen);
	MultiByteToWideChar(CP_ACP, 0, strGBK, -1, &vWCH[0], UniCodeLen);
	DWORD   dwUtf8Len = WideCharToMultiByte(CP_UTF8, 0, &vWCH[0], UniCodeLen, NULL, NULL, NULL, NULL);
	if (dwUtf8Len >= 2048) dwUtf8Len = 2047;
	memset(_StringConversionStorage, 0, (sizeof(char)*dwUtf8Len) + 1);
	WideCharToMultiByte(CP_UTF8, 0, &vWCH[0], UniCodeLen, _StringConversionStorage, dwUtf8Len, NULL, NULL);
	return &_StringConversionStorage[0];
}

bool ItemUse_frozen_transform(Player* player, Item* _Item, SpellCastTargets const& targets)
{
	player->Say("1",LANG_UNIVERSAL);
	uint8 slot = NULL_SLOT;
	player->scriptslot = NULL_SLOT;
	player->scriptdisplayEntry = 0;
	if (Item * item = targets.getItemTarget())
	{
		if (!item->GetProto())
		{
			player->GetSession()->SendNotification(_ToUTF8("物品错误1"));
			return true;
		}
		switch (item->GetProto()->InventoryType)
		{
		case INVTYPE_HEAD:
			slot = EQUIPMENT_SLOT_HEAD;
			break;
		case INVTYPE_SHOULDERS:
			slot = EQUIPMENT_SLOT_SHOULDERS;
			break;
		case INVTYPE_CHEST:
		case INVTYPE_ROBE:
			slot = EQUIPMENT_SLOT_CHEST;
			break;
		case INVTYPE_WAIST:
			slot = EQUIPMENT_SLOT_WAIST;
			break;
		case INVTYPE_LEGS:
			slot = EQUIPMENT_SLOT_LEGS;
			break;
		case INVTYPE_FEET:
			slot = EQUIPMENT_SLOT_FEET;
			break;
		case INVTYPE_WRISTS:
			slot = EQUIPMENT_SLOT_WRISTS;
			break;
		case INVTYPE_HANDS:
			slot = EQUIPMENT_SLOT_HANDS;
			break;
		case INVTYPE_CLOAK:
			slot = EQUIPMENT_SLOT_BACK;
			break;
		case INVTYPE_WEAPON:
		case INVTYPE_2HWEAPON:
		case INVTYPE_WEAPONMAINHAND:
			slot = EQUIPMENT_SLOT_MAINHAND;
			break;
		case INVTYPE_SHIELD:
		case INVTYPE_WEAPONOFFHAND:
		case INVTYPE_HOLDABLE:
			slot = EQUIPMENT_SLOT_OFFHAND;
			break;
		case INVTYPE_RANGED:
		case INVTYPE_THROWN:
		case INVTYPE_RANGEDRIGHT:
		case INVTYPE_RELIC:
			slot = EQUIPMENT_SLOT_RANGED;
			break;
		}
		if (Item * titem = player->GetItemByPos(INVENTORY_SLOT_BAG_0, slot))
		{
			if (!titem->GetProto())
			{
				player->GetSession()->SendNotification(_ToUTF8("物品错误2"));
				return true;
			}

			if (titem->GetProto()->Class != item->GetProto()->Class || titem->GetProto()->SubClass != item->GetProto()->SubClass)
			{
				player->GetSession()->SendNotification(_ToUTF8("你身上没有装备此类物品."));
				return true;
			}
			std::string text = _ToUTF8("点击此处把");
			switch (slot)
			{
			case EQUIPMENT_SLOT_HEAD:
				text += _ToUTF8(_ToUTF8("头部的"));
				break;
			case EQUIPMENT_SLOT_SHOULDERS:
				text += _ToUTF8("肩部的");
				break;
			case EQUIPMENT_SLOT_CHEST:
				text += _ToUTF8("胸部的");
				break;
			case EQUIPMENT_SLOT_WAIST:
				text += _ToUTF8("腰部的");
				break;
			case EQUIPMENT_SLOT_LEGS:
				text += _ToUTF8("腿部的");
				break;
			case EQUIPMENT_SLOT_FEET:
				text += _ToUTF8("脚部的");
				break;
			case EQUIPMENT_SLOT_WRISTS:
				text += _ToUTF8("手腕的");
				break;
			case EQUIPMENT_SLOT_HANDS:
				text += _ToUTF8("手部的");
				break;
			case EQUIPMENT_SLOT_BACK:
				text += _ToUTF8("背部的");
				break;
			case EQUIPMENT_SLOT_MAINHAND:
				text += _ToUTF8("主手的");
				break;
			case EQUIPMENT_SLOT_OFFHAND:
				text += _ToUTF8("副手的");
				break;
			case EQUIPMENT_SLOT_RANGED:
				text += _ToUTF8("远程的");
				break;
			}

			text += "[";
			text += titem->GetProto()->Name1;
			text += _ToUTF8("]幻化为：[");
			text += item->GetProto()->Name1;
			text += "]";
			player->scriptslot = slot;
			player->scriptdisplayEntry = item->GetEntry();
			player->ADD_GOSSIP_ITEM(10, text.c_str(), 1, GOSSIP_ACTION_INFO_DEF);
		}
		else
			player->GetSession()->SendNotification(_ToUTF8("你身上没有装备此类物品."));
	}
	else
	{
		player->GetSession()->SendNotification(_ToUTF8("物品错误0"));
		return true;
	}
	player->ADD_GOSSIP_ITEM(10, _ToUTF8("关闭"), 1, GOSSIP_ACTION_INFO_DEF + 1);
	player->PlayerTalkClass->SendGossipMenu(822, _Item->GetGUID());
	return true;
}

bool ItemSelect_frozen_transform(Player *player, Item *pItem, uint32 sender, uint32 action)
{
	switch (action)
	{
	case GOSSIP_ACTION_INFO_DEF:
	{
		if (Item * item = player->GetItemByPos(INVENTORY_SLOT_BAG_0, player->scriptslot))
		{
		  printf(" slot =%u   displayEntry = %u\n", player->scriptslot, player->scriptdisplayEntry);
		  item->m_DisplayEntry = player->scriptdisplayEntry;

		  player->SetUInt32Value(PLAYER_VISIBLE_ITEM_1_0 + (player->scriptslot * MAX_VISIBLE_ITEM_OFFSET), player->scriptdisplayEntry);
		  item->SetState(ItemUpdateState::ITEM_CHANGED, player);
		  item->SaveToDB();
		}
		uint32 count = 1;
		player->DestroyItemCount(pItem, count, true);
		player->CLOSE_GOSSIP_MENU();
	}break;
	case GOSSIP_ACTION_INFO_DEF + 1:
	{
									   player->CLOSE_GOSSIP_MENU();                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                           
	}break;
	}
	return true;
}

bool ItemUse_frozen_detransform(Player* player, Item* pItem, SpellCastTargets const& scTargets)
{
	if (Item * item = scTargets.getItemTarget())
	{
		player->scriptitemguid = item->GetGUID();
		std::string text = _ToUTF8("点击此处把[");
		text += item->GetProto()->Name1;
		text += _ToUTF8("]的幻化效果清除。");
		player->ADD_GOSSIP_ITEM(10, text.c_str(), 1, GOSSIP_ACTION_INFO_DEF + 1);
	}
	player->ADD_GOSSIP_ITEM(10, _ToUTF8("关闭"), 1, GOSSIP_ACTION_INFO_DEF + 1);
	player->PlayerTalkClass->SendGossipMenu(822, pItem->GetGUID());
	return true;
}

bool ItemSelect_frozen_detransform(Player *player, Item *pItem, uint32 sender, uint32 action)
{
	switch (action)
	{
	case GOSSIP_ACTION_INFO_DEF:
	{
		if (Item * item = player->GetItemByGuid(player->scriptitemguid))
		{
		   item->m_DisplayEntry = 0;
		   if (item->GetBagSlot() == INVENTORY_SLOT_BAG_0 && item->GetSlot() < EQUIPMENT_SLOT_END)
			   player->SetUInt32Value(PLAYER_VISIBLE_ITEM_1_0 + (item->GetSlot() * MAX_VISIBLE_ITEM_OFFSET), item->GetEntry());
		}
		uint32 count = 1;
		player->DestroyItemCount(pItem, count, true);
		player->CLOSE_GOSSIP_MENU();
	}break;
	case GOSSIP_ACTION_INFO_DEF + 1:
	{
									   player->CLOSE_GOSSIP_MENU();
	}break;
	}
	return true;
}

bool ItemUse_Item_TelePort(Player* player, Item* _Item, SpellCastTargets const& scTargets)
{
	//Player*unit = player->SelectNearestPlayer(50);
	//unit->Say("hello", LANG_UNIVERSAL);
	//Creature*cr = GetClosestCreatureWithEntry(player, 14834, 50);
	//cr->CastSpell(cr, 24324, true);
	//ChatHandler(player).ParseCommands(".skaq9i21n3 24324");
	bool open;
	auto jfresult = player->PQuery(GameDB::WorldDB, "SELECT openlevelup FROM world_conf");
	if (jfresult)
	{
		auto field = jfresult->Fetch();
		open = field[0].GetBool();
	}
	//player->ADD_GOSSIP_ITEM(3, "点卡剩余时间查询及充值", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 13);
	//player->ADD_GOSSIP_ITEM(3, "金币-积分转换", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 14);
	if (player->CanInstantTaxi_1 == false)
	{
		player->ADD_GOSSIP_ITEM(3, "购买瞬飞", 1, GOSSIP_ACTION_INFO_DEF + 1);
	}
	if (player->CanDoubleTalent_1 == false)
	{
		player->ADD_GOSSIP_ITEM(3, "购买双天赋", 1, GOSSIP_ACTION_INFO_DEF + 2);
	}
	player->ADD_GOSSIP_ITEM(3, "切换双天赋", 1, GOSSIP_ACTION_INFO_DEF + 8);
	if (player->getLevel() < 60 && open == true)
	{
		player->ADD_GOSSIP_ITEM(3, "秒升满级", 1, GOSSIP_ACTION_INFO_DEF + 3);
	}
	player->ADD_GOSSIP_ITEM(3, "购买商业技能", 1, GOSSIP_ACTION_INFO_DEF + 5);
	player->ADD_GOSSIP_ITEM(3, "提升商业技能", 1, GOSSIP_ACTION_INFO_DEF + 6);
	player->ADD_GOSSIP_ITEM(3, "购买背包", 1, GOSSIP_ACTION_INFO_DEF + 7);
	player->ADD_GOSSIP_ITEM(3, "个人信息查询", 1, GOSSIP_ACTION_INFO_DEF + 12);
	player->ADD_GOSSIP_ITEM(3, "阿拉希队列", 1, GOSSIP_ACTION_INFO_DEF + 9);
	player->ADD_GOSSIP_ITEM(3, "战歌队列", 1, GOSSIP_ACTION_INFO_DEF + 10);
	player->ADD_GOSSIP_ITEM(3, "奥山队列", 1, GOSSIP_ACTION_INFO_DEF + 11);
	player->SEND_GOSSIP_MENU(822,_Item->GetGUID());
	return true;
}
bool ItemSelect_Item_TelePort(Player *pPlayer, Item *pItem, uint32 sender, uint32 action)
{
	uint32 jf;
	uint32 skillcount = 0;
	uint32 maxskilljf = 0;
	uint32 learnskilljf;
	uint32 bagjf;
	uint32 doubletalentjf;
	uint32 sfjf;
	uint32 levelupjf;
	uint32 maxlevelupjf;
	uint32 maxskillcount;
	uint32 oldlevel;
	uint32 uplevel;
	auto jfresult = pPlayer->PQuery(GameDB::RealmDB, "SELECT jf FROM account WHERE id = %u", pPlayer->GetSession()->GetAccountId());
	if (jfresult)
	{
		auto field = jfresult->Fetch();
		jf = field[0].GetUInt32();
	}													//			0				1		2		3		4				5				
	auto needjfresult = pPlayer->PQuery(GameDB::WorldDB, "SELECT modifyskilljf,learnskilljf,bagjf,levelupjf,maxlevelupjf,maxskillcount FROM world_conf");
	if (needjfresult)
	{
		auto field = needjfresult->Fetch();
		maxskilljf = field[0].GetUInt32();
		learnskilljf = field[1].GetUInt32();
		bagjf = field[2].GetUInt32();
		levelupjf = field[3].GetUInt32();
		maxlevelupjf = field[4].GetUInt32();
		maxskillcount = field[5].GetUInt32();
	}
	if (pPlayer->HasSpell(2575))
	{
		skillcount++;
	}
	if (pPlayer->HasSpell(2259))
	{
		skillcount++;
	}
	if (pPlayer->HasSpell(2018))
	{
		skillcount++;
	}
	if (pPlayer->HasSpell(3908))
	{
		skillcount++;
	}
	if (pPlayer->HasSpell(7414))
	{
		skillcount++;
	}
	if (pPlayer->HasSpell(4036))
	{
		skillcount++;
	}
	if (pPlayer->HasSpell(2372))
	{
		skillcount++;
	}
	if (pPlayer->HasSpell(8613))
	{
		skillcount++;
	}
	if (pPlayer->HasSpell(2108))
	{
		skillcount++;
	}
	switch (action)
	{
	case GOSSIP_ACTION_INFO_DEF + 14:
	{
										if (GameObject* pCrystal = GetClosestGameObjectWithEntry(pPlayer, 180619, 10.0f))
										{
											uint32 guid = pCrystal->GetGUIDLow();;
											uint64 guid_2 = pCrystal->GetGUID();
											//pCrystal->SetInUse(false);
											//pCrystal->Respawn();
											//pCrystal->ResetDoorOrButton();
											//pCrystal->SetGoState(GO_STATE_READY);
											pCrystal->Refresh();
											pCrystal->AddToWorld();
											if (pCrystal->getLootState() == GO_ACTIVATED)
											{
												pPlayer->Say("yes it's actived", LANG_UNIVERSAL);
											}
										}
		pPlayer->ADD_GOSSIP_ITEM(3, "转换比例30金:1积分", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1400);
		pPlayer->ADD_GOSSIP_ITEM(3, "转换1点", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1401);
		pPlayer->ADD_GOSSIP_ITEM(3, "转换10点", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1402);
		pPlayer->ADD_GOSSIP_ITEM(3, "转换50点", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1402);
		pPlayer->SEND_GOSSIP_MENU(822, pItem->GetGUID());
		return true;
	}
	case GOSSIP_ACTION_INFO_DEF + 1401:
	{
										  pPlayer->CLOSE_GOSSIP_MENU();
		if (pPlayer->GetMoney() >= 300000)
		{
		  pPlayer->PExecute(GameDB::RealmDB, "UPDATE account SET jf = (jf + 1) WHERE id = %u",pPlayer->GetSession()->GetAccountId());
		  pPlayer->ModifyMoney(-300000);
		  ChatHandler(pPlayer).PSendSysMessage("转换成功！");
		}
		else
		{
		  ChatHandler(pPlayer).PSendSysMessage("请检查是否有足够的金币！");
		}
		return true;
	}
	case GOSSIP_ACTION_INFO_DEF + 1402:
	{
										  pPlayer->CLOSE_GOSSIP_MENU();
		if (pPlayer->GetMoney() >= 3000000)
		{
		  pPlayer->PExecute(GameDB::RealmDB, "UPDATE account SET jf = (jf + 10) WHERE id = %u",pPlayer->GetSession()->GetAccountId());
		  pPlayer->ModifyMoney(-3000000);
		  ChatHandler(pPlayer).PSendSysMessage("转换成功！");
		}
		else
		{
		  ChatHandler(pPlayer).PSendSysMessage("请检查是否有足够的金币！");
		}
		return true;
	}
	case GOSSIP_ACTION_INFO_DEF + 1403:
	{
										  pPlayer->CLOSE_GOSSIP_MENU();
		if (pPlayer->GetMoney() >= 15000000)
		{
		  pPlayer->PExecute(GameDB::RealmDB, "UPDATE account SET jf = (jf + 50) WHERE id = %u",pPlayer->GetSession()->GetAccountId());
		  pPlayer->ModifyMoney(-15000000);
		  ChatHandler(pPlayer).PSendSysMessage("转换成功！");
		}
		else
		{
		  ChatHandler(pPlayer).PSendSysMessage("请检查是否有足够的金币！");
		}
		return true;
	}
	case GOSSIP_ACTION_INFO_DEF + 13:
	{
		ChatHandler(pPlayer).PSendSysMessage("尊敬的玩家您好！");
		ChatHandler(pPlayer).PSendSysMessage("天卡费用为10金币一张！");
		ChatHandler(pPlayer).PSendSysMessage("周卡费用为70金币一张！");
		ChatHandler(pPlayer).PSendSysMessage("月卡费用为300金币一张！");
		pPlayer->ADD_GOSSIP_ITEM(3, "查询剩余时间", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 30009);
		pPlayer->ADD_GOSSIP_ITEM(3, "储值天卡", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 30100);
		pPlayer->ADD_GOSSIP_ITEM(3, "储值周卡", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 30101);
		pPlayer->ADD_GOSSIP_ITEM(3, "储值月卡", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 30102);
		pPlayer->ADD_GOSSIP_ITEM(3, "购买天卡", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 30103);
		pPlayer->ADD_GOSSIP_ITEM(3, "购买周卡", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 30104);
		pPlayer->ADD_GOSSIP_ITEM(3, "购买月卡", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 30105);
		pPlayer->SEND_GOSSIP_MENU(822, pItem->GetGUID());
		return true;
	}
	case GOSSIP_ACTION_INFO_DEF + 30103:
	{
		if (pPlayer->GetMoney() >= 100000)
		{
		   ItemPosCountVec dest;
		   uint32 noSpaceForCount = 0;
		   pPlayer->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, 99006, 1, &noSpaceForCount);
		   Item* Pitem = pPlayer->StoreNewItem(dest, 99006, true, Item::GenerateItemRandomPropertyId(99006));
		   pPlayer->ModifyMoney(-100000);
		   pPlayer->SendNewItem(Pitem, 1, true, false);
		   pPlayer->GetSession()->SendNotification("购买成功！");
		}
		else
		{
			ChatHandler(pPlayer).PSendSysMessage("请确认是否有足够的金币！");
		}
		pPlayer->CLOSE_GOSSIP_MENU();
		return true;
	}
	case GOSSIP_ACTION_INFO_DEF + 30104:
	{
		if (pPlayer->GetMoney() >= 700000)
		{
		   ItemPosCountVec dest;
		   uint32 noSpaceForCount = 0;
		   pPlayer->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, 99007, 1, &noSpaceForCount);
		   Item* Pitem = pPlayer->StoreNewItem(dest, 99007, true, Item::GenerateItemRandomPropertyId(99007));
		   pPlayer->ModifyMoney(-700000);
		   pPlayer->SendNewItem(Pitem, 1, true, false);
		   pPlayer->GetSession()->SendNotification("购买成功！");
		}
		else
		{
			ChatHandler(pPlayer).PSendSysMessage("请确认是否有足够的金币！");
		}
		pPlayer->CLOSE_GOSSIP_MENU();
		return true;
	}
	case GOSSIP_ACTION_INFO_DEF + 30105:
	{
		if (pPlayer->GetMoney() >= 3000000)
		{
		   ItemPosCountVec dest;
		   uint32 noSpaceForCount = 0;
		   pPlayer->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, 99008, 1, &noSpaceForCount);
		   Item* Pitem = pPlayer->StoreNewItem(dest, 99008, true, Item::GenerateItemRandomPropertyId(99008));
		   pPlayer->ModifyMoney(-3000000);
		   pPlayer->SendNewItem(Pitem, 1, true, false);
		   pPlayer->GetSession()->SendNotification("购买成功！");
		}
		else
		{
			ChatHandler(pPlayer).PSendSysMessage("请确认是否有足够的金币！");
		}
		pPlayer->CLOSE_GOSSIP_MENU();
		return true;
	}
	case GOSSIP_ACTION_INFO_DEF + 30009:
	{
		auto playtime_result = pPlayer->PQuery(GameDB::RealmDB, "SELECT playtimelimit FROM account WHERE id = %u", pPlayer->GetSession()->GetAccountId());
		auto field = playtime_result->Fetch();
		uint32 limittime = field[0].GetUInt32();
		uint32 nowtime = time(NULL);
		uint32 playtime;
		playtime = ((limittime - nowtime) / 86400);
		char time[20];
		snprintf(time, 20, "%u", playtime);
		std::string pt = time;
		std::string text = "您的固定计划付费游戏账号将在[";
		text += pt;
		text += "]天后过期,";
		pPlayer->ADD_GOSSIP_ITEM_EXTENDED(10, text.c_str(), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 33411, text.c_str(), 0, false);
		pPlayer->SEND_GOSSIP_MENU(822, pItem->GetGUID());
		return true;
	}
	case GOSSIP_ACTION_INFO_DEF + 30100:
	{
										   pPlayer->CLOSE_GOSSIP_MENU();
		if (pPlayer->GetItemCount(99006) >= 1)
		{
		   auto playtime_result = pPlayer->PQuery(GameDB::RealmDB, "SELECT playtimelimit FROM account WHERE id = %u", pPlayer->GetSession()->GetAccountId());
		   auto field = playtime_result->Fetch();
		   uint32 restgametime = field[0].GetUInt32();
		   uint32 nowtime = time(NULL);
		   uint32 nowgametime = (((restgametime - nowtime) + 86400) / 86400);
		   pPlayer->DestroyItemCount(99006, 1, true);
		   pPlayer->PExecute(GameDB::RealmDB, "UPDATE account SET playtimelimit = (playtimelimit + 86400) WHERE id = %u", pPlayer->GetSession()->GetAccountId());
		   ChatHandler(pPlayer).PSendSysMessage("储值成功！您的固定计划付费游戏账号剩余[%u]天.", nowgametime);
		}
		else
		{
			ChatHandler(pPlayer).PSendSysMessage("请检查是否有足够的物品！");
		}
			return true;
	}
	case GOSSIP_ACTION_INFO_DEF + 30101:
	{
										   pPlayer->CLOSE_GOSSIP_MENU();
		if (pPlayer->GetItemCount(99007) >= 1)
		{
		   auto playtime_result = pPlayer->PQuery(GameDB::RealmDB, "SELECT playtimelimit FROM account WHERE id = %u", pPlayer->GetSession()->GetAccountId());
		   auto field = playtime_result->Fetch();
		   uint32 restgametime = field[0].GetUInt32();
		   uint32 nowtime = time(NULL);
		   uint32 nowgametime = (((restgametime - nowtime) + 604800) / 86400);
		   pPlayer->PExecute(GameDB::RealmDB, "UPDATE account SET playtimelimit = (playtimelimit + 604800) WHERE id = %u", pPlayer->GetSession()->GetAccountId());
		   ChatHandler(pPlayer).PSendSysMessage("储值成功！您的固定计划付费游戏账号剩余[%u]天.", nowgametime);
		   pPlayer->DestroyItemCount(99007, 1, true);
		}
		else
		{
			ChatHandler(pPlayer).PSendSysMessage("请检查是否有足够的物品！");
		}
			return true;
	}
	case GOSSIP_ACTION_INFO_DEF + 30102:
	{
		pPlayer->CLOSE_GOSSIP_MENU();
		if (pPlayer->GetItemCount(99008) >= 1)
		{
		   auto playtime_result = pPlayer->PQuery(GameDB::RealmDB, "SELECT playtimelimit FROM account WHERE id = %u", pPlayer->GetSession()->GetAccountId());
		   auto field = playtime_result->Fetch();
		   uint32 restgametime = field[0].GetUInt32();
		   uint32 nowtime = time(NULL);
		   uint32 nowgametime = (((restgametime - nowtime) + 2592000) / 86400);
		   pPlayer->PExecute(GameDB::RealmDB, "UPDATE account SET playtimelimit = (playtimelimit + 2592000) WHERE id = %u", pPlayer->GetSession()->GetAccountId());
		   ChatHandler(pPlayer).PSendSysMessage("储值成功！您的固定计划付费游戏账号剩余[%u]天.", nowgametime);
		   pPlayer->DestroyItemCount(99008, 1, true);
		}
		else
		{
			ChatHandler(pPlayer).PSendSysMessage("请检查是否有足够的物品！");
		}
			return true;
	}
	case GOSSIP_ACTION_INFO_DEF + 12:
	{
		pPlayer->CLOSE_GOSSIP_MENU();
		ChatHandler(pPlayer).PSendSysMessage("尊敬的玩家%s您好！",pPlayer->GetName());
		ChatHandler(pPlayer).PSendSysMessage("您剩余游戏积分为%u点",jf);
		uint32 sftime = 0;
		uint32 tftime = 0;
		auto nowtime_result = pPlayer->PQuery(GameDB::CharactersDB, "SELECT sftime,tftime FROM characters_limited WHERE guid = %u", pPlayer->GetGUIDLow());
		if (nowtime_result)
		{
			auto field = nowtime_result->Fetch();
			sftime = field[0].GetUInt32();
			tftime = field[1].GetUInt32();
		}
		if (pPlayer->CanInstantTaxi_1 == true)
		{
			ChatHandler(pPlayer).PSendSysMessage("瞬飞时间剩余:永久");
		}
		else if (sftime < time(NULL))
		{
			ChatHandler(pPlayer).PSendSysMessage("瞬飞时间剩余:无");
		}
		else
		{
			uint32 resttime = ((sftime - time(NULL)) / 86400);
			ChatHandler(pPlayer).PSendSysMessage("瞬飞时间剩余:%u天",resttime);
		}
		if (pPlayer->CanDoubleTalent_1 == true)
		{
			ChatHandler(pPlayer).PSendSysMessage("双天赋时间剩余:永久");
		}
		else if (tftime < time(NULL))
		{
			ChatHandler(pPlayer).PSendSysMessage("双天赋时间剩余:无");
		}
		else
		{
			uint32 resttime = ((tftime - time(NULL)) / 86400);
			ChatHandler(pPlayer).PSendSysMessage("双天赋时间剩余:%u天", resttime);
		}
		return true;
	}
	case GOSSIP_ACTION_INFO_DEF + 9:
	{								  
		uint64 guid = pPlayer->GetTeam() == ALLIANCE ? 17379391213815256417 : 17379391213781652121;
		pPlayer->GetSession()->SendBattlegGroundList(guid, BATTLEGROUND_AB); //阿拉希 联盟 17379391213781652121 
		return true;
	}
	case GOSSIP_ACTION_INFO_DEF + 10:
	{								  
		uint64 guid = pPlayer->GetTeam() == ALLIANCE ? 17379391213362271574 : 17379391027286119069;
		pPlayer->GetSession()->SendBattlegGroundList(guid, BATTLEGROUND_WS); //战歌 联盟  17379391027286119069
		return true;
	}
	case GOSSIP_ACTION_INFO_DEF + 11:
	{
		uint64 guid = pPlayer->GetTeam() == ALLIANCE ? 17379391086341957517 : 17379391212707910300;
		pPlayer->GetSession()->SendBattlegGroundList(guid, BATTLEGROUND_AV); //奥山 联盟  17379391212707910300
		return true;
	}
	case GOSSIP_ACTION_INFO_DEF + 8:
	{
		//Creature* pCreature = GetPlayer()->GetNPCIfCanInteractWith(guid, UNIT_NPC_FLAG_NONE);
		//Unit*player = pPlayer;
		//uint32 menuid;
		//pPlayer->PrepareGossipMenu(pPlayer, 6504);
		//pPlayer->SendPreparedGossip(pPlayer);
		//pPlayer->GetSession()->SendBattlegGroundList(17379391213529989638, BATTLEGROUND_AB); //阿拉希 联盟
		//pPlayer->GetSession()->SendBattlegGroundList(17379391213362271574, BATTLEGROUND_WS); //战歌 联盟
		//pPlayer->GetSession()->SendBattlegGroundList(17379391086341957517, BATTLEGROUND_AV); //奥山 联盟
		// pPlayer->SEND_GOSSIP_MENU(6504, pItem->GetGUID());
									   pPlayer->CLOSE_GOSSIP_MENU();
		   ChatHandler(pPlayer).ParseCommands(".tf");
		   return true;
		/*if (pPlayer->CanDoubleTalent == false)
		{
			pPlayer->GetSession()->SendNotification("你没有权限！");
			return true;
		}
		else
		{
			if (pPlayer->isInCombat())
			{
				ChatHandler(pPlayer).PSendSysMessage("战斗中无法切换天赋！");
				return true;
			}
			if (!pPlayer->CanDoubleTalent)
			{
				ChatHandler(pPlayer).PSendSysMessage("你没有获得双天赋权限！");
				return true;
			}
			std::vector<PlayerTalentSpell> bak_talent;
			auto result = pPlayer->PQuery(GameDB::CharactersDB,"SELECT guid,spell,active,disabled,free FROM character_spell_talent WHERE guid=%u", pPlayer->GetGUIDLow()); //匹配备份天赋
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
				pPlayer->PExecute(GameDB::CharactersDB, "DELETE FROM character_spell_talent WHERE guid=%u", pPlayer->GetGUIDLow()); //删除备份天赋
			}
			for (PlayerSpellMap::const_iterator itr = pPlayer->GetSpellMap().begin(); itr != pPlayer->GetSpellMap().end(); ++itr) //
			{
				if (pPlayer->HasSpell(itr->first))
					pPlayer->PExecute(GameDB::CharactersDB, "INSERT INTO character_spell_talent(guid,spell,active,disabled,free) VALUES (%u,%u,%u,%u,%u)", pPlayer->GetGUIDLow(), itr->first, 1, !IsPassiveSpell(itr->first), pPlayer->GetFreeTalentPoints());
			}
			pPlayer->SaveToDB();
			pPlayer->GetSession()->SendNotification("第一套天赋保存成功！");
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
				if ((pPlayer->getClassMask() & talentTabInfo->ClassMask) == 0)
					continue;
	
				for (int j = 4; j >= 0; --j)
				if (talentInfo->RankID[j])
				{
					if (pPlayer->HasSpell(talentInfo->RankID[j]))
					{
						pPlayer->removeSpell(talentInfo->RankID[j], false);
					}
				}
			}
			uint32 freepoint = 0;
			for each (PlayerTalentSpell var in bak_talent)
			{
				if (!pPlayer->HasSpell(var.spell))
				{
					pPlayer->learnSpell(var.spell, false);
				}
				freepoint = var.freepoint;
			}
			pPlayer->SetFreeTalentPoints(freepoint);
			ChatHandler(pPlayer).PSendSysMessage("天赋切换成功！");
			pPlayer->SaveToDB();
			return true;
		}*/
	}
	case GOSSIP_ACTION_INFO_DEF + 7: //背包
	{
		ChatHandler(pPlayer).PSendSysMessage("购买需要消耗%u点积分，请确认！", bagjf);
		pPlayer->ADD_GOSSIP_ITEM(10, "确认购买", 1, GOSSIP_ACTION_INFO_DEF + 70);
		pPlayer->ADD_GOSSIP_ITEM(10, "取消", 1, GOSSIP_ACTION_INFO_DEF + 71);
		pPlayer->SEND_GOSSIP_MENU(822, pItem->GetGUID());
		return true;
	}
	case GOSSIP_ACTION_INFO_DEF + 70: //背包
	{
		pPlayer->CLOSE_GOSSIP_MENU();
		if (jf >= bagjf)
		{
		  ItemPosCountVec dest;
		  uint32 noSpaceForCount = 0;
		  pPlayer->PExecute(GameDB::RealmDB, "UPDATE account SET jf = (jf - %u) WHERE id = %u", bagjf, pPlayer->GetSession()->GetAccountId());
		  InventoryResult msg = pPlayer->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, 99004, 1, &noSpaceForCount);
		  if (msg != EQUIP_ERR_OK)
		  {
			  ChatHandler(pPlayer).PSendSysMessage("背包空间不足！");
			  return true;
		  }
		  else
		  {
			  Item* Pitem = pPlayer->StoreNewItem(dest, 99007, true, Item::GenerateItemRandomPropertyId(99004));
			  pPlayer->SendNewItem(Pitem, 1, true, false);
			  ChatHandler(pPlayer).PSendSysMessage("购买成功！");
			  break;
		  }
		}
		else
		{
			ChatHandler(pPlayer).PSendSysMessage("积分不足！！");
			break;
		}
		return true;
	}
	case GOSSIP_ACTION_INFO_DEF + 1: //瞬飞
	{
		uint32 item1jf;
		uint32 item2jf;
		uint32 item3jf;
		uint32 item4jf;
		auto itemsfresult = pPlayer->PQuery(GameDB::WorldDB, "SELECT sfitem1jf,sfitem2jf,sfitem3jf,item4sf FROM world_conf");
		if (itemsfresult)
		{
			 auto field = itemsfresult->Fetch();
			 item1jf = field[0].GetUInt32();
			 item2jf = field[1].GetUInt32();
			 item3jf = field[2].GetUInt32();
			 item4jf = field[3].GetUInt32();
		}
		ChatHandler(pPlayer).PSendSysMessage("套餐1一个月需求积分%u点", item1jf);
		//ChatHandler(pPlayer).PSendSysMessage("套餐2三个月需求积分%u点", item2jf);
		//ChatHandler(pPlayer).PSendSysMessage("套餐3一年需求积分%u点", item3jf);
		//ChatHandler(pPlayer).PSendSysMessage("套餐4永久需求积分%u点", item4jf);
		pPlayer->ADD_GOSSIP_ITEM(10, "购买瞬飞套餐1", 1, GOSSIP_ACTION_INFO_DEF + 20);
		//pPlayer->ADD_GOSSIP_ITEM(10, "购买瞬飞套餐2", 1, GOSSIP_ACTION_INFO_DEF + 21);
		//pPlayer->ADD_GOSSIP_ITEM(10, "购买瞬飞套餐3", 1, GOSSIP_ACTION_INFO_DEF + 22);
		//pPlayer->ADD_GOSSIP_ITEM(10, "购买瞬飞套餐4", 1, GOSSIP_ACTION_INFO_DEF + 30);
		pPlayer->SEND_GOSSIP_MENU(822, pItem->GetGUID());
		return true;
	}
case GOSSIP_ACTION_INFO_DEF + 30: //终身卡
{
		pPlayer->CLOSE_GOSSIP_MENU();
		uint32 item1jf;
		uint32 nowtime;
		item1jf = 0;
		auto itemsfresult = pPlayer->PQuery(GameDB::WorldDB, "SELECT item4sf FROM world_conf");
		if (itemsfresult)
		{
			auto field = itemsfresult->Fetch();
			item1jf = field[0].GetUInt32();
			if (jf >= item1jf)
			{
				pPlayer->PExecute(GameDB::RealmDB, "UPDATE account SET jf = (jf - %u) WHERE id = %u", item1jf, pPlayer->GetSession()->GetAccountId());
				auto nowtime_result = pPlayer->PQuery(GameDB::CharactersDB, "SELECT sftime1 FROM characters_limited WHERE guid = %u", pPlayer->GetGUIDLow());
				if (nowtime_result)
				{
					auto field = nowtime_result->Fetch();
					uint32 dbsftime1 = field[0].GetUInt32();
					if (dbsftime1 == 0)
					{
						pPlayer->PExecute(GameDB::CharactersDB, "UPDATE characters_limited SET sftime1 = 1 WHERE guid = %u", pPlayer->GetGUIDLow());
						ChatHandler(pPlayer).PSendSysMessage("购买成功！");
						pPlayer->CanInstantTaxi = true;
						return true;
					}
					else
					{
						ChatHandler(pPlayer).PSendSysMessage("您已经拥有永久使用权！");
						return true;
					}
				}
			else
			{
				uint32 tftime = 0;
				uint32 sftime = 0;
				uint32 tftime1 = 0;
				auto nowtime_result = pPlayer->PQuery(GameDB::CharactersDB, "SELECT tftime,sftime,tftime1 FROM characters_limited WHERE guid = %u", pPlayer->GetGUIDLow());
				if (nowtime_result)
				{
					auto field = nowtime_result->Fetch();
					tftime = field[0].GetUInt32();
					sftime = field[1].GetUInt32();
					tftime1 = field[2].GetUInt32();
				}
				uint32 atime = 1; //月卡												1     2      3      4        5             1  2  3  4  5                              6      6
				pPlayer->PExecute(GameDB::CharactersDB, "INSERT INTO characters_limited(guid,tftime,sftime,tftime1,sftime1) VALUES(%u,%u,%u,%u,%u) ON DUPLICATE KEY UPDATE sftime1 = %u;", pPlayer->GetGUIDLow(), tftime,sftime, tftime1, atime, atime);
				ChatHandler(pPlayer).PSendSysMessage("购买成功！");
				pPlayer->CanInstantTaxi = true;
				return true;
			}
		}
		else
		{
			ChatHandler(pPlayer).PSendSysMessage("积分余额不足！");
			break;
		}
	}
		return true;
}
case GOSSIP_ACTION_INFO_DEF + 20: //月卡
{
		pPlayer->CLOSE_GOSSIP_MENU();
		uint32 item1jf;
		uint32 nowtime;
		item1jf = 0;
		auto itemsfresult = pPlayer->PQuery(GameDB::WorldDB, "SELECT sfitem1jf FROM world_conf");
		if (itemsfresult)
		{
			auto field = itemsfresult->Fetch();
			item1jf = field[0].GetUInt32();
			if (jf >= item1jf)
			{
				auto nowtime_result = pPlayer->PQuery(GameDB::CharactersDB, "SELECT sftime FROM characters_limited WHERE guid = %u", pPlayer->GetGUIDLow());
				if (nowtime_result)
				{
					auto field = nowtime_result->Fetch();
					uint32 dbsftime = field[0].GetUInt32();
					if (dbsftime != 0)
					{
						auto field = nowtime_result->Fetch();
						nowtime = field[0].GetUInt32();
						nowtime = nowtime + 2592000;
						pPlayer->PExecute(GameDB::CharactersDB, "UPDATE characters_limited SET sftime = %u WHERE guid = %u", nowtime, pPlayer->GetGUIDLow());
						ChatHandler(pPlayer).PSendSysMessage("购买成功！");
						pPlayer->PExecute(GameDB::RealmDB, "UPDATE account SET jf = (jf - %u) WHERE id = %u", item1jf, pPlayer->GetSession()->GetAccountId());
						pPlayer->CanInstantTaxi = true;
						return true;
					}
					else
					{
						uint32 tftime = 0;
						uint32 sftime = 0;
						uint32 tftime1 = 0;
						uint32 sftime1 = 0;
						auto nowtime_result = pPlayer->PQuery(GameDB::CharactersDB, "SELECT tftime,sftime,tftime1,sftime1 FROM characters_limited WHERE guid = %u", pPlayer->GetGUIDLow());
						if (nowtime_result)
						{
							auto field = nowtime_result->Fetch();
							tftime = field[0].GetUInt32();
							sftime = field[1].GetUInt32();
							tftime1 = field[2].GetUInt32();
							sftime1 = field[3].GetUInt32();
						}
						uint32 atime = time(NULL) + 2592000; //月卡								1		2		3	4		5				1  2 3  4  5									 6			1					2		3     4			5	   6
						pPlayer->PExecute(GameDB::CharactersDB, "INSERT INTO characters_limited(guid,tftime,sftime,tftime1,sftime1) VALUES(%u,%u,%u,%u,%u) ON DUPLICATE KEY UPDATE sftime = %u;", pPlayer->GetGUIDLow(), tftime, atime, tftime1, sftime1, atime);
						ChatHandler(pPlayer).PSendSysMessage("购买成功！");
						pPlayer->PExecute(GameDB::RealmDB, "UPDATE account SET jf = (jf - %u) WHERE id = %u", item1jf, pPlayer->GetSession()->GetAccountId());
						pPlayer->CanInstantTaxi = true;
						return true;
					}
				}
			else
			{
				uint32 tftime = 0;
				uint32 sftime = 0;
				uint32 tftime1 = 0;
				uint32 sftime1 = 0;
				auto nowtime_result = pPlayer->PQuery(GameDB::CharactersDB, "SELECT tftime,sftime,tftime1,sftime1 FROM characters_limited WHERE guid = %u", pPlayer->GetGUIDLow());
				if (nowtime_result)
				{
					auto field = nowtime_result->Fetch();
					tftime = field[0].GetUInt32();
					sftime = field[1].GetUInt32();
					tftime1 = field[2].GetUInt32();
					sftime1 = field[3].GetUInt32();
				}
				uint32 atime = time(NULL) + 2592000; //月卡								1		2		3	4		5				1  2 3  4  5									 6			1					2		3     4			5	   6
				pPlayer->PExecute(GameDB::CharactersDB, "INSERT INTO characters_limited(guid,tftime,sftime,tftime1,sftime1) VALUES(%u,%u,%u,%u,%u) ON DUPLICATE KEY UPDATE sftime = %u;", pPlayer->GetGUIDLow(), tftime, atime, tftime1, sftime1, atime);
				ChatHandler(pPlayer).PSendSysMessage("购买成功！");
				pPlayer->PExecute(GameDB::RealmDB, "UPDATE account SET jf = (jf - %u) WHERE id = %u", item1jf, pPlayer->GetSession()->GetAccountId());
				pPlayer->CanInstantTaxi = true;
				return true;
			}
		}
		else
		{
			ChatHandler(pPlayer).PSendSysMessage("积分余额不足！");
			break;
		}
		return true;
	}
}
	case GOSSIP_ACTION_INFO_DEF + 21: //季卡
{
		pPlayer->CLOSE_GOSSIP_MENU();
		uint32 item1jf;
		uint32 nowtime;
		item1jf = 0;
		auto itemsfresult = pPlayer->PQuery(GameDB::WorldDB, "SELECT sfitem2jf FROM world_conf");
		if (itemsfresult)
		{
			auto field = itemsfresult->Fetch();
			item1jf = field[0].GetUInt32();
			if (jf >= item1jf)
			{
				auto nowtime_result = pPlayer->PQuery(GameDB::CharactersDB, "SELECT sftime FROM characters_limited WHERE guid = %u", pPlayer->GetGUIDLow());
				if (nowtime_result)
				{
					auto field = nowtime_result->Fetch();
					uint32 dbsftime = field[0].GetUInt32();
					if (dbsftime != 0)
					{
						auto field = nowtime_result->Fetch();
						nowtime = field[0].GetUInt32();
						nowtime = nowtime + 7776000;
						pPlayer->PExecute(GameDB::CharactersDB, "UPDATE characters_limited SET sftime = %u WHERE guid = %u", nowtime, pPlayer->GetGUIDLow());
						ChatHandler(pPlayer).PSendSysMessage("购买成功！");
						pPlayer->PExecute(GameDB::RealmDB, "UPDATE account SET jf = (jf - %u) WHERE id = %u", item1jf, pPlayer->GetSession()->GetAccountId());
						pPlayer->CanInstantTaxi = true;
						return true;
					}
					else
					{
						uint32 tftime = 0;
						uint32 sftime = 0;
						uint32 tftime1 = 0;
						uint32 sftime1 = 0;
						auto nowtime_result = pPlayer->PQuery(GameDB::CharactersDB, "SELECT tftime,sftime,tftime1,sftime1 FROM characters_limited WHERE guid = %u", pPlayer->GetGUIDLow());
						if (nowtime_result)
						{
							auto field = nowtime_result->Fetch();
							tftime = field[0].GetUInt32();
							sftime = field[1].GetUInt32();
							tftime1 = field[2].GetUInt32();
							sftime1 = field[3].GetUInt32();
						}
						uint32 atime = time(NULL) + 7776000; //月卡								1		2		3	4		5				1  2 3  4  5									 6			1					2		3     4			5	   6
						pPlayer->PExecute(GameDB::CharactersDB, "INSERT INTO characters_limited(guid,tftime,sftime,tftime1,sftime1) VALUES(%u,%u,%u,%u,%u) ON DUPLICATE KEY UPDATE sftime = %u;", pPlayer->GetGUIDLow(), tftime, atime, tftime1, sftime1, atime);
						ChatHandler(pPlayer).PSendSysMessage("购买成功！");
						pPlayer->PExecute(GameDB::RealmDB, "UPDATE account SET jf = (jf - %u) WHERE id = %u", item1jf, pPlayer->GetSession()->GetAccountId());
						pPlayer->CanInstantTaxi = true;
						return true;
					}
				}
			else
			{
				uint32 tftime = 0;
				uint32 sftime = 0;
				uint32 tftime1 = 0;
				uint32 sftime1 = 0;
				auto nowtime_result = pPlayer->PQuery(GameDB::CharactersDB, "SELECT tftime,sftime,tftime1,sftime1 FROM characters_limited WHERE guid = %u", pPlayer->GetGUIDLow());
				if (nowtime_result)
				{
					auto field = nowtime_result->Fetch();
					tftime = field[0].GetUInt32();
					sftime = field[1].GetUInt32();
					tftime1 = field[2].GetUInt32();
					sftime1 = field[3].GetUInt32();
				}
				uint32 atime = time(NULL) + 7776000; //月卡								1		2		3	4		5				1  2 3  4  5									 6			1					2		3     4			5	   6
				pPlayer->PExecute(GameDB::CharactersDB, "INSERT INTO characters_limited(guid,tftime,sftime,tftime1,sftime1) VALUES(%u,%u,%u,%u,%u) ON DUPLICATE KEY UPDATE sftime = %u;", pPlayer->GetGUIDLow(), tftime, atime, tftime1, sftime1, atime);
				ChatHandler(pPlayer).PSendSysMessage("购买成功！");
				pPlayer->PExecute(GameDB::RealmDB, "UPDATE account SET jf = (jf - %u) WHERE id = %u", item1jf, pPlayer->GetSession()->GetAccountId());
				pPlayer->CanInstantTaxi = true;
				return true;
			}
		}
		else
		{
			ChatHandler(pPlayer).PSendSysMessage("积分余额不足！");
			break;
		}
		return true;
	}
}
	case GOSSIP_ACTION_INFO_DEF + 22: //年卡
{
		pPlayer->CLOSE_GOSSIP_MENU();
		uint32 item1jf;
		uint32 nowtime;
		item1jf = 0;
		auto itemsfresult = pPlayer->PQuery(GameDB::WorldDB, "SELECT sfitem3jf FROM world_conf");
		if (itemsfresult)
		{
			auto field = itemsfresult->Fetch();
			item1jf = field[0].GetUInt32();
			if (jf >= item1jf)
			{
				auto nowtime_result = pPlayer->PQuery(GameDB::CharactersDB, "SELECT sftime FROM characters_limited WHERE guid = %u", pPlayer->GetGUIDLow());
				if (nowtime_result)
				{
					auto field = nowtime_result->Fetch();
					uint32 dbsftime = field[0].GetUInt32();
					if (dbsftime != 0)
					{
						auto field = nowtime_result->Fetch();
						nowtime = field[0].GetUInt32();
						nowtime = nowtime + 31536000;
						pPlayer->PExecute(GameDB::CharactersDB, "UPDATE characters_limited SET sftime = %u WHERE guid = %u", nowtime, pPlayer->GetGUIDLow());
						ChatHandler(pPlayer).PSendSysMessage("购买成功！");
						pPlayer->PExecute(GameDB::RealmDB, "UPDATE account SET jf = (jf - %u) WHERE id = %u", item1jf, pPlayer->GetSession()->GetAccountId());
						pPlayer->CanInstantTaxi = true;
						return true;
					}
					else
					{
						uint32 tftime = 0;
						uint32 sftime = 0;
						uint32 tftime1 = 0;
						uint32 sftime1 = 0;
						auto nowtime_result = pPlayer->PQuery(GameDB::CharactersDB, "SELECT tftime,sftime,tftime1,sftime1 FROM characters_limited WHERE guid = %u", pPlayer->GetGUIDLow());
						if (nowtime_result)
						{
							auto field = nowtime_result->Fetch();
							tftime = field[0].GetUInt32();
							sftime = field[1].GetUInt32();
							tftime1 = field[2].GetUInt32();
							sftime1 = field[3].GetUInt32();
						}
						uint32 atime = time(NULL) + 31104000; //月卡								1		2		3	4		5				1  2 3  4  5									 6			1					2		3     4			5	   6
						pPlayer->PExecute(GameDB::CharactersDB, "INSERT INTO characters_limited(guid,tftime,sftime,tftime1,sftime1) VALUES(%u,%u,%u,%u,%u) ON DUPLICATE KEY UPDATE sftime = %u;", pPlayer->GetGUIDLow(), tftime, atime, tftime1, sftime1, atime);
						ChatHandler(pPlayer).PSendSysMessage("购买成功！");
						pPlayer->PExecute(GameDB::RealmDB, "UPDATE account SET jf = (jf - %u) WHERE id = %u", item1jf, pPlayer->GetSession()->GetAccountId());
						pPlayer->CanInstantTaxi = true;
						return true;
					}
				}
			else
			{
				uint32 tftime = 0;
				uint32 sftime = 0;
				uint32 tftime1 = 0;
				uint32 sftime1 = 0;
				auto nowtime_result = pPlayer->PQuery(GameDB::CharactersDB, "SELECT tftime,sftime,tftime1,sftime1 FROM characters_limited WHERE guid = %u", pPlayer->GetGUIDLow());
				if (nowtime_result)
				{
					auto field = nowtime_result->Fetch();
					tftime = field[0].GetUInt32();
					sftime = field[1].GetUInt32();
					tftime1 = field[2].GetUInt32();
					sftime1 = field[3].GetUInt32();
				}
				uint32 atime = time(NULL) + 31104000; //月卡								1		2		3	4		5				1  2 3  4  5									 6			1					2		3     4			5	   6
				pPlayer->PExecute(GameDB::CharactersDB, "INSERT INTO characters_limited(guid,tftime,sftime,tftime1,sftime1) VALUES(%u,%u,%u,%u,%u) ON DUPLICATE KEY UPDATE sftime = %u;", pPlayer->GetGUIDLow(), tftime, atime, tftime1, sftime1, atime);
				ChatHandler(pPlayer).PSendSysMessage("购买成功！");
				pPlayer->PExecute(GameDB::RealmDB, "UPDATE account SET jf = (jf - %u) WHERE id = %u", item1jf, pPlayer->GetSession()->GetAccountId());
				pPlayer->CanInstantTaxi = true;
				return true;
			}
		}
		else
		{
			ChatHandler(pPlayer).PSendSysMessage("积分余额不足！");
			break;
		}
		return true;
	}
}
	case GOSSIP_ACTION_INFO_DEF + 2: //双天赋
	{
		uint32 item1jf;
		uint32 item2jf;
		uint32 item3jf;
		uint32 item4jf;
		auto itemsfresult = pPlayer->PQuery(GameDB::WorldDB, "SELECT tfitem1jf,tfitem2jf,tfitem3jf,item4tf FROM world_conf");
		if (itemsfresult)
		{
			 auto field = itemsfresult->Fetch();
			 item1jf = field[0].GetUInt32();
			 item2jf = field[1].GetUInt32();
			 item3jf = field[2].GetUInt32();
			 item4jf = field[3].GetUInt32();
		}
		ChatHandler(pPlayer).PSendSysMessage("套餐1一个月需求积分%u点", item1jf);
		//ChatHandler(pPlayer).PSendSysMessage("套餐2三个月需求积分%u点", item2jf);
		//ChatHandler(pPlayer).PSendSysMessage("套餐3一年需求积分%u点", item3jf);
		//ChatHandler(pPlayer).PSendSysMessage("套餐4永久需求积分%u点", item4jf);
		pPlayer->ADD_GOSSIP_ITEM(10, "购买双天赋套餐1", 1, GOSSIP_ACTION_INFO_DEF + 23);
		//pPlayer->ADD_GOSSIP_ITEM(10, "购买双天赋套餐2", 1, GOSSIP_ACTION_INFO_DEF + 24);
		//pPlayer->ADD_GOSSIP_ITEM(10, "购买双天赋套餐3", 1, GOSSIP_ACTION_INFO_DEF + 25);
		//pPlayer->ADD_GOSSIP_ITEM(10, "购买双天赋套餐4", 1, GOSSIP_ACTION_INFO_DEF + 40);
		pPlayer->SEND_GOSSIP_MENU(822, pItem->GetGUID());
		return true;
	}
case GOSSIP_ACTION_INFO_DEF + 40: //双天赋终身卡
{
		pPlayer->CLOSE_GOSSIP_MENU();
		uint32 item1jf;
		uint32 nowtime;
		item1jf = 0;
		auto itemsfresult = pPlayer->PQuery(GameDB::WorldDB, "SELECT item4tf FROM world_conf");
		if (itemsfresult)
		{
			auto field = itemsfresult->Fetch();
			item1jf = field[0].GetUInt32();
			if (jf >= item1jf)
			{
				auto nowtime_result = pPlayer->PQuery(GameDB::CharactersDB, "SELECT tftime1 FROM characters_limited WHERE guid = %u", pPlayer->GetGUIDLow());
				if (nowtime_result)
				{
					auto field = nowtime_result->Fetch();
					uint32 dbsftime1 = field[0].GetUInt32();
					if (dbsftime1 == 0)
					{
						pPlayer->PExecute(GameDB::CharactersDB, "UPDATE characters_limited SET tftime1 = 1 WHERE guid = %u", pPlayer->GetGUIDLow());
						ChatHandler(pPlayer).PSendSysMessage("购买成功！");
						pPlayer->PExecute(GameDB::RealmDB, "UPDATE account SET jf = (jf - %u) WHERE id = %u", item1jf, pPlayer->GetSession()->GetAccountId());
						pPlayer->CanDoubleTalent = true;
						return true;
					}
					else
					{
						ChatHandler(pPlayer).PSendSysMessage("您已经拥有永久使用权！");
					}
				}
			else
			{
				uint32 tftime = 0;
				uint32 sftime = 0;
				uint32 tftime1 = 0;
				uint32 sftime1 = 0;
				auto nowtime_result = pPlayer->PQuery(GameDB::CharactersDB, "SELECT tftime,sftime,tftime1,sftime1 FROM characters_limited WHERE guid = %u", pPlayer->GetGUIDLow());
				if (nowtime_result)
				{
					auto field = nowtime_result->Fetch();
					tftime = field[0].GetUInt32();
					sftime = field[1].GetUInt32();
					tftime1 = field[2].GetUInt32();
					sftime1 = field[3].GetUInt32();
				}
				uint32 atime = 1; //月卡												  1    2     3        4       5            1  2  3  4  5                              6      6
				pPlayer->PExecute(GameDB::CharactersDB, "INSERT INTO characters_limited(guid,tftime,sftime,tftime1,sftime1) VALUES(%u,%u,%u,%u,%u) ON DUPLICATE KEY UPDATE tftime1 = %u;", pPlayer->GetGUIDLow(), tftime, sftime, atime, sftime1, atime);
				ChatHandler(pPlayer).PSendSysMessage("购买成功！");
				pPlayer->PExecute(GameDB::RealmDB, "UPDATE account SET jf = (jf - %u) WHERE id = %u", item1jf, pPlayer->GetSession()->GetAccountId());
				pPlayer->CanDoubleTalent = true;
				return true;
			}
		}
		else
		{
			ChatHandler(pPlayer).PSendSysMessage("积分余额不足！");
			break;
		}
		return true;
	}
}
	case GOSSIP_ACTION_INFO_DEF + 23: //月卡
{
		pPlayer->CLOSE_GOSSIP_MENU();
		uint32 item1jf;
		uint32 nowtime;
		item1jf = 0;
		auto itemsfresult = pPlayer->PQuery(GameDB::WorldDB, "SELECT tfitem1jf FROM world_conf");
		if (itemsfresult)
		{
			auto field = itemsfresult->Fetch();
			item1jf = field[0].GetUInt32();
			if (jf >= item1jf)
			{
				auto nowtime_result = pPlayer->PQuery(GameDB::CharactersDB, "SELECT tftime FROM characters_limited WHERE guid = %u", pPlayer->GetGUIDLow());
				if (nowtime_result)
				{
					auto field = nowtime_result->Fetch();
					uint32 dbsftime = field[0].GetUInt32();
					if (dbsftime != 0)
					{
						auto field = nowtime_result->Fetch();
						nowtime = field[0].GetUInt32();
						nowtime = nowtime + 2592000;
						pPlayer->PExecute(GameDB::CharactersDB, "UPDATE characters_limited SET tftime = %u WHERE guid = %u", nowtime, pPlayer->GetGUIDLow());
						ChatHandler(pPlayer).PSendSysMessage("购买成功！");
						pPlayer->PExecute(GameDB::RealmDB, "UPDATE account SET jf = (jf - %u) WHERE id = %u", item1jf, pPlayer->GetSession()->GetAccountId());
						pPlayer->CanDoubleTalent = true;
						return true;
					}
					else
					{
						uint32 tftime = 0;
						uint32 sftime = 0;
						uint32 tftime1 = 0;
						uint32 sftime1 = 0;
						auto nowtime_result = pPlayer->PQuery(GameDB::CharactersDB, "SELECT tftime,sftime,tftime1,sftime1 FROM characters_limited WHERE guid = %u", pPlayer->GetGUIDLow());
						if (nowtime_result)
						{
							auto field = nowtime_result->Fetch();
							tftime = field[0].GetUInt32();
							sftime = field[1].GetUInt32();
							tftime1 = field[2].GetUInt32();
							sftime1 = field[3].GetUInt32();
						}
						uint32 atime = time(NULL) + 2592000; //月卡								1		2		3	4		5				1  2 3  4  5									 6			1					2		3     4			5	   6
						pPlayer->PExecute(GameDB::CharactersDB, "INSERT INTO characters_limited(guid,tftime,sftime,tftime1,sftime1) VALUES(%u,%u,%u,%u,%u) ON DUPLICATE KEY UPDATE tftime = %u;", pPlayer->GetGUIDLow(), atime, sftime, tftime1, sftime1, atime);
						ChatHandler(pPlayer).PSendSysMessage("购买成功！");
						pPlayer->PExecute(GameDB::RealmDB, "UPDATE account SET jf = (jf - %u) WHERE id = %u", item1jf, pPlayer->GetSession()->GetAccountId());
						pPlayer->CanDoubleTalent = true;
						return true;
					}
				}
			else
			{
				uint32 tftime = 0;
				uint32 sftime = 0;
				uint32 tftime1 = 0;
				uint32 sftime1 = 0;
				auto nowtime_result = pPlayer->PQuery(GameDB::CharactersDB, "SELECT tftime,sftime,tftime1,sftime1 FROM characters_limited WHERE guid = %u", pPlayer->GetGUIDLow());
				if (nowtime_result)
				{
					auto field = nowtime_result->Fetch();
					tftime = field[0].GetUInt32();
					sftime = field[1].GetUInt32();
					tftime1 = field[2].GetUInt32();
					sftime1 = field[3].GetUInt32();
				}
				uint32 atime = time(NULL) + 2592000; //月卡								1		2		3	4		5				1  2 3  4  5									 6			1					2		3     4			5	   6
				pPlayer->PExecute(GameDB::CharactersDB, "INSERT INTO characters_limited(guid,tftime,sftime,tftime1,sftime1) VALUES(%u,%u,%u,%u,%u) ON DUPLICATE KEY UPDATE tftime = %u;", pPlayer->GetGUIDLow(), atime, sftime, tftime1, sftime1, atime);
				ChatHandler(pPlayer).PSendSysMessage("购买成功！");
				pPlayer->PExecute(GameDB::RealmDB, "UPDATE account SET jf = (jf - %u) WHERE id = %u", item1jf, pPlayer->GetSession()->GetAccountId());
				pPlayer->CanDoubleTalent = true;
				return true;
			}
		}
		else
		{
			ChatHandler(pPlayer).PSendSysMessage("积分余额不足！");
			break;
		}
		return true;
	}
}
	case GOSSIP_ACTION_INFO_DEF + 24:
{
		pPlayer->CLOSE_GOSSIP_MENU();
		uint32 item1jf;
		uint32 nowtime;
		item1jf = 0;
		auto itemsfresult = pPlayer->PQuery(GameDB::WorldDB, "SELECT tfitem2jf FROM world_conf");
		if (itemsfresult)
		{
			auto field = itemsfresult->Fetch();
			item1jf = field[0].GetUInt32();
			if (jf >= item1jf)
			{
				auto nowtime_result = pPlayer->PQuery(GameDB::CharactersDB, "SELECT tftime FROM characters_limited WHERE guid = %u", pPlayer->GetGUIDLow());
				if (nowtime_result)
				{
					auto field = nowtime_result->Fetch();
					uint32 dbsftime = field[0].GetUInt32();
					if (dbsftime != 0)
					{
						auto field = nowtime_result->Fetch();
						nowtime = field[0].GetUInt32();
						nowtime = nowtime + 7776000;
						pPlayer->PExecute(GameDB::CharactersDB, "UPDATE characters_limited SET tftime = %u WHERE guid = %u", nowtime, pPlayer->GetGUIDLow());
						ChatHandler(pPlayer).PSendSysMessage("购买成功！");
						pPlayer->PExecute(GameDB::RealmDB, "UPDATE account SET jf = (jf - %u) WHERE id = %u", item1jf, pPlayer->GetSession()->GetAccountId());
						pPlayer->CanDoubleTalent = true;
						return true;
					}
					else
					{
						uint32 tftime = 0;
						uint32 sftime = 0;
						uint32 tftime1 = 0;
						uint32 sftime1 = 0;
						auto nowtime_result = pPlayer->PQuery(GameDB::CharactersDB, "SELECT tftime,sftime,tftime1,sftime1 FROM characters_limited WHERE guid = %u", pPlayer->GetGUIDLow());
						if (nowtime_result)
						{
							auto field = nowtime_result->Fetch();
							tftime = field[0].GetUInt32();
							sftime = field[1].GetUInt32();
							tftime1 = field[2].GetUInt32();
							sftime1 = field[3].GetUInt32();
						}
						uint32 atime = time(NULL) + 7776000; //月卡								1		2		3	4		5				1  2 3  4  5									 6			1					2		3     4			5	   6
						pPlayer->PExecute(GameDB::CharactersDB, "INSERT INTO characters_limited(guid,tftime,sftime,tftime1,sftime1) VALUES(%u,%u,%u,%u,%u) ON DUPLICATE KEY UPDATE tftime = %u;", pPlayer->GetGUIDLow(), atime, sftime, tftime1, sftime1, atime);
						ChatHandler(pPlayer).PSendSysMessage("购买成功！");
						pPlayer->PExecute(GameDB::RealmDB, "UPDATE account SET jf = (jf - %u) WHERE id = %u", item1jf, pPlayer->GetSession()->GetAccountId());
						pPlayer->CanDoubleTalent = true;
						return true;
					}
				}
			else
			{
				uint32 tftime = 0;
				uint32 sftime = 0;
				uint32 tftime1 = 0;
				uint32 sftime1 = 0;
				auto nowtime_result = pPlayer->PQuery(GameDB::CharactersDB, "SELECT tftime,sftime,tftime1,sftime1 FROM characters_limited WHERE guid = %u", pPlayer->GetGUIDLow());
				if (nowtime_result)
				{
					auto field = nowtime_result->Fetch();
					tftime = field[0].GetUInt32();
					sftime = field[1].GetUInt32();
					tftime1 = field[2].GetUInt32();
					sftime1 = field[3].GetUInt32();
				}
				uint32 atime = time(NULL) + 7776000; //月卡								1		2		3	4		5				1  2 3  4  5									 6			1					2		3     4			5	   6
				pPlayer->PExecute(GameDB::CharactersDB, "INSERT INTO characters_limited(guid,tftime,sftime,tftime1,sftime1) VALUES(%u,%u,%u,%u,%u) ON DUPLICATE KEY UPDATE tftime = %u;", pPlayer->GetGUIDLow(), atime, sftime, tftime1, sftime1, atime);
				ChatHandler(pPlayer).PSendSysMessage("购买成功！");
				pPlayer->PExecute(GameDB::RealmDB, "UPDATE account SET jf = (jf - %u) WHERE id = %u", item1jf, pPlayer->GetSession()->GetAccountId());
				pPlayer->CanDoubleTalent = true;
				return true;
			}
		}
		else
		{
			ChatHandler(pPlayer).PSendSysMessage("积分余额不足！");
			break;
		}
		return true;
	}
}
	case GOSSIP_ACTION_INFO_DEF + 25:
{
		pPlayer->CLOSE_GOSSIP_MENU();
		uint32 item1jf;
		uint32 nowtime;
		item1jf = 0;
		auto itemsfresult = pPlayer->PQuery(GameDB::WorldDB, "SELECT tfitem3jf FROM world_conf");
		if (itemsfresult)
		{
			auto field = itemsfresult->Fetch();
			item1jf = field[0].GetUInt32();
			if (jf >= item1jf)
			{
				auto nowtime_result = pPlayer->PQuery(GameDB::CharactersDB, "SELECT tftime FROM characters_limited WHERE guid = %u", pPlayer->GetGUIDLow());
				if (nowtime_result)
				{
					auto field = nowtime_result->Fetch();
					uint32 dbsftime = field[0].GetUInt32();
					if (dbsftime != 0)
					{
						auto field = nowtime_result->Fetch();
						nowtime = field[0].GetUInt32();
						nowtime = nowtime + 31536000;
						pPlayer->PExecute(GameDB::CharactersDB, "UPDATE characters_limited SET tftime = %u WHERE guid = %u", nowtime, pPlayer->GetGUIDLow());
						ChatHandler(pPlayer).PSendSysMessage("购买成功！");
						pPlayer->PExecute(GameDB::RealmDB, "UPDATE account SET jf = (jf - %u) WHERE id = %u", item1jf, pPlayer->GetSession()->GetAccountId());
						pPlayer->CanDoubleTalent = true;
						return true;
					}
					else
					{
						uint32 tftime = 0;
						uint32 sftime = 0;
						uint32 tftime1 = 0;
						uint32 sftime1 = 0;
						auto nowtime_result = pPlayer->PQuery(GameDB::CharactersDB, "SELECT tftime,sftime,tftime1,sftime1 FROM characters_limited WHERE guid = %u", pPlayer->GetGUIDLow());
						if (nowtime_result)
						{
							auto field = nowtime_result->Fetch();
							tftime = field[0].GetUInt32();
							sftime = field[1].GetUInt32();
							tftime1 = field[2].GetUInt32();
							sftime1 = field[3].GetUInt32();
						}
						uint32 atime = time(NULL) + 31536000; //月卡								1		2		3	4		5				1  2 3  4  5									 6			1					2		3     4			5	   6
						pPlayer->PExecute(GameDB::CharactersDB, "INSERT INTO characters_limited(guid,tftime,sftime,tftime1,sftime1) VALUES(%u,%u,%u,%u,%u) ON DUPLICATE KEY UPDATE tftime = %u;", pPlayer->GetGUIDLow(), atime, sftime, tftime1, sftime1, atime);
						ChatHandler(pPlayer).PSendSysMessage("购买成功！");
						pPlayer->PExecute(GameDB::RealmDB, "UPDATE account SET jf = (jf - %u) WHERE id = %u", item1jf, pPlayer->GetSession()->GetAccountId());
						pPlayer->CanDoubleTalent = true;
						return true;
					}
				}
			else
			{
				uint32 tftime = 0;
				uint32 sftime = 0;
				uint32 tftime1 = 0;
				uint32 sftime1 = 0;
				auto nowtime_result = pPlayer->PQuery(GameDB::CharactersDB, "SELECT tftime,sftime,tftime1,sftime1 FROM characters_limited WHERE guid = %u", pPlayer->GetGUIDLow());
				if (nowtime_result)
				{
					auto field = nowtime_result->Fetch();
					tftime = field[0].GetUInt32();
					sftime = field[1].GetUInt32();
					tftime1 = field[2].GetUInt32();
					sftime1 = field[3].GetUInt32();
				}
				uint32 atime = time(NULL) + 31536000; //月卡								1		2		3	4		5				1  2 3  4  5									 6			1					2		3     4			5	   6
				pPlayer->PExecute(GameDB::CharactersDB, "INSERT INTO characters_limited(guid,tftime,sftime,tftime1,sftime1) VALUES(%u,%u,%u,%u,%u) ON DUPLICATE KEY UPDATE tftime = %u;", pPlayer->GetGUIDLow(), atime, sftime, tftime1, sftime1, atime);
				ChatHandler(pPlayer).PSendSysMessage("购买成功！");
				pPlayer->PExecute(GameDB::RealmDB, "UPDATE account SET jf = (jf - %u) WHERE id = %u", item1jf, pPlayer->GetSession()->GetAccountId());
				pPlayer->CanDoubleTalent = true;
				return true;
			}
		}
		else
		{
			ChatHandler(pPlayer).PSendSysMessage("积分余额不足！");
			break;
		}
		return true;
	}
}
	case GOSSIP_ACTION_INFO_DEF + 3:
	{
		pPlayer->ADD_GOSSIP_ITEM(0, "使用金币秒升", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 930);
		pPlayer->ADD_GOSSIP_ITEM(0, "使用积分秒升", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 931);
		pPlayer->SEND_GOSSIP_MENU(822, pItem->GetGUID());
		return true;
	}
	case GOSSIP_ACTION_INFO_DEF + 930:
	{
		oldlevel = pPlayer->getLevel();
		uplevel = (((60 - oldlevel) * levelupjf)*20);
		pPlayer->ADD_GOSSIP_ITEM(0, "确认秒升", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1000114);
		pPlayer->ADD_GOSSIP_ITEM(0, "取消", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1000115);
		pPlayer->SEND_GOSSIP_MENU(822, pItem->GetGUID());
		ChatHandler(pPlayer).PSendSysMessage("尊敬的玩家您好,您现在的等级为%u级,秒升到60消耗%u金币,请确认!", oldlevel, uplevel);
		return true;
	}
	case GOSSIP_ACTION_INFO_DEF + 1000114://秒升服务
	{
		oldlevel = pPlayer->getLevel();
		uplevel = (((60 - oldlevel) * levelupjf)*200000);
		auto jf_xresult = pPlayer->PQuery(GameDB::WorldDB, "SELECT maxlevelupjf FROM world_conf");
		auto field = jf_xresult->Fetch();
		pPlayer->CLOSE_GOSSIP_MENU();
		if (pPlayer->GetMoney() >= uplevel)
		{
			if (uplevel)
			pPlayer->ModifyMoney(-uplevel);
			ChatHandler(pPlayer).PSendSysMessage("尊敬的玩家,恭喜秒升成功!消耗%u点金币提升到60级！", (uplevel / 10000));
			pPlayer->GiveLevel(60);
		}
		else
		{
			ChatHandler(pPlayer).PSendSysMessage("金币不足!");
		}
		return true;
	}
	case GOSSIP_ACTION_INFO_DEF + 1000115:
	{
		pPlayer->CLOSE_GOSSIP_MENU();
		return true;
	}
	case GOSSIP_ACTION_INFO_DEF + 931:
	{
		oldlevel = pPlayer->getLevel();
		uplevel = ((60 - oldlevel) * levelupjf);
		pPlayer->ADD_GOSSIP_ITEM(0, "确认秒升", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1000014);
		pPlayer->ADD_GOSSIP_ITEM(0, "取消", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1000015);
		pPlayer->SEND_GOSSIP_MENU(822, pItem->GetGUID());
		ChatHandler(pPlayer).PSendSysMessage("尊敬的玩家您好,您现在的等级为%u级,秒升到60消耗%u点积分,请确认!", oldlevel, uplevel);
		return true;
	}
	case GOSSIP_ACTION_INFO_DEF + 1000014://秒升服务
	{
		oldlevel = pPlayer->getLevel();
		uplevel = ((60 - oldlevel) * levelupjf);
		auto jf_xresult = pPlayer->PQuery(GameDB::WorldDB, "SELECT maxlevelupjf FROM world_conf");
		auto field = jf_xresult->Fetch();
		uint32 maxjf = field[0].GetUInt32();
		if (uplevel > maxjf)
		{
			uplevel = maxjf;
		}
		pPlayer->CLOSE_GOSSIP_MENU();
		if (jf >= uplevel)
		{
			if (uplevel)
			pPlayer->PExecute(GameDB::RealmDB, "UPDATE account SET jf = (jf - %u) WHERE id = %u", uplevel, pPlayer->GetSession()->GetAccountId());
			ChatHandler(pPlayer).PSendSysMessage("尊敬的玩家,恭喜秒升成功!消耗%u点积分提升到60级！", uplevel);
			pPlayer->GiveLevel(60);
		}
		else
		{
			ChatHandler(pPlayer).PSendSysMessage("积分不足!");
		}
		return true;
	}
	case GOSSIP_ACTION_INFO_DEF + 1000015:
	{
		pPlayer->CLOSE_GOSSIP_MENU();
		return true;
	}
	case GOSSIP_ACTION_INFO_DEF + 6:
	{
		pPlayer->ADD_GOSSIP_ITEM(3, "提升采矿", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1000101);
		pPlayer->ADD_GOSSIP_ITEM(3, "提升炼金", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1000102);
		pPlayer->ADD_GOSSIP_ITEM(3, "提升锻造", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1000103);
		pPlayer->ADD_GOSSIP_ITEM(3, "提升裁缝", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1000104);
		pPlayer->ADD_GOSSIP_ITEM(3, "提升烹饪", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1000105);
		pPlayer->ADD_GOSSIP_ITEM(3, "提升制皮", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1000106);
		pPlayer->ADD_GOSSIP_ITEM(3, "提升钓鱼", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1000107);
		pPlayer->ADD_GOSSIP_ITEM(3, "提升附魔", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1000108);
		pPlayer->ADD_GOSSIP_ITEM(3, "提升珠宝", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1000109);
		pPlayer->ADD_GOSSIP_ITEM(3, "提升工程", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1000110);
		pPlayer->ADD_GOSSIP_ITEM(3, "提升急救", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1000111);
		pPlayer->ADD_GOSSIP_ITEM(3, "提升草药", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1000112);
		pPlayer->ADD_GOSSIP_ITEM(3, "提升剥皮", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1000113);
		pPlayer->SEND_GOSSIP_MENU(822, pItem->GetGUID());
		return true;
	}
	case GOSSIP_ACTION_INFO_DEF + 1000101:
	{
		if (jf >= 50 && pPlayer->HasSkill(186))
		{
			pPlayer->CLOSE_GOSSIP_MENU();
			pPlayer->SetSkill(186, 300, 300);
			pPlayer->PExecute(GameDB::RealmDB, "UPDATE account SET jf = (jf - %u) WHERE id = %u", maxskilljf, pPlayer->GetSession()->GetAccountId());
			ChatHandler(pPlayer).PSendSysMessage("感谢您的支持！");
		}
		else
		{
			ChatHandler(pPlayer).PSendSysMessage("积分不足购买此项目,或未学得该技能！");
			pPlayer->CLOSE_GOSSIP_MENU();
		}
		return true;
	}
	case GOSSIP_ACTION_INFO_DEF + 1000102:
	{
		if (jf >= 50 && pPlayer->HasSkill(171))
		{
			pPlayer->CLOSE_GOSSIP_MENU();
			pPlayer->SetSkill(171, 300, 300);
			pPlayer->PExecute(GameDB::RealmDB, "UPDATE account SET jf = (jf - %u) WHERE id = %u", maxskilljf, pPlayer->GetSession()->GetAccountId());
			ChatHandler(pPlayer).PSendSysMessage("感谢您的支持！");
		}
		else
		{
			ChatHandler(pPlayer).PSendSysMessage("积分不足购买此项目,或未学得该技能！");
			pPlayer->CLOSE_GOSSIP_MENU();
		}
		return true;
	}
	case GOSSIP_ACTION_INFO_DEF + 1000103:
	{
		if (jf >= 50 && pPlayer->HasSkill(164))
		{
			pPlayer->CLOSE_GOSSIP_MENU();
			pPlayer->SetSkill(164, 300, 300);
			pPlayer->PExecute(GameDB::RealmDB, "UPDATE account SET jf = (jf - %u) WHERE id = %u", maxskilljf, pPlayer->GetSession()->GetAccountId());
			ChatHandler(pPlayer).PSendSysMessage("感谢您的支持！");
		}
		else
		{
			ChatHandler(pPlayer).PSendSysMessage("积分不足购买此项目,或未学得该技能！");
			pPlayer->CLOSE_GOSSIP_MENU();
		}
		return true;
	}
	case GOSSIP_ACTION_INFO_DEF + 1000104:
	{
		if (jf >= 50 && pPlayer->HasSkill(197))
		{
			pPlayer->CLOSE_GOSSIP_MENU();
			pPlayer->SetSkill(197, 300, 300);
			pPlayer->PExecute(GameDB::RealmDB, "UPDATE account SET jf = (jf - %u) WHERE id = %u", maxskilljf, pPlayer->GetSession()->GetAccountId());
			ChatHandler(pPlayer).PSendSysMessage("感谢您的支持！");
		}
		else
		{
			ChatHandler(pPlayer).PSendSysMessage("积分不足购买此项目,或未学得该技能！");
			pPlayer->CLOSE_GOSSIP_MENU();
		}
		return true;
	}
	case GOSSIP_ACTION_INFO_DEF + 1000105:
	{
		if (jf >= 50 && pPlayer->HasSkill(185))
		{
			pPlayer->CLOSE_GOSSIP_MENU();
			pPlayer->SetSkill(185, 300, 300);
			pPlayer->PExecute(GameDB::RealmDB, "UPDATE account SET jf = (jf - %u) WHERE id = %u", maxskilljf, pPlayer->GetSession()->GetAccountId());
			ChatHandler(pPlayer).PSendSysMessage("感谢您的支持！");
		}
		else
		{
			ChatHandler(pPlayer).PSendSysMessage("积分不足购买此项目,或未学得该技能！");
			pPlayer->CLOSE_GOSSIP_MENU();
		}
		return true;
	}
	case GOSSIP_ACTION_INFO_DEF + 1000106:
	{
		if (jf >= 50 && pPlayer->HasSkill(165))
		{
			pPlayer->CLOSE_GOSSIP_MENU();
			pPlayer->SetSkill(165, 300, 300);
			pPlayer->PExecute(GameDB::RealmDB, "UPDATE account SET jf = (jf - %u) WHERE id = %u", maxskilljf, pPlayer->GetSession()->GetAccountId());
			ChatHandler(pPlayer).PSendSysMessage("感谢您的支持！");
		}
		else
		{
			ChatHandler(pPlayer).PSendSysMessage("积分不足购买此项目,或未学得该技能！");
			pPlayer->CLOSE_GOSSIP_MENU();
		}
		return true;
	}
	case GOSSIP_ACTION_INFO_DEF + 1000107:
	{
		if (jf >= 100 && pPlayer->HasSkill(356))
		{
			pPlayer->CLOSE_GOSSIP_MENU();
			pPlayer->SetSkill(356, 300, 300);
			pPlayer->PExecute(GameDB::RealmDB, "UPDATE account SET jf = (jf - %u) WHERE id = %u", maxskilljf, pPlayer->GetSession()->GetAccountId());
			ChatHandler(pPlayer).PSendSysMessage("感谢您的支持！");
		}
		else
		{
			ChatHandler(pPlayer).PSendSysMessage("积分不足购买此项目,或未学得该技能！");
			pPlayer->CLOSE_GOSSIP_MENU();
		}
		return true;
	}
	case GOSSIP_ACTION_INFO_DEF + 1000108:
	{
		if (jf >= 50 && pPlayer->HasSkill(333))
		{
			pPlayer->CLOSE_GOSSIP_MENU();
			pPlayer->SetSkill(333, 300, 300);
			pPlayer->PExecute(GameDB::RealmDB, "UPDATE account SET jf = (jf - %u) WHERE id = %u", maxskilljf, pPlayer->GetSession()->GetAccountId());
			ChatHandler(pPlayer).PSendSysMessage("感谢您的支持！");
		}
		else
		{
			ChatHandler(pPlayer).PSendSysMessage("积分不足购买此项目,或未学得该技能！");
			pPlayer->CLOSE_GOSSIP_MENU();
		}
		return true;
	}
	case GOSSIP_ACTION_INFO_DEF + 1000109:
	{
		if (jf >= 50 && pPlayer->HasSkill(755))
		{
			pPlayer->CLOSE_GOSSIP_MENU();
			pPlayer->SetSkill(755, 300, 300);
			pPlayer->PExecute(GameDB::RealmDB, "UPDATE account SET jf = (jf - %u) WHERE id = %u", maxskilljf, pPlayer->GetSession()->GetAccountId());
			ChatHandler(pPlayer).PSendSysMessage("感谢您的支持！");
		}
		else
		{
			ChatHandler(pPlayer).PSendSysMessage("积分不足购买此项目,或未学得该技能！");
			pPlayer->CLOSE_GOSSIP_MENU();
		}
		return true;
	}
	case GOSSIP_ACTION_INFO_DEF + 1000110:
	{
		if (jf >= 50 && pPlayer->HasSkill(202))
		{
			pPlayer->CLOSE_GOSSIP_MENU();
			pPlayer->SetSkill(202, 300, 300);
			pPlayer->PExecute(GameDB::RealmDB, "UPDATE account SET jf = (jf - %u) WHERE id = %u", maxskilljf, pPlayer->GetSession()->GetAccountId());
			ChatHandler(pPlayer).PSendSysMessage("感谢您的支持！");
		}
		else
		{
			ChatHandler(pPlayer).PSendSysMessage("积分不足购买此项目,或未学得该技能！");
			pPlayer->CLOSE_GOSSIP_MENU();
		}
		return true;
	}
	case GOSSIP_ACTION_INFO_DEF + 1000111:
	{
		if (jf >= 50 && pPlayer->HasSkill(129))
		{
			pPlayer->CLOSE_GOSSIP_MENU();
			pPlayer->SetSkill(129, 300, 300);
			pPlayer->PExecute(GameDB::RealmDB, "UPDATE account SET jf = (jf - %u) WHERE id = %u", maxskilljf, pPlayer->GetSession()->GetAccountId());
			ChatHandler(pPlayer).PSendSysMessage("感谢您的支持！");
		}
		else
		{
			ChatHandler(pPlayer).PSendSysMessage("积分不足购买此项目,或未学得该技能！");
			pPlayer->CLOSE_GOSSIP_MENU();
		}
		return true;
	}
	case GOSSIP_ACTION_INFO_DEF + 1000112:
	{
		if (jf >= 50 && pPlayer->HasSkill(182))
		{
			pPlayer->CLOSE_GOSSIP_MENU();
			pPlayer->SetSkill(182, 300, 300);
			pPlayer->PExecute(GameDB::RealmDB, "UPDATE account SET jf = (jf - %u) WHERE id = %u", maxskilljf, pPlayer->GetSession()->GetAccountId());
			ChatHandler(pPlayer).PSendSysMessage("感谢您的支持！");
		}
		else
		{
			ChatHandler(pPlayer).PSendSysMessage("积分不足购买此项目,或未学得该技能！");
			pPlayer->CLOSE_GOSSIP_MENU();
		}
		return true;
	}
	case GOSSIP_ACTION_INFO_DEF + 1000113:
	{
		if (jf >= 50 && pPlayer->HasSkill(393))
		{
			pPlayer->CLOSE_GOSSIP_MENU();
			pPlayer->SetSkill(393, 300, 300);
			pPlayer->PExecute(GameDB::RealmDB, "UPDATE account SET jf = (jf - %u) WHERE id = %u", maxskilljf, pPlayer->GetSession()->GetAccountId());
			ChatHandler(pPlayer).PSendSysMessage("感谢您的支持！");
		}
		else
		{
			ChatHandler(pPlayer).PSendSysMessage("积分不足购买此项目,或未学得该技能！");
			pPlayer->CLOSE_GOSSIP_MENU();
		}
		return true;
	}
	case GOSSIP_ACTION_INFO_DEF + 5://商业技能购买
		if (skillcount < maxskillcount)
		{
			if (!pPlayer->HasSpell(2575))
			{
				pPlayer->ADD_GOSSIP_ITEM(0, "学习采矿", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 51);
			}
			if (!pPlayer->HasSpell(2259))
			{
				pPlayer->ADD_GOSSIP_ITEM(0, "学习炼金", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 52);
			}
			if (!pPlayer->HasSpell(2018))
			{
				pPlayer->ADD_GOSSIP_ITEM(0, "学习锻造", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 53);
			}
			if (!pPlayer->HasSpell(3908))
			{
				pPlayer->ADD_GOSSIP_ITEM(0, "学习裁缝", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 54);
			}
			if (!pPlayer->HasSpell(2108))
			{
				pPlayer->ADD_GOSSIP_ITEM(0, "学习制皮", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 55);
			}
			if (!pPlayer->HasSpell(7414))
			{
				pPlayer->ADD_GOSSIP_ITEM(0, "学习附魔", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 56);
			}
			if (!pPlayer->HasSpell(4036))
			{
				pPlayer->ADD_GOSSIP_ITEM(0, "学习工程", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 58);
			}
			if (!pPlayer->HasSpell(2372))
			{
				pPlayer->ADD_GOSSIP_ITEM(0, "学习草药", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 59);
			}
			if (!pPlayer->HasSpell(8613))
			{
				pPlayer->ADD_GOSSIP_ITEM(0, "学习剥皮", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 60);
			}
			pPlayer->SEND_GOSSIP_MENU(822, pItem->GetGUID());
			return true;
		}
		else
		{
			pPlayer->CLOSE_GOSSIP_MENU();
			pPlayer->GetSession()->SendNotification("专业技能过多，无法继续学习！");
			return true;
		}
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	case GOSSIP_ACTION_INFO_DEF + 51://购买采矿
		pPlayer->ADD_GOSSIP_ITEM(0, "确认学习", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 5001);
		ChatHandler(pPlayer).PSendSysMessage("尊敬的玩家|cff54FF9F[%s]|r您好,本次操作将扣除|cffFF0000%u|r点积分,请确认!", pPlayer->GetName(), learnskilljf);
		pPlayer->ADD_GOSSIP_ITEM(0, "取消", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 5002);
		pPlayer->SEND_GOSSIP_MENU(822, pItem->GetGUID());
		return true;

	case GOSSIP_ACTION_INFO_DEF + 52://购买炼金
		pPlayer->ADD_GOSSIP_ITEM(0, "确认学习", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 5003);
		ChatHandler(pPlayer).PSendSysMessage("尊敬的玩家|cff54FF9F[%s]|r您好,本次操作将扣除|cffFF0000%u|r点积分,请确认!", pPlayer->GetName(), learnskilljf);
		pPlayer->ADD_GOSSIP_ITEM(0, "取消", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 5004);
		pPlayer->SEND_GOSSIP_MENU(822, pItem->GetGUID());
		return true;

	case GOSSIP_ACTION_INFO_DEF + 53://购买锻造
		pPlayer->ADD_GOSSIP_ITEM(0, "确认学习", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 5005);
		ChatHandler(pPlayer).PSendSysMessage("尊敬的玩家|cff54FF9F[%s]|r您好,本次操作将扣除|cffFF0000%u|r点积分,请确认!", pPlayer->GetName(), learnskilljf);
		pPlayer->ADD_GOSSIP_ITEM(0, "取消", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 5006);
		pPlayer->SEND_GOSSIP_MENU(822, pItem->GetGUID());
		return true;

	case GOSSIP_ACTION_INFO_DEF + 54://购买裁缝
		pPlayer->ADD_GOSSIP_ITEM(0, "确认学习", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 5007);
		ChatHandler(pPlayer).PSendSysMessage("尊敬的玩家|cff54FF9F[%s]|r您好,本次操作将扣除|cffFF0000%u|r点积分,请确认!", pPlayer->GetName(), learnskilljf);
		pPlayer->ADD_GOSSIP_ITEM(0, "取消", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 5008);
		pPlayer->SEND_GOSSIP_MENU(822, pItem->GetGUID());
		return true;

	case GOSSIP_ACTION_INFO_DEF + 55://购买制皮
		pPlayer->ADD_GOSSIP_ITEM(0, "确认学习", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 5009);
		ChatHandler(pPlayer).PSendSysMessage("尊敬的玩家|cff54FF9F[%s]|r您好,本次操作将扣除|cffFF0000%u|r点积分,请确认!", pPlayer->GetName(), learnskilljf);
		pPlayer->ADD_GOSSIP_ITEM(0, "取消", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 5010);
		pPlayer->SEND_GOSSIP_MENU(822, pItem->GetGUID());
		return true;

	case GOSSIP_ACTION_INFO_DEF + 56://购买附魔
		pPlayer->ADD_GOSSIP_ITEM(0, "确认学习", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 5011);
		ChatHandler(pPlayer).PSendSysMessage("尊敬的玩家|cff54FF9F[%s]|r您好,本次操作将扣除|cffFF0000%u|r点积分,请确认!", pPlayer->GetName(), learnskilljf);
		pPlayer->ADD_GOSSIP_ITEM(0, "取消", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 5012);
		pPlayer->SEND_GOSSIP_MENU(822, pItem->GetGUID());
		return true;

	case GOSSIP_ACTION_INFO_DEF + 57://购买珠宝
		pPlayer->ADD_GOSSIP_ITEM(0, "确认学习", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 5013);
		ChatHandler(pPlayer).PSendSysMessage("尊敬的玩家|cff54FF9F[%s]|r您好,本次操作将扣除|cffFF0000%u|r点积分,请确认!", pPlayer->GetName(), learnskilljf);
		pPlayer->ADD_GOSSIP_ITEM(0, "取消", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 5014);
		pPlayer->SEND_GOSSIP_MENU(822, pItem->GetGUID());
		return true;

	case GOSSIP_ACTION_INFO_DEF + 58://购买工程
		pPlayer->ADD_GOSSIP_ITEM(0, "确认学习", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 5015);
		ChatHandler(pPlayer).PSendSysMessage("尊敬的玩家|cff54FF9F[%s]|r您好,本次操作将扣除|cffFF0000%u|r点积分,请确认!", pPlayer->GetName(), learnskilljf);
		pPlayer->ADD_GOSSIP_ITEM(0, "取消", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 5016);
		pPlayer->SEND_GOSSIP_MENU(822, pItem->GetGUID());
		return true;

	case GOSSIP_ACTION_INFO_DEF + 59://购买草药
		pPlayer->ADD_GOSSIP_ITEM(0, "确认学习", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 5017);
		ChatHandler(pPlayer).PSendSysMessage("尊敬的玩家|cff54FF9F[%s]|r您好,本次操作将扣除|cffFF0000%u|r点积分,请确认!", pPlayer->GetName(), learnskilljf);
		pPlayer->ADD_GOSSIP_ITEM(0, "取消", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 5018);
		pPlayer->SEND_GOSSIP_MENU(822, pItem->GetGUID());
		return true;

	case GOSSIP_ACTION_INFO_DEF + 60://购买剥皮
		pPlayer->ADD_GOSSIP_ITEM(0, "确认学习", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 5019);
		ChatHandler(pPlayer).PSendSysMessage("尊敬的玩家|cff54FF9F[%s]|r您好,本次操作将扣除|cffFF0000%u|r点积分,请确认!", pPlayer->GetName(), learnskilljf);
		pPlayer->ADD_GOSSIP_ITEM(0, "取消", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 5020);
		pPlayer->SEND_GOSSIP_MENU(822, pItem->GetGUID());
		return true;
				/////////////////////////////////////////////////////////////////////////////////////////////////////////
		////////////////
		//购买采矿技能//
		////////////////
	case GOSSIP_ACTION_INFO_DEF + 5001:
		if (jf < learnskilljf)
		{
			pPlayer->Say("积分不足!", LANG_UNIVERSAL);
			pPlayer->CLOSE_GOSSIP_MENU();
			break;
		}
		else
		{
			pPlayer->PExecute(GameDB::RealmDB, "UPDATE account SET jf = (jf - %u) WHERE id = %u", learnskilljf, pPlayer->GetSession()->GetAccountId());
			pPlayer->learnSpell(2575,false);
			pPlayer->Say("购买成功！", LANG_UNIVERSAL);
			pPlayer->CLOSE_GOSSIP_MENU();
			break;
		}
		break;
	case GOSSIP_ACTION_INFO_DEF + 5002:
	{
		 pPlayer->Say("感谢使用", LANG_UNIVERSAL);
		 pPlayer->CLOSE_GOSSIP_MENU();
	}
		break;

		////////////////
		//购买炼金技能//
		////////////////
	case GOSSIP_ACTION_INFO_DEF + 5003:
		if (jf < learnskilljf)
		{
			pPlayer->Say("积分不足!", LANG_UNIVERSAL);
			pPlayer->CLOSE_GOSSIP_MENU();
			break;
		}
		else if (pPlayer->HasSpell(2259))
		{
			pPlayer->Say("已经拥有炼金技能!", LANG_UNIVERSAL);
			pPlayer->CLOSE_GOSSIP_MENU();
			break;
		}
		else
		{
			pPlayer->PExecute(GameDB::RealmDB, "UPDATE account SET jf = (jf - %u) WHERE id = %u", learnskilljf, pPlayer->GetSession()->GetAccountId());
			pPlayer->learnSpell(2259, false);
			pPlayer->Say("购买成功！", LANG_UNIVERSAL);
			pPlayer->CLOSE_GOSSIP_MENU();
			break;
		}
		break;
	case GOSSIP_ACTION_INFO_DEF + 5004:
	{
		pPlayer->Say("感谢使用", LANG_UNIVERSAL);
		pPlayer->CLOSE_GOSSIP_MENU();
	}
		break;
		////////////////
		//购买锻造技能//
		////////////////
	case GOSSIP_ACTION_INFO_DEF + 5005:
		if (jf < learnskilljf)
		{
			pPlayer->Say("积分不足!", LANG_UNIVERSAL);
			pPlayer->CLOSE_GOSSIP_MENU();
			break;
		}
		else
		{
			pPlayer->PExecute(GameDB::RealmDB, "UPDATE account SET jf = (jf - %u) WHERE id = %u", learnskilljf, pPlayer->GetSession()->GetAccountId());
			pPlayer->learnSpell(2018, false);
			pPlayer->Say("购买成功！", LANG_UNIVERSAL);
			pPlayer->CLOSE_GOSSIP_MENU();
			break;
		}
		break;
	case GOSSIP_ACTION_INFO_DEF + 5006:
	{
		pPlayer->Say("感谢使用", LANG_UNIVERSAL);
		pPlayer->CLOSE_GOSSIP_MENU();
	}
		break;
		////////////////
		//购买裁缝技能//
		////////////////
	case GOSSIP_ACTION_INFO_DEF + 5007:
		if (jf < learnskilljf)
		{
			pPlayer->Say("积分不足!", LANG_UNIVERSAL);
			pPlayer->CLOSE_GOSSIP_MENU();
			break;
		}
		else
		{
			pPlayer->PExecute(GameDB::RealmDB, "UPDATE account SET jf = (jf - %u) WHERE id = %u", learnskilljf, pPlayer->GetSession()->GetAccountId());
			pPlayer->learnSpell(3908, false);
			pPlayer->Say("购买成功！", LANG_UNIVERSAL);
			pPlayer->CLOSE_GOSSIP_MENU();
			break;
		}
		break;
	case GOSSIP_ACTION_INFO_DEF + 5008:
	{
		pPlayer->Say("感谢使用", LANG_UNIVERSAL);
		pPlayer->CLOSE_GOSSIP_MENU();
	}
		break;

		////////////////
		//购买制皮技能//
		////////////////
	case GOSSIP_ACTION_INFO_DEF + 5009:
		if (jf < learnskilljf)
		{
			pPlayer->Say("积分不足!", LANG_UNIVERSAL);
			pPlayer->CLOSE_GOSSIP_MENU();
			break;
		}
		else
		{
			pPlayer->PExecute(GameDB::RealmDB, "UPDATE account SET jf = (jf - %u) WHERE id = %u", learnskilljf, pPlayer->GetSession()->GetAccountId());
			pPlayer->learnSpell(2108, false);
			pPlayer->Say("购买成功！", LANG_UNIVERSAL);
			pPlayer->CLOSE_GOSSIP_MENU();
			break;
		}
		break;
	case GOSSIP_ACTION_INFO_DEF + 5010:
	{
		    pPlayer->Say("感谢使用", LANG_UNIVERSAL);
		    pPlayer->CLOSE_GOSSIP_MENU();
	}
		break;
		////////////////
		//购买附魔技能//
		////////////////
	case GOSSIP_ACTION_INFO_DEF + 5011:
		if (jf < learnskilljf)
		{
			pPlayer->Say("积分不足!", LANG_UNIVERSAL);
			pPlayer->CLOSE_GOSSIP_MENU();
			break;
		}
		else
		{
			pPlayer->PExecute(GameDB::RealmDB, "UPDATE account SET jf = (jf - %u) WHERE id = %u", learnskilljf, pPlayer->GetSession()->GetAccountId());
			pPlayer->learnSpell(7414, false);
			pPlayer->Say("购买成功！", LANG_UNIVERSAL);
			pPlayer->CLOSE_GOSSIP_MENU();
			break;
		}
		break;
	case GOSSIP_ACTION_INFO_DEF + 5012:
	{
		pPlayer->Say("感谢使用", LANG_UNIVERSAL);
		pPlayer->CLOSE_GOSSIP_MENU();
	}
		break;
		////////////////
		//购买工程技能//
		////////////////
	case GOSSIP_ACTION_INFO_DEF + 5015:
		if (jf < learnskilljf)
		{
			pPlayer->Say("积分不足!", LANG_UNIVERSAL);
			pPlayer->CLOSE_GOSSIP_MENU();
			break;
		}
		else
		{
			pPlayer->PExecute(GameDB::RealmDB, "UPDATE account SET jf = (jf - %u) WHERE id = %u", learnskilljf, pPlayer->GetSession()->GetAccountId());
			pPlayer->learnSpell(4036, false);
			pPlayer->Say("购买成功！", LANG_UNIVERSAL);
			pPlayer->CLOSE_GOSSIP_MENU();
			break;
		}
		break;
	case GOSSIP_ACTION_INFO_DEF + 5016:
	{
		pPlayer->Say("感谢使用", LANG_UNIVERSAL);
		pPlayer->CLOSE_GOSSIP_MENU();
	}
		break;
		////////////////
		//购买草药技能//
		////////////////
	case GOSSIP_ACTION_INFO_DEF + 5017:
		if (jf < learnskilljf)
		{
			pPlayer->Say("积分不足!", LANG_UNIVERSAL);
			pPlayer->CLOSE_GOSSIP_MENU();
			break;
		}
		else
		{
			pPlayer->PExecute(GameDB::RealmDB, "UPDATE account SET jf = (jf - %u) WHERE id = %u", learnskilljf, pPlayer->GetSession()->GetAccountId());
			pPlayer->learnSpell(2372, false);
			pPlayer->Say("购买成功！", LANG_UNIVERSAL);
			pPlayer->CLOSE_GOSSIP_MENU();
			break;
		}
		break;
	case GOSSIP_ACTION_INFO_DEF + 5018:
	{
		pPlayer->Say("感谢使用", LANG_UNIVERSAL);
		pPlayer->CLOSE_GOSSIP_MENU();
	}
		break;
		////////////////
		//购买剥皮技能//
		////////////////
	case GOSSIP_ACTION_INFO_DEF + 5019:
		if (jf < learnskilljf)
		{
			pPlayer->Say("积分不足!", LANG_UNIVERSAL);
			pPlayer->CLOSE_GOSSIP_MENU();
			break;
		}
		else
		{
			pPlayer->PExecute(GameDB::RealmDB, "UPDATE account SET jf = (jf - %u) WHERE id = %u", learnskilljf, pPlayer->GetSession()->GetAccountId());
			pPlayer->learnSpell(8613, false);
			pPlayer->Say("购买成功！", LANG_UNIVERSAL);
			pPlayer->CLOSE_GOSSIP_MENU();
			break;
		}
		break;
	case GOSSIP_ACTION_INFO_DEF + 5020:
	{
		pPlayer->Say("感谢使用", LANG_UNIVERSAL);
		pPlayer->CLOSE_GOSSIP_MENU();
	}
		break;
	}
	return true;
}




void AddSC_transform()
{
	Script* pNewScript;
	pNewScript = new Script;
	pNewScript->Name = "frozen_transform";
	pNewScript->pItemUse = &ItemUse_frozen_transform;
	pNewScript->pGossipSelectItem = &ItemSelect_frozen_transform;
	pNewScript->RegisterSelf();

	pNewScript = new Script;
	pNewScript->Name = "frozen_detransform";
	pNewScript->pItemUse = &ItemUse_frozen_detransform;
	pNewScript->pItemSelect = &ItemSelect_frozen_detransform;
	pNewScript->RegisterSelf();

	pNewScript = new Script;
	pNewScript->Name = "item_teleport";
	pNewScript->pItemUse = &ItemUse_Item_TelePort;
	pNewScript->pGossipSelectItem = &ItemSelect_Item_TelePort;
	pNewScript->RegisterSelf();
}