#pragma once

#include <algorithm>
#include <unordered_map>
#include <vector>
#include <iterator>
#include <iostream>

namespace caches
{
int idealCache (size_t size, std::vector <int> vec);

template <typename KeyT>
class ideal_cache
{
    size_t sz_;

    bool isFull = false;
    bool isPrintable_ = false;

    
    using T = typename std::pair <KeyT, int>;
    using IterVector = typename std::vector<T>::iterator;

    std::unordered_map<KeyT, int> predictor = {};
    std::unordered_map<KeyT, IterVector> indexer = {};

    std::vector <T> indexedVector = {};


    int emulateCache()
    {
        predictor.clear();
        int hits = 0;


        for (auto it = indexedVector.begin(), itend = indexedVector.end(); it != itend; ++it)
        {
            printerStart<KeyT> (it->first);

            auto itInCache = predictor.find (it->first);
            if (itInCache != predictor.end())
            {
                itInCache->second = it->second;
                hits++;
            }
            else if (sz_ == predictor.size())
            {
                auto biggest = findBiggestInPredictor();
                predictor.erase (biggest);
                predictor.insert ({it->first, it->second});
            }
            else if (sz_ > predictor.size())
            {
                predictor.insert ({it->first, it->second});
            }

            decrementLenInCache();
            printerEnd();
        }

        return hits;
    }

    auto findBiggestInPredictor()
    {
        auto it = predictor.begin();
        auto max = it;

        auto itend = predictor.end();

        for (; it != itend; it++)
        {
            if (it->second > max->second)
                max = it;
        }
        return max;
    }

    void printer()
    {
        if (isPrintable_)
        {
            std::cout << "printing the cache:" << std::endl;

            for (auto it = predictor.begin(), itend = predictor.end(); it != itend; ++it)
            {
                std::cout << "element: " << it->first << std::endl;
            }
        }
    }

    void printerEnd()
    {
        if (isPrintable_)
        {
            std::cout << std::endl << "End" << std::endl;
            printer();
            std::cout << "////////////////////////////////////" << std::endl;
        }
    }

    template <typename T>
    void printerStart (T page)
    {
        if (isPrintable_)
        {
            std::cout << "////////////////////////////////////" << std::endl;

            std::cout << std::endl << "Input: " << page << std::endl;

            std::cout << "Beginning" << std::endl;
            printer ();
        }
    }

    void decrementLenInCache()
    {
        for (auto it = predictor.begin(), itend = predictor.end(); it != itend; ++it)
            it->second--;
    }
   
public:   
    ideal_cache (size_t sz, bool isPrintable) : sz_(sz), isPrintable_ (isPrintable) {};
    
    int prepareAndRun (std::vector<T> inputArray)
    {
        auto vectorEnd = inputArray.end();

        for (auto it = inputArray.begin(), itend = inputArray.end(); it < itend; ++it)
        {
            auto itIndexed = indexer.find (it->first);
            if (itIndexed == indexer.end())
            {
                int lengthToEnd = std::distance (it, vectorEnd);
                indexer.insert ({it->first, it});
                it->second = lengthToEnd;
            }
            else
            {
                int len = (int) (it - itIndexed->second);
                it->second = vectorEnd - it;
                itIndexed->second->second = len;
                itIndexed->second = it;
            }
        }

        for (auto it = inputArray.begin(), itend = inputArray.end(); it < itend; ++it)
        {
            indexedVector.push_back (*it);
        }

        return emulateCache();
    }
};

int idealCache (size_t size, std::vector <int> vec)
{
    ideal_cache<int> testCache {size, 0};

    std::vector <std::pair<int, int>> newVec;
    for (auto it = vec.begin(), itend = vec.end(); it != itend; ++it)
        newVec.push_back ({*it, 0});
    int hits = testCache.prepareAndRun (newVec);

    return hits;
}

}
    