#ifndef SYSTEM_IO_H_
#define SYSTEM_IO_H_


void Init_IO();

uint32_t Read_IO( uint32_t Address, void* Data, uint8_t Data_Length);
uint32_t Write_IO( uint32_t Address, void* Data, uint8_t Data_Length);


#endif // SYSTEM_IO_H_
