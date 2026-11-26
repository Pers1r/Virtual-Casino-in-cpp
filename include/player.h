#ifndef PLAYER_H
#define PLAYER_H
#include <string>
#include "global.h"

class Player {
    private:
        int cash;
        const std::string saveFile = GlobalConfig::SAVE_FILE_PATH;
    public:
        Player( int _cash);
        ~Player();

        // Getters
        int getCash() const;
        // Setters
        void setCash(int newCashAmount);
        
        // Methods
        void addCash(int amount);
        bool loseCash(int amount);
        bool isBankrupt();

        void saveToFile();
        void loadFromFile();

        // Loan
        void takeLoan(int amount);

};;

#endif