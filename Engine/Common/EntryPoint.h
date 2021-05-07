#pragma once

//! Seagull Engine embedded the entrypoint inside the engine
//! to avoid some user-side problem

#include "Common/System/IApp.h"
#include "Engine/SEngine.h"

#include <stdlib.h>

int main(int argv, char** argc)
{
	extern SG::IApp* SG::GetAppInstance();

	SG::SEngine engine;
	SG::IApp* app = SG::GetAppInstance();
	engine.OnInit();

	app->OnInit();

	engine.OnUpdate();
	app->OnUpdate();

	app->OnExit();
	engine.OnExit();

	system("pause");
}