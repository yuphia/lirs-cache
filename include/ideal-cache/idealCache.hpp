#pragma once

#include <algorithm>
#include <unordered_map>
#include <vector>
#include <iterator>
#include <iostream>
#include <map>

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
    using MultimapIter = typename std::multimap<int, KeyT>::iterator;


    std::multimap<int, KeyT> predictor = {};
    std::unordered_map<KeyT, MultimapIter> predictorHelper {};

    std::unordered_map<KeyT, IterVector> indexer = {};

    std::vector <T> indexedVector = {};

    int emulateCache()
    {
        predictor.clear();
        int hits = 0;

        for (auto it = indexedVector.begin(), itend = indexedVector.end(); it != itend; ++it)
        {
            printerStart<KeyT> (it->first);

            auto itInCache = predictorHelper.find (it->first);
            if (itInCache != predictorHelper.end())
            {
                predictor.erase (itInCache->second);
                predictorHelper.erase (itInCache);

                auto it2 = predictorHelper.insert ({it->first, predictor.insert ({it->second, it->first})});

                hits++;
            }
            else if (sz_ == predictorHelper.size())   
            {
                auto biggest = --predictor.end();

                if (biggest->first > it->second)
                {
                    predictorHelper.erase (predictorHelper.find (biggest->second));
                    predictor.erase (biggest);

                    predictorHelper.insert ({it->first, predictor.insert ({it->second, it->first})});
                }
            }
            else if (sz_ > predictorHelper.size())
                predictorHelper.insert ({it->first, predictor.insert ({it->second, it->first})});

            printerEnd();
        }

        return hits;
    }

    auto findBiggestInPredictor() const
    {
        auto it = predictor.begin();
        auto max = it;

        auto itend = predictor.end();

        for (; it != itend; ++it)
        {
            if (it->second > max->second)
                max = it;
        }
        return max;
    }

    void printer() const
    {
        if (isPrintable_)
        {
            std::cout << "printing the cache:" << std::endl;

            for (auto it = predictor.begin(), itend = predictor.end(); it != itend; ++it)
            {
                std::cout << "element: " << it->second << ", " << it->first << std::endl;
            }
            std::cout << std::endl;
        }
    }

    void printerEnd() const
    {
        if (isPrintable_)
        {
            std::cout << std::endl << "End" << std::endl;
            printer();
            std::cout << "////////////////////////////////////" << std::endl;
        }
    }

    template <typename T>
    void printerStart (T page) const
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

    void printVec()
    {
        for (auto x : indexedVector)
            std::cout << x.first << " : " << x.second << std::endl;
    }
   
public:   
    ideal_cache (size_t sz, bool isPrintable) : sz_(sz), isPrintable_ (isPrintable) {};
    
    int prepareAndRun (std::vector<T> inputArray)
    {
        auto vectorBegin = inputArray.begin();
        auto vectorEnd = inputArray.end();

        for (auto it = inputArray.begin(), itend = inputArray.end(); it < itend; ++it)
        {
            auto itIndexed = indexer.find (it->first);
            if (itIndexed == indexer.end())
            {
                int index = std::distance (vectorBegin, vectorEnd);
                indexer.insert ({it->first, it});
                it->second = index;
            }
            else
            {
                auto len = std::distance (vectorBegin, it);
                it->second = std::distance (vectorBegin, vectorEnd);
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
    