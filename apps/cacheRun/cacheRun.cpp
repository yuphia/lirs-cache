#include <cassert>
#include <iostream>

#include "lirs-cache/cache.hpp"


int main()
{
    int hits = 0;
    int inputs_amount = 0;
    size_t cache_sz = 0;

    std::cin >> inputs_amount >> cache_sz;
    assert (std::cin.good());

    caches::lirs_cache_t <int> cache {cache_sz};

    for (int i = 0; i < inputs_amount; i++)
    {
        int q;
        std::cin >> q;
        assert(std::cin.good());
        if (cache.newPageHandler(q, slowGetPageInt))
            hits += 1;        
    }
    
    std::cout << hits << std::endl; 

    return 0;
}

