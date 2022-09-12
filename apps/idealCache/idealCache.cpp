#include "ideal-cache/idealCache.hpp"
#include <cassert>

int main()
{
    size_t size = 0;
    size_t amount = 0;

    std::cin >> size >> amount; 
    assert (std::cin.good());

    using PairIntInt = std::pair<int, int>;
    std::vector <PairIntInt> blank = {};

    for(size_t i = 0; i < amount; i++)
    {
        int q = 0;
        std::cin >> q;
        assert (std::cin.good());

        auto pair = std::make_pair (q, 0);
        blank.push_back (pair);
    }

    caches::ideal_cache<int> thisIdealCache {size, 0};
    int hits = thisIdealCache.prepareAndRun (blank);
    std::cout << hits << std::endl;

    return 0;
}