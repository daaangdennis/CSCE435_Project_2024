#include <iostream>
#include <vector>
#include <algorithm>
#include <cmath>
#include <limits>
#include <random>
#include <chrono>

unsigned int r, s;
std::vector<unsigned int> matrix;

void printMatrix(const std::string &stepName)
{
    std::cout << stepName << ":\n";
    for (unsigned int i = 0; i < r; ++i)
    {
        for (unsigned int j = 0; j < s; ++j)
        {
            std::cout << matrix[i * s + j] << " ";
        }
        std::cout << "\n";
    }
    std::cout << "\n";
}

void sortColumns()
{
    for (unsigned int j = 0; j < s; ++j)
    {
        std::vector<unsigned int> column;
        for (unsigned int i = 0; i < r; ++i)
        {
            column.push_back(matrix[i * s + j]);
        }
        std::sort(column.begin(), column.end());
        for (unsigned int i = 0; i < r; ++i)
        {
            matrix[i * s + j] = column[i];
        }
    }
}

void transpose()
{
    std::vector<unsigned int> temp = matrix;
    for (unsigned int i = 0; i < r; ++i)
    {
        for (unsigned int j = 0; j < s; ++j)
        {
            matrix[j * r + i] = temp[i * s + j];
        }
    }
    temp = matrix;
    for (unsigned int i = 0; i < r * s; ++i)
    {
        unsigned int new_row = i / s;
        unsigned int new_col = i % s;
        matrix[new_row * s + new_col] = temp[i];
    }
}

void untranspose()
{
    std::vector<unsigned int> temp = matrix;
    for (unsigned int i = 0; i < r; ++i)
    {
        for (unsigned int j = 0; j < s; ++j)
        {
            matrix[i * s + j] = temp[j * r + i];
        }
    }
    temp = matrix;
    for (unsigned int i = 0; i < r * s; ++i)
    {
        unsigned int new_row = i / s;
        unsigned int new_col = i % s;
        matrix[new_row * s + new_col] = temp[i];
    }
}

void shift()
{
    unsigned int shift_amount = r / 2;
    std::vector<unsigned int> shifted_matrix(r * (s + 1), std::numeric_limits<unsigned int>::max());

    for (unsigned int i = 0; i < r; ++i)
    {
        for (unsigned int j = 0; j < s; ++j)
        {
            unsigned int new_row = i + shift_amount;
            if (new_row < r)
            {
                shifted_matrix[new_row * (s + 1) + j] = matrix[i * s + j];
            }
            else
            {
                shifted_matrix[(new_row - r) * (s + 1) + j + 1] = matrix[i * s + j];
            }
        }
    }

    for (unsigned int i = 0; i < shift_amount; ++i)
    {
        shifted_matrix[i * (s + 1)] = 0; // Use 0 instead of -inf for unsigned int
    }

    matrix = shifted_matrix;
    s += 1;
}

void unshift()
{
    unsigned int shift_amount = r / 2;
    std::vector<unsigned int> unshifted_matrix(r * (s - 1));

    for (unsigned int i = 0; i < r; ++i)
    {
        for (unsigned int j = 0; j < s - 1; ++j)
        {
            unsigned int old_row = (i + shift_amount) % r;
            unsigned int old_col = (old_row < shift_amount) ? j + 1 : j;
            unshifted_matrix[i * (s - 1) + j] = matrix[old_row * s + old_col];
        }
    }

    matrix = unshifted_matrix;
    s -= 1;
}

void column_sort()
{
    printMatrix("Start");
    sortColumns();
    printMatrix("Step 1: Sort columns");
    transpose();
    printMatrix("Step 2: Transpose");
    sortColumns();
    printMatrix("Step 3: Sort columns");
    untranspose();
    printMatrix("Step 4: Untranspose");
    sortColumns();
    printMatrix("Step 5: Sort columns");
    shift();
    printMatrix("Step 6: Shift");
    sortColumns();
    printMatrix("Step 7: Sort columns");
    unshift();
    printMatrix("Step 8: Unshift (Final result)");
}

// int main()
// {
//     std::cout << "Enter the number of rows (r): ";
//     std::cin >> r;
//     std::cout << "Enter the number of columns (s): ";
//     std::cin >> s;

//     if (r % s != 0 || r <= 2 * (s - 1) * (s - 1))
//     {
//         std::cerr << "Invalid matrix dimensions" << std::endl;
//         return 1;
//     }

//     try
//     {
//         matrix = generateRandomInput(r, s);
//         column_sort();
//     }
//     catch (const std::exception &e)
//     {
//         std::cerr << "Error: " << e.what() << std::endl;
//         return 1;
//     }

//     return 0;
// }
// std::vector<unsigned int> generateRandomInput(unsigned int rows, unsigned int cols)
// {
//     std::vector<unsigned int> input(rows * cols);
//     unsigned int max_value = rows * cols;

//     // Fill the input vector with values from 0 to max_value - 1
//     for (unsigned int i = 0; i < max_value; ++i)
//     {
//         input[i] = i;
//     }

//     // Shuffle the vector to randomize the order
//     std::random_device rd;
//     std::mt19937 gen(rd());
//     std::shuffle(input.begin(), input.end(), gen);

//     return input;
// }