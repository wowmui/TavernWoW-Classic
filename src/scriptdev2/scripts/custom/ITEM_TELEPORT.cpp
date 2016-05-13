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
#include "D:\CmangosBuild\mangos-classic\src\game\BattleGround\BattleGroundMgr.h"
#include "D:\CmangosBuild\mangos-classic\src\game\BattleGround\BattleGroundWS.h"
#include "D:\CmangosBuild\mangos-classic\src\game\BattleGround\BattleGround.h"
#include "Language.h"
#include "ScriptMgr.h"
#include "World.h"

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
	player->ADD_GOSSIP_ITEM(10, "购买瞬飞", 1, GOSSIP_ACTION_INFO_DEF + 1);
	player->ADD_GOSSIP_ITEM(10, "购买双天赋", 1, GOSSIP_ACTION_INFO_DEF + 2);
	if (player->getLevel() < 60)
	{
		player->ADD_GOSSIP_ITEM(10, "秒升满级", 1, GOSSIP_ACTION_INFO_DEF + 3);
	}
	player->ADD_GOSSIP_ITEM(10, "购买商业技能", 1, GOSSIP_ACTION_INFO_DEF + 5);
	player->ADD_GOSSIP_ITEM(10, "提升商业技能", 1, GOSSIP_ACTION_INFO_DEF + 6);
	player->ADD_GOSSIP_ITEM(10, "购买背包", 1, GOSSIP_ACTION_INFO_DEF + 7);
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
		  pPlayer->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, 99004, 1, &noSpaceForCount);
		  Item* Pitem = pPlayer->StoreNewItem(dest, 99007, true, Item::GenerateItemRandomPropertyId(99004));
		  pPlayer->SendNewItem(Pitem, 1, true, false);
		  ChatHandler(pPlayer).PSendSysMessage("购买成功！");
		  break;
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
		if (pPlayer->GetItemCount(99004) >= 1 ||
			pPlayer->GetItemCount(99005) >= 1 ||
			pPlayer->GetItemCount(99006) >= 1)
		{
			ChatHandler(pPlayer).PSendSysMessage("你已经有瞬飞物品了！");
		}
		else
		{
			pPlayer->ADD_GOSSIP_ITEM(10, "购买瞬飞套餐1", 1, GOSSIP_ACTION_INFO_DEF + 20);
			pPlayer->ADD_GOSSIP_ITEM(10, "购买瞬飞套餐2", 1, GOSSIP_ACTION_INFO_DEF + 21);
			pPlayer->ADD_GOSSIP_ITEM(10, "购买瞬飞套餐3", 1, GOSSIP_ACTION_INFO_DEF + 22);
			pPlayer->SEND_GOSSIP_MENU(822, pItem->GetGUID());
		}
		return true;
	}
	case GOSSIP_ACTION_INFO_DEF + 20:
	{
		pPlayer->CLOSE_GOSSIP_MENU();
		uint32 item1jf;
		item1jf = 0;
		auto itemsfresult = pPlayer->PQuery(GameDB::WorldDB, "SELECT sfitem1jf FROM world_conf");
		if (itemsfresult)
		{
			auto field = itemsfresult->Fetch();
			item1jf = field[0].GetUInt32();
		}
		if (jf >= item1jf)
		{
			pPlayer->PExecute(GameDB::RealmDB, "UPDATE account SET jf = (jf - %u)", item1jf);
			ItemPosCountVec dest;
			uint32 noSpaceForCount = 0;
			pPlayer->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, 99004, 1, &noSpaceForCount);
			Item* Pitem = pPlayer->StoreNewItem(dest, 99004, true, Item::GenerateItemRandomPropertyId(99004));
			pPlayer->SendNewItem(Pitem, 1, true, false);
			ChatHandler(pPlayer).PSendSysMessage("购买成功！");
			break;
		}
		else
		{
			ChatHandler(pPlayer).PSendSysMessage("积分余额不足！");
			break;
		}
		return true;
	}
	case GOSSIP_ACTION_INFO_DEF + 21:
	{
		pPlayer->CLOSE_GOSSIP_MENU();
		uint32 item2jf;
		item2jf = 0;
		auto itemsfresult = pPlayer->PQuery(GameDB::WorldDB, "SELECT sfitem2jf FROM world_conf");
		if (itemsfresult)
		{
			auto field = itemsfresult->Fetch();
			item2jf = field[0].GetUInt32();
		}
		if (jf >= item2jf)
		{
			pPlayer->PExecute(GameDB::RealmDB, "UPDATE account SET jf = (jf - %u)", item2jf);
			ItemPosCountVec dest;
			uint32 noSpaceForCount = 0;
			pPlayer->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, 99005, 1, &noSpaceForCount);
			Item* Pitem = pPlayer->StoreNewItem(dest, 99005, true, Item::GenerateItemRandomPropertyId(99005));
			pPlayer->SendNewItem(Pitem, 1, true, false);
			ChatHandler(pPlayer).PSendSysMessage("购买成功！");
			break;
		}
		else
		{
			ChatHandler(pPlayer).PSendSysMessage("积分余额不足！");
			break;
		}
		return true;
	}
	case GOSSIP_ACTION_INFO_DEF + 22:
	{
		pPlayer->CLOSE_GOSSIP_MENU();
		uint32 item3jf;
		item3jf = 0;
		auto itemsfresult = pPlayer->PQuery(GameDB::WorldDB, "SELECT sfitem3jf FROM world_conf");
		if (itemsfresult)
		{
			auto field = itemsfresult->Fetch();
			item3jf = field[0].GetUInt32();
		}
		if (jf > item3jf)
		{
			pPlayer->PExecute(GameDB::RealmDB, "UPDATE account SET jf = (jf - %u)", item3jf);
			ItemPosCountVec dest;
			uint32 noSpaceForCount = 0;
			pPlayer->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, 99006, 1, &noSpaceForCount);
			Item* Pitem = pPlayer->StoreNewItem(dest, 99006, true, Item::GenerateItemRandomPropertyId(99006));
			pPlayer->SendNewItem(Pitem, 1, true, false);
			ChatHandler(pPlayer).PSendSysMessage("购买成功！");
		break;
		}
		else
		{
			ChatHandler(pPlayer).PSendSysMessage("积分余额不足！");
			break;
		}
		return true;
	}
	case GOSSIP_ACTION_INFO_DEF + 2: //双天赋
	{
		if (pPlayer->GetItemCount(99001) >= 1 ||
			pPlayer->GetItemCount(99002) >= 1 ||
			pPlayer->GetItemCount(99003) >= 1)
		{
			ChatHandler(pPlayer).PSendSysMessage("你已经有双天赋物品了！");
		}
		else
		{
			pPlayer->ADD_GOSSIP_ITEM(10, "购买双天赋套餐1", 1, GOSSIP_ACTION_INFO_DEF + 23);
			pPlayer->ADD_GOSSIP_ITEM(10, "购买双天赋套餐2", 1, GOSSIP_ACTION_INFO_DEF + 24);
			pPlayer->ADD_GOSSIP_ITEM(10, "购买双天赋套餐3", 1, GOSSIP_ACTION_INFO_DEF + 25);
			pPlayer->SEND_GOSSIP_MENU(822, pItem->GetGUID());
		}
		return true;
	}
	case GOSSIP_ACTION_INFO_DEF + 23:
	{
		pPlayer->CLOSE_GOSSIP_MENU();
		uint32 item1jf;
		item1jf = 0;
		auto itemsfresult = pPlayer->PQuery(GameDB::WorldDB, "SELECT tfitem1jf FROM world_conf");
		if (itemsfresult)
		{
			auto field = itemsfresult->Fetch();
			item1jf = field[0].GetUInt32();
		}
		if (jf >= item1jf)
		{
			pPlayer->PExecute(GameDB::RealmDB, "UPDATE account SET jf = (jf - %u)", item1jf);
			ItemPosCountVec dest;
			uint32 noSpaceForCount = 0;
			pPlayer->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, 99001, 1, &noSpaceForCount);
			Item* Pitem = pPlayer->StoreNewItem(dest, 99001, true, Item::GenerateItemRandomPropertyId(99001));
			pPlayer->SendNewItem(Pitem, 1, true, false);
			ChatHandler(pPlayer).PSendSysMessage("购买成功！");
			break;
		}
		else
		{
			ChatHandler(pPlayer).PSendSysMessage("积分余额不足！");
			break;
		}
		return true;
	}
	case GOSSIP_ACTION_INFO_DEF + 24:
	{
		pPlayer->CLOSE_GOSSIP_MENU();
		uint32 item2jf;
		item2jf = 0;
		auto itemsfresult = pPlayer->PQuery(GameDB::WorldDB, "SELECT tfitem2jf FROM world_conf");
		if (itemsfresult)
		{
			auto field = itemsfresult->Fetch();
			item2jf = field[0].GetUInt32();
		}
		if (jf >= item2jf)
		{
			pPlayer->PExecute(GameDB::RealmDB, "UPDATE account SET jf = (jf - %u)", item2jf);
			ItemPosCountVec dest;
			uint32 noSpaceForCount = 0;
			pPlayer->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, 99002, 1, &noSpaceForCount);
			Item* Pitem = pPlayer->StoreNewItem(dest, 99002, true, Item::GenerateItemRandomPropertyId(99002));
			pPlayer->SendNewItem(Pitem, 1, true, false);
			ChatHandler(pPlayer).PSendSysMessage("购买成功！");
			break;
		}
		else
		{
			ChatHandler(pPlayer).PSendSysMessage("积分余额不足！");
			break;
		}
		return true;
	}
	case GOSSIP_ACTION_INFO_DEF + 25:
	{
		pPlayer->CLOSE_GOSSIP_MENU();
		uint32 item3jf;
		item3jf = 0;
		auto itemsfresult = pPlayer->PQuery(GameDB::WorldDB, "SELECT tfitem3jf FROM world_conf");
		if (itemsfresult)
		{
			auto field = itemsfresult->Fetch();
			item3jf = field[0].GetUInt32();
		}
		if (jf >= item3jf)
		{
			pPlayer->PExecute(GameDB::RealmDB, "UPDATE account SET jf = (jf - %u)", item3jf);
			ItemPosCountVec dest;
			uint32 noSpaceForCount = 0;
			pPlayer->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, 99003, 1, &noSpaceForCount);
			Item* Pitem = pPlayer->StoreNewItem(dest, 99003, true, Item::GenerateItemRandomPropertyId(99003));
			pPlayer->SendNewItem(Pitem, 1, true, false);
			break;
		}
		else
		{
			ChatHandler(pPlayer).PSendSysMessage("积分余额不足！");
			break;
		}
		return true;
	}
	case GOSSIP_ACTION_INFO_DEF + 3:
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
		auto field = jfresult->Fetch();
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
		if (jf <= learnskilljf)
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
		if (jf <= learnskilljf)
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
		if (jf <= learnskilljf)
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
		if (jf <= learnskilljf)
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
		if (jf <= learnskilljf)
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
		if (jf <= learnskilljf)
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
		if (jf <= learnskilljf)
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
		if (jf <= learnskilljf)
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
		if (jf <= learnskilljf)
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