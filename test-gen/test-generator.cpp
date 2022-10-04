#include <iostream>
#include <fstream>

int main()
{
    srand (time(NULL));
    int amount = 0;
    int multiplier = 0;
    std::cin >> amount;
    std::cin >> multiplier;

    std::ofstream testDataFile;
    testDataFile.open ("test.dat", std::ios::trunc);
    
    testDataFile << 2*amount/10 << " ";
    testDataFile << amount << " ";


    for (int i = 0; i < amount; i++)
    {
        testDataFile << static_cast<int> ((rand() % (amount))/multiplier) << " ";
    }
    
    testDataFile << std::endl;

    testDataFile.close();

    return 0;
}