#ifndef SYSTEM_CONTROL_H_
#define SYSTEM_CONTROL_H_


uint32_t Read_Control( uint32_t Address, void* Data, uint16_t Data_Length);

uint32_t Write_Control( uint32_t Address, void* Data, uint16_t Data_Length);


#endif // SYSTEM_CONTROL_H_
