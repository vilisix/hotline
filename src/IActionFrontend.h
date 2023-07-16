#pragma once
#include <functional>

namespace Hotline {
	class ActionSet;

	class IActionFrontend {
	public:
		virtual ~IActionFrontend() = default;

		virtual void Draw(ActionSet& set) = 0;
		virtual void Reset() = 0;
		virtual void SetExitCallback(std::function<void()> callback) = 0;
	};
}