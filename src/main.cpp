#include <Arduino.h>
#include <Wire.h>

#include <avr/io.h>
#include <avr/interrupt.h>

const int IN1 = 4;
const int IN2 = 7;
const int IN3 = 8;
const int StartPin = 2;
const int DelayX=2200;

enum state {IDLE,OPEN_COLUNMS ,OPEN_ROWS,  OPEN_CELL_FROM_LEFT ,  OPEN_CELL_FROM_RIGHT , OPEN_CELL_BY_CELL,CHECK_INPUT,TEST_ALL};
enum state StateMachine = CHECK_INPUT;
void startBitISR() {
  // This function will be called when the start pin changes
  
  StateMachine = CHECK_INPUT;  // Set the state machine to check input
}

void setup() {
  Serial.begin(9600);

  Wire.begin();  // Initialize I2C as master
  Wire.setClock(75000);
  pinMode(IN1, INPUT);
  pinMode(IN2, INPUT);
  pinMode(IN3, INPUT);
  pinMode(StartPin, INPUT_PULLUP);
  pinMode(13, OUTPUT);
  attachInterrupt(digitalPinToInterrupt(StartPin), startBitISR, CHANGE);  // Attach interrupt to the start pin

  
}

int bitInOne, bitInTwo, bitInThree,start_bit;
byte error, address;
int devicesFound = 0;


uint8_t rowLUTforOPENrowsONLY[][2]={
//Row 1-8
{0x26,0b11111110}, //this row 1
{0x26,0b11111101}, //this row 2
{0x26,0b11111011}, //this row 3
{0x26,0b11110111}, //this row 4
{0x26,0b11101111}, //this row 5
{0x26,0b11011111}, //this row 6
{0x26,0b10111111}, //this row 7
{0x26,0b01111111}, //this row 8
{0x26,0b11111111}, //off rows 1-8
{0x27,0b11111110}, //this row 9
{0x27,0b11111101}, //this row 10
{0x27,0b11111011}, //this row 11
{0x27,0b11111111}, //off rows 9-11


};
uint8_t rowLUT[][2]={
//Row 1-8
{0x26,0b11111110}, //this row 1
{0x26,0b11111101}, //this row 2
{0x26,0b11111011}, //this row 3
{0x26,0b11110111}, //this row 4
{0x26,0b11101111}, //this row 5
{0x26,0b11011111}, //this row 6
{0x26,0b10111111}, //this row 7
{0x26,0b01111111}, //this row 8

//{0x26,0b11111111}, //off rows 1-8

{0x27,0b11111110}, //this row 9
{0x27,0b11111101}, //this row 10
{0x27,0b11111011}, //this row 11
//{0x27,0b11111111}, //off rows 9-11


};
uint8_t colLUT[][2]={
 
  {0x22,0b11011111}, //this control col 1
  {0x22,0b11101111}, //this control col 2
  {0x22,0b11110111}, //this control col 3
  {0x22,0b1111011}, //this control col 4
  {0x22,0b11111101}, //this control col 5
  {0x22,0b11111110},  //this control col 6
//  // {0x22,0b11111111},// off col 1-6

  {0x23,0b11111101}, //this control col 7
  {0x23,0b11111110}, //this control col 8
//   //{0x23,0b11111111}, //off col 7-8

  {0x21,0b11111101}, //this control col 9
  {0x21,0b11111110}, //this control col 10
  //{0x21,0b00000000}, ////off col 

    
  {0x20,0b01111111}, //this control col 11
  {0x20,0b10111111}, //this control col 12
  {0x20,0b11011111}, //this control col 13
  {0x20,0b11101111}, //this control col 14
  {0x20,0b11110111}, //this control col 15
  {0x20,0b11111011}, //this control col 16
  {0x20,0b11111101}, //this control col 17
  {0x20,0b11111110}, //this control col 18
  {0x20,0b11111111}, ///off col 11-18
 // {0x20,0b00000000}, ///off col 11-18

};
uint8_t colLUTRR[][2]={
 

  {0x20,0b11111110}, //this control col 18
  {0x20,0b11111101}, //this control col 17
  {0x20,0b11111011}, //this control col 16
  {0x20,0b11110111}, //this control col 15
  {0x20,0b11101111}, //this control col 14
  {0x20,0b11011111}, //this control col 13
  {0x20,0b10111111}, //this control col 12
  {0x20,0b01111111}, //this control col 11
  {0x21,0b11111110}, //this control col 10
  {0x21,0b11111101}, //this control col 9
  {0x23,0b11111110}, //this control col 8
  {0x23,0b11111101}, //this control col 7
  {0x22,0b11111110},  //this control col 6
  {0x22,0b11111101}, //this control col 5
  {0x22,0b1111011}, //this control col 4
  {0x22,0b11110111}, //this control col 3
  {0x22,0b11101111}, //this control col 2
  {0x22,0b11011111}, //this control col 1




  {0x20,0b11111111}, ///off col 11-18
 // {0x20,0b00000000}, ///off col 11-18

};
uint8_t offROWLUT[][2]={
{0x26,0b11111111},{0x27,0b11111111}};
uint8_t offCOLLUT[][2]={
  {0x22,0b11111111},// off col 1-6
  {0x23,0b11111111}, //off col 7-8
  {0x21,0b01111111}, ////off col 9-10
  {0x20,0b11111111}, ///off col 12-18
};
uint8_t offLUT[][2]={
{0x20,0b11111111}, ///off col 12-18
{0x21,0b11111111}, //off 
{0x22,0b11111111}, //off col 1-6
{0x23,0b11111111}, //off col 7-8
{0x26,0b11111111}, //off rows 1-8
{0x27,0b11111111}, //off rows 9-11

};


void writeByteToI2C(byte deviceAddress, byte data) {
    Wire.beginTransmission(deviceAddress);
    Wire.write(data);
    Wire.endTransmission();
}

void closeAll() { // clear bits of all the lookup table
    for (int index = 0; index < sizeof(offLUT) /2; index++) {
        uint8_t addressToSend = offLUT[index][0];
        uint8_t dataToSend = offLUT[index][1];
        writeByteToI2C(addressToSend,dataToSend);

    }
}

void ColClose(){ // This function open all  columns from left to right  

  for(int index = 0;  index <sizeof(offCOLLUT)/2; index++){
      
      uint8_t dataToSend=offCOLLUT[ index][1];
      uint8_t addressToSend=offCOLLUT[ index][0];
      writeByteToI2C(addressToSend,dataToSend);
 
    }

}

void ColumnsCheck(){ // This function open all rows from left to right  
  for(int index = 0;  index <sizeof(colLUT)/2; index++){

      uint8_t dataToSend=colLUT[ index][1];
      uint8_t addressToSend=colLUT[ index][0];
      ColClose();
      writeByteToI2C(addressToSend, dataToSend);
      delay(100);
    }
  ColClose();
} 

void RowsClose(){ // This function open all  columns from left to right  

  for(int index = 0;  index <sizeof(offROWLUT)/2; index++){
      
      uint8_t dataToSend=offROWLUT[ index][1];
      uint8_t addressToSend=offROWLUT[ index][0];
      writeByteToI2C(addressToSend,dataToSend);
 
    }

}

void OpenCellLeft(int delay_before,int delay_after){ // This function open all rows from left to right  

for(int index_row = 0;  index_row <sizeof(rowLUT)/2; index_row++){
    
      uint8_t dataToSend=rowLUT[ index_row][1];
      uint8_t addressToSend=rowLUT[ index_row][0];
      RowsClose();
      writeByteToI2C(addressToSend, dataToSend);
      delay(delay_before);

         for (int index_col =0; index_col<sizeof(colLUT)/2;index_col++)
              {
                  uint8_t dataToSend=colLUT[ index_col][1];
                  uint8_t addressToSend=colLUT[ index_col][0];
                  ColClose();
                  writeByteToI2C(addressToSend, dataToSend);
                  delay(delay_after);

              }

    }
     RowsClose();

 
} 

void OpenCellRR(int delay_before,int delay_after){ // This function open all rows from left to right  
for(int index_row = 0;  index_row <sizeof(rowLUT)/2; index_row++){
    
      uint8_t dataToSend=rowLUT[ index_row][1];
      uint8_t addressToSend=rowLUT[ index_row][0];
      RowsClose();
      writeByteToI2C(addressToSend, dataToSend);
      delay(delay_before);

         for (int index_col =0; index_col<sizeof(colLUTRR)/2;index_col++)
              {
                  uint8_t dataToSend=colLUTRR[ index_col][1];
                  uint8_t addressToSend=colLUTRR[ index_col][0];
                  ColClose();
                  writeByteToI2C(addressToSend, dataToSend);
                  delay(delay_after);

              }

    }
     RowsClose();

 
} 

void OpenCellbyCell(){ // This function open all rows from left to right  
for(int index_row = 0;  index_row <sizeof(rowLUT)/2; index_row++){
    
      uint8_t dataToSend_row=rowLUT[ index_row][1];
      uint8_t addressToSend_row=rowLUT[ index_row][0];

     

         for (int index_col =0; index_col<sizeof(colLUT)/2;index_col++)
              {
                  uint8_t dataToSend_col=colLUT[ index_col][1];
                  uint8_t addressToSend_col=colLUT[ index_col][0];
                 
              
                  writeByteToI2C(addressToSend_row, dataToSend_row); //open row
                  delay(150);     
                  writeByteToI2C(addressToSend_col, dataToSend_col); // open col    
                  delay(150);
                  RowsClose();
                  ColClose();
                  delay(150);
                             
              }
                 
                  

    }
    closeAll();

 
} 

void RowsCheck(){ // This function open all rows from left to right  
  for(int index = 0;  index <sizeof(rowLUTforOPENrowsONLY)/2; index++){
      uint8_t dataToSend=rowLUTforOPENrowsONLY[ index][1];
      uint8_t addressToSend=rowLUTforOPENrowsONLY[ index][0];
      writeByteToI2C(addressToSend, dataToSend);
      if(index==8)
      {
        delay(100);
      }
      
      else
      {
        delay(DelayX);
      }
     
    }
    

}

void get_inputs_status(){

//   bitInOne=1; 
//   bitInTwo=1;
//   bitInThree=1;
//   start_bit=1;
  bitInOne=1-digitalRead(IN1); 
  bitInTwo=1-digitalRead(IN2);
  bitInThree=1-digitalRead(IN3);
  start_bit=digitalRead(StartPin);
  if (start_bit)
  {
    digitalWrite(13,HIGH);
  }
  

} 



void loop() {
    get_inputs_status();
    // Serial.print("The first bit is: ");
    // Serial.print(bitInOne);
    // Serial.print(" |The SEC bit is: ");
    // Serial.print(bitInTwo);
    // Serial.print(" |The THIRD bit is: ");
    // Serial.print(bitInThree);
    // Serial.print(" |start: ");
    // Serial.print(start_bit);
    // Serial.println();



    // Update input status

    // Check input and update state machine
    switch (StateMachine) {
        case CHECK_INPUT:
            if (bitInOne == 0 && bitInTwo == 0 && bitInThree == 1 && start_bit == HIGH)  {
                StateMachine = OPEN_ROWS;
            } else if (bitInOne == 0 && bitInTwo == 1 && bitInThree == 0 && start_bit == HIGH) {
                StateMachine = OPEN_COLUNMS;
            } else if (bitInOne == 0 && bitInTwo == 1 && bitInThree == 1 && start_bit == HIGH) {
                StateMachine = OPEN_CELL_FROM_LEFT;
            } else if (bitInOne == 1 && bitInTwo == 0 && bitInThree == 0 && start_bit == HIGH) {
                StateMachine = OPEN_CELL_FROM_RIGHT;
            } else if (bitInOne == 1 && bitInTwo == 0 && bitInThree == 1 && start_bit == HIGH) {
                StateMachine = OPEN_CELL_BY_CELL;
            } else if (bitInOne == 1 && bitInTwo == 1 && bitInThree == 1 && start_bit == HIGH) {
                StateMachine = TEST_ALL;
            }
            break;

        case OPEN_ROWS:
            RowsCheck();
             
            closeAll();
            delay(1000);
            StateMachine=IDLE;

            break;

        case OPEN_COLUNMS:
            ColumnsCheck();
            closeAll();
            delay(1000);
            StateMachine=IDLE;
            break;

        case OPEN_CELL_FROM_LEFT:
            OpenCellLeft(100, 50);    
            closeAll();
            delay(1000);
            StateMachine=IDLE;
            break;

        case OPEN_CELL_FROM_RIGHT:
            OpenCellRR(100, 50);
            closeAll();
            delay(1000);
            StateMachine=IDLE;
            break;

        case OPEN_CELL_BY_CELL:
            OpenCellbyCell();
            closeAll();
            delay(1000);
            StateMachine=IDLE;
            break;
        case IDLE:
             get_inputs_status();
             StateMachine=CHECK_INPUT;
        
           

            break;

        case TEST_ALL:

            RowsCheck();   
            closeAll();
            delay(5000);

            ColumnsCheck();
            closeAll();
            delay(5000);

            OpenCellRR(350,150);
            closeAll();
            delay(5000);

            OpenCellLeft(350,150);
            closeAll();
            delay(5000);

            OpenCellbyCell();
            closeAll();
            delay(5000);


            break;
    }
    
    
}
