/**
 * @file sparse_mul_pipeline.h
 * @brief RVSS Control Unit
 * @author Praneeth Kodavati, https://github.com/Praneeth-iith
 */
#ifndef SPARSE_MUL_PIPELINE_H
#define SPARSE_MUL_PIPELINE_H

#include "vm/vec_alu.h"
#include "vm/memory_controller.h"

#include "vm/registers.h"
#include <stack>
#include <vector>
#include <array>
#include <queue>
#include <iostream>
#include <cstdint>

namespace sparsePipeline {


class SparseMulPipeline{

private:

struct writeData{
  uint64_t addr;
  std::array<int64_t, RegisterFile::VEC_DIM> vectorData;
  std::array<bool, RegisterFile::VEC_DIM> canWrite;
};


struct StorageUnit{
  MemoryController &memory_controller_ ;
  uint32_t colsparse;
  uint32_t rowsparse;
  uint32_t coldense;
  uint32_t rowdense;
  uint64_t cycles;
  uint64_t metaBaseAddr; //address of meta data 
  uint64_t sparseBaseAddr;
  uint64_t destinationBaseAddr;
  uint64_t tempMetaAddr;
  uint64_t tempSparseAddr;
  uint64_t denseBaseAddr;
  uint32_t cur_row;
  uint32_t cur_col;
  uint32_t lane;
  bool isDone;
  std::array<int64_t, RegisterFile::VEC_DIM> accumulator;
  std::deque<writeData> writeBackBuffer;

  StorageUnit(MemoryController &ref)
        : memory_controller_(ref) {}
};

static std::array<int64_t, RegisterFile::VEC_DIM> zero;
  
void offsetFetch(); // call it OF stage 
void vectorFetch(); // call it VF stage
void svmul();       // call it SV stage stands for scalar-vector Multiplication
void acc();  // call it Accumulate stage
void runCycle();

std::array<bool, RegisterFile::VEC_DIM> writeControls(uint32_t lane);  //gives write controls for output vector


void writeOutputfromBuffer(writeData data);

struct OF_VF_Reg{
  bool isValid;
  bool isLast;
  uint32_t rowIndex;
  uint32_t colIndex;
  uint32_t lane;
  uint64_t sparseAddress;
};

struct VF_SV_Reg{
  bool isValid;
  bool isLast;
  uint32_t rowIndex;
  uint32_t colIndex;
  int64_t scalar;
  uint32_t lane;
  std::array<int64_t, RegisterFile::VEC_DIM> denserow;
};

struct SV_ACC_Reg{
  bool isValid;
  bool isLast;
  uint32_t rowIndex;
  uint32_t colIndex;
  uint32_t lane;
  std::array<int64_t, RegisterFile::VEC_DIM> resultantrow;
};

OF_VF_Reg OF_VF;
VF_SV_Reg VF_SV;
SV_ACC_Reg SV_ACC;
StorageUnit base;

public:
  

  SparseMulPipeline(uint64_t sparseAddr, uint64_t denseAddr,uint64_t metaAddr, uint64_t outputAddr, MemoryController &memory_controller_ref);
  void execute();
  uint64_t getCycles();
  uint64_t getSize();
};

}

#endif