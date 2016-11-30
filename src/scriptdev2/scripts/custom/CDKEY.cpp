#include "precompiled.h"
#include "ObjectMgr.h"

#if _MSC_VER >= 1600 // VC2010
#pragma execution_character_set("utf-8")
#endif

bool GossipHello_custom_cdkey(Player* pPlayer, Creature* pCreature)
{
	pPlayer->ADD_GOSSIP_ITEM_EXTENDED(0, "※酒馆小调新手礼包兑换※", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1, "|cffEEB422※※酒馆小调经典怀旧服务器介绍※※|r \n\n QQ群: 185738294|r\n\n |cffCD6889官方网站:http://www.wswow.online/ |r \n\n |cffAB82FF保持公平合理特色公益服务器特性\n\n 拒绝GM参与游戏\n\n 拒绝出售装备，声望\n\n 拒绝任何私下勾当如卖隐藏技能 \n\n 拒绝任何影响游戏环境的行为\n\n 极力维护良好的游戏氛围|r \n\n |cffCDBE70★★玩吧魔兽特色系统介绍★★|r  \n\n |cffCAFF70★幻化系统★|r,|cffCAE1FF★军衔系统★|r\n\n |cffBF3EFF★双重天赋★|r,|cff8B7500★战友招募★|r \n\n |cffFF3E96★前缀附魔★|r,|cffFFBBFF★瞬飞系统★|r \n\n |cffFF7F24★★特色地图★★|r,|cffFFD700★★随身功能★★|r \n\n |cffCAE1FF★★各种功能持续开发中...★★|r", 0, true);
	pPlayer->SEND_GOSSIP_MENU(822, pCreature->GetGUID());
	return true;
}

bool GossipSelectWithCode_custom_cdkey(Player* pPlayer, Creature* pCreature, uint32 sender, uint32 action, const char* sCode)
{
	if (action == GOSSIP_ACTION_INFO_DEF + 1)
	{
		auto cdkey_result = pPlayer->PQuery(GameDB::WorldDB, "SELECT id FROM cdkey WHERE cdkey ='%s'", sCode);//自动搜寻匹配scode的cdkey字段
		if (cdkey_result) //如果cdkey与scode匹配
		{
			auto field = cdkey_result->Fetch(); //自动将匹配的行中的数据保存起来?
			uint32 id = field[0].GetUInt32(); //将(保存起来?)的列0填充到(id)
			auto items_result = pPlayer->PQuery(GameDB::WorldDB,"SELECT item,count FROM cdkey_items WHERE cdkey_id = %u", id);
			do
			{
				auto item_field = items_result->Fetch();
				uint32 item = item_field[0].GetUInt32();
				uint32 count = item_field[1].GetUInt32();
				//ItemPrototype const* markProto = sObjectMgr->::GetItemPrototype(item);
				ItemPosCountVec dest;
				uint32 noSpaceForCount = 0;
				pPlayer->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, item, count, &noSpaceForCount);
				Item* Pitem = pPlayer->StoreNewItem(dest, item, true, Item::GenerateItemRandomPropertyId(item));
				pPlayer->SendNewItem(Pitem, count, true, false);
				pPlayer->GetSession()->SendNotification("领取成功！");
			} while (items_result->NextRow());
			pPlayer->PExecute(GameDB::WorldDB,"DELETE FROM cdkey WHERE id=%u", id);
			pPlayer->PExecute(GameDB::WorldDB,"DELETE FROM cdkey_items WHERE cdkey_id=%u", id);
		}
		else
		{
			ChatHandler(pPlayer).PSendSysMessage("CDKEY错误,无法领取");
			return true;
		}
	}
	return true;
}

void AddSC_custom_cdkey()
{
	Script* script;
	script = new Script();
	script->Name = "custom_cdkey";
	script->pGossipHello = &GossipHello_custom_cdkey;
	script->pGossipSelectWithCode = &GossipSelectWithCode_custom_cdkey;
	script->RegisterSelf();
}