#include <iostream>
#include <string>
#include <map>

class Warehouse {
    std::map<const std::string, int> fruits;
public:
    /**
     * @brief Adds a specified number of fruits to the warehouse.
     *
     * This function takes a string representing the name of the fruit and an integer representing the count of fruits to be added.
     * It adds the specified count of fruits to the warehouse inventory.
     *
     * @param fruit The name of the fruit to be added, e,g, (mango, apple...).
     * @param count The number of fruits to be added.
     */
    void addFruits(const std::string &fruit, int count) {
        fruits[fruit] += count;
    }

    bool takeFruit(const std::string &fruit) {
        if (fruits[fruit] > 0) {
            fruits[fruit]--;
            return true;
        }
        return false;
    }

    void printAllFruits() {
        for (const auto &item: fruits) {
            std::cout << item.first << ":" << item.second << "\n";
        }
    }
};