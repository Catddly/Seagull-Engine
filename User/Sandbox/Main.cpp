#include "StdAfx.h"

class MyApp : public SG::IApp
{
public:
	virtual void OnInit() override
	{
		std::cout << "User OnInit()\n";
	}

	virtual void OnUpdate() override
	{
		std::cout << "User OnUpdate()\n";
	}

	virtual void OnExit() override
	{
		std::cout << "User OnExit()\n";
	}
};

SG::IApp* SG::GetAppInstance()
{
	return new MyApp();
}