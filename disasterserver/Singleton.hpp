#ifndef SINGLETON_HPP
#define SINGLETON_HPP

// https://de.wikibooks.org/wiki/C%2B%2B-Programmierung:_Entwurfsmuster:_Singleton
template <typename C>
class Singleton
{
public:
    static C* getInstance()
    {
        if (!_instance)
            _instance = new C ();
        return _instance;
    }
    virtual ~Singleton () {
        _instance = nullptr;
    }
private:
    static C* _instance;
protected:
    Singleton () = default;
};
template <typename C> C* Singleton <C>::_instance = nullptr;

#endif //SINGLETON_HPP
