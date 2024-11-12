#include <cstdlib>
#include <iostream>
#include "BCHGen.h"

using namespace std;

string BCHGen(const string& inputNumber) {
    // Pads the user-inputted number with 0's to ensure a length of 6
    string paddedInput = inputNumber;
    paddedInput.insert(0, 6 - paddedInput.length(), '0');

    // Initialises an array of size 10 to store the user input and the additional digits that will be calculated
    // First 6 digits of the padded input are stored within digits
    int digits[10] = {0};
    for (int i = 0; i < 6; i++){
        digits[i] = paddedInput[i] - '0';
    }

    // d7, d8, d9, d10, are all arrays containing sets of multipliers to be used in the calculation of the 7th, 8th, 9th and 10th digit
    // For each array a sum is calculated, s7, s8, s9, s10. This is calculated by multiplacation of the first 6 digits by their
    // corresponding multiplier stored in the arrays and then adding them together.
    // Each sum % 11 will obtain a digit from 0,9 and this is then stored in the array digits as an additional digit.
    // If any of these additional digits are 10 or larger then the program will return "unusable number"
    // If all digits are less than 10 then these digits are deemed valid and added to the initial 6 digit code to make a 10 digit one
    // If this is the case then the completed number is printed to the console
    int d7[6] = {4, 10, 9, 2, 1, 7};
    int s7 = 0;
    for (int i = 0; i < 6; i++){
        s7 += d7[i] * digits[i];
    }
    digits[6] = s7 % 11;
    if(digits[6] >= 10){
        return "unusable number";
    }
    
    int d8[6] = {7, 8, 7, 1, 9, 6};
    int s8 = 0;
    for (int i = 0; i < 6; i++){
        s8 += d8[i] * digits[i];
    }
    digits[7] = s8 % 11;
    if(digits[7] >= 10){
        return "unusable number";
    }

    int d9[6] = {9, 1, 7, 8, 7, 7};
    int s9 = 0;
    for (int i = 0; i < 6; i++){
        s9 += d9[i] * digits[i];
    }
    digits[8] = s9 % 11;
    if(digits[8] >= 10){
        return "unusable number";
    }

    int d10[6] = {1, 2, 9, 10, 4, 1};
    int s10 = 0;
    for (int i = 0; i < 6; i++){
        s10 += d10[i] * digits[i];
    }
    digits[9] = s10 % 11;
    if(digits[9] >= 10){
        return "unusable number";
    }

    string bch = paddedInput;
    for (int i = 6; i < 10; i++){
        if (digits[i] >= 10){
            return "unusable number";
        }
        bch += to_string(digits[i]);
    }

    return bch;
}

using namespace std;

int main() {
    // Prompts the user to enter a number
    string inputNumber;
    cout << "Enter a number" << endl;
    cin >> inputNumber;

    // Checks the user inputted number against the conditions within BCHGen
    string bch = BCHGen(inputNumber);
    if (bch == "unusable number") {
        cout << endl << bch << endl;
        return EXIT_FAILURE;
    }

    // Print completed number to the console
    cout << endl << "number generated: " << bch << endl;
    return 0;
}