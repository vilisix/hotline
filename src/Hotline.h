#pragma once
#include <memory>
#include <string>
#include "FuzzyScorer.h"

namespace Hotline {
	class ActionSet;

	class Hotline {
	public:
		Hotline(std::shared_ptr<ActionSet> set, bool showRecents = true);
		virtual ~Hotline() = default;

		virtual void Update();
		virtual void Toggle();

	private:
		std::vector<FuzzyScore>& GetCurrentVariantContainer();
		void Reset();
		void HandleKeyInput();
		void HandleTextInput(const std::string& input);
		void HandleApplyCommand();
		void DrawVariants(const std::vector<FuzzyScore> variants);
		void DrawVariant(const FuzzyScore& variant);

		std::shared_ptr<ActionSet> _set;
        bool _showRecents;

		bool _isActive = false;
		int _selectionIndex = 0;

		char _inputBuffer[128] = "";
		std::string _textInput;
		std::vector<FuzzyScore> _queryVariants;
		std::vector<FuzzyScore> _recentCommands;
    };
}