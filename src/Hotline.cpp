#include "Hotline.h"

#include <memory>

#include "ActionSet.h"

namespace Hotline{

Hotline::Hotline(std::shared_ptr<ActionSet> set)
: _set(set)
{}

void Hotline::Draw()
{

}

}
