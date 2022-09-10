#pragma once

#include <iterator>
#include <list>
#include <unordered_map>
#include <vector>
#include <algorithm>

int slowGetPageInt (int key);

namespace caches 
{

template <typename PageT, typename KeyT = int>
struct lirs_cache_t
{
    size_t sz_;
    size_t hir_sz_ = 0;    

    bool isFull = false;
    bool isLirFull = false;

    lirs_cache_t (size_t sz) : sz_(sz) 
    {
        if (sz < 10)
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

        std::cout << "IS HIT = " << (hit != cache_.end()) << std::endl;
        std::cout << "isFull = " << isFull << std::endl;
        std::cout << "isLirFull = " << isLirFull << std::endl;

        printer();

        if (!isFull)
        {
            bool isInCache = !(hit == cache_.end());
            
            if (!isLirFull && !isInCache)
                lirNotFullAndNotInCache (key, slowGetPage);
            else if (!isLirFull)
                lirHandler (key);      
            else if (isInCache)
                fillingLirHit (key);

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

            printer();

            return isInCache;
        }

        if (hit == cache_.end())
        { 
            hirNonResidentHandler (key, slowGetPage);
            printer();
            return false;            
        }

        if (hit->second.second == LIR)
            lirHandler (key);
        else if (hit->second.second == HIR)
            hirResidentHandler (key);

        printer();
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
        if(lirIt == lirsStack.end())
        {
            lirsStack.push_front ({key, HIR});
            hirList.push_front ({key, HIR});
        }
        else
            lirHandler (key);
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
        std::cout << "Hir non resident" << std::endl;

        auto it = findInList (key, HIR, &lirsStack);

        cache_.insert ({key, slowGetPageWrap (slowGetPage, HIR, key)});

        if (it == lirsStack.end())
        {
            hirList.push_front ({key, HIR});
            lirsStack.push_front ({key, HIR});

            dltFromHirAndCache();
        }
        else
        {
            lirsStack.erase (it);
            lirsStack.push_front ({key, LIR});

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
    std::cout << "Hir resident" << std::endl;

    auto keyItInLirs = findInList (key, HIR, &lirsStack);

    if (keyItInLirs == lirsStack.end())
    {
        auto hirListIt = findInList (key, HIR, &hirList);
        
        hirList.erase (hirListIt);
        hirList.push_front ({key, HIR});

        if (hirList.size() != 1)
            dltFromHirAndCache();
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

        if (hirList.size() != 1)
            dltFromHirAndCache();
    }
}

template <typename PageT, typename KeyT>
void lirs_cache_t<PageT, KeyT>::lirHandler (KeyT key)
{
    std::cout << "Lir resident" << std::endl;

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

}

int slowGetPageInt (int key) {return key;}