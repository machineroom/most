/*
 *  Copyright(c) Siemens AG, Muenchen, Germany, 2005, 2006, 2007
 *                           Bernhard Walle <bernhard.walle@gmx.de>
 *                           Gernot Hillier <gernot.hillier@siemens.com>
 *                           All rights reserved.
 *
 * ----------------------------------------------------------------------------
 * The contents of this file are subject to the Mozilla Public License
 * Version 1.1 (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 * 
 * Software distributed under the License is distributed on an "AS IS"
 * basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
 * License for the specific language governing rights and limitations
 * under the License.
 *
 * The Original Code is Siemens code.
 * 
 * The Initial Developer of the Original Code is Siemens AG.
 * Portions created by the Initial Developer are Copyright (C) 2005-06
 * the Initial Developer. All Rights Reserved.
 * ----------------------------------------------------------------------------
 */
#ifndef DEBUG_H
#define DEBUG_H

#ifdef HAVE_CONFIG_H
#include <config/config.h>
#endif

#include <stdio.h>

#if !defined(DOXYGEN) && (defined(DEBUG) || defined(TRACE))

#if defined(__NetBSD__) || (defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L)
#define __DBG_FUNC__    __func__
#elif defined(__GNUC__) && __GNUC__ >= 3
#define __DBG_FUNC__	__FUNCTION__
#else
#define __DBG_FUNC__    "??"
#endif

#endif // !defined(DOXYGEN) && (defined(DEBUG) || defined(TRACE))


#ifdef DEBUG
#    define PRINT_DBG(fmt, ...) {                                               \
         fprintf(stderr, " DEBUG[%s:%d] %s(): "fmt"\n", __FILE__, __LINE__,     \
                __DBG_FUNC__, ## __VA_ARGS__);                                  \
     }
#    define PERR_DEBUG(msg)                                                     \
         do {                                                                   \
              char buffer[1024];                                                \
              snprintf(buffer, 1024, " PERROR [%s:%d] %s %s",                   \
                       __FILE__, __LINE__, __DBG_FUNC__, msg);                  \
              perror(buffer);                                                   \
         } while (0);
    
#else  
#    define PRINT_DBG(fmt, ...)   { do {} while(0); }
#    define PERR_DEBUG(msg)       { do {} while(0); }
#endif


#ifdef TRACE
#    define PRINT_TRACE(fmt, ...) {                                              \
         fprintf(stderr, "  TRACE[%s:%d] %s(): "fmt"\n", __FILE__, __LINE__,     \
                __DBG_FUNC__, ## __VA_ARGS__);                                   \
     }
#else  
#    define PRINT_TRACE(fmt, ...)   { do {} while(0); }
#endif

#endif /* DEBUG_H */

/* vim: set ts=4 et sw=4: */
