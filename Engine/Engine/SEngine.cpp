#include "StdAfx.h"
#include "SEngine.h"

namespace SG
{

	void SEngine::OnInit()
	{
		std::cout << "Engine OnInit()\n";
	}

	void SEngine::OnUpdate()
	{
		std::cout << "Engine OnUpdate()\n";
	}

	void SEngine::OnExit()
	{
		std::cout << "Engine OnExit()\n";
	}

}