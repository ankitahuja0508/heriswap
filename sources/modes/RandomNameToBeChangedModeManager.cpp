/*
	This file is part of Heriswap.

	@author Soupe au Caillou - Pierre-Eric Pelloux-Prayer
	@author Soupe au Caillou - Gautier Pelloux-Prayer

	Heriswap is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, version 3.

	Heriswap is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with Heriswap.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "RandomNameToBeChangedModeManager.h"

#include <iomanip>
#include <sstream>

#include <base/PlacementHelper.h>
#include <base/MathUtil.h>

#include "systems/ContainerSystem.h"
#include "systems/ButtonSystem.h"
#include "systems/TextRenderingSystem.h"

#include "DepthLayer.h"
#include "Game.h"
#include "CombinationMark.h"
#include "GridSystem.h"

RandomNameToBeChangedGameModeManager::RandomNameToBeChangedGameModeManager(Game* game, SuccessManager* successMgr, StorageAPI* sAPI) : GameModeManager(game, successMgr, sAPI) {
}

RandomNameToBeChangedGameModeManager::~RandomNameToBeChangedGameModeManager() {
}

void RandomNameToBeChangedGameModeManager::Setup() {
	GameModeManager::Setup();
}
#define RATE 1
void RandomNameToBeChangedGameModeManager::Enter() {
	time = 0;
	limit = 100;
	leavesDone = 0;
	points = 0;
	bonus = MathUtil::RandomInt(theGridSystem.Types);
	pts.clear();
	pts.push_back(Vector2(0,0));

	pts.push_back(Vector2(limit,1));//need limit leaves to end game

	generateLeaves(0, 8);

	GameModeManager::Enter();
	
    spawn = MathUtil::RandomFloat() * RATE;
}

void RandomNameToBeChangedGameModeManager::Exit() {
	//if we didn't give up
	GameModeManager::Exit();
}

void RandomNameToBeChangedGameModeManager::GameUpdate(float dt) {
	time+=dt;
	
	spawn += dt;
	while (spawn >= RATE) {
		
		
	}
}

float RandomNameToBeChangedGameModeManager::GameProgressPercent() {
	return MathUtil::Min(1.0f, (float)time/limit);
}

void RandomNameToBeChangedGameModeManager::UiUpdate(float dt) {
	//Score
	{
	std::stringstream a;
	a.precision(0);
	a << storageAPI->getMyRank(points, RandomNameToBeChanged, theGridSystem.sizeToDifficulty()) << ". " << std::fixed << points;
	TEXT_RENDERING(uiHelper.scoreProgress)->text = a.str();
	}

	updateHerisson(dt, time, 0);

#ifdef DEBUG
	if (_debug) {
		for(int i=0; i<8; i++) {
			std::stringstream text;
			text << countBranchLeavesOfType(i);
			if (i == 7) {
				text << ":" << leavesDone;
			}
			TEXT_RENDERING(debugEntities[2*i+1])->text = text.str();
			TEXT_RENDERING(debugEntities[2*i+1])->hide = false;
			TEXT_RENDERING(debugEntities[2*i+1])->color = Color(0.2, 0.2, 0.2);
		}
	}
#endif
}

void RandomNameToBeChangedGameModeManager::ScoreCalc(int nb, unsigned int type) {
	int p = (10 * nb * nb * nb) / 6;
	if (type == bonus) {
		p *= 2;
		deleteLeaves(~0b0, MathUtil::Min((int)branchLeaves.size(), nb));
		leavesDone+=2*nb;
	} else {
		deleteLeaves(~0b0, MathUtil::Min((int)branchLeaves.size(), nb));
		leavesDone+=nb;
	}
	p *= (theGridSystem.sizeToDifficulty() + 1);
	points += p;
}

void RandomNameToBeChangedGameModeManager::TogglePauseDisplay(bool paused) {
	GameModeManager::TogglePauseDisplay(paused);
}

void RandomNameToBeChangedGameModeManager::WillScore(int count, int type, std::vector<Entity>& out) {
    int nb = MathUtil::Min((int)branchLeaves.size(), count);
    for (unsigned int i=0; nb>0 && i<branchLeaves.size(); i++) {
		CombinationMark::markCellInCombination(branchLeaves[i].e);
        out.push_back(branchLeaves[i].e);
        nb--;
    }
}

int RandomNameToBeChangedGameModeManager::saveInternalState(uint8_t** out) {
    uint8_t* tmp;
    int parent = GameModeManager::saveInternalState(&tmp);
    int s = sizeof(leavesDone);
    uint8_t* ptr = *out = new uint8_t[parent + s];
    ptr = (uint8_t*) mempcpy(ptr, tmp, parent);
    ptr = (uint8_t*) mempcpy(ptr, &leavesDone, sizeof(leavesDone));

    delete[] tmp;
    return (parent + s);
}

const uint8_t* RandomNameToBeChangedGameModeManager::restoreInternalState(const uint8_t* in, int size) {
    in = GameModeManager::restoreInternalState(in, size);
    memcpy(&leavesDone, in, sizeof(leavesDone)); in += sizeof(leavesDone);
    return in;
}
