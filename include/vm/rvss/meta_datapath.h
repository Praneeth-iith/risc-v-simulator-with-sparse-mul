/**
 * @file the file which handles meta instruction
 * @brief   this is a seperate data path for meta instruction 
 * @author Praneeth Kodavti , https://github.com/Praneeth-iith
 */

#ifndef META_DATAPATH_H
#define META_DATAPATH_H
#include "vm/memory_controller.h"
#include <cstdint>
#include <iostream>

namespace metapath {

class MetaDataPath{

private: 

struct StorageUnit{
  MemoryController &memory_controller_ ;
  uint64_t baseAddress;
  uint64_t sparseAddress;
  uint32_t columns;
  uint32_t rows;
  uint32_t cur_col;
  uint32_t cur_row;
  uint64_t cycles;
  uint32_t offset;
  StorageUnit(MemoryController &ref)
        : memory_controller_(ref) {}
};
StorageUnit base;

public:
    MetaDataPath(uint64_t metaAddr, uint64_t sparseAddr, uint32_t rows, uint32_t columns, MemoryController &memory_controller_ref);    
    void execute();
    uint64_t getCycles();
};

}

#endif