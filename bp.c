/* 046267 Computer Architecture - Spring 2020 - HW #1 */
/* This file should hold your implementation of the predictor simulator */
#include <stdlib.h>
#include "bp_api.h"
typedef struct{
	uint32_t tag;
	uint32_t PC;
	uint8_t* hist;
}BTB_entry;

int power(int base, int pow);
int find_tag(unsigned Size);//find a hexa helper to help calculate tag;
int calc_stm_entry(uint32_t pc,uint8_t hist);//find the entry in the State Machine Table
uint32_t calc_tag(uint32_t pc);//calculate tag from pc


//globals
int btb_entry_calc;//helper to calculate the position in the BTB array, a hex number to and with PC;
int max_hist_value;//maximum value, in decimal, that hist can get, so, for example, id hist_size=4->max hist value = 2^4 = 16;
int tag_help;//size of tag in bits, help calculate tag for comperison;
int share;//what kind of share is the SMT_entry calcuated by;
unsigned DefaultFSM_state;//default FSM state;
int bit_num;//log2(btb_size);

bool GlobalHist;
bool GlobalTable;
SIM_stats sim;

//tables
uint8_t* hist_buf;//history buffer
BTB_entry* BTB;//BTB table- array ,contains tag,prediction PC, pointer to history;
uint8_t* SMT;//state machines table

int BP_init(unsigned btbSize, unsigned historySize, unsigned tagSize, unsigned fsmState,
			bool isGlobalHist, bool isGlobalTable, int Shared){

    //initialize global var's
    btb_entry_calc = find_tag(btbSize); 
    max_hist_value=power(2,historySize);
    tag_help=tagSize;
    share=Shared;
    DefaultFSM_state = fsmState;
    GlobalHist=true;
    GlobalTable=true;
   // SIM_stats sim;
    sim.br_num=0;
    sim.flush_num=0;
    sim.size=0;
    //itarator ;
    int i;

//allocate btb 
    BTB=(BTB_entry*)malloc(sizeof(BTB_entry)*btbSize);
    if (BTB==NULL)return -1; //malloc check;
    sim.size=(tagSize+30)*btbSize;
    for (i=0; i<btbSize; i++){
        BTB[i].PC=0;
        BTB[i].tag=0;
    }
//allocate hitory
    if(isGlobalHist==true){
        GlobalHist=true;
        hist_buf=(uint8_t*)malloc(sizeof(uint8_t));
        if(hist_buf==NULL){//malloc check;
            free(BTB);
            return -1;
        }
        sim.size = sim.size + historySize;
        *hist_buf = 0;

        for(i=0; i<btbSize; i++){
            BTB[i].hist=hist_buf;
        }
    }
    else if(isGlobalHist==false){
        GlobalHist=false;
        hist_buf=(uint8_t*)malloc(sizeof(uint8_t)*btbSize);
        if(hist_buf==NULL){ //malloc check;
            free(BTB);
            return -1;
        }
        sim.size = sim.size + historySize*btbSize;
        for (i=0; i<btbSize; i++){
            hist_buf[i]=0;
            BTB[i].hist=&(hist_buf[i]);
        }
    }

//allocate smt and initialize

    int smt_size=0;
    if (isGlobalTable==true){
        GlobalTable=true;
        smt_size = power(2,historySize);
    }
    else {
        GlobalTable=false;
        smt_size = power(2,historySize)*btbSize;
    }
    
    SMT = (uint8_t*)malloc(sizeof(uint8_t)*smt_size);
    if (SMT==NULL){ //malloc check;
        free(BTB);
        free(hist_buf);
        return -1;
    }
    sim.size = sim.size + smt_size*2;
    for( i=0; i<smt_size; i++){
        SMT[i]=fsmState;
    }
	return 0;
}

bool BP_predict(uint32_t pc, uint32_t *dst){
    sim.br_num++;//add branch to statistics;
    int btb_entry = pc & btb_entry_calc;
    btb_entry=btb_entry>>2;//calculate btb table entry;
    uint8_t hist = *(BTB[btb_entry].hist);//history value
    int stm_entry = calc_stm_entry(pc,hist);///State machine entry;
    uint32_t tag = calc_tag(pc);//calculate tag for comparisson;
    uint8_t state = SMT[stm_entry];
    if ((BTB[btb_entry].tag == tag) && (state > 1)){     
        *dst = BTB[btb_entry].PC;
        return true;
    }
    *dst = pc+4;
    return false;
}

void BP_update(uint32_t pc, uint32_t targetPc, bool taken, uint32_t pred_dst){
    int btb_entry = pc & btb_entry_calc;
    btb_entry=btb_entry>>2;//calculate entry to the btb table;
    uint8_t hist = *(BTB[btb_entry].hist);//find the current history state;
    int stm_entry = calc_stm_entry(pc,hist);//find currentstm entry;
    uint32_t tag = calc_tag(pc);//tag for comparisson;
    uint8_t state=0;

    //new branch command, enter command data;
    if(BTB[btb_entry].tag != tag){
        BTB[btb_entry].tag = tag;
        BTB[btb_entry].PC = targetPc;
        if(GlobalHist==false){
            hist = 0;
        }

        if(GlobalTable == false){
            int helper = max_hist_value * btb_entry;
            for(int i=0;i<max_hist_value;i++){
                SMT[helper + i] = DefaultFSM_state;
            }
        }
        
    stm_entry = calc_stm_entry(pc,hist);
    }

    //update target
    if (BTB[btb_entry].PC != targetPc){
        BTB[btb_entry].PC = targetPc;
    }

    //update by the results of the branch;
    state = SMT[stm_entry];
    if (taken==true){
        hist=((hist<<1)+1)%max_hist_value;
        if (state!=3)state++;
    }
    else {
        hist=(hist<<1)%max_hist_value;
        if (state!=0)state--;   
    }
    SMT[stm_entry] = state;
    *(BTB[btb_entry].hist)=hist;
    if((((pred_dst!=targetPc)&&(taken==true))||((pred_dst!=pc+4)&&(taken==false)))){
        sim.flush_num++;
    }
	return;
}

void BP_GetStats(SIM_stats *curStats){
    curStats->br_num=sim.br_num;
    curStats->flush_num=sim.flush_num;
    curStats->size=sim.size;
    free(SMT);
    free(hist_buf);
    free(BTB);
	return;
}
///////////////////////////////////////////////help functions////////////////////////////////

int power(int base, int pow){
    int ret_val=1;
    for(int i=0;i<pow;i++){
        ret_val = ret_val*base;
    }
    return ret_val;
}
int find_tag(unsigned Size){
    int ret_val;
    switch(Size){
        case 32 : ret_val = 0x007c; bit_num = 5; break;//global entry helper/////0000000001111100//two zeroes in the end
        case 16 : ret_val = 0x003c; bit_num = 4; break;
        case 8  : ret_val = 0x001c; bit_num = 3; break;
        case 4  : ret_val = 0x000c; bit_num = 2; break;
        case 2  : ret_val = 0x0004; bit_num = 1; break;
        case 1  : ret_val = 0x0000; bit_num = 0; break;
        default : ret_val = -1; break;
    }
    return ret_val; 
}

int calc_stm_entry(uint32_t pc,uint8_t hist){
    int helper;
    if (GlobalTable==true){
        if (share == 0){
            helper = hist; 
        }
        else if(share == 1){
            helper = (pc>>2)^hist;
        }
        else {
            helper = (pc>>16)^hist;
        }
        helper = helper%max_hist_value;
        return helper;
    }
    else {
        int btb_entry = pc & btb_entry_calc;
        btb_entry=btb_entry>>2;
        return (btb_entry*max_hist_value+hist); 
    }
}
uint32_t calc_tag(uint32_t pc){
    uint32_t helper = pc>>(bit_num+2);
    int modul = power (2,tag_help);
    return (helper % modul);
}
