#pragma once

template<typename T>
class Singleton
{
public:
    static T& GetInstance() noexcept
    {
        static T g_instance{};
        return g_instance;
    }

protected:
    Singleton() = default;
    ~Singleton() = default;

public:
    Singleton(const Singleton&) = delete;
    Singleton& operator=(const Singleton&) = delete;
    Singleton(Singleton&&) = delete;
    Singleton& operator=(Singleton&&) = delete;
};