#include "engine.h"


pgBuffer::BufferType ConstantBuffer::GetType() const
{
    return pgBuffer::ConstantBuffer;
}

unsigned int ConstantBuffer::GetElementCount() const
{
    return 1;
}
