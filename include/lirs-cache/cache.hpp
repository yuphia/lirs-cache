#pragma once

#include <iterator>
#include <list>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <iostream>

int slowGetPageInt (int key);

namespace caches 
{
int lirsCache (size_t size, std::vector <int> vec);

template <typename PageT, typename KeyT = int>
struct lirs_cache_t
{
    size_t sz_;
    size_t hir_sz_ = 0;    

    bool isFull = false;
    bool isLirFull = false;

    lirs_cache_t (size_t sz) : sz_(sz) 
    {
        if (sz < 10 && sz > 1)
            hir_sz_ = 2;
        else if (sz == 1)
            hir_sz_ = 1;
        else
            hir_sz_ = sz/10;
    };

    enum State 
    {
        LIR = 0,
        HIR = 1
    };
    
    using PageAndState = typename std::pair <PageT, State>;
    using T = typename std::pair <KeyT, State>;  

    std::unordered_map<KeyT, PageAndState> cache_ = {};
    std::list<T> lirsStack = {};
    std::list<T> hirList = {};

    template <typename F> 
    bool newPageHandler (KeyT key, F slowGetPage)
    {
        auto hit = cache_.find (key);
        if (!isFull)
        {
            bool isInCache = !(hit == cache_.end());
            
            if (!isLirFull && !isInCache)
                lirNotFullAndNotInCache (key, slowGetPage);
            else if (!isLirFull)
                lirHandler (key);      
            else if (isInCache)
            {
                fillingLirHit (key);
            }
            else
            {
                auto keyItInLirs = findInList (key, HIR, &lirsStack);

                if (keyItInLirs == lirsStack.end())
                    fillingNonResidentHirWasNotInStk (key, slowGetPage);
                else 
                    fillingNonResidentHirWasInStk (key, slowGetPage);

                if (hirList.size() >= hir_sz_)
                    isFull = true;                
            }

            return isInCache;
        }

        if (hit == cache_.end())
        { 
            hirNonResidentHandler (key, slowGetPage);
            return false;            
        }

        if (hit->second.second == LIR)
            lirHandler (key);
        else if (hit->second.second == HIR)
            hirResidentHandler (key);

        return true;
    }

// Typed handlers for not full cache

    template <typename F>
    void lirNotFullAndNotInCache (KeyT key, F slowGetPage)
    {
        lirsStack.push_front ({key, LIR});

        cache_.insert ({key, slowGetPageWrap (slowGetPage, LIR, key)});

        if (lirsStack.size() >= sz_ - hir_sz_)            
            isLirFull = true;
    }

    template <typename F>
    void fillingNonResidentHirWasInStk (KeyT key, F slowGetPage)
    {
        cache_.insert ({key, slowGetPageWrap (slowGetPage, LIR, key)});
        lirsStack.push_front ({key, LIR});
        
        /*else
            hirList.push_front ({key, HIR});*/
    }

    void fillingLirHit (KeyT key)
    {
        auto lirIt = findInList (key, LIR, &lirsStack);
        auto lirIt2 = findInList (key, HIR, &lirsStack);
        auto hirIt = findInList (key, HIR, &hirList);
        if(lirIt == lirsStack.end() && lirIt2 == lirsStack.end() && hirIt == hirList.end())
        {
            lirsStack.push_front ({key, HIR});
            hirList.push_front ({key, HIR});
        }
        else if (lirIt != lirsStack.end())
            lirHandler (key);
        else if (lirIt2 != lirsStack.end())
            hirResidentHandler (key);
        else if (hirIt != hirList.end())
            hirResidentHandler (key);
    }

    template <typename F>
    void fillingNonResidentHirWasNotInStk (KeyT key, F slowGetPage)
    {
        cache_.insert ({key, slowGetPageWrap (slowGetPage, HIR, key)});

        hirList.push_front ({key, HIR});
        lirsStack.push_front ({key, HIR});

        /*lirBottomToHirFront();
        stackPrune();*/
    }

// Typed handlers
    void lirHandler (KeyT key);
    void hirResidentHandler (KeyT key); 

    template <typename F> 
    void hirNonResidentHandler (KeyT key, F slowGetPage)
    {
        auto it = findInList (key, HIR, &lirsStack);


        if (it == lirsStack.end())
        {
            hirList.push_front ({key, HIR});
            lirsStack.push_front ({key, HIR});
            cache_.insert ({key, slowGetPageWrap (slowGetPage, HIR, key)});
            dltFromHirAndCache();
        }
        else
        {
            lirsStack.erase (it);
            lirsStack.push_front ({key, LIR});
            cache_.insert ({key, slowGetPageWrap (slowGetPage, LIR, key)});

            lirBottomToHirFront();            
            dltFromHirAndCache();
        }
    }
////////////////////////////////////////////////////////////////////////////////////////

// Service functions used for moving, inserting, finding and deleting blocks in the lirs Stack, hash table and hir list
    void lirBottomToHirFront();
    void dltFromHirAndCache();
    auto findInList (KeyT key, State state, std::list<T> *thisList);
////////////////////////////////////////////////////////////////////////////////////////

    template <typename F> PageAndState slowGetPageWrap (F slowGetPage, State state, KeyT key)
    {
        PageAndState value = std::make_pair (slowGetPage(key), state);

        return value;
    }

    void stackPrune();
    void printer();
};

template <typename PageT, typename KeyT>
void lirs_cache_t<PageT, KeyT>::hirResidentHandler (KeyT key)
{
    auto keyItInLirs = findInList (key, HIR, &lirsStack);

    if (keyItInLirs == lirsStack.end())
    {
        auto hirListIt = findInList (key, HIR, &hirList);
        hirList.erase (hirListIt);
        hirList.push_front ({key, HIR});
    }            
    else 
    {
        auto hirListIt = findInList (key, HIR, &hirList);
        hirList.erase (hirListIt);
        lirsStack.erase (keyItInLirs);
        lirsStack.push_front ({key, LIR});
        
        auto cachePlacementKey = cache_.find (key);
        if (cachePlacementKey != cache_.end())
            cachePlacementKey->second.second = LIR;

        lirBottomToHirFront();
    }
}

template <typename PageT, typename KeyT>
void lirs_cache_t<PageT, KeyT>::lirHandler (KeyT key)
{
    static int counter = 0;
    auto keyItInLirs = findInList (key, LIR, &lirsStack);
    
    lirsStack.erase (keyItInLirs);
    stackPrune();
    lirsStack.push_front ({key, LIR});
    counter++;
}


template <typename PageT, typename KeyT>
void lirs_cache_t <PageT, KeyT>::lirBottomToHirFront()
{
    auto it = cache_.find (lirsStack.back().first);
    if (it != cache_.end())
        it->second.second = HIR;

    T tempPair = std::make_pair (lirsStack.back().first, HIR);
    hirList.push_front (tempPair);
    lirsStack.pop_back();
    stackPrune();       
}

template <typename PageT, typename KeyT>
void lirs_cache_t <PageT, KeyT>::dltFromHirAndCache()
{
    auto itInCache_ = cache_.find (hirList.back().first);
    cache_.erase (itInCache_);
    hirList.pop_back();
}

template <typename PageT, typename KeyT>
auto lirs_cache_t <PageT, KeyT>::findInList (KeyT key, State state, std::list<T> *thisList)
{
    T thisPair = std::make_pair (key, state); 
    return std::find (thisList->begin(), thisList->end(), thisPair); 
}

template <typename PageT, typename KeyT>
void lirs_cache_t <PageT, KeyT>::stackPrune()
{
    while (lirsStack.back().second == HIR)
        lirsStack.pop_back();
}

template <typename PageT, typename KeyT>
void lirs_cache_t <PageT, KeyT>::printer()
{
    std::cout << std::endl << "Lirs stack S" << std::endl;
    for (typename std::list<T>::iterator it = lirsStack.begin(); it != lirsStack.end(); it++)
    {
        if (it->second == 1)
            std::cout << "key:" << it->first << "; value: HIR" << std::endl;
        else
            std::cout << "key:" << it->first << "; value: LIR" << std::endl;
    }

    std::cout << "Hir list Q" << std::endl;
    for (typename std::list<T>::iterator it = hirList.begin(); it != hirList.end(); it++)        
    {
        if (it->second == 1)
            std::cout << "key:" << it->first << "; value: HIR" << std::endl;
        else
            std::cout << "key:" << it->first << "; value: LIR" << std::endl;
    }
    
    std::cout << std::endl <<"Cache contents" << std::endl;
    for (typename std::unordered_map<KeyT, PageAndState>::iterator it = cache_.begin(); it != cache_.end(); it++)        
    {
        if (it->second.second == 1)
            std::cout << "key:" << it->first << "; value: HIR" << std::endl;
        else
            std::cout << "key:" << it->first << "; value: LIR" << std::endl;
    }

    std::cout << std::endl;
}


int lirsCache (size_t size, std::vector <int> vec)
{
    int hits = 0;
    lirs_cache_t <int, int> testCache {size};
    
    for (auto it = vec.begin(); it != vec.end(); it++)
        if (testCache.newPageHandler(*it, slowGetPageInt))
            hits += 1;

    return hits;
}

}

int slowGetPageInt (int key) {return key;}