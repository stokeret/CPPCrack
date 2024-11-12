#include "bch.h"
#include <iostream>
#include <string>
#include <cctype>

using namespace std;

// Calculates the mod 11 of integer x, to ensure a positive result 11 is added to the mod result which is then "modded" by 11 again.
int mod11(int x) {
    return ((x % 11) + 11) % 11;
}

// Calculates the modular multiplicative inverse of integer x, modulo 11. It utilises the mod11 function to ensure x will be positive.
// It then uses a for loop to find the value of i that will satisy (x * i) mod 11 = 1. This value is then returned as the inverse.
int inverse(int x) {
    x = mod11(x);
    for (int i = 1; i < 11; i++) {
        if ((x * i) % 11 == 1) {
            // If no inverse exists return -1
            return i;
        }
    }
    return -1;
}

// Calculates the square root of x mod 11 using a square root table. I had to use a square root table as C++ does not have a way to
// calculate the roots of a given field. mod11 function is called to normalise x then it will return the corresponding square root
// from the table which is stored in the array sqrt_table[11]. If there are no square roots then the function will return -1. 
int sqrt_mod11(int x) {
    x = mod11(x);
    int sqrt_table[11] = {-1, 1, -1, 5, 2, 4, -1, -1, -1, 3, -1};
    return sqrt_table[x];
}

// Calculates the syndromes, s1, s2, s3, s4, it simply multiplies each element of the array with the corresponding coefficient, sums
// up the products and then uses the mod11 fucntion to take the result mod 11.
int calculateSyndrome(int d[], int coefficients[]) {
    int sum = 0;
    for (int i = 0; i < 10; i++) {
        sum += d[i] * coefficients[i];
    }
    return mod11(sum);
}

int main() {
    // Variable decleration and input handling
    string input;
    int d[10];
    cout << "Enter a ten-digit BCH(10,6) codeword: ";
    cin >> input;

    // Check to validate that the input is of length 10
    if (input.length() != 10) {
        cout << "Invalid input! You must enter exactly ten digits." << endl;
        return 1;
    }

    // Check to validate that only inputs containing digits 0 - 9 are passed through the program
    for (int i = 0; i < 10; i++) {
        if (!isdigit(input[i])) {
            cout << "Invalid input! Only digits 0-9 are allowed." << endl;
            return 1;
        }
        d[i] = input[i] - '0'; // Convert input char string to int
    }
    
    // Print the input to console
    cout << "input: ";
    for (int i = 0; i < 10; i++) {
        cout << d[i];
    }
    cout << endl;

    // Syndrome coefficiant array
    int s1_coefficients[10] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
    int s2_coefficients[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    int s3_coefficients[10] = {1, 4, 9, 5, 3, 3, 5, 9, 4, 1};
    int s4_coefficients[10] = {1, 8, 5, 9, 4, 7, 2, 6, 3, 10};

    // The d array which stores input and the coefficiants for the syndromes are sent to calculateSyndrome function
    int s1 = calculateSyndrome(d, s1_coefficients);
    int s2 = calculateSyndrome(d, s2_coefficients);
    int s3 = calculateSyndrome(d, s3_coefficients);
    int s4 = calculateSyndrome(d, s4_coefficients);

    // Check for no present errors
    if (s1 == 0 && s2 == 0 && s3 == 0 && s4 == 0) {
        cout << "No error." << endl;
        return 0;
    }

    // Calculate the values of P, Q and R, using the mod11 function
    int P = mod11(s2 * s2 - s1 * s3);
    int Q = mod11(s1 * s4 - s2 * s3);
    int R = mod11(s3 * s3 - s2 * s4);

    // If P, Q and R all equal 0 then we know it is a single error
    if (P == 0 && Q == 0 && R == 0) {
        // If s1 is zero we cannot compute the inverse
        if (s1 == 0) {
            cout << "Error: s1 is zero, cannot compute inverse." << endl;
            return 1;
        }
        // Calculate i as s2 * s1^-1 mod 11;
        int i = mod11(s2 * inverse(s1));
        // Check to validate i's value
        if (i <= 0 || i > 10) {
            cout << "Error: Invalid error position i, Indicating more than 2 errors." << endl;
            return 1;
        }
        // Complete error correction and modify the input and validate using the mod11 function before passing to the correctedInput variable
        // If the value is 10 then it is replaced with 0, not sure if this is correct or if 10 should equal an error but the program works regardless
        d[i - 1] = mod11(d[i - 1] - s1);
        if (d[i - 1] == 10) d[i - 1] = 0;

        // Print single_error and specified variables then print the corrected output
        cout << "single_error(i=" << i << ", a=" << s1 << ", syn(" << s1 << ", " << s2 << ", " << s3 << ", " << s4 << "))" << endl;
        string correctedInput;
        for (int j = 0; j < 10; j++) {
            correctedInput += to_string(d[j]);
        }
        cout << "output: " << correctedInput << endl;
    } else {
        // Calculate the value of the discriminant
        int discriminant = mod11(Q * Q - 4 * P * R);
        // Calculate the square root of the discriminant using sqrt_mod11 function
        int sqrtDiscriminant = sqrt_mod11(discriminant);

        // Check that neither the discriminant or the square root of the discriminant mod 11 = 0 or -1 as this would indicate more than 2 errors
        if (discriminant == 0 || sqrtDiscriminant == -1) {
            cout << "Error: No square root found for the discriminant, indicating more than two errors." << endl;
            cout << "syn(" << s1 << ", " << s2 << ", " << s3 << ", " << s4 << "), pqr(" << P << ", " << Q << ", " << R << ")" << endl; 
            return 1;
        }

        // Calculate (2 * P)^-1
        int twoPInverse = inverse(2 * P);

        // Check to validate twoPInverse
        if (twoPInverse == -1) {
            cout << "Error: Inverse of 2P does not exist, indicating more than 2 errors." << endl;
            return 1;            
        }

        // Calculating i and j after calculating the square root of the discriminant and multiplying by the inverse of (2 * P)
        int i = mod11(mod11(-Q + sqrtDiscriminant) * twoPInverse);
        int j = mod11(mod11(-Q - sqrtDiscriminant) * twoPInverse);

        // Check to validate the values of i and j
        if (i <= 0 || i > 10 || j <= 0 || j > 10 || i == j) {
            cout << "Error: Invalid error positions i or j, indicating more than 2 errors." << endl;
            return 1;
        }

        // Calculate the inverse of (i - j)
        int ijInverse = inverse(mod11(i - j));
        
        // Check to validate the value of the inverse of i - j
        if (ijInverse == -1) {
            cout << "Error: Inverse of (i - j) does not exist, indicating more than two errors." << endl;
            return 1;
        }

        // Calculate the value of b
        int b = mod11(mod11(i * s1 - s2) * ijInverse);

        // Calculate the value of a
        int a = mod11(s1 - b);

        // Check to validate the values of a and b
        if (a == 10 || b == 10) {
            cout << "Error: a or b has a value of 10, indicating more than two errors." << endl;
            return 1;
        }

        // Same as above but for positions i and j
        d[i - 1] = mod11(d[i - 1] - a);
        d[j - 1] = mod11(d[j - 1] - b);

        // Same as above but for positions i and j
        if (d[i - 1] == 10) d[i - 1] = 0;
        if (d[j - 1] == 10) d[j - 1] = 0;

        // Output recognised double_error as well as variables, i, a, j and b, aswell as syndromes, s1, s2, s3, s4 and finally the values of the parameters P, Q and R
        cout << "double_error(i=" << i << ", a=" << a << ", j=" << j << ", b=" << b << "), syn(" << s1 << ", " << s2 << ", " << s3 << ", " << s4 << "), PQR(" << P << ", " << Q << ", " << R << ")" << endl;
        string correctedInput;
        for (int k = 0; k < 10; k++) {
            correctedInput += to_string(d[k]);
        }
        // Output the error corrected string
        cout << "output: " << correctedInput << endl;
    }
    return 0;
}