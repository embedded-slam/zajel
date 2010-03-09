/***************************************************************************************************
 *
 * zajel - an embedded communication framework for multi-threaded/multi-core environment.
 *
 * Copyright © 2009  Mohamed Galal El-Din, Karim Emad Morsy.
 *
 ***************************************************************************************************
 *
 * This file is part of zajel library.
 *
 * zajel is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or any later version.
 *
 * zajel is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with zajel. If not, see <http://www.gnu.org/licenses/>.
 *
 ***************************************************************************************************
 *
 * For more information, questions, or inquiries please contact:
 *
 * Mohamed Galal El-Din:    mohamed.g.ebrahim@gmail.com
 * Karim Emad Morsy:        karim.e.morsy@gmail.com
 *
 **************************************************************************************************/


/***************************************************************************************************
 *
 *  I N C L U D E S
 *
 **************************************************************************************************/
#include <stddef.h>
#include "zajel.h"

/***************************************************************************************************
 *
 *  M A C R O S
 *
 **************************************************************************************************/
/*
 * TODO: mgalal on Mar 6, 2010
 *
 * To be removed, debug is only defined temporarily to simplify development.
 */
#define DEBUG 1


/***************************************************************************************************
 *  Macro Name  : ZAJEL_IS_ITEM_REGISTERED
 *
 *  Arguments   : item
 *
 *  Description : This macro can be used to set/get the registeration status of the given item.
 *
 *  Returns     : boolean.
 **************************************************************************************************/
#define ZAJEL_IS_ITEM_REGISTERED(item) ((item).isRegistered)
/***************************************************************************************************
 *
 *  T Y P E S
 *
 **************************************************************************************************/

/***************************************************************************************************
 * Enumeration Name:
 * zajel_component_dynamic_relation_e
 *
 * Enumeration Description:
 * Lists the different relation that can exist between any two components.
 **************************************************************************************************/
typedef enum zajel_component_dynamic_relation
{
    /*Both components are running on the same thread, on the same core*/
    ZAJEL_COMPONENT_DYNAMIC_RELATION_SAME_THREAD        = 0,
    /*Both components are running on different threads, on the same core*/
    ZAJEL_COMPONENT_DYNAMIC_RELATION_DIFFERENT_THREADS  = 1,
    /*Both components are running on different threads, on different cores*/
    ZAJEL_COMPONENT_DYNAMIC_RELATION_DIFFERENT_CORES    = 2
} zajel_component_dynamic_relation_e;

/***************************************************************************************************
 * Structure Name:
 * zajel_message_information_s
 *
 * Structure Description:
 * structure_description.
 **************************************************************************************************/
typedef struct zajel_message_information
{
    /*Message Handler*/
    zajel_message_handler_function  messageHandlerFunction;
#ifdef DEBUG
    /*TRUE if the message is registered*/
    bool_t  isRegistered;
    /*Message identifier*/
    uint32_t                        messageID;
    /*Textual representation of the message, for debugging purposes*/
    char*                           messageName_ptr;
#endif /*DEBUG*/
} zajel_message_information_s;

/***************************************************************************************************
 * Structure Name:
 * zajel_component_information_parameters_s
 *
 * Structure Description:
 * This structure holds the parameters that constitutes the component handler.
 **************************************************************************************************/
typedef struct zajel_component_information_parameters
{
    /*Thread identifier on which the component runs, supports up to 256 different thread*/
    uint8_t threadID;
    /*Core identifier on which the component runs, support up to 256 different core*/
    uint8_t coreID;
    /*Configuration and status bitmap*/
    uint8_t bitmapByte;
#ifdef DEBUG
    /*TRUE if the component is registered*/
    bool_t  isRegistered;
    /*Component identifier, supports up to 256 different component*/
    uint8_t componentID;
    /*Component Name*/
    char*   componentName_ptr;
#endif /*DEBUG*/
} zajel_component_information_parameters_s;

/***************************************************************************************************
 * Union Name:
 * zajel_component_information_u
 *
 * Union Description:
 * This union holds component related information in two different formats, either as structure with
 * separate parameters, or as a numeric handle.
 **************************************************************************************************/
typedef union zajel_component_information
{
    /*
     * Holds an encoded number that uniquely identify each component in the system, note that it maps
     * directly to the first 4 bytes of the parameters structures
     */
    uint32_t                                    handle;
    /*Holds the parameters the constitutes the handle*/
    zajel_component_information_parameters_s    parameters;
} zajel_component_information_u;

/***************************************************************************************************
 * Structure Name:
 * zajel_thread_information_s
 *
 * Structure Description:
 * This structure holds the thread related information.
 **************************************************************************************************/
typedef struct zajel_thread_information
{
    /*Core identifier on which this thread runs*/
    uint32_t                            coreID;
    /*This call back function is used to deliver synchronous messages to the thread*/
    zajel_thread_blocking_callback      blockingCallback;
    /*This call back function is used to deliver asynchronous messages to the thread*/
    zajel_thread_non_blocking_callback  nonblockingCallback;
#ifdef DEBUG
    /*TRUE if the thread is registered*/
    bool_t                              isRegistered;
    /*Thread identifier, this is meant to be user-assigned rather than the OS-assigned*/
    uint32_t                            threadID;
    /*Thread textual name, to be used for debugging*/
    char*                               threadName_ptr;
#endif /*DEBUG*/
} zajel_thread_information_s;

/***************************************************************************************************
 * Structure Name:
 * zajel_core_information_s
 *
 * Structure Description:
 * This structure holds the core related information.
 **************************************************************************************************/
typedef struct zajel_core_information
{
    /*This call back function is used to deliver synchronous messages to the core*/
    zajel_core_blocking_callback        blockingCallback;
    /*This call back function is used to deliver asynchronous messages to the core*/
    zajel_core_non_blocking_callback    nonblockingCallback;
#ifdef DEBUG
    /*core identifier, this is meant to be user-assigned rather than the OS-assigned*/
    uint32_t                            coreID;
    /*TRUE if the core is registered*/
    bool_t                              isRegistered;
    /*core textual name, to be used for debugging*/
    char*                               coreName_ptr;
#endif /*DEBUG*/
} zajel_core_information_s;

/***************************************************************************************************
 * Structure Name:
 * zajel_s
 *
 * Structure Description:
 * Holds the control block for the communication framework..
 **************************************************************************************************/
struct zajel
{
    /*Number of messages the framework will handle*/
    uint32_t                                    messageCount;
    /*Number of components the framework will handle*/
    uint32_t                                    componentCount;
    /*Number of threads the framework will handle*/
    uint32_t                                    threadCount;
    /*Number of cores the framework will handle*/
    uint32_t                                    coreCount;
    /*An array that holds the registered message handlers*/
    zajel_message_information_s*                messageInformationArray_ptr;
    /*An array that holds the component related information like the component ID, thread/core ID*/
    zajel_component_information_u*              componentInformationArray_ptr;
    /*An array that holds the thread related information*/
    zajel_thread_information_s*                 threadInformationArray_ptr;
    /*An array that holds the core related information*/
    zajel_core_information_s*                   coreInformationArray_ptr;
    /*The deallocation function pointer to be used when destroying the control block*/
    deallocation_function                       deallocationFunction_ptr;
};


/***************************************************************************************************
 *
 *  I N T E R N A L   F U N C T I O N   D E C L A R A T I O N S
 *
 **************************************************************************************************/



/***************************************************************************************************
 *  Name        : zajel_get_component_dynamic_relation
 *
 *  Arguments   : uint32_t sourceComponentID,
 *                uint32_t destinationComponentID
 *
 *  Description : Determines the dynamic relation between the two given component.
 *
 *  Returns     : zajel_component_dynamic_relation_e.
 **************************************************************************************************/
zajel_component_dynamic_relation_e zajel_get_component_dynamic_relation(uint32_t sourceComponentID,
                                                                        uint32_t destinationComponentID);


/***************************************************************************************************
 *
 *  I N T E R F A C E   F U N C T I O N   D E F I N I T I O N S
 *
 **************************************************************************************************/


zajel_status_e zajel_init(zajel_s**             zajelPointer_ptr,
                          uint32_t              messageCount,
                          uint32_t              componentCount,
                          uint32_t              threadCount,
                          uint32_t              coreCount,
                          allocation_function   allocationFunction_ptr,
                          deallocation_function deallocationFunction_ptr COMMA()
                          FILE_AND_LINE_FOR_TYPE())
{
    /*Temporarily holds the allocated structure, just to avoid using the multiple indirection frequently*/
    zajel_s* zajel_ptr;
    /*Status of the initialization*/
    zajel_status_e status;
    /*Temporary counter*/
    uint32_t i;

    status = ZAJEL_STATUS_SUCCESS;
    /*
     * This function is responsible for:
     ***********************************************************************************************
     *
     * o Validating inputs.
     * o Saving the necessary information from the input.
     * o Allocating the required resources.
     * o Initializing the control block variables.
     */

    ASSERT((NULL != zajelPointer_ptr),
           "zajel: Invalid address to pointer passed (zajel_ptr)!",
           fileName,
           lineNumber);
    ASSERT((NULL == *zajelPointer_ptr),
           "zajel: Address passed of a non-null pointer, expecting an address of a null-initialized "
           "pointer (zajel_ptr), this might be a sign of double initialization using the same pointer!",
           fileName,
           lineNumber);
    ASSERT((messageCount > 0),
           "zajel: Message count must be greater than zero!",
           fileName,
           lineNumber);
    ASSERT((componentCount > 0),
           "zajel: Component count must be greater than zero!",
           fileName,
           lineNumber);
    ASSERT((threadCount > 0),
           "zajel: Thread count must be greater than zero!",
           fileName,
           lineNumber);
    ASSERT((coreCount > 0),
           "zajel: Core count must be greater than zero!",
           fileName,
           lineNumber);

    /* HINT: [PROGRAMMING]
     **********************************************************************************************
     * It is safer to use the sizeof(*pointer) instead of sizeof(type), simply because if the type
     * was later changed, the latter method will "logically" fail unless the type passed to sizeof()
     * was changed too. Unlike the former method, which relies on the variable name, and leave the
     * the size evaluation of the associated type to be performed by the compiler.
     */
    zajel_ptr = (zajel_s*) allocationFunction_ptr(sizeof(*zajel_ptr));
    ASSERT((NULL != zajel_ptr),
           "zajel: Failed to allocate a memory for the control block!",
           fileName,
           lineNumber);
    /*
     * TODO: mgalal on Feb 27, 2010
     *
     * Add proper exception handling in this function, allocation failure, invalid parameters, etc...
     */
    zajel_ptr->messageInformationArray_ptr = (zajel_message_information_s*)
            allocationFunction_ptr(messageCount * sizeof(*zajel_ptr->messageInformationArray_ptr));
    ASSERT((NULL != zajel_ptr->messageInformationArray_ptr),
           "zajel: Failed to allocate storage for the message handler array!",
           fileName,
           lineNumber);

    zajel_ptr->componentInformationArray_ptr = (zajel_component_information_u*)
            allocationFunction_ptr(componentCount * sizeof(*zajel_ptr->componentInformationArray_ptr));
    ASSERT((NULL != zajel_ptr->componentInformationArray_ptr),
           "zajel: Failed to allocate storage for the component information array!",
           fileName,
           lineNumber);

    zajel_ptr->threadInformationArray_ptr = (zajel_thread_information_s*)
            allocationFunction_ptr(threadCount * sizeof(*zajel_ptr->threadInformationArray_ptr));
    ASSERT((NULL != zajel_ptr->threadInformationArray_ptr),
           "zajel: Failed to allocate storage for the thread information array!",
           fileName,
           lineNumber);

    zajel_ptr->coreInformationArray_ptr = (zajel_core_information_s*)
            allocationFunction_ptr(coreCount * sizeof(*zajel_ptr->coreInformationArray_ptr));
    ASSERT((NULL != zajel_ptr->coreInformationArray_ptr),
           "zajel: Failed to allocate storage for the core information array!",
           fileName,
           lineNumber);

#ifdef DEBUG
    for(i = 0; i < messageCount; ++i)
    {
        /*<Reset all message handlers>*/

        ZAJEL_IS_ITEM_REGISTERED(zajel_ptr->messageInformationArray_ptr[i]) = FALSE;
    } /*for: <Reset all message handlers>*/

    for(i = 0; i < componentCount; ++i)
    {
        /*<Reset all component information>*/

        ZAJEL_IS_ITEM_REGISTERED(zajel_ptr->componentInformationArray_ptr[i].parameters) = FALSE;
    } /*for: <Reset all component information>*/

    for(i = 0; i < threadCount; ++i)
    {
        /*<Reset all thread information>*/
        ZAJEL_IS_ITEM_REGISTERED(zajel_ptr->threadInformationArray_ptr[i]) = FALSE;
    } /*for: <Reset all thread information>*/

    for(i = 0; i < coreCount; ++i)
    {
        /*<Reset all core information>*/

        ZAJEL_IS_ITEM_REGISTERED(zajel_ptr->coreInformationArray_ptr[i]) = FALSE;
    } /*for: <Reset all core information>*/
#endif /*DEBUG*/

    zajel_ptr->messageCount             = messageCount;
    zajel_ptr->componentCount           = componentCount;
    zajel_ptr->threadCount              = threadCount;
    zajel_ptr->deallocationFunction_ptr = deallocationFunction_ptr;

    /*Copy the initialized pointer to the one pointed to the passed double pointer*/
    *zajelPointer_ptr = zajel_ptr;

    return status;
}

void zajel_destroy(zajel_s** zajelPointer_ptr COMMA()
                   FILE_AND_LINE_FOR_TYPE())
{
    zajel_s* zajel_ptr;
    /*
     * This function is responsible for:
     ***********************************************************************************************
     *
     * o Validating inputs.
     * o Releasing the allocated resources.
     * o Resetting the control block pointer.
     */

    ASSERT((NULL != zajelPointer_ptr),
           "zajel: Invalid address to pointer passed (zajel_ptr)!",
           fileName,
           lineNumber);
    ASSERT((NULL != *zajelPointer_ptr),
           "zajel: Address passed of a null pointer, expecting an address of a not-null pointer (zajel_ptr)"
           "This might be a sign of double destroy using the same pointer!",
           fileName,
           lineNumber);

    zajel_ptr = *zajelPointer_ptr;

    zajel_ptr->deallocationFunction_ptr(zajel_ptr->messageInformationArray_ptr);
    zajel_ptr->deallocationFunction_ptr(zajel_ptr->componentInformationArray_ptr);
    zajel_ptr->deallocationFunction_ptr(zajel_ptr->threadInformationArray_ptr);
    zajel_ptr->deallocationFunction_ptr(zajel_ptr->coreInformationArray_ptr);
    zajel_ptr->deallocationFunction_ptr(zajel_ptr);

    /*
     * Here the original pointer to pointer must be used, to make sure that the passed pointer is
     * reset, this is done to protect against the accidental use of the pointer after destroying it.
     */
    *zajelPointer_ptr = NULL;
}

void zajel_regsiter_message(zajel_s*                        zajel_ptr,
                            uint32_t                        messageID,
                            zajel_message_handler_function  messageHandler_ptr,
                            char*                           messageName_Ptr COMMA()
                            FILE_AND_LINE_FOR_TYPE())
{
    /*
     * This function is responsible for:
     ***********************************************************************************************
     *
     * o Validating inputs.
     * o Registering the given message handler for the given message ID.
     */
    ASSERT((NULL != zajel_ptr),
           "zajel: Invalid pointer to the control block!",
           fileName,
           lineNumber);
    ASSERT((messageID < zajel_ptr->messageCount),
           "zajel: MessageID passed must be less than the total message count used during initialization!",
           fileName,
           lineNumber);
    ASSERT((NULL != messageHandler_ptr),
           "zajel: Message handler cannot be null!",
           fileName,
           lineNumber);
    ASSERT(('\0' != messageName_Ptr[0]),
           "zajel: Message name cannot be an empty string!",
           fileName,
           lineNumber);
    ASSERT((FALSE == ZAJEL_IS_ITEM_REGISTERED(zajel_ptr->messageInformationArray_ptr[messageID])),
           "zajel: Message is already registerd!",
           fileName,
           lineNumber);


    zajel_ptr->messageInformationArray_ptr[messageID].messageHandlerFunction    = messageHandler_ptr;
#ifdef DEBUG
    zajel_ptr->messageInformationArray_ptr[messageID].messageName_ptr           = messageName_Ptr;
    zajel_ptr->messageInformationArray_ptr[messageID].isRegistered              = TRUE;
    zajel_ptr->messageInformationArray_ptr[messageID].messageID                 = messageID;
#endif /*DEBUG*/
}

void zajel_regsiter_component(zajel_s*  zajel_ptr,
                              uint32_t  componentID,
                              uint32_t  threadID,
                              char*     componentName_Ptr COMMA()
                              FILE_AND_LINE_FOR_TYPE())
{
    /*
     * This function is responsible for:
     ***********************************************************************************************
     *
     * o Validating inputs.
     * o Registering the given component for the given ID.
     */
    ASSERT((NULL != zajel_ptr),
           "zajel: Invalid pointer to the control block!",
           fileName,
           lineNumber);
    ASSERT((componentID < zajel_ptr->componentCount),
           "zajel: componentID passed must be less than the total component count used during initialization!",
           fileName,
           lineNumber);
    ASSERT(('\0' != componentName_Ptr[0]),
           "zajel: Component name cannot be an empty string!",
           fileName,
           lineNumber);
    ASSERT((TRUE == ZAJEL_IS_ITEM_REGISTERED(zajel_ptr->threadInformationArray_ptr[threadID])),
           "zajel: Thread is not registered!",
           fileName,
           lineNumber);
    ASSERT((FALSE == ZAJEL_IS_ITEM_REGISTERED(zajel_ptr->componentInformationArray_ptr[componentID])),
           "zajel: Component is already registered!",
           fileName,
           lineNumber);

    zajel_ptr->componentInformationArray_ptr[componentID].parameters.threadID           = threadID;
#ifdef DEBUG
    zajel_ptr->componentInformationArray_ptr[componentID].parameters.componentName_ptr  = componentName_Ptr;
    zajel_ptr->componentInformationArray_ptr[componentID].parameters.isRegistered       = TRUE;
    zajel_ptr->componentInformationArray_ptr[componentID].parameters.componentID        = componentID;
#endif /*DEBUG*/
}

void zajel_regsiter_thread(zajel_s*                             zajel_ptr,
                           uint32_t                             threadID,
                           uint32_t                             coreID,
                           zajel_thread_blocking_callback       blockingCallback,
                           zajel_thread_non_blocking_callback   nonblockingCallback,
                           char*                                threadName_Ptr COMMA()
                           FILE_AND_LINE_FOR_TYPE())
{
    /*
     * This function is responsible for:
     ***********************************************************************************************
     *
     * o Validating inputs.
     * o Registering the given thread for the given ID.
     */
    ASSERT((NULL != zajel_ptr),
           "zajel: Invalid pointer to the control block!",
           fileName,
           lineNumber);
    ASSERT((threadID < zajel_ptr->threadCount),
           "zajel: threadID passed must be less than the total thread count used during initialization!",
           fileName,
           lineNumber);
    ASSERT(('\0' != threadName_Ptr[0]),
           "zajel: thread name cannot be an empty string!",
           fileName,
           lineNumber);
    ASSERT((TRUE == ZAJEL_IS_ITEM_REGISTERED(zajel_ptr->coreInformationArray_ptr[coreID])),
           "zajel: core is not registered!",
           fileName,
           lineNumber);
    ASSERT((FALSE == ZAJEL_IS_ITEM_REGISTERED(zajel_ptr->threadInformationArray_ptr[threadID])),
           "zajel: thread is already registered!",
           fileName,
           lineNumber);
    ASSERT((NULL != blockingCallback),
           "zajel: blockingCallback cannot be NULL!",
           fileName,
           lineNumber);
    ASSERT((NULL != nonblockingCallback),
           "zajel: nonblockingCallback cannot be NULL!",
           fileName,
           lineNumber);


    zajel_ptr->threadInformationArray_ptr[threadID].coreID              = coreID;
    zajel_ptr->threadInformationArray_ptr[threadID].blockingCallback    = blockingCallback;
    zajel_ptr->threadInformationArray_ptr[threadID].nonblockingCallback = nonblockingCallback;

#ifdef DEBUG
    zajel_ptr->threadInformationArray_ptr[threadID].threadName_ptr      = threadName_Ptr;
    zajel_ptr->threadInformationArray_ptr[threadID].isRegistered        = TRUE;
    zajel_ptr->threadInformationArray_ptr[threadID].threadID            = threadID;
#endif /*DEBUG*/
}

void zajel_regsiter_core(zajel_s*                           zajel_ptr,
                         uint32_t                           coreID,
                         zajel_core_blocking_callback       blockingCallback,
                         zajel_core_non_blocking_callback   nonblockingCallback,
                         char*                              coreName_Ptr COMMA()
                         FILE_AND_LINE_FOR_TYPE())
{
    /*
     * This function is responsible for:
     ***********************************************************************************************
     *
     * o Validating inputs.
     * o Registering the given core for the given ID.
     */
    ASSERT((NULL != zajel_ptr),
           "zajel: Invalid pointer to the control block!",
           fileName,
           lineNumber);
    ASSERT((coreID < zajel_ptr->coreCount),
           "zajel: coreID passed must be less than the total core count used during initialization!",
           fileName,
           lineNumber);
    ASSERT(('\0' != coreName_Ptr[0]),
           "zajel: Core name cannot be an empty string!",
           fileName,
           lineNumber);
    ASSERT((FALSE == ZAJEL_IS_ITEM_REGISTERED(zajel_ptr->coreInformationArray_ptr[coreID])),
           "zajel: Core is already registered!",
           fileName,
           lineNumber);
    ASSERT((NULL != blockingCallback),
           "zajel: blockingCallback cannot be NULL!",
           fileName,
           lineNumber);
    ASSERT((NULL != nonblockingCallback),
           "zajel: nonblockingCallback cannot be NULL!",
           fileName,
           lineNumber);

    zajel_ptr->coreInformationArray_ptr[coreID].blockingCallback    = blockingCallback;
    zajel_ptr->coreInformationArray_ptr[coreID].nonblockingCallback = nonblockingCallback;

#ifdef DEBUG
    zajel_ptr->coreInformationArray_ptr[coreID].coreName_ptr        = coreName_Ptr;
    zajel_ptr->coreInformationArray_ptr[coreID].isRegistered        = TRUE;
    zajel_ptr->coreInformationArray_ptr[coreID].coreID              = coreID;
#endif /*DEBUG*/
}

void zajel_send(zajel_s*                zajel_ptr,
                uint32_t                sourceComponentID,
                uint32_t                destinationComponentID,
                uint32_t                messageID,
                zajel_delivery_type_e   deliveryType,
                void*                   message_ptr COMMA()
                FILE_AND_LINE_FOR_TYPE())
{
    zajel_component_dynamic_relation_e dynamicRelation;

    dynamicRelation = zajel_get_component_dynamic_relation(sourceComponentID,
                                                           destinationComponentID);

    switch(dynamicRelation)
    {
        /*<This switch checks the dynamic relation between both components and act accordingly>*/

        case ZAJEL_COMPONENT_DYNAMIC_RELATION_SAME_THREAD:
            /*<Both components are running in the same thread>*/

            switch(deliveryType)
            {
                /*<Checks the requested delivery type and act>*/

                case ZAJEL_DELIVERY_TYPE_SYNCHRONOUS:
                    /*<Sender will not proceed (blocked) until the receiver finishes>*/

                    break;/*<Sender will not proceed (blocked) until the receiver finishes>*/
                case ZAJEL_DELIVERY_TYPE_ASYNCHRONOUS:
                    /*<Sender will continue independent of the receiver >*/

                    break; /*<Sender will continue independent of the receiver>*/
                default:
                    /*<Invalid delivery type requested>*/
                    ASSERT((FALSE),
                           "zajel: Invalid delivery type requested!",
                           fileName,
                           lineNumber);
                    break;/*<Invalid delivery type requested>*/
            } /*switch: <Checks the requested delivery type and act>*/

            break;/*<Both components are running in the same thread>*/
        case ZAJEL_COMPONENT_DYNAMIC_RELATION_DIFFERENT_THREADS:
            /*<Both components are running in different threads, same core>*/

            break;/*<Both components are running in different threads, same core>*/
        case ZAJEL_COMPONENT_DYNAMIC_RELATION_DIFFERENT_CORES:
            /*<Both components are running in different threads, different cores>*/

            break;/*<Both components are running in different threads, different cores>*/
        default:
            /*<Invalid dynamic relation>*/
            ASSERT((FALSE),
                   "zajel: Invalid dynamic relation received!",
                   fileName,
                   lineNumber);
            break;/*<Invalid dynamic relation>*/
    } /*switch: <This switch checks the dynamic relation between both components and act accordingly>*/
}



/***************************************************************************************************
 *
 *  I N T E R N A L   F U N C T I O N   D E F I N I T I O N S
 *
 **************************************************************************************************/

zajel_component_dynamic_relation_e zajel_get_component_dynamic_relation(uint32_t sourceComponentID,
                                                                        uint32_t destinationComponentID)
{
    /*
     * FIXME: mgalal on Mar 6, 2010
     *
     * Needs to be fixed according to the discussion with Karim.
     */
    return ZAJEL_COMPONENT_DYNAMIC_RELATION_SAME_THREAD;
}
