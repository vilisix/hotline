#pragma once
#include <functional>

namespace hotline {
	class ActionSet;

	class IActionFrontend {
	public:
		virtual ~IActionFrontend() = default;

		virtual void Draw(ActionSet& set) = 0;
		virtual void Reset() = 0;
		virtual void SetExitCallback(std::function<void()> callback) = 0;
	};

	class IProviderFrontend {
	public:
		virtual ~IProviderFrontend() = default;

		virtual void Draw(ActionSet& set) = 0;
	};
}