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


#ifndef FALSE
#define FALSE                           0
#endif

#ifndef TRUE
#define TRUE                            1
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

/*Memory allocation function prototype*/
typedef void*(*allocation_function)(size_t bytesCount);

/*Memory de-allocation function prototype*/
typedef void (*zajel_deallocation_function)(void* ptr);

/*Locking function prototype*/
typedef void (*lock_function)();

/*Unlocking function prototype*/
typedef void (*unlock_function)();

/*Defines the prototypes of message handler functions*/
typedef uint32_t (*zajel_message_handler_function) (void* message_ptr);

/*
 * FIXME: mgalal on Mar 5, 2010
 *
 * The four callbacks below needs to have interface properly adjusted.
 */
/*Defines the prototype of callback function used to deliver synchronous messages to a thread*/
typedef void (*zajel_thread_blocking_callback) (void);

/*Defines the prototype of callback function used to deliver asynchronous messages to a thread*/
typedef void (*zajel_thread_non_blocking_callback) (void);

/*Defines the prototype of callback function used to deliver synchronous messages to a core*/
typedef void (*zajel_core_blocking_callback) (void);

/*Defines the prototype of callback function used to deliver asynchronous messages to a core*/
typedef void (*zajel_core_non_blocking_callback) (void);

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
 * Enumeration Name:
 * zajel_delivery_type_e
 *
 * Enumeration Description:
 * Determines if the message delivery is synchronous or asynchronous.
 **************************************************************************************************/
typedef enum zajel_delivery_type
{
    /*
     * Sender will block after sending the message, until receiver finishes handling the message.
     */
    ZAJEL_DELIVERY_TYPE_SYNCHRONOUS   = 1,
     /*
      * Sender will not block after sending the message.
      */
    ZAJEL_DELIVERY_TYPE_ASYNCHRONOUS  = 2
}zajel_delivery_type_e;

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
 *  Description : This function register a message to zajel framework.
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
 *  Description : This function register a component to zajel framework.
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
 *  Arguments   : zajel_s*                             zajel_ptr,
 *                uint32_t                             threadID,
 *                uint32_t                             coreID,
 *                zajel_thread_blocking_callback       blockingCallback,
 *                zajel_thread_non_blocking_callback   nonblockingCallback,
 *                char*                                threadName_Ptr COMMA()
 *                FILE_AND_LINE_FOR_TYPE()
 *
 *  Description : This function register a thread to zajel framework.
 *
 *  Returns     : void.
 **************************************************************************************************/
void zajel_regsiter_thread(zajel_s*                             zajel_ptr,
                           uint32_t                             threadID,
                           uint32_t                             coreID,
                           zajel_thread_blocking_callback       blockingCallback,
                           zajel_thread_non_blocking_callback   nonblockingCallback,
                           char*                                threadName_Ptr COMMA()
                           FILE_AND_LINE_FOR_TYPE());

/***************************************************************************************************
 *  Name        : zajel_regsiter_core
 *
 *  Arguments   : zajel_s*                           zajel_ptr,
 *                uint32_t                           coreID,
 *                char*                              coreName_Ptr COMMA()
 *                FILE_AND_LINE_FOR_TYPE()
 *
 *  Description : This function register a core to zajel framework.
 *
 *  Returns     : void.
 **************************************************************************************************/
void zajel_regsiter_core(zajel_s*                           zajel_ptr,
                         uint32_t                           coreID,
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
 *  Arguments   : zajel_s*                zajel_ptr,
 *                uint32_t                sourceComponentID,
 *                uint32_t                destinationComponentID,
 *                uint32_t                messageID,
 *                zajel_delivery_type_e   deliveryType,
 *                void*                   message_ptr COMMA()
 *                FILE_AND_LINE_FOR_TYPE()
 *
 *  Description : This function sends the given message to the registered handler, and it takes care
 *                  of details like in which thread that hander runs, what is the logical sending
 *                  method (synch/asynch) and how to physically achieve it.
 *
 *  Returns     : void.
 **************************************************************************************************/
void zajel_send(zajel_s*                zajel_ptr,
                uint32_t                sourceComponentID,
                uint32_t                destinationComponentID,
                uint32_t                messageID,
                zajel_delivery_type_e   deliveryType,
                void*                   message_ptr COMMA()
                FILE_AND_LINE_FOR_TYPE());


#endif /* ZAJEL_H_ */
