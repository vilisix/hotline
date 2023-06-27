#pragma once
#include <memory>
#include <string>

namespace Hotline{

class ActionSet;

class HotlineBase{
    virtual void Draw() = 0;
};

class Hotline : public HotlineBase{
public:
    Hotline(std::shared_ptr<ActionSet> set);
    void Draw() override;

private:
    std::shared_ptr<ActionSet> _set;
    std::string _query;
};

}