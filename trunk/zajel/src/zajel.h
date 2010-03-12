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
#ifndef ZAJEL_H_
#define ZAJEL_H_

#include <stdint.h>

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

/*This message is reserved for inter-core synchronous message synchronization*/
#define ZAJEL_ACK_MESSAGE_ID            (0)

#ifndef FALSE
#define FALSE                           (0)
#endif

#ifndef TRUE
#define TRUE                            (1)
#endif

#ifndef INLINE
#define INLINE __inline__
#endif

#ifndef STATIC
#define STATIC static
#endif

#ifdef DEBUG

#define FILE_NAME                       fileName
#define LINE_NUMBER                     lineNumber
#define FILE_AND_LINE_FOR_TYPE()        const char*  FILE_NAME, uint32_t LINE_NUMBER
#define FILE_AND_LINE_FOR_CALL()        FILE_NAME, LINE_NUMBER
#define FILE_AND_LINE_FOR_REF()        __FILE__,  __LINE__
#define COMMA()                         ,
#define ASSERT(condition, message, file, line)                                                     \
{                                                                                                  \
    uint8_t* ptr = NULL;                                                                           \
    if(FALSE == (condition))                                                                       \
    {                                                                                              \
        printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");\
        printf("!! ASSERTION FAILED\n");                                                           \
        printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");\
        printf("!! File: %s\n", __FILE__);                                                         \
        printf("!! Line: %d\n", __LINE__);                                                         \
        printf("!! Function: %s\n", __FUNCTION__);                                                 \
        printf("!! Message: %s\n", (message));                                                     \
        printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");\
        printf("!! Caller file: %s\n", (file));                                                    \
        printf("!! Caller line: %d\n", ((int)line));                                               \
        printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");\
        *ptr = 0;                                                                                  \
    } /*if(Assertion Failed)*/                                                                     \
}
#else

#define FILE_NAME
#define LINE_NUMBER
#define FILE_AND_LINE_FOR_TYPE()
#define FILE_AND_LINE_FOR_CALL()
#define FILE_AND_LINE_FOR_REF()
#define COMMA()
#define ASSERT(condition, message, file, line)

#endif


/***************************************************************************************************
 *
 *  T Y P E S
 *
 **************************************************************************************************/

/*The communication framework control block*/
typedef struct zajel zajel_s;

/*boolean type*/
typedef uint8_t bool_t;

/*Supports up to 255 unique message id (0 is reserved for ack). In case more messages are needed,
 * a bigger type (short, long) can be used instead of byte*/
typedef uint8_t message_id;

/***************************************************************************************************
 * Enumeration Name:
 * zajel_status_e
 *
 * Enumeration Description:
 * Represent the different status that can be returned from the library functions.
 **************************************************************************************************/
typedef enum zajel_status
{
    ZAJEL_STATUS_SUCCESS = 0,
    ZAJEL_STATUS_FAILURE = 1
} zajel_status_e;

/***************************************************************************************************
 * Structure Name:
 * zajel_message_descriptor_s
 *
 * Structure Description:
 * This structure shall be the first member in any message used by the framework, the framework will
 * take care of filling it properly.
 **************************************************************************************************/
typedef struct zajel_message_descriptor
{
    /*Message Identifier*/
    message_id messageID;
    /*Sender component identifier */
    uint8_t    sourceComponentID;
    /*Receiver component identifier*/
    uint8_t    destinationComponentID;
    /*Shall message be sent synchronously (i.e. sender will wait for the receiver to finish before continuing)*/
    bool_t     isSynchronous;
} zajel_message_descriptor_s;

/*Memory allocation function prototype*/
typedef void*(*allocation_function)(size_t bytesCount);

/*Memory de-allocation function prototype*/
typedef void (*zajel_deallocation_function)(void* ptr);

/*Locking function prototype*/
typedef void (*lock_function)();

/*Unlocking function prototype*/
typedef void (*unlock_function)();

/*Defines the prototypes of message handler functions*/
typedef void (*zajel_message_handler_function) (zajel_message_descriptor_s*);

/*
 * A callback function when called, shall block the sending (caller) thread, and it is used for
 * synchronous message delivery.
 */
typedef void (*zajel_block_callback) (void*);

/*
 * A callback function when called, shall unblock the receiving thread, and it is used for synchronous
 * message delivery.
 */
typedef void (*zajel_unblock_callback) (void*);

/*Called by the framework so that the receiver thread handle the given message*/
typedef void (*zajel_handle_message_callback) (zajel_message_descriptor_s*);

/*Called by the framework so that the receiver core handle the given message*/
typedef void (*zajel_core_handle_message_callback) (zajel_message_descriptor_s*);


/***************************************************************************************************
 *
 *  I N T E R F A C E   F U N C T I O N   D E C L A R A T I O N S
 *
 **************************************************************************************************/


/***************************************************************************************************
 *  Name        : zajel_init
 *
 *  Arguments   : zajel_s**             zajelPointer_ptr,
 *                allocation_function   allocationFunction_ptr,
 *                zajel_deallocation_function deallocationFunction_ptr COMMA()
 *                FILE_AND_LINE_FOR_TYPE()
 *
 *  Description : This function initializes the zajel framework.
 *
 *  Returns     : void.
 **************************************************************************************************/
void zajel_init(zajel_s**                     zajelPointer_ptr,
                allocation_function           allocationFunction_ptr,
                zajel_deallocation_function   deallocationFunction_ptr COMMA()
                FILE_AND_LINE_FOR_TYPE());

/***************************************************************************************************
 *  Name        : zajel_destroy
 *
 *  Arguments   : zajel_s* zajelPointer_ptr COMMA()
 *                FILE_AND_LINE_FOR_TYPE()
 *
 *  Description : This function destroy the framework, releasing any allocated resources.
 *
 *  Returns     : void.
 **************************************************************************************************/
void zajel_destroy(zajel_s** zajelPointer_ptr COMMA()
                   FILE_AND_LINE_FOR_TYPE());

/***************************************************************************************************
 *  Name        : zajel_regsiter_message
 *
 *  Arguments   : zajel_s*                        zajel_ptr,
 *                uint32_t                        messageID,
 *                zajel_message_handler_function  messageHandler_ptr,
 *                char*                           messageName_Ptr COMMA()
 *                FILE_AND_LINE_FOR_TYPE()
 *
 *  Description : This function register a message to zajel framework. Only message handled by
 *                  component running on a specific core needs to be registered to zajel instance
 *                  running on the same core.
 *
 *  Returns     : void.
 **************************************************************************************************/
void zajel_regsiter_message(zajel_s*                        zajel_ptr,
                            uint32_t                        messageID,
                            zajel_message_handler_function  messageHandler_ptr,
                            char*                           messageName_Ptr COMMA()
                            FILE_AND_LINE_FOR_TYPE());

/***************************************************************************************************
 *  Name        : zajel_regsiter_component
 *
 *  Arguments   : zajel_s*  zajel_ptr,
 *                uint32_t  componentID,
 *                uint32_t  threadID,
 *                char*     componentName_Ptr COMMA()
 *                FILE_AND_LINE_FOR_TYPE()
 *
 *  Description : This function register a component to zajel framework. All system components needs
 *                  to be registered on each core.
 *
 *  Returns     : void.
 **************************************************************************************************/
void zajel_regsiter_component(zajel_s*  zajel_ptr,
                              uint32_t  componentID,
                              uint32_t  threadID,
                              char*     componentName_Ptr COMMA()
                              FILE_AND_LINE_FOR_TYPE());

/***************************************************************************************************
 *  Name        : zajel_regsiter_thread
 *
 *  Arguments   : zajel_s*                         zajel_ptr,
 *                uint32_t                         threadID,
 *                uint32_t                         coreID,
 *                zajel_handle_message_callback    handleMessageCallback,
 *                zajel_block_callback             blockCallback,
 *                zajel_unblock_callback           unblockCallback,
 *                void*                            synchronizationPrimitive_ptr,
 *                char*                            threadName_Ptr COMMA()
 *                FILE_AND_LINE_FOR_TYPE()
 *
 *  Description : This function register a thread to zajel framework.All system threads needs
 *                  to be registered on each core.
 *
 *  Returns     : void.
 **************************************************************************************************/
void zajel_regsiter_thread(zajel_s*                         zajel_ptr,
                           uint32_t                         threadID,
                           uint32_t                         coreID,
                           zajel_handle_message_callback    handleMessageCallback,
                           zajel_block_callback             blockCallback,
                           zajel_unblock_callback           unblockCallback,
                           void*                            synchronizationPrimitive_ptr,
                           char*                            threadName_Ptr COMMA()
                           FILE_AND_LINE_FOR_TYPE());

/***************************************************************************************************
 *  Name        : zajel_regsiter_core
 *
 *  Arguments   : zajel_s*                           zajel_ptr,
 *                uint32_t                           coreID,
 *                zajel_core_handle_message_callback handleMessageCallback,
 *                char*                              coreName_Ptr COMMA()
 *                FILE_AND_LINE_FOR_TYPE()
 *
 *  Description : This function register a core to zajel framework. All system cores needs
 *                  to be registered on each core.
 *
 *  Returns     : void.
 **************************************************************************************************/
void zajel_regsiter_core(zajel_s*                           zajel_ptr,
                         uint32_t                           coreID,
                         zajel_core_handle_message_callback handleMessageCallback,
                         char*                              coreName_Ptr COMMA()
                         FILE_AND_LINE_FOR_TYPE());

/*
 * TODO: mgalal on Mar 6, 2010
 *
 * Add function to perform remapping (component to thread and thread to core).
 */

/***************************************************************************************************
 *  Name        : zajel_send
 *
 *  Arguments   : zajel_s*  zajel_ptr,
 *                void*     message_ptr COMMA()
 *                FILE_AND_LINE_FOR_TYPE()
 *
 *  Description : This function sends the given message to the registered handler, and it takes care
 *                  of details like in which thread that hander runs, what is the logical sending
 *                  method (synch/asynch) and how to physically achieve it.
 *
 *  Returns     : void.
 **************************************************************************************************/
void zajel_send(zajel_s*    zajel_ptr,
                void*       message_ptr COMMA()
                FILE_AND_LINE_FOR_TYPE());

/***************************************************************************************************
 *  Name        : zajel_acknowledge
 *
 *  Arguments   : zajel_s*  zajel_ptr,
 *                void*     message_ptr COMMA()
 *                FILE_AND_LINE_FOR_TYPE()
 *
 *  Description : This function is used by the receiver thread to acknowledge the reception of a
 *                  synchronous message.
 *
 *  Returns     : void.
 **************************************************************************************************/
void zajel_acknowledge(zajel_s* zajel_ptr,
                       void*    message_ptr COMMA()
                       FILE_AND_LINE_FOR_TYPE());

/***************************************************************************************************
 *  Name        : zajel_deliver
 *
 *  Arguments   : zajel_s* zajel_ptr,
 *                void*    message_ptr,
 *                uint32_t callerThreadID COMMA()
 *                FILE_AND_LINE_FOR_TYPE()
 *
 *  Description : This function is used by the receiver core to deliver the received message
 *                  internally to one of the threads on the same core, or to unblock a thread on that
 *                  core, which was blocked after sending synchronous message.
 *
 *  Returns     : void.
 **************************************************************************************************/
void zajel_deliver(zajel_s* zajel_ptr,
                   void*    message_ptr,
                   uint32_t callerThreadID COMMA()
                   FILE_AND_LINE_FOR_TYPE());

#endif /* ZAJEL_H_ */
