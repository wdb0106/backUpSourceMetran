#include "MyQueue.h"
#include "DebugTrace.h"

Q::Q (UNSIGNED QueueLength, UNSIGNED ItemSize)
{
    xQueue = xQueueCreate(QueueLength, ItemSize);
    ASSERTION(NULL != xQueue);
}

Q::~Q (void)
{
    vQueueDelete(xQueue);
}

STATUS Q::Send (const void* pvItemToQueue, uint32_t xTicksToWait)
{
    STATUS status = 0;
    if(xQueueSend(xQueue, pvItemToQueue, xTicksToWait))
    {
        status = 1;
    }
    else
    {
        status = 0;
    }
    return status;
}

STATUS Q::SendToFront (const void* pvItemToQueue, uint32_t xTicksToWait)
{
    STATUS status = 0;
    if(xQueueSendToFront(xQueue, pvItemToQueue, xTicksToWait))
    {
        status = 1;
    }
    else
    {
        status = 0;
    }
    return status;
}

STATUS Q::SendToBack (const void* pvItemToQueue, uint32_t xTicksToWait)
{
    STATUS status = 0;
    if(xQueueSendToBack(xQueue, pvItemToQueue, xTicksToWait))
    {
        status = 1;
    }
    else
    {
        status = 0;
    }
    return status;
}

STATUS Q::Retrieve (void* pvBuffer, uint32_t xTicksToWait)
{
    STATUS status = 0;
    if(xQueueReceive(xQueue, pvBuffer, xTicksToWait))
    {
        status = 1;
    }
    else
    {
        status = 0;
    }
    return status;
}