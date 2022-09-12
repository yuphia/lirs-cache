#include "ideal-cache/idealCache.hpp"
#include "lirs-cache/cache.hpp"
#include <cassert>

int main()
{
    size_t size = 0;
    size_t amount = 0;

    int lirsHits = 0;

    std::cin >> size >> amount; 
    assert (std::cin.good());

    using PairIntInt = std::pair<int, int>;
    std::vector <PairIntInt> blank = {};

    caches::lirs_cache_t <int> cache {size};

    for(size_t i = 0; i < amount; i++)
    {
        int q = 0;
        std::cin >> q;
        assert (std::cin.good());
        
        if (cache.newPageHandler(q, slowGetPageInt))
            lirsHits += 1;        

        auto pair = std::make_pair (q, 0);
        blank.push_back (pair);
    }

    caches::ideal_cache<int> thisIdealCache {size, 0};
    int hits = thisIdealCache.prepareAndRun (blank);
    std::cout << "Ideal: " << hits << std::endl;
    std::cout << "Lirs: " << lirsHits << std::endl;

    return 0;
}