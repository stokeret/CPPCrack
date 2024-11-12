#include <string>
#include <iostream>
#include "luhn.h"

using namespace std;

// Function decleration for credit card check
bool isValidCreditCard(const string& cardNumber) {
    int sum = 0;
    bool alternate = false;
    // Iterate through card number right to left
    for (int i = cardNumber.length() - 1; i >= 0; i--) {
        int digit = cardNumber[i] - '0';

        if (alternate) {
            digit *= 2;
            if (digit > 9)
                digit -= 9;
        }

        sum += digit;
        alternate = !alternate;
    }

    // The card number is valid if the sum modulo 10 = 0
    return (sum % 10 == 0);
}

// Function to calculate check digit
char calculateCheckDigit(const string& cardNumber) {
    int sum = 0;
    bool alternate = false;

    // Virtually the same as IsValidCreditCard func
    for (int i = cardNumber.length() - 1; i >= 0; i--) {
        int digit = cardNumber[i] - '0';

        if (!alternate) {
            digit *= 2;
            if (digit > 9)
                digit -= 9;
        }

        sum += digit;
        alternate = !alternate;
    }

    // Calculate check digit
    int checkDigit = (10 - (sum % 10)) % 10;
    return checkDigit + '0';
}

int main(){
    string cardNumber;
    int checkDigit;

    cout << "Enter a 15 or 16 credit card number: ";
    // User input handling, stored in a string.
    cin >> cardNumber;

    if (cardNumber.length() >= 15 && cardNumber.length() <= 16) {
        if (cardNumber.length() == 15) {
            // If card number has a length of 15 digits, calculate and append the check digit
            checkDigit = calculateCheckDigit(cardNumber);
            cout << "The calculated check digit for this credit card number is " << calculateCheckDigit(cardNumber) << endl;            
            cardNumber = cardNumber += checkDigit;
            if (isValidCreditCard(cardNumber) == true) {
                // Checks the card number inputted against the isValidCreditCard function
                // Prints the CC number to console and tells the user it is valid.
                cout << cardNumber << " is the valid full credit card number" << endl;
            }
        } else if (cardNumber.length() == 16 && isValidCreditCard(cardNumber) == true) {
            cout << cardNumber << " is a valid credit card number" << endl;
        } 
        // Checks for invalid credit card numbers
        else if (cardNumber == "0000000000000000" || "000000000000000") {
            cout << "Sorry, this is not a valid credit card number" << endl;
        }
    } else {
        cout << "Sorry, this program only takes inputs of length 15 or 16" << endl;
    }
    return 0;
}