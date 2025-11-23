/**
 * @file matrixmul_validation_unit.h
 * @brief a validation unit which handles errors caused by multiplying incompatible matrices
 * @author Praneeth Kodavti , https://github.com/Praneeth-iith
 */

#ifndef MATRIX_VALIDATION_UNIT_H
#define MATRIX_VALIDATION_UNIT_H

#include "vm_base.h"

#include <iostream>
#include <string>
#include <vector>

class Validator{
private:
    MemoryController &memory_controller_ ;

public:
    explicit Validator(MemoryController &mem) : memory_controller_(mem) {}

    bool validate(uint64_t addr1, uint64_t addr2);
};

#endif