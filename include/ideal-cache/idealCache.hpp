#pragma once

#include <algorithm>
#include <unordered_map>
#include <vector>
#include <iterator>
#include <iostream>

namespace caches
{
template <typename KeyT>
struct ideal_cache
{
    size_t sz_;

    bool isFull = false;

    ideal_cache (size_t sz) : sz_(sz) {};
    
    using T = typename std::pair <KeyT, int>;
    using IterVector = typename std::vector<T>::iterator;

    std::unordered_map<KeyT, int> predictor = {};
    std::unordered_map<KeyT, IterVector> indexer = {};

    std::vector <T> indexedVector = {};

    int prepareAndRun (std::vector<T> inputArray)
    {
        auto vectorEnd = inputArray.end();

        for (auto it = inputArray.begin(); it < inputArray.end(); it++)
        {
            auto itIndexed = indexer.find (it->first);
            if (itIndexed == indexer.end())
            {
                int lengthToEnd = (int)(vectorEnd - it);
                indexer.insert ({it->first, it});
                it->second = lengthToEnd;
            }
            else
            {
                int len = (int) (it - itIndexed->second);
                it->second = vectorEnd - it;
                itIndexed->second->second = len;
                itIndexed->second = it;
                std::cout << "here: " << len << " " << it->first << std::endl; 
            }
        }

        for (auto it = inputArray.begin(); it < inputArray.end(); it++)
        {
            indexedVector.push_back (*it);
        }

        return emulateCache();
    }

    int emulateCache()
    {
        predictor.clear();
        int hits = 0;

        for (auto it = indexedVector.begin(); it != indexedVector.end(); it++)
        {
            std::cout << "////////////////////////////////////" << std::endl;

            std::cout << std::endl << "Input: " << it->first << std::endl;

            std::cout << "Beginning" << std::endl;
            printer ();

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

            std::cout << std::endl << "End" << std::endl;
            printer();
            std::cout << "////////////////////////////////////" << std::endl;
            decrementLenInCache();
        }

        return hits;
    }

    auto findBiggestInPredictor()
    {
        auto it = predictor.begin();
        auto max = it;

        for (; it != predictor.end(); it++)
        {
            if (it->second > max->second)
                max = it;
        }
        return max;
    }

    void printer()
    {
        std::cout << "printing the cache:" << std::endl;

        for (auto it = predictor.begin(); it != predictor.end(); it++)
        {
            std::cout << "element: " << it->first << std::endl;
        }
    }

    void decrementLenInCache()
    {
        for (auto it = predictor.begin(); it != predictor.end(); it++)
            it->second--;
    }
};

}