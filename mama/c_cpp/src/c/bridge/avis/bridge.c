/* $Id: bridge.c,v 1.1.2.7 2011/10/02 19:02:18 ianbell Exp $
 *
 * OpenMAMA: The open middleware agnostic messaging API
 * Copyright (C) 2011 NYSE Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA
 */

#include <avis/elvin.h>

#ifdef WIN32
#include <wombat/wincompat.h>
#endif /* WIN32 */


#include <mama/mama.h>
#include <timers.h>
#include <queueimpl.h>
#include "avisbridgefunctions.h"
#include "avisdefs.h"
#include "transportbridge.h"

timerHeap gTimerHeap;

/*Responsible for creating the bridge impl structure*/
void avisBridge_createImpl (mamaBridge* result)
{
    if (!result) return;
    *result = NULL;

    mamaBridgeImpl* impl = (mamaBridgeImpl*)calloc (1, sizeof (mamaBridgeImpl));
    if (!impl)
    {
        mama_log (MAMA_LOG_LEVEL_SEVERE, "avisBridge_createImpl(): "
                "Could not allocate mem for impl.");
        return;
    }

    avisBridgeImpl* avisBridge = (avisBridgeImpl*) calloc(1, sizeof(avisBridgeImpl));

    /*Populate the bridge impl structure with the function pointers*/
    INITIALIZE_BRIDGE (impl, avis);

    mamaBridgeImpl_setClosure((mamaBridge) impl, avisBridge);

    *result = (mamaBridge)impl;
}

const char*
avisBridge_getVersion (void)
{
        return (const char*) VERSION;
}

const char*
avisBridge_getName (void)
{
    return "avis";
}

#define DEFAULT_PAYLOAD_NAME    "avismsg"
#define DEFAULT_PAYLOAD_ID      MAMA_PAYLOAD_AVIS

mama_status
avisBridge_getDefaultPayloadId (char**name, char* id)
{
    *name=DEFAULT_PAYLOAD_NAME;
    *id=DEFAULT_PAYLOAD_ID;

     return MAMA_STATUS_OK;
}


mama_status
avisBridge_open (mamaBridge bridgeImpl)
{
    mama_status status = MAMA_STATUS_OK;
    mamaBridgeImpl* impl =  (mamaBridgeImpl*)bridgeImpl;

    mama_log (MAMA_LOG_LEVEL_FINEST, "avisBridge_open(): Entering.");

    if (MAMA_STATUS_OK !=
       (status =  mamaQueue_create (&impl->mDefaultEventQueue, bridgeImpl)))
    {
        mama_log (MAMA_LOG_LEVEL_ERROR, "avisBridge_open():"
            "Failed to create Avis queue.");
        return status;
    }

    mamaQueue_setQueueName (impl->mDefaultEventQueue,
                            "AVIS_DEFAULT_MAMA_QUEUE");

    mama_log (MAMA_LOG_LEVEL_NORMAL,
              "avisBridge_open(): Successfully created Avis queue");

    if (0 != createTimerHeap (&gTimerHeap))
    {
        mama_log (MAMA_LOG_LEVEL_NORMAL,
                "avisBridge_open(): Failed to initialize timers.");
        return MAMA_STATUS_PLATFORM;
    }

    if (0 != startDispatchTimerHeap (gTimerHeap))
    {
        mama_log (MAMA_LOG_LEVEL_NORMAL,
                "avisBridge_open(): Failed to start timer thread.");
        return MAMA_STATUS_PLATFORM;
    }

    return MAMA_STATUS_OK;
}

mama_status
avisBridge_close (mamaBridge bridgeImpl)
{
   mama_log (MAMA_LOG_LEVEL_FINEST, "avisBridge_close(): Entering.");

   mama_status status   = MAMA_STATUS_OK;
   mamaBridgeImpl* impl =  (mamaBridgeImpl*)bridgeImpl;


   if (0 != destroyHeap (gTimerHeap))
   {
        mama_log (MAMA_LOG_LEVEL_ERROR, "avisBridge_close():"
                "Failed to destroy Avis timer heap.");
        status = MAMA_STATUS_PLATFORM;
   }

   mamaQueue_destroyWait(impl->mDefaultEventQueue);

   free (impl);
   return status;
}


mama_status
avisBridge_start(mamaQueue defaultEventQueue)
{
    mama_log (MAMA_LOG_LEVEL_FINER, "avisBridge_start(): Start dispatching on default event queue.");

    mama_status status = MAMA_STATUS_OK;

    // start Avis event loop(s)
    avisBridgeImpl* avisBridge;
    if (MAMA_STATUS_OK != (status = mamaBridgeImpl_getClosure((mamaBridge) mamaQueueImpl_getBridgeImpl(defaultEventQueue), (void**) &avisBridge))) {
        mama_log (MAMA_LOG_LEVEL_ERROR, "avisBridge_start(): Could not get Elvin object");
        return status;
    }
    if (MAMA_STATUS_OK != (status = avisTransportBridge_start(avisBridge->mTransportBridge))) {
        mama_log (MAMA_LOG_LEVEL_ERROR, "avisBridge_start(): Could not start dispatching on Avis");
        return status;
    }

    // start Mama event loop
    return mamaQueue_dispatch(defaultEventQueue);
}

mama_status
avisBridge_stop(mamaQueue defaultEventQueue)
{
    mama_log (MAMA_LOG_LEVEL_FINER, "avisBridge_stop(): Stopping bridge.");

    mama_status status = MAMA_STATUS_OK;

    avisBridgeImpl* avisBridge;
    if (MAMA_STATUS_OK != (status = mamaBridgeImpl_getClosure((mamaBridge) mamaQueueImpl_getBridgeImpl(defaultEventQueue), (void**) &avisBridge))) {
        mama_log (MAMA_LOG_LEVEL_ERROR, "avisBridge_stop(): Could not get Elvin object");
        return status;
    }
    if (MAMA_STATUS_OK != (status = avisTransportBridge_stop(avisBridge->mTransportBridge))) {
        mama_log (MAMA_LOG_LEVEL_ERROR, "avisBridge_stop(): Could not stop dispatching on Avis %d", status);
        return status;
    }

    // stop Mama event loop
    status = mamaQueue_stopDispatch (defaultEventQueue);
    if (status != MAMA_STATUS_OK)
    {
        mama_log (MAMA_LOG_LEVEL_ERROR, "avisBridge_stop(): Failed to unblock  queue.");
        return status;
    }

    return MAMA_STATUS_OK;
}
