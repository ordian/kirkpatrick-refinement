#ifndef _CIRCULAR_H
#define _CIRCULAR_H

#include <list>

template <typename T>
typename std::list<T>::iterator
next(typename std::list<T>::iterator iter,
     std::list<T> & container)
{
    auto result = std::next(iter);
    if (result == container.end())
        return container.begin();
    return result;
}

template <typename T>
typename std::list<T>::iterator
prev(typename std::list<T>::iterator iter,
     std::list<T> & container)
{
    if (iter == container.begin())
        return std::prev(container.end());
    return std::prev(iter);
}

#endif // _CIRCULAR_H
