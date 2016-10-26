
#include "precompiled.h"

struct PlayFunCheat_IngnoreAI : public ScriptedAI
{
	PlayFunCheat_IngnoreAI(Creature* pCreature) : ScriptedAI(pCreature) { Reset(); }

	uint32 CheckTimer;                                    // Timer for random chat

	// *** HANDLED FUNCTION ***
	// This is called whenever the core decides we need to evade
	void Reset() override
	{
		CheckTimer = 10000;
	}

	void UpdateAI(const uint32 uiDiff) override
	{
		if (CheckTimer <= uiDiff)
		{
			Map*pMap = m_creature->GetMap();
			Map::PlayerList const& PlList = pMap->GetPlayers();

			if (PlList.isEmpty())
				return;

			for (Map::PlayerList::const_iterator i = PlList.begin(); i != PlList.end(); ++i)
			{
				if (Player* pPlayer = i->getSource())
				{
					float dis = m_creature->GetDistance(pPlayer);
					if (dis < 100.0f)
					{
						pPlayer->PlayFunCheatIngnoreTimer = 180000;
						pPlayer->PlayFunCheatIngnore = true;
					}

				}
			}
			CheckTimer = 10000;
		}
		else CheckTimer -= uiDiff;
	}
};

CreatureAI* GetAI_PlayFunCheat_Ingnore(Creature* pCreature)
{
	return new PlayFunCheat_IngnoreAI(pCreature);
}

void AddSC_PlayFunCheat_Ingnore()
{
	Script* pNewScript;

	pNewScript = new Script;
	pNewScript->Name = "PlayFunCheat_Ingnore";
	pNewScript->GetAI = &GetAI_PlayFunCheat_Ingnore;
	pNewScript->RegisterSelf(false);
}
