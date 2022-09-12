#include <gtest/gtest.h>
#include "lirs-cache/cache.hpp"

TEST(CacheTest, test1) {
    std::vector<int> arr1 = {2, 2, 1, 3, 3, 4, 1, 5, 6, 2};
    EXPECT_EQ(2, caches::lirsCache(3, arr1));//10
}
 
TEST(CacheTest, test2) {
    std::vector<int> arr2 = {1, 2, 3, 4, 5, 1, 2, 3, 4, 5};
    EXPECT_EQ(5, caches::lirsCache(5, arr2));//10
}

TEST(CacheTest, test3) {
    std::vector<int> arr3 = {4, 7, 3, 6, 85, 34, 6, 8, 5, 3, 4, 7, 8, 6, 4, 4, 5, 6, 7, 8};
    EXPECT_EQ(7, caches::lirsCache(5, arr3));//20
}

TEST(CacheTest, test4) {
    std::vector<int> arr4 = {1, 2, 3, 4, 5, 6, 1, 2, 7, 8};
    EXPECT_EQ(2, caches::lirsCache(5, arr4));//10
}

TEST(CacheTest, test5) {
    std::vector<int> arr5 = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
    EXPECT_EQ(33, caches::lirsCache(1, arr5));//33
}

TEST(CacheTest, test6) {
    std::vector<int> arr6 = {2, 4, 2, 4, 6, 4, 6, 7, 6, 6, 7, 9, 6, 4, 3, 5, 7, 8, 6, 5, 4, 3, 4, 5, 7, 8, 8, 7, 6};
    EXPECT_EQ(19, caches::lirsCache(5, arr6));//29
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}