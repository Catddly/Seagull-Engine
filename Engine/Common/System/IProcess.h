#pragma once

namespace SG
{
	//! @Interface 
	//! Abstraction of a process
	struct IProcess
	{
		virtual ~IProcess() = default;

		virtual void OnInit() = 0;
		virtual void OnUpdate() = 0;
		virtual void OnShutdown() = 0;
	};

}