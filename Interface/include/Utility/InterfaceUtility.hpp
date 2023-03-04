#pragma once


// --
namespace Mythos::Utility
{

    auto PrioritySort(std::unique_ptr<Module>& a, std::unique_ptr<Module>& b)
    {
        return a->priority < b->priority;
    };

}
