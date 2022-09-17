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
class lirs_cache_t
{
    size_t sz_;
    size_t hir_sz_ = 0;    

    bool isFull = false;
    bool isLirFull = false;


    enum State 
    {
        LIR = 0,
        HIR = 1
    };

    
    using T = typename std::pair <KeyT, State>;  
    using IterType = typename std::list <T>::iterator;
    struct StatePageAndIterator
    {
        State state;
        PageT page;
        IterType iterInLirsStack;
        IterType iterInHirList;

        StatePageAndIterator (State state_, PageT page_, IterType iterLirs_, IterType iterHir_) 
                              : state(state_), page (page_), iterInLirsStack (iterLirs_), iterInHirList (iterHir_){};
    };
    
    struct StateAndIterator
    {
        State state;
        IterType iterInLirsStack;

        StateAndIterator (State state_, IterType iterLirs_)
                          : state (state_), iterInLirsStack (iterLirs_){};
    };

    using ItInCache = typename std::unordered_map <KeyT, StatePageAndIterator*>::iterator;

    std::unordered_map<KeyT, StatePageAndIterator*> cache_ = {};
    std::unordered_map<KeyT, StateAndIterator*> hirNonResidentInLirsHolder = {};
    std::list<T> lirsStack = {};
    std::list<T> hirList = {};


// Typed handlers for not full cache

    void lirNotFullAndNotInCache (KeyT key, PageT (*slowGetPage)(KeyT key))
    {
        lirsStack.push_front ({key, LIR});
        cache_.insert ({key, slowGetPageWrap (slowGetPage, LIR, key, lirsStack.begin(), hirList.end())});

        if (lirsStack.size() >= sz_ - hir_sz_)            
            isLirFull = true;
    }

    void fillingNonResidentHirWasInStk (KeyT key, PageT (*slowGetPage)(KeyT key))
    {
        lirsStack.push_front ({key, LIR});
        cache_.insert ({key, slowGetPageWrap (slowGetPage, LIR, key, lirsStack.begin(), hirList.end())});
    }

    void fillingLirHit (KeyT key, ItInCache iter)
    {
        auto lirIt = iter->second->iterInLirsStack;
        auto lirIt2 = iter->second->iterInLirsStack;
        auto hirIt = iter->second->iterInHirList;
        if(lirIt == lirsStack.end() && lirIt2 == lirsStack.end() && hirIt == hirList.end())
        {
            lirsStack.push_front ({key, HIR});
            hirList.push_front ({key, HIR});

            iter->second->iterInLirsStack = lirsStack.begin();
            iter->second->iterInHirList = hirList.begin();
        }
        else if (lirIt != lirsStack.end() && hirIt == hirList.end())
        {
            lirHandler (key, iter);
            return;
        }
        else if (lirIt2 != lirsStack.end())
        {
            hirResidentHandler (key, iter);
        }
        else if (hirIt != hirList.end())
        {
            hirResidentHandler (key, iter);
        }
    }

    void fillingNonResidentHirWasNotInStk (KeyT key, PageT (*slowGetPage)(KeyT key))
    {

        hirList.push_front ({key, HIR});
        lirsStack.push_front ({key, HIR});

        cache_.insert ({key, slowGetPageWrap (slowGetPage, HIR, key, lirsStack.begin(), hirList.begin())});
        /*lirBottomToHirFront();
        stackPrune();*/
    }

// Typed handlers
    void lirHandler (KeyT key, ItInCache cacheIt);
    void hirResidentHandler (KeyT key, ItInCache cacheIt); 

    void hirNonResidentHandler (KeyT key, PageT (*slowGetPage)(KeyT key))
    {
        auto it = hirNonResidentInLirsHolder.find (key); //findInList (key, HIR, &lirsStack);


        if ( it == hirNonResidentInLirsHolder.end())
        {
            hirList.push_front ({key, HIR});
            lirsStack.push_front ({key, HIR});
            cache_.insert ({key, slowGetPageWrap (slowGetPage, HIR, key, lirsStack.begin(), hirList.begin())});
            dltFromHirAndCache();
        }
        else
        {
            lirsStack.erase (it->second->iterInLirsStack);
            lirsStack.push_front ({key, LIR});
            cache_.insert ({key, slowGetPageWrap (slowGetPage, LIR, key, lirsStack.begin(), hirList.end())});

            lirBottomToHirFront();            
            dltFromHirAndCache();

            delete it->second;
            hirNonResidentInLirsHolder.erase (it);
        }
    }
////////////////////////////////////////////////////////////////////////////////////////

// Service functions used for moving, inserting, g and deleting blocks in the lirs Stack, hash table and hir list
    void lirBottomToHirFront();
    void dltFromHirAndCache();
    auto findInList (KeyT key, State state, std::list<T> *thisList) const;
////////////////////////////////////////////////////////////////////////////////////////

    StatePageAndIterator* slowGetPageWrap (PageT (*slowGetPage)(KeyT key), State state, KeyT key,
                                                                IterType iterLirs, IterType iterHir)
    {
        StatePageAndIterator* value = new StatePageAndIterator (state, slowGetPage(key), iterLirs, iterHir);

        return value;
    }

    void stackPrune();
    void printer() const;

public:
    bool newPageHandler (KeyT key, PageT (*slowGetPage)(KeyT key))
    {
        auto hit = cache_.find (key);
        if (!isFull)
        {
            bool isInCache = !(hit == cache_.end());
            
            if (!isLirFull && !isInCache)
                lirNotFullAndNotInCache (key, slowGetPage);
            else if (!isLirFull)
                lirHandler (key, hit);      
            else if (isInCache)
            {
                fillingLirHit (key, hit);
            }
            else
            {
                auto keyItInLirs = hirNonResidentInLirsHolder.find (key);//findInList (key, HIR, &lirsStack);

                if (keyItInLirs == hirNonResidentInLirsHolder.end())
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

        if (hit->second->state == LIR)
            lirHandler (key, hit);
        else if (hit->second->state == HIR)
            hirResidentHandler (key, hit);

        return true;
    }
    
    
    lirs_cache_t (size_t sz) : sz_(sz) 
    {
        if (sz < 10 && sz > 1)
            hir_sz_ = 2;
        else if (sz == 1)
            hir_sz_ = 1;
        else
            hir_sz_ = sz/10;
    };

    ~lirs_cache_t ()
    {
        for (auto it = cache_.begin(), itend = cache_.end(); it != itend; ++it)
            delete it->second;

        for (auto it = hirNonResidentInLirsHolder.begin(), itend = hirNonResidentInLirsHolder.end(); 
             it != itend; ++it)
            delete it->second;
        
    }
};

template <typename PageT, typename KeyT>
void lirs_cache_t<PageT, KeyT>::hirResidentHandler (KeyT key, ItInCache cacheIt)
{
    auto keyItInLirs = cacheIt->second->iterInLirsStack;//findInList (key, HIR, &lirsStack);

    if (keyItInLirs == lirsStack.end())
    {
        auto hirListIt = cacheIt->second->iterInHirList;//findInList (key, HIR, &hirList);
        hirList.erase (hirListIt);
        hirList.push_front ({key, HIR});

        cacheIt->second->iterInHirList = hirList.begin();
    }            
    else 
    {

        auto hirListIt = cacheIt->second->iterInHirList;
        hirList.erase (hirListIt);
        lirsStack.erase (keyItInLirs);
        lirsStack.push_front ({key, LIR});
        
        cacheIt->second->state = LIR;
        cacheIt->second->iterInLirsStack = lirsStack.begin();
        cacheIt->second->iterInHirList = hirList.end();

        lirBottomToHirFront();
    }
}

template <typename PageT, typename KeyT>
void lirs_cache_t<PageT, KeyT>::lirHandler (KeyT key, ItInCache cacheIt)
{
    auto keyItInLirs = cacheIt->second->iterInLirsStack;
    
    lirsStack.erase (keyItInLirs);
    stackPrune();
    lirsStack.push_front ({key, LIR});
    cacheIt->second->iterInLirsStack = lirsStack.begin();
}


template <typename PageT, typename KeyT>
void lirs_cache_t <PageT, KeyT>::lirBottomToHirFront()
{
    auto it = cache_.find (lirsStack.back().first);

    T tempPair = std::make_pair (lirsStack.back().first, HIR);
    hirList.push_front (tempPair);
    
    if (it != cache_.end())
    {
        it->second->state = HIR;
        it->second->iterInLirsStack = lirsStack.end();
        it->second->iterInHirList = hirList.begin();
    }

    lirsStack.pop_back();
    stackPrune();       
}

template <typename PageT, typename KeyT>
void lirs_cache_t <PageT, KeyT>::dltFromHirAndCache()
{
    auto itInCache_ = cache_.find (hirList.back().first);

    if (itInCache_->second->iterInLirsStack != lirsStack.end())
        hirNonResidentInLirsHolder.insert ({hirList.back().first, 
                                            new StateAndIterator (HIR, itInCache_->second->iterInLirsStack)});

    delete itInCache_->second;
    cache_.erase (itInCache_);
    hirList.pop_back();
}

template <typename PageT, typename KeyT>
auto lirs_cache_t <PageT, KeyT>::findInList (KeyT key, State state, std::list<T> *thisList) const
{
    T thisPair = std::make_pair (key, state); 
    return std::find (thisList->begin(), thisList->end(), thisPair); 
}

template <typename PageT, typename KeyT>
void lirs_cache_t <PageT, KeyT>::stackPrune()
{
    while (lirsStack.back().second == HIR)
    {
        auto itCache = cache_.find (lirsStack.back().first);
        if (itCache != cache_.end())
            itCache->second->iterInLirsStack = lirsStack.end();
        
        auto itInHirHolder = hirNonResidentInLirsHolder.find (lirsStack.back().first);
        if (itInHirHolder != hirNonResidentInLirsHolder.end())    
        {            
            delete itInHirHolder->second;
            hirNonResidentInLirsHolder.erase (itInHirHolder);
        }

        lirsStack.pop_back();
    }
}

template <typename PageT, typename KeyT>
void lirs_cache_t <PageT, KeyT>::printer() const
{
    std::cout << std::endl << "Lirs stack S" << std::endl;
    for (typename std::list<T>::iterator it = lirsStack.begin(), itend = lirsStack.end(); it != itend; ++it)
    {
        if (it->second == HIR)
            std::cout << "key:" << it->first << "; value: HIR" << std::endl;
        else
            std::cout << "key:" << it->first << "; value: LIR" << std::endl;
    }

    std::cout << "Hir list Q" << std::endl;
    for (typename std::list<T>::iterator it = hirList.begin(), itend = hirList.end(); it != itend; ++it)        
    {
        if (it->second == HIR)
            std::cout << "key:" << it->first << "; value: HIR" << std::endl;
        else
            std::cout << "key:" << it->first << "; value: LIR" << std::endl;
    }
    
    std::cout << std::endl <<"Cache contents" << std::endl;
    for (ItInCache it = cache_.begin(), itend = cache_.end(); it != itend; ++it)        
    {
        if (it->second->state == HIR)
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
    
    for (std::vector<int>::iterator it = vec.begin(), itend = vec.end(); it != itend; ++it)
    {
        if (testCache.newPageHandler(*it, slowGetPageInt))
            hits += 1;
    }
    return hits;
}

}

int slowGetPageInt (int key) {return key;}