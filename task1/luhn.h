#ifndef LUHN_H
#define LUHN_H

#include <string>

bool isValidCreditCard(const std::string& cardNumber);
char calculateCheckDigit(const std::string& cardNumber);

#endif // LUHN_H