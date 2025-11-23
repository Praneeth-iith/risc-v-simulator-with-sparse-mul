/**
 * @file the file which handles meta instruction
 * @brief   this is a seperate data path for meta instruction 
 * @author Praneeth Kodavti , https://github.com/Praneeth-iith
 */

#include "vm/rvss/meta_datapath.h"

namespace metapath{

    MetaDataPath::MetaDataPath(uint64_t metaAddr, uint64_t sparseAddr, uint32_t rows, uint32_t columns,
        MemoryController &memory_controller_ref)
        :                       base(memory_controller_ref)
    {
        base.baseAddress = metaAddr;
        base.columns = columns;
        base.sparseAddress = sparseAddr;
        base.rows = rows;
        base.cur_row = 0;
        base.cur_col = 0;
        base.offset = 0;
        base.cycles = 0;
    }

    void MetaDataPath::execute(){
        base.offset=1;
        base.cur_col=1;
        base.sparseAddress += 8;
        while(base.cur_row<base.rows){
            if(base.cur_col == base.columns){
                base.offset += (1<<31);
                base.cur_row++;
                base.cur_col=0;
                base.memory_controller_.WriteWord(base.baseAddress,base.offset);
                //std::cout<<base.memory_controller_.ReadWord(base.baseAddress)<<" ";
                base.offset=1;
                base.cur_col++;
                base.baseAddress += 4;
                base.sparseAddress+=8;
                continue;
            }
            auto value = base.memory_controller_.ReadDoubleWord(base.sparseAddress);
            if(value!=0){
                base.memory_controller_.WriteWord(base.baseAddress,base.offset);
                //std::cout<<base.memory_controller_.ReadWord(base.baseAddress)<<" ";
                base.offset=1;
                base.cur_col++;
                base.baseAddress += 4;
            }else{
                base.offset++;
                base.cur_col++;
            }
            base.sparseAddress += 8;
            base.cycles++;
        }
    }
    
    uint64_t MetaDataPath::getCycles(){
        return base.cycles;
    }
}