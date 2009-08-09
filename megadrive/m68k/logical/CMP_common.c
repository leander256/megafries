#include "CMP_common.h"


uint16_t CMP_B( uint8_t Destination, uint8_t Source)
{
	uint8_t Result = Destination - Source;
	uint16_t SR = 0;

	if( (Result & 0x80) != 0)
		CCR_SET_N(SR);
	else
	{
		if( Result == 0)
			CCR_SET_Z(SR);
	}

	Source &= 0x80;
	Destination &= 0x80;
	Result &= 0x80;

	if( (!Source && Destination && !Result) || (Source && !Destination && Result))
		CCR_SET_V(SR);
	if( (Source && !Destination) || (Result && !Destination) || (Source && Result))
		CCR_SET_C(SR);

	return SR;
}

uint16_t CMP_W( uint16_t Destination, uint16_t Source)
{
	uint16_t Result = Destination - Source;
	uint16_t SR = 0;

	if( (Result & 0x8000) != 0)
		CCR_SET_N(SR);
	else
	{
		if( Result == 0)
			CCR_SET_Z(SR);
	}

	Source &= 0x8000;
	Destination &= 0x8000;
	Result &= 0x8000;

	if( (!Source && Destination && !Result) || (Source && !Destination && Result))
		CCR_SET_V(SR);
	if( (Source && !Destination) || (Result && !Destination) || (Source && Result))
		CCR_SET_C(SR);

	return SR;
}

uint16_t CMP_L( uint32_t Destination, uint32_t Source)
{
	uint32_t Result = Destination - Source;
	uint16_t SR = 0;

	if( (Result & 0x80000000) != 0)
		CCR_SET_N(SR);
	else
	{
		if( Result == 0)
			CCR_SET_Z(SR);
	}

	Source &= 0x80000000;
	Destination &= 0x80000000;
	Result &= 0x80000000;

	if( (!Source && Destination && !Result) || (Source && !Destination && Result))
		CCR_SET_V(SR);
	if( (Source && !Destination) || (Result && !Destination) || (Source && Result))
		CCR_SET_C(SR);

	return SR;
}
