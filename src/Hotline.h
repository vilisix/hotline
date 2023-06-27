#pragma once
#include <memory>
#include <string>
#include "FuzzyScorer.h"

namespace Hotline{

class ActionSet;

class HotlineBase{
    virtual void Update() = 0;
};

class Hotline : public HotlineBase{
public:
    Hotline(std::shared_ptr<ActionSet> set);
    void Update() override;
    void Toggle();
private:
    void Reset();
    void HandleKeyInput();
    void HandleTextInput(const std::string& input);
    void DrawVariants();
    void DrawVariant(const FuzzyScore& variant);
    std::shared_ptr<ActionSet> _set;
    std::string _query;

    bool _isActive = false;
    int _selectionIndex = 0;
    std::string _textInput;
    std::vector<FuzzyScore> _variants;
};

}