#pragma once

#include "base/EntityManager.h"
#include "base/TouchInputManager.h"

#include "systems/TransformationSystem.h"
#include "systems/RenderingSystem.h"
#include "systems/ADSRSystem.h"

#include "Game.h"
#include "GameStateManager.h"
#include "DepthLayer.h"

enum FadeType {
	FadeIn,
	FadeOut
};

class FadeGameStateManager : public GameStateManager {
	public:
		FadeGameStateManager(FadeType fade, GameState whoAmI, GameState whoIsNext, GameStateManager* enterDelegate, GameStateManager* exitDelegate);
		void Setup();
		void Enter();
		GameState Update(float dt);
		void Exit();
	private:
		Entity eFading;
		FadeType fading;
		GameState iAm, heIs;
		float timeout, accum;
		GameStateManager* enterDelegate, *exitDelegate;
};


