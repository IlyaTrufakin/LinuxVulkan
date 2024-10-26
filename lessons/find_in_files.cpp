#include <memory>
#include <warehouse.h>

namespace {
    void example() {
        auto warehouse = std::make_unique<Warehouse>();
        warehouse->addFruits("peach", 3);
        warehouse->addFruits("pineapple", 5);
        warehouse->addFruits("mango", 1);
        warehouse->addFruits("apple", 5);

        auto result = warehouse->takeFruit("apple");
        if (result) {
            std::cout << "This apple was delicious!\n";
        }

        warehouse->printAllFruits();
    }
}