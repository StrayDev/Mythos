#pragma once

// STL
#include <memory>

template<typename T>
class IMakeUnique 
{
public:

    virtual ~IMakeUnique() = default;

    template<typename... Args>
    static std::unique_ptr<T> MakeUnique(Args&&... args) 
    {
        return std::make_unique<T>(std::forward<Args>(args)...);
    }

};