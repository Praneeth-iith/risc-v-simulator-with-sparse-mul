
#include "vm/rvss/sparse_mul_pipeline.h"


namespace sparsePipeline {

std::array<bool, RegisterFile::VEC_DIM> SparseMulPipeline::writeControls(uint32_t lane){
    std::array<bool, RegisterFile::VEC_DIM> canWrite;
    for(int i = 0 ; i < (int)RegisterFile::VEC_DIM ; i++) canWrite[i]=true;
    int32_t excess = (1+lane)*(uint64_t)RegisterFile::VEC_DIM - base.coldense;
    int end = (int)RegisterFile::VEC_DIM - 1;
    while(excess>0){
        canWrite[end--] = false;
        excess--;
    }
    return canWrite;
}

void SparseMulPipeline::writeOutputfromBuffer(writeData data){
    
    auto canWrite = data.canWrite;
    auto address = data.addr;
    auto vec = data.vectorData;
   
    for(int i = 0 ; i < (int)RegisterFile::VEC_DIM ; i++){
        if(canWrite[i]){
            base.memory_controller_.WriteDoubleWord(address,vec[i]);
            //std::cout<<vec[i]<<" ";
        }
        address += 8;
    }

}

void SparseMulPipeline::offsetFetch(){
    
    uint32_t data = base.memory_controller_.ReadWord(base.tempMetaAddr);
    uint32_t comp = (1<<31);
    bool msb = comp & data;
    comp--;
    uint32_t offset = comp & data;
    OF_VF.isLast =  msb;
    OF_VF.colIndex = base.cur_col;
    OF_VF.rowIndex = base.cur_row;
    OF_VF.sparseAddress = base.tempSparseAddr;
    OF_VF.lane = base.lane;
    OF_VF.isValid=true;
    base.tempSparseAddr += offset*8;
    base.tempMetaAddr += 4;
    if(msb){
        base.cur_col=0;
        base.cur_row++;
        if(base.cur_row == base.rowsparse){
            if(base.lane*(uint64_t)RegisterFile::VEC_DIM + RegisterFile::VEC_DIM >= base.coldense){
                    base.isDone=true;
            }else{
                base.cur_row=0;
                base.cur_col=0;
                base.lane++;
                base.tempMetaAddr = base.metaBaseAddr;
                base.tempSparseAddr = base.sparseBaseAddr;
            }
        }
    }else{
        base.cur_col += offset;
    }
}

void SparseMulPipeline::vectorFetch(){
    if(!OF_VF.isValid){
        VF_SV.isValid=false;
        return;
    }
    uint64_t scalar = base.memory_controller_.ReadDoubleWord(OF_VF.sparseAddress);
    auto vec =  base.memory_controller_.ReadVector(base.denseBaseAddr + 8*base.coldense*OF_VF.colIndex + 8*(OF_VF.lane)*RegisterFile::VEC_DIM);
    
    if(!base.writeBackBuffer.empty()){
        writeOutputfromBuffer(base.writeBackBuffer.front());
        base.writeBackBuffer.pop_front();
    }

    std::array<int64_t,RegisterFile::VEC_DIM> densevec;
    for(int i = 0 ; i < (int)RegisterFile::VEC_DIM ; i++) densevec[i] = static_cast<int64_t> (vec[i]);
    
    VF_SV.denserow = densevec;
    VF_SV.colIndex = OF_VF.colIndex;
    VF_SV.rowIndex = OF_VF.rowIndex;
    VF_SV.scalar = static_cast<int64_t>(scalar);
    VF_SV.isLast = OF_VF.isLast;
    VF_SV.lane = OF_VF.lane;
    VF_SV.isValid=true;
}

void SparseMulPipeline::svmul(){
    if(!VF_SV.isValid){
        SV_ACC.isValid=false;
        return;
    }
    int64_t scalar = VF_SV.scalar;
    auto vec = VF_SV.denserow;
    
    auto resultant = vec_alu::alu::vsmul(vec,scalar);
    SV_ACC.colIndex = VF_SV.colIndex;
    SV_ACC.rowIndex = VF_SV.rowIndex;
    SV_ACC.isLast =   VF_SV.isLast;
    SV_ACC.lane = VF_SV.lane;
    SV_ACC.resultantrow = resultant;
    SV_ACC.isValid=true;
}

void SparseMulPipeline::acc(){
    if(!SV_ACC.isValid) {
        return;
    }
    auto vec = SV_ACC.resultantrow;
    base.accumulator = vec_alu::alu::vadd(base.accumulator,vec);
    
    if(SV_ACC.isLast){
        writeData output;
        output.canWrite = writeControls(SV_ACC.lane);
        output.vectorData = base.accumulator;
        output.addr = base.destinationBaseAddr + (SV_ACC.rowIndex)*base.coldense*8 + RegisterFile::VEC_DIM*SV_ACC.lane*8; 
        base.writeBackBuffer.push_back(output);
        base.accumulator = zero;
    }
}

void SparseMulPipeline::runCycle(){
    acc();
    svmul();
    vectorFetch();
    offsetFetch();
}

void SparseMulPipeline::execute(){
    while(!base.isDone){
        runCycle();
        base.cycles++;
    }
    acc();
    svmul();
    vectorFetch();
    base.cycles++;
    acc();
    svmul();
    base.cycles++;
    acc();
    base.cycles++;

    while(!base.writeBackBuffer.empty()){
        writeOutputfromBuffer(base.writeBackBuffer.front());
        base.writeBackBuffer.pop_front();
        base.cycles++;
    }
}

std::array<int64_t, RegisterFile::VEC_DIM> SparseMulPipeline::zero{};

SparseMulPipeline::SparseMulPipeline(uint64_t sparseAddr_param, uint64_t denseAddr_param,
                                     uint64_t metaAddr_param, uint64_t outputAddr_param,
                                     MemoryController &memory_ref)
    :                                base(memory_ref)
    {
        auto initial_sparse_addr = sparseAddr_param;
        auto initial_dense_addr = denseAddr_param;
        this->base.memory_controller_ = memory_ref;
        this->base.rowsparse = base.memory_controller_.ReadWord(initial_sparse_addr);
        initial_sparse_addr += 4;
        this->base.colsparse = base.memory_controller_.ReadWord(initial_sparse_addr);
        initial_sparse_addr += 4;
        this->base.rowdense = base.memory_controller_.ReadWord(initial_dense_addr);
        initial_dense_addr += 4;
        this->base.coldense = base.memory_controller_.ReadWord(initial_dense_addr);
        initial_dense_addr += 4;
        OF_VF.isValid=false;
        SV_ACC.isValid=false;
        VF_SV.isValid=false;
        this->base.lane=0;
        this->base.cycles=0;
        this->base.cur_col=0;
        this->base.cur_row=0;
        this->base.isDone=false;
        this->base.accumulator = zero;
        this->base.destinationBaseAddr = outputAddr_param;
        this->base.metaBaseAddr = metaAddr_param;
        this->base.tempMetaAddr = metaAddr_param;
        this->base.sparseBaseAddr = initial_sparse_addr;
        this->base.denseBaseAddr = initial_dense_addr;
        this->base.tempSparseAddr = initial_sparse_addr;
}

uint64_t SparseMulPipeline::getCycles(){
    return base.cycles;
}

uint64_t SparseMulPipeline::getSize(){
    return (uint64_t)base.coldense * (uint64_t)base.rowsparse;
}

}