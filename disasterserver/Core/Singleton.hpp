#ifndef SINGLETON_HPP
#define SINGLETON_HPP

template <typename C>
class Singleton
{
public:
    Singleton(const Singleton&) = delete;
    Singleton& operator=(const Singleton&) = delete;

    static C& getInstance()
    {
        static C instance;
        return instance;
    }

protected:
    Singleton() = default;
    ~Singleton() = default;
};

#endif //SINGLETON_HPP
