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


/*
 * TODO: mgalal on Mar 9, 2010
 *
 * It might be required to add another field to the message descriptor to hold user-status for
 * synchronous messages.
 */

/***************************************************************************************************
 *
 *  I N C L U D E S
 *
 **************************************************************************************************/
#include <stddef.h>
#include <stdio.h>
#include "zajel.h"

/***************************************************************************************************
 *
 *  M A C R O S
 *
 **************************************************************************************************/


/*Total number of messages*/
#define ZAJEL_MESSAGE_COUNT     (100)
/*Total number of components*/
#define ZAJEL_COMPONENT_COUNT   (10)
/*Total number of threads*/
#define ZAJEL_THREAD_COUNT      (4)
/*Total number of cores*/
#define ZAJEL_CORE_COUNT        (2)

/***************************************************************************************************
 *  Macro Name  : ZAJEL_IS_ITEM_REGISTERED
 *
 *  Arguments   : item
 *
 *  Description : This macro can be used to set/get the registration status of the given item.
 *
 *  Returns     : boolean.
 **************************************************************************************************/
#define ZAJEL_IS_ITEM_REGISTERED(item) ((item).isRegistered)

/***************************************************************************************************
 *  Macro Name  : ZAJEL_THREAD_SYNCHRONIZE
 *
 *  Arguments   : cfw_ptr, desc_ptr, action
 *
 *  Description : This macro locks the calling thread.
 *
 *  Returns     : None.
 **************************************************************************************************/
#define ZAJEL_THREAD_SYNCHRONIZE(cfw_ptr, id, action)                                              \
{                                                                                                  \
    zajel_component_information_u*  component_ptr;                                                 \
    zajel_thread_information_s*     thread_ptr;                                                    \
                                                                                                   \
    component_ptr =  &(cfw_ptr)->componentInformationArray[(id)];                                  \
    thread_ptr    =  &(cfw_ptr)->threadInformationArray[component_ptr->parameters.threadID];       \
                                                                                                   \
    thread_ptr->action##Callback(thread_ptr->synchronizationPrimitive_ptr);                        \
}

/***************************************************************************************************
 *  Macro Name  : ZAJEL_THREAD_HANDLE_MESSAGE
 *
 *  Arguments   : cfw_ptr, desc_ptr
 *
 *  Description : This macro deliver the given message (descriptor) to the given thread.
 *
 *  Returns     : None.
 **************************************************************************************************/
#define ZAJEL_THREAD_HANDLE_MESSAGE(cfw_ptr, desc_ptr)                                             \
{                                                                                                  \
    zajel_component_information_u*  component_ptr;                                                 \
    zajel_thread_information_s*     thread_ptr;                                                    \
                                                                                                   \
    component_ptr =  &(cfw_ptr)->componentInformationArray[(desc_ptr)->destinationComponentID];    \
    thread_ptr    =  &(cfw_ptr)->threadInformationArray[component_ptr->parameters.threadID];       \
                                                                                                   \
    thread_ptr->handleMessageCallback((desc_ptr));                                                 \
}

/***************************************************************************************************
 *  Macro Name  : ZAJEL_CORE_HANDLE_MESSAGE
 *
 *  Arguments   : controlBlock_ptr, desc_ptr
 *
 *  Description : This macro deliver the given message (descriptor) to the given core.
 *
 *  Returns     : None.
 **************************************************************************************************/
#define ZAJEL_CORE_HANDLE_MESSAGE(controlBlock_ptr, desc_ptr)                                      \
{                                                                                                  \
    zajel_component_information_u*  destinationComponent_ptr;                                      \
    zajel_core_information_s*       destinationCore_ptr;                                           \
                                                                                                   \
    destinationComponent_ptr =  &(controlBlock_ptr)->componentInformationArray[(desc_ptr)->destinationComponentID];\
    destinationCore_ptr      =  &(controlBlock_ptr)->coreInformationArray[destinationComponent_ptr->parameters.coreID];\
                                                                                                   \
    destinationCore_ptr->handleMessageCallback((desc_ptr));                                        \
}


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
    ZAJEL_COMPONENT_DYNAMIC_RELATION_SAME_CORE          = 1,
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
    bool_t                          isRegistered;
    /*Message identifier*/
    message_id                      messageID;
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
    /*for padding*/
    uint8_t reserved1;
    /*for padding*/
    uint8_t reserved2;
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
    uint32_t                        coreID;
    /*This callback function is used to deliver the messages to the thread*/
    zajel_handle_message_callback   handleMessageCallback;
    /*
     * This can be any synchronization primitive passed by the framework to the blockFunction in order
     * to support synchronous messages
     */
    void*                           synchronizationPrimitive_ptr;
    /*
     * This function will be called by the framework after sending synchronous messages, and it will
     * block the sending thread until the receiving thread finishes.
     *
     * It is not applicable for "same thread" synchronous message.
     */
    zajel_block_callback            blockCallback;
    /*
     * This function will be called by the receiving thread of synchronous message to unblock the
     * sending thread.
     *
     * It is not applicable for "same thread" synchronous message.
     */
    zajel_unblock_callback          unblockCallback;

#ifdef DEBUG
    /*TRUE if the thread is registered*/
    bool_t                          isRegistered;
    /*Thread identifier, this is meant to be user-assigned rather than the OS-assigned*/
    uint32_t                        threadID;
    /*Thread textual name, to be used for debugging*/
    char*                           threadName_ptr;
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
    /*This call back function is used to deliver messages to the destination core*/
    zajel_core_handle_message_callback  handleMessageCallback;
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
    /*An array that holds the registered message handlers*/
    zajel_message_information_s     messageInformationArray[ZAJEL_MESSAGE_COUNT];
    /*An array that holds the component related information like the component ID, thread/core ID*/
    zajel_component_information_u   componentInformationArray[ZAJEL_COMPONENT_COUNT];
    /*An array that holds the thread related information*/
    zajel_thread_information_s      threadInformationArray[ZAJEL_THREAD_COUNT];
    /*An array that holds the core related information*/
    zajel_core_information_s        coreInformationArray[ZAJEL_CORE_COUNT];
    /*The deallocation function pointer to be used when destroying the control block*/
    zajel_deallocation_function     deallocationFunction_ptr;
};

/***************************************************************************************************
 *
 *  I N T E R N A L   F U N C T I O N   D E C L A R A T I O N S
 *
 **************************************************************************************************/



/***************************************************************************************************
 *  Name        : zajel_get_component_dynamic_relation
 *
 *  Arguments   : zajel_s*    zajel_ptr
 *                uint32_t    sourceComponentID,
 *                uint32_t    destinationComponentID
 *
 *  Description : Determines the dynamic relation between the two given component.
 *
 *  Returns     : zajel_component_dynamic_relation_e.
 **************************************************************************************************/
zajel_component_dynamic_relation_e zajel_get_component_dynamic_relation(zajel_s*    zajel_ptr,
                                                                        uint32_t    sourceComponentID,
                                                                        uint32_t    destinationComponentID);


/***************************************************************************************************
 *
 *  I N T E R F A C E   F U N C T I O N   D E F I N I T I O N S
 *
 **************************************************************************************************/


void zajel_init(zajel_s**                     zajelPointer_ptr,
                allocation_function           allocationFunction_ptr,
                zajel_deallocation_function   deallocationFunction_ptr COMMA()
                FILE_AND_LINE_FOR_TYPE())
{
    /*Temporarily holds the allocated structure, just to avoid using the multiple indirection frequently*/
    zajel_s* zajel_ptr;
    /*Status of the initialization*/
    /*Temporary counter*/
    uint32_t i;

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
    ASSERT((ZAJEL_MESSAGE_COUNT > 0),
           "zajel: Message count must be greater than zero!",
           fileName,
           lineNumber);
    ASSERT((ZAJEL_COMPONENT_COUNT > 0),
           "zajel: Component count must be greater than zero!",
           fileName,
           lineNumber);
    ASSERT((ZAJEL_THREAD_COUNT > 0),
           "zajel: Thread count must be greater than zero!",
           fileName,
           lineNumber);
    ASSERT((ZAJEL_CORE_COUNT > 0),
           "zajel: Core count must be greater than zero!",
           fileName,
           lineNumber);
    /*
     * TODO: mgalal on Feb 27, 2010
     *
     * Add proper exception handling in this function, allocation failure, invalid parameters, etc...
     */

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


#ifdef DEBUG
    for(i = 0; i < ZAJEL_MESSAGE_COUNT; ++i)
    {
        /*<Reset all message handlers>*/

        ZAJEL_IS_ITEM_REGISTERED(zajel_ptr->messageInformationArray[i]) = FALSE;
    } /*for: <Reset all message handlers>*/

    for(i = 0; i < ZAJEL_COMPONENT_COUNT; ++i)
    {
        /*<Reset all component information>*/

        ZAJEL_IS_ITEM_REGISTERED(zajel_ptr->componentInformationArray[i].parameters) = FALSE;
    } /*for: <Reset all component information>*/

    for(i = 0; i < ZAJEL_THREAD_COUNT; ++i)
    {
        /*<Reset all thread information>*/
        ZAJEL_IS_ITEM_REGISTERED(zajel_ptr->threadInformationArray[i]) = FALSE;
    } /*for: <Reset all thread information>*/

    for(i = 0; i < ZAJEL_CORE_COUNT; ++i)
    {
        /*<Reset all core information>*/

        ZAJEL_IS_ITEM_REGISTERED(zajel_ptr->coreInformationArray[i]) = FALSE;
    } /*for: <Reset all core information>*/
#endif /*DEBUG*/

    zajel_ptr->deallocationFunction_ptr = deallocationFunction_ptr;

    /*Copy the initialized pointer to the one pointed to the passed double pointer*/
    *zajelPointer_ptr = zajel_ptr;
} /*function: zajel_init*/

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

    zajel_ptr->deallocationFunction_ptr(zajel_ptr);

    /*
     * Here the original pointer to pointer must be used, to make sure that the passed pointer is
     * reset, this is done to protect against the accidental use of the pointer after destroying it.
     */
    *zajelPointer_ptr = NULL;
} /*function: zajel_destory*/

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
    ASSERT((messageID < ZAJEL_MESSAGE_COUNT),
           "zajel: MessageID passed must be less than the total message count used during initialization!",
           fileName,
           lineNumber);
    ASSERT((messageID),
           "zajel: Zero cannot be used as a message ID, as it is reserved by the framework for acknowledge!",
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
    ASSERT((FALSE == ZAJEL_IS_ITEM_REGISTERED(zajel_ptr->messageInformationArray[messageID])),
           "zajel: Message is already registerd!",
           fileName,
           lineNumber);


    zajel_ptr->messageInformationArray[messageID].messageHandlerFunction    = messageHandler_ptr;
#ifdef DEBUG
    zajel_ptr->messageInformationArray[messageID].messageName_ptr           = messageName_Ptr;
    zajel_ptr->messageInformationArray[messageID].isRegistered              = TRUE;
    zajel_ptr->messageInformationArray[messageID].messageID                 = messageID;
#endif /*DEBUG*/
} /*function: zajel_register_message*/

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
    ASSERT((componentID < ZAJEL_COMPONENT_COUNT),
           "zajel: componentID passed must be less than the total component count used during initialization!",
           fileName,
           lineNumber);
    ASSERT(('\0' != componentName_Ptr[0]),
           "zajel: Component name cannot be an empty string!",
           fileName,
           lineNumber);
    ASSERT((TRUE == ZAJEL_IS_ITEM_REGISTERED(zajel_ptr->threadInformationArray[threadID])),
           "zajel: Thread is not registered!",
           fileName,
           lineNumber);
    ASSERT((FALSE == ZAJEL_IS_ITEM_REGISTERED(zajel_ptr->componentInformationArray[componentID].parameters)),
           "zajel: Component is already registered!",
           fileName,
           lineNumber);

    zajel_ptr->componentInformationArray[componentID].parameters.threadID           = threadID;
#ifdef DEBUG
    zajel_ptr->componentInformationArray[componentID].parameters.componentName_ptr  = componentName_Ptr;
    zajel_ptr->componentInformationArray[componentID].parameters.isRegistered       = TRUE;
    zajel_ptr->componentInformationArray[componentID].parameters.componentID        = componentID;
#endif /*DEBUG*/
} /*function: zajel_register_component*/

void zajel_regsiter_thread(zajel_s*                         zajel_ptr,
                           uint32_t                         threadID,
                           uint32_t                         coreID,
                           zajel_handle_message_callback    handleMessageCallback,
                           zajel_block_callback             blockCallback,
                           zajel_unblock_callback           unblockCallback,
                           void*                            synchronizationPrimitive_ptr,
                           char*                            threadName_Ptr COMMA()
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
    ASSERT((threadID < ZAJEL_THREAD_COUNT),
           "zajel: threadID passed must be less than the total thread count used during initialization!",
           fileName,
           lineNumber);
    ASSERT(('\0' != threadName_Ptr[0]),
           "zajel: thread name cannot be an empty string!",
           fileName,
           lineNumber);
    ASSERT((TRUE == ZAJEL_IS_ITEM_REGISTERED(zajel_ptr->coreInformationArray[coreID])),
           "zajel: core is not registered!",
           fileName,
           lineNumber);
    ASSERT((FALSE == ZAJEL_IS_ITEM_REGISTERED(zajel_ptr->threadInformationArray[threadID])),
           "zajel: thread is already registered!",
           fileName,
           lineNumber);
    ASSERT((NULL != handleMessageCallback),
           "zajel: handleMessageCallback cannot be NULL!",
           fileName,
           lineNumber);
    ASSERT((NULL != blockCallback),
           "zajel: blockCallback cannot be NULL!",
           fileName,
           lineNumber);
    ASSERT((NULL != unblockCallback),
           "zajel: unblockCallback cannot be NULL!",
           fileName,
           lineNumber);


    zajel_ptr->threadInformationArray[threadID].coreID                          = coreID;
    zajel_ptr->threadInformationArray[threadID].handleMessageCallback           = handleMessageCallback;
    zajel_ptr->threadInformationArray[threadID].blockCallback                   = blockCallback;
    zajel_ptr->threadInformationArray[threadID].unblockCallback                 = unblockCallback;
    zajel_ptr->threadInformationArray[threadID].synchronizationPrimitive_ptr    = synchronizationPrimitive_ptr;

#ifdef DEBUG
    zajel_ptr->threadInformationArray[threadID].threadName_ptr      = threadName_Ptr;
    zajel_ptr->threadInformationArray[threadID].isRegistered        = TRUE;
    zajel_ptr->threadInformationArray[threadID].threadID            = threadID;
#endif /*DEBUG*/
} /*function: zajel_register_thread*/

void zajel_regsiter_core(zajel_s*                           zajel_ptr,
                         uint32_t                           coreID,
                         zajel_core_handle_message_callback handleMessageCallback,
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
    ASSERT((coreID < ZAJEL_CORE_COUNT),
           "zajel: coreID passed must be less than the total core count used during initialization!",
           fileName,
           lineNumber);
    ASSERT(('\0' != coreName_Ptr[0]),
           "zajel: Core name cannot be an empty string!",
           fileName,
           lineNumber);
    ASSERT((FALSE == ZAJEL_IS_ITEM_REGISTERED(zajel_ptr->coreInformationArray[coreID])),
           "zajel: Core is already registered!",
           fileName,
           lineNumber);
    ASSERT((NULL != handleMessageCallback),
           "zajel: handleMessageCallback cannot be NULL!",
           fileName,
           lineNumber);

    zajel_ptr->coreInformationArray[coreID].handleMessageCallback   = handleMessageCallback;


#ifdef DEBUG
    zajel_ptr->coreInformationArray[coreID].coreName_ptr            = coreName_Ptr;
    zajel_ptr->coreInformationArray[coreID].isRegistered            = TRUE;
    zajel_ptr->coreInformationArray[coreID].coreID                  = coreID;
#endif /*DEBUG*/
} /*function: zajel_register_core*/

void zajel_send(zajel_s*    zajel_ptr,
                void*       message_ptr COMMA()
                FILE_AND_LINE_FOR_TYPE())
{
    zajel_message_descriptor_s*         descriptor_ptr;
    zajel_component_dynamic_relation_e  dynamicRelation;

    descriptor_ptr = (zajel_message_descriptor_s*) message_ptr;

    ASSERT((NULL != zajel_ptr),
           "zajel: Invalid control block pointer!",
           fileName,
           lineNumber);
    ASSERT((NULL != message_ptr),
           "zajel: message_cannot equal NULL!",
           fileName,
           lineNumber);
    ASSERT((descriptor_ptr->messageID < ZAJEL_MESSAGE_COUNT),
           "zajel: Message ID is greater than the supported message count!",
           fileName,
           lineNumber);
    ASSERT((descriptor_ptr->messageID),
           "zajel: Zero cannot be used as a message ID, as it is reserved by the framework for acknowledge!",
           fileName,
           lineNumber);
    ASSERT((descriptor_ptr->sourceComponentID < ZAJEL_COMPONENT_COUNT),
           "zajel: Source component ID is greater than the supported message count!",
           fileName,
           lineNumber);
    ASSERT((descriptor_ptr->destinationComponentID < ZAJEL_COMPONENT_COUNT),
           "zajel: Destination component ID is greater than the supported message count!",
           fileName,
           lineNumber);
    ASSERT(((TRUE == descriptor_ptr->isSynchronous) || (FALSE == descriptor_ptr->isSynchronous)),
           "zajel: isSynchronous is niether true nor false!",
           fileName,
           lineNumber);


    dynamicRelation = zajel_get_component_dynamic_relation(zajel_ptr,
                                                           descriptor_ptr->sourceComponentID,
                                                           descriptor_ptr->destinationComponentID);

    switch(dynamicRelation)
    {
        /*<This switch checks the dynamic relation between both components and act accordingly>*/

        case ZAJEL_COMPONENT_DYNAMIC_RELATION_SAME_THREAD:
            /*<Both components are running in the same thread>*/

            if(descriptor_ptr->isSynchronous)
            {
                /*<Synchronous message, call the handler directly>*/
                zajel_ptr->messageInformationArray[descriptor_ptr->messageID].messageHandlerFunction(descriptor_ptr);
            } /*if: <Synchronous message, call the handler directly>*/
            else
            {
                /*<Asynchronous message, deliver the message to the destination thread>*/
                ZAJEL_THREAD_HANDLE_MESSAGE(zajel_ptr,
                                            descriptor_ptr);
            } /*else: <Asynchronous message, deliver the message to the destination thread>*/

            break;/*<Both components are running in the same thread>*/
        case ZAJEL_COMPONENT_DYNAMIC_RELATION_SAME_CORE:
            /*<Both components are running in different threads, same core>*/

            ZAJEL_THREAD_HANDLE_MESSAGE(zajel_ptr,
                                        descriptor_ptr);

            if(descriptor_ptr->isSynchronous)
            {
                /*<Message is synchronous, framework will now block the source (calling) thread>*/
                ZAJEL_THREAD_SYNCHRONIZE(zajel_ptr,
                                         descriptor_ptr->sourceComponentID,
                                         block);
            } /*if: <Message is synchronous, framework will now block the source (calling) thread>*/

            break;/*<Both components are running in different threads, same core>*/
        case ZAJEL_COMPONENT_DYNAMIC_RELATION_DIFFERENT_CORES:
            /*<Both components are running in different threads, different cores>*/

            ZAJEL_CORE_HANDLE_MESSAGE(zajel_ptr,
                                      descriptor_ptr);

            if(descriptor_ptr->isSynchronous)
            {
                /*<Message is synchronous, framework will now block the source (calling) thread>*/
                ZAJEL_THREAD_SYNCHRONIZE(zajel_ptr,
                                         descriptor_ptr->sourceComponentID,
                                         block);
            } /*if: <Message is synchronous, framework will now block the source (calling) thread>*/

            break;/*<Both components are running in different threads, different cores>*/
        default:
            /*<Invalid dynamic relation>*/
            ASSERT((FALSE),
                   "zajel: Invalid dynamic relation received!",
                   fileName,
                   lineNumber);
            break;/*<Invalid dynamic relation>*/
    } /*switch: <This switch checks the dynamic relation between both components and act accordingly>*/
} /*function: zajel_send*/

void zajel_acknowledge(zajel_s*                zajel_ptr,
                       void*                   message_ptr COMMA()
                       FILE_AND_LINE_FOR_TYPE())
{
    zajel_message_descriptor_s          ackDescriptor;
    zajel_message_descriptor_s*         descriptor_ptr;
    zajel_component_dynamic_relation_e  dynamicRelation;

    descriptor_ptr = (zajel_message_descriptor_s*) message_ptr;

    ASSERT((NULL != zajel_ptr),
           "zajel: Invalid control block pointer!",
           fileName,
           lineNumber);
    ASSERT((NULL != message_ptr),
           "zajel: message_cannot equal NULL!",
           fileName,
           lineNumber);
    ASSERT((descriptor_ptr->messageID < ZAJEL_MESSAGE_COUNT),
           "zajel: Message ID is greater than the supported message count!",
           fileName,
           lineNumber);
    ASSERT((descriptor_ptr->messageID),
           "zajel: Zero cannot be used as a message ID, as it is reserved by the framework for acknowledge!",
           fileName,
           lineNumber);
    ASSERT((descriptor_ptr->sourceComponentID < ZAJEL_COMPONENT_COUNT),
           "zajel: Source component ID is greater than the supported message count!",
           fileName,
           lineNumber);
    ASSERT((descriptor_ptr->destinationComponentID < ZAJEL_COMPONENT_COUNT),
           "zajel: Destination component ID is greater than the supported message count!",
           fileName,
           lineNumber);
    ASSERT(((TRUE == descriptor_ptr->isSynchronous) || (FALSE == descriptor_ptr->isSynchronous)),
           "zajel: isSynchronous is neither true nor false!",
           fileName,
           lineNumber);


    dynamicRelation = zajel_get_component_dynamic_relation(zajel_ptr,
                                                           descriptor_ptr->sourceComponentID,
                                                           descriptor_ptr->destinationComponentID);

    switch(dynamicRelation)
    {
        /*<This switch checks the dynamic relation between both components and act accordingly>*/

        case ZAJEL_COMPONENT_DYNAMIC_RELATION_SAME_THREAD:
            /*<Both components are running in the same thread>*/

            ASSERT((TRUE == descriptor_ptr->isSynchronous),
                   "zajel: It is not allowed to acknowledge asynchronous message!",
                   fileName,
                   lineNumber);
            ASSERT((FALSE),
                   "zajel: It is not allowed to ack a message sent from the same thread!",
                   fileName,
                   lineNumber);

            break;/*<Both components are running in the same thread>*/
        case ZAJEL_COMPONENT_DYNAMIC_RELATION_SAME_CORE:
            /*<Both components are running in different threads, same core>*/

            ASSERT((descriptor_ptr->isSynchronous),
                   "zajel: It is not allowed to acknowledge asynchronous message!",
                   fileName,
                   lineNumber);

            ZAJEL_THREAD_SYNCHRONIZE(zajel_ptr,
                                     descriptor_ptr->sourceComponentID,
                                     unblock);

            break;/*<Both components are running in different threads, same core>*/
        case ZAJEL_COMPONENT_DYNAMIC_RELATION_DIFFERENT_CORES:
            /*<Both components are running in different threads, different cores>*/

            ASSERT((TRUE == descriptor_ptr->isSynchronous),
                   "zajel: It is not allowed to acknowledge asynchronous message!",
                   fileName,
                   lineNumber);

            /*Adjusting the message parameter so that it is delivered to the original source*/
            ackDescriptor.messageID                 = ZAJEL_ACK_MESSAGE_ID;
            ackDescriptor.sourceComponentID         = descriptor_ptr->destinationComponentID;
            ackDescriptor.destinationComponentID    = descriptor_ptr->sourceComponentID;
            ackDescriptor.isSynchronous             = descriptor_ptr->isSynchronous;

            zajel_send(zajel_ptr,
                       (void*)&ackDescriptor COMMA()
                       FILE_AND_LINE_FOR_CALL());

            break;/*<Both components are running in different threads, different cores>*/
        default:
            /*<Invalid dynamic relation>*/
            ASSERT((FALSE),
                   "zajel: Invalid dynamic relation received!",
                   fileName,
                   lineNumber);
            break;/*<Invalid dynamic relation>*/
    } /*switch: <This switch checks the dynamic relation between both components and act accordingly>*/
} /*function: zajel_acknowledge*/

void zajel_deliver(zajel_s* zajel_ptr,
                   void*    message_ptr COMMA()
                   FILE_AND_LINE_FOR_TYPE())
{
    zajel_message_descriptor_s*         descriptor_ptr;
    zajel_component_dynamic_relation_e  dynamicRelation;

    descriptor_ptr = (zajel_message_descriptor_s*) message_ptr;

    ASSERT((NULL != zajel_ptr),
           "zajel: Invalid control block pointer!",
           fileName,
           lineNumber);
    ASSERT((NULL != message_ptr),
           "zajel: message_cannot equal NULL!",
           fileName,
           lineNumber);
    ASSERT((descriptor_ptr->messageID < ZAJEL_MESSAGE_COUNT),
           "zajel: Message ID is greater than the supported message count!",
           fileName,
           lineNumber);
    ASSERT((descriptor_ptr->messageID),
           "zajel: Zero cannot be used as a message ID, as it is reserved by the framework for acknowledge!",
           fileName,
           lineNumber);
    ASSERT((descriptor_ptr->sourceComponentID < ZAJEL_COMPONENT_COUNT),
           "zajel: Source component ID is greater than the supported message count!",
           fileName,
           lineNumber);
    ASSERT((descriptor_ptr->destinationComponentID < ZAJEL_COMPONENT_COUNT),
           "zajel: Destination component ID is greater than the supported message count!",
           fileName,
           lineNumber);
    ASSERT(((TRUE == descriptor_ptr->isSynchronous) || (FALSE == descriptor_ptr->isSynchronous)),
           "zajel: isSynchronous is niether true nor false!",
           fileName,
           lineNumber);


    dynamicRelation = zajel_get_component_dynamic_relation(zajel_ptr,
                                                           descriptor_ptr->sourceComponentID,
                                                           descriptor_ptr->destinationComponentID);

    switch(dynamicRelation)
    {
        /*<This switch checks the dynamic relation between both components and act accordingly>*/

        case ZAJEL_COMPONENT_DYNAMIC_RELATION_SAME_THREAD:
            /*<Both components are running in the same thread>*/
            /*@fallthrough@*/
        case ZAJEL_COMPONENT_DYNAMIC_RELATION_SAME_CORE:
            /*<Both components are running in different threads, same core>*/

            ASSERT((FALSE),
                   "zajel: This function (zajel_deliver) shall not be used for same-thread or same-core communications!",
                   fileName,
                   lineNumber);
            break;/*<Both components are running in different threads, same core>*/
        case ZAJEL_COMPONENT_DYNAMIC_RELATION_DIFFERENT_CORES:
            /*<Both components are running in different threads, different cores>*/

            if(ZAJEL_ACK_MESSAGE_ID != descriptor_ptr->messageID)
            {
                /*<Normal message received from a different core>*/

                ASSERT((descriptor_ptr->isSynchronous),
                       "zajel: Acknowledge is received for an asynchronous message!",
                       fileName,
                       lineNumber);

                ZAJEL_THREAD_HANDLE_MESSAGE(zajel_ptr,
                                            descriptor_ptr);
            } /*if: <Normal message received from a different core>*/
            else
            {
                /*<Acknowledge message received from a different core>*/
                ZAJEL_THREAD_SYNCHRONIZE(zajel_ptr,
                                         descriptor_ptr->destinationComponentID,
                                         unblock);
            } /*else: <Acknowledge message received from a different core>*/

            break;/*<Both components are running in different threads, different cores>*/
        default:
            /*<Invalid dynamic relation>*/
            ASSERT((FALSE),
                   "zajel: Invalid dynamic relation received!",
                   fileName,
                   lineNumber);
            break;/*<Invalid dynamic relation>*/
    } /*switch: <This switch checks the dynamic relation between both components and act accordingly>*/
} /*function: zajel_deliver*/


/***************************************************************************************************
 *
 *  I N T E R N A L   F U N C T I O N   D E F I N I T I O N S
 *
 **************************************************************************************************/


zajel_component_dynamic_relation_e zajel_get_component_dynamic_relation(zajel_s*    zajel_ptr,
                                                                        uint32_t    sourceComponentID,
                                                                        uint32_t    destinationComponentID)
{
    zajel_component_information_u decision;
    /*
     * FIXME: mgalal on Mar 6, 2010
     *
     * Needs to be fixed according to the discussion with Karim.
     */

    decision.handle =   zajel_ptr->componentInformationArray[sourceComponentID].handle ^
                        zajel_ptr->componentInformationArray[destinationComponentID].handle;

    return ((!decision.parameters.threadID) ?   (ZAJEL_COMPONENT_DYNAMIC_RELATION_SAME_THREAD) :
            (!decision.parameters.coreID)   ?   (ZAJEL_COMPONENT_DYNAMIC_RELATION_SAME_CORE) :
                                                (ZAJEL_COMPONENT_DYNAMIC_RELATION_DIFFERENT_CORES));

} /*function: zajel_get_component_dynamic_relation*/
/*function: */
