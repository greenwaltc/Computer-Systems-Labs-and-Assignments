#include <stdio.h>
#include <stdlib.h>

#include "utils.h"

const int MAX_MEM_SIZE  = (1 << 13);

void fetchStage(int *icode, int *ifun, int *rA, int *rB, wordType *valC, wordType *valP) {
  wordType PC =  getPC();
  byteType byte = getByteFromMemory(PC);
  *icode = byte >> 4;
  *ifun = (0xF & byte);
  
  if (*icode == NOP){ 
    *valP = PC + 1;
    *rA = 0;
    *rB = 0;
    *valC = 0;
  }
  
  if (*icode == HALT){
    setStatus(STAT_HLT);
    *valP = PC + 1;
  }
  
  if (*icode == RRMOVQ || *icode == OPQ || *icode == PUSHQ || *icode == POPQ){
    byteType rA_rB = getByteFromMemory(PC + 1);
    *rA = rA_rB >> 4;
    *rB = (0xf & rA_rB);
    *valP = PC + 2;
  }
  
  if(*icode == IRMOVQ || *icode == RMMOVQ || *icode == MRMOVQ){
    byteType rA_rB = getByteFromMemory(PC + 1);
    *rA = rA_rB >> 4;
    *rB = (0xf & rA_rB);
    *valC = getWordFromMemory(PC + 2);
    *valP = PC + 10;
  }
  
  if(*icode == JXX || *icode == CALL){
    *valC = getWordFromMemory(PC + 1);
    *valP = PC + 9;
  }
  
  if(*icode == 9){
    //Do nothing
  }
  
}

void decodeStage(int icode, int rA, int rB, wordType *valA, wordType *valB) {
  if (icode == OPQ || icode == RMMOVQ){
    *valA = getRegister(rA);
    *valB = getRegister(rB);
  }
  
  if (icode == POPQ){
    *valA = getRegister(RSP);
    *valB = getRegister(RSP);
  }
  
  if (icode == CALL){
    *valB = getRegister(RSP);
  }
  
  if (icode == RET){
    *valA = getRegister(RSP);
    *valB = getRegister(RSP);
  }
  
  if (icode == RRMOVQ){
    *valA = getRegister(rA);
  }
  
  if (icode == MRMOVQ){
    *valB = getRegister(rB);
  }
  
  if (icode == PUSHQ){
    *valA = getRegister(rA);
    *valB = getRegister(RSP);
  }
  
}

void executeStage(int icode, int ifun, wordType valA, wordType valB, wordType valC, wordType *valE, bool *Cnd) {
  if (icode == OPQ){
    if (ifun == ADD){ //Addition
      *valE = valB + valA;
      if (*valE < 0){
        setFlags(1, 0, 0);
      }
      else if (*valE == 0){
        setFlags(0, 1, 0);
      }
      if (valA > 0 && valB > 0 && *valE < 0){
        setFlags(1, 0 ,1);
      }
      if (valA < 0 && valB < 0 && *valE > 0){
        setFlags(0, 0, 1);
      }
    }
    else if (ifun == SUB){ //Subtraction
      *valE = valB - valA;
      if (*valE < 0){
        setFlags(1, 0, 0);
      }
      else if (*valE == 0){
        setFlags(0, 1, 0);
      }
      
    }
    else if (ifun == AND){ //andq
      *valE = valA & valB;
      if (valA == 0 && valB == 0){
        setFlags(0, 1, 0);
      }
      else if (valA < 0 && valB < 0){
        setFlags(1, 0, 0);
      }
    }
    else if (ifun == XOR){ //xorq
      *valE = valA ^ valB;
      if (valA < 0 && valB < 0 && *valE > 0){
        setFlags(0, 0, 1);
      }
      if (valA < 0 && valB < 0 && *valE == 0){
        setFlags(0, 1, 1);
      }
    }
  }
  
  if (icode == RRMOVQ){
    *valE = 0 + valA;
  }
  
  if (icode == IRMOVQ){
    *valE = 0 + valC;
  }
  
  if (icode == RMMOVQ || icode == MRMOVQ){
    *valE = valB + valC;
  }
  
  if (icode == PUSHQ || icode == CALL){
    *valE = valB + (-8);
  }
  
  if (icode == POPQ || icode == RET){
    *valE = valB + 8;
  }
  
  if (icode == JXX){
    *Cnd = Cond(ifun);
  }
}

void memoryStage(int icode, wordType valA, wordType valP, wordType valE, wordType *valM) {
  if (icode == RMMOVQ){
    setWordInMemory(valE, valA);
  }
  
  if (icode == MRMOVQ){
    *valM = getWordFromMemory(valE);
  }
  
  if (icode == PUSHQ){
    setWordInMemory(valE, valA);
  }
  
  if (icode == POPQ){
    *valM = getWordFromMemory(valA);
  }
  
  if (icode == CALL){
    setWordInMemory(valE, valP);
  }
  
  if (icode == RET){
    *valM = getWordFromMemory(valA);
  }
  
}

void writebackStage(int icode, wordType rA, wordType rB, wordType valE, wordType valM) {
  if (icode == OPQ || icode == RRMOVQ || icode == IRMOVQ){
    setRegister(rB, valE);
  }
  
  if (icode == MRMOVQ){
    setRegister(rA, valM);
  }
  
  if (icode == PUSHQ || icode == CALL || icode == RET){
    setRegister(RSP, valE);
  }
  
  if (icode == POPQ){
    setRegister(RSP, valE);
    setRegister(rA, valM);
  }
}

void pcUpdateStage(int icode, wordType valC, wordType valP, bool Cnd, wordType valM) {
  if (icode != JXX && icode != CALL && icode != RET){
    setPC(valP);
  }
  else if (icode == JXX){
    if (Cnd){
      setPC(valC);
    }
    else {
      setPC(valP);
    }
  }
  else if (icode == CALL){
    setPC(valC);
  }
  else if (icode == RET) { // icode == RET
    setPC(valM);
  }
}

void stepMachine(int stepMode) {
  /* FETCH STAGE */
  int icode = 0, ifun = 0;
  int rA = 0, rB = 0;
  wordType valC = 0;
  wordType valP = 0;
 
  /* DECODE STAGE */
  wordType valA = 0;
  wordType valB = 0;

  /* EXECUTE STAGE */
  wordType valE = 0;
  bool Cnd = 0;

  /* MEMORY STAGE */
  wordType valM = 0;

  fetchStage(&icode, &ifun, &rA, &rB, &valC, &valP);
  applyStageStepMode(stepMode, "Fetch", icode, ifun, rA, rB, valC, valP, valA, valB, valE, Cnd, valM);

  decodeStage(icode, rA, rB, &valA, &valB);
  applyStageStepMode(stepMode, "Decode", icode, ifun, rA, rB, valC, valP, valA, valB, valE, Cnd, valM);
  
  executeStage(icode, ifun, valA, valB, valC, &valE, &Cnd);
  applyStageStepMode(stepMode, "Execute", icode, ifun, rA, rB, valC, valP, valA, valB, valE, Cnd, valM);
  
  memoryStage(icode, valA, valP, valE, &valM);
  applyStageStepMode(stepMode, "Memory", icode, ifun, rA, rB, valC, valP, valA, valB, valE, Cnd, valM);
  
  writebackStage(icode, rA, rB, valE, valM);
  applyStageStepMode(stepMode, "Writeback", icode, ifun, rA, rB, valC, valP, valA, valB, valE, Cnd, valM);
  
  pcUpdateStage(icode, valC, valP, Cnd, valM);
  applyStageStepMode(stepMode, "PC update", icode, ifun, rA, rB, valC, valP, valA, valB, valE, Cnd, valM);

  incrementCycleCounter();
}

/** 
 * main
 * */
int main(int argc, char **argv) {
  int stepMode = 0;
  FILE *input = parseCommandLine(argc, argv, &stepMode);

  initializeMemory(MAX_MEM_SIZE);
  initializeRegisters();
  loadMemory(input);

  applyStepMode(stepMode);
  while (getStatus() != STAT_HLT) {
    stepMachine(stepMode);
    applyStepMode(stepMode);
#ifdef DEBUG
    printMachineState();
    printf("\n");
#endif
  }
  printMachineState();
  return 0;
}