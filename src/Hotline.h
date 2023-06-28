#pragma once
#include <memory>
#include <string>
#include "FuzzyScorer.h"

namespace Hotline {
	class ActionSet;

	class Hotline {
	public:
		Hotline(std::shared_ptr<ActionSet> set);
		virtual ~Hotline() = default;

		virtual void Update();
		virtual void Toggle();

	private:
		void Reset();
		void HandleKeyInput();
		void HandleTextInput(const std::string& input);
		void DrawVariants();
		void DrawVariant(const FuzzyScore& variant);

		std::shared_ptr<ActionSet> _set;

		bool _isActive = false;
		int _selectionIndex = 0;

		char _inputBuffer[128] = "";
		std::string _textInput;
		std::vector<FuzzyScore> _variants;
	};
}