/**
 * @file vec_alu.h
 * @brief Contains the definition of the alu class for performing vector operations.
 * @author Vishank Singh, httpa://github.com/VishankSingh
 */
#ifndef VEC_ALU_H
#define VEC_ALU_H

#include <cfenv>
#include <cmath>
#include <cstdint>
#include <ostream>
#include <array>
#include <vm/registers.h>



namespace vec_alu {

    enum class Vec_aluop{
        vadd,
        vsmul,
    };

    class alu {
        public:
        alu() = default;
        ~alu() = default;

        [[nodiscard]] static std::array<int64_t, RegisterFile::VEC_DIM> vadd(std::array<int64_t, RegisterFile::VEC_DIM> v1, std::array<int64_t, RegisterFile::VEC_DIM> v2) ;
        [[nodiscard]] static std::array<int64_t, RegisterFile::VEC_DIM> vsmul(std::array<int64_t, RegisterFile::VEC_DIM> v1, int64_t s) ;

    };
}



#endif //VEC_ALU.H