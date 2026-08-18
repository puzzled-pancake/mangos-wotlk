/*
 * This file is part of the CMaNGOS Project. See AUTHORS file for Copyright information
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef MANGOSSERVER_ERRORS_H
#define MANGOSSERVER_ERRORS_H

#include "Common.h"

// Normal assert.
#define WPError(CONDITION) \
if (!(CONDITION)) \
{ \
    assert(STRINGIZE(CONDITION) && 0); \
    fprintf(stderr, "Critical Error: A condition which must never be false was found to be false. \
Server was shut down to protect data integrity.\nIf this error is occurring frequently, please \
recompile the software in debug mode to get more details.\n\n%s(): %s\n", __FUNCTION__, STRINGIZE(CONDITION)); \
    std::abort(); \
}

/*
 * Pocket Realm embedding patch (POCKET_EMBEDDED).
 *
 * CMaNGOS terminates the process on unrecoverable startup errors (missing DBC/
 * map files, broken world tables, schema mismatches) via raw exit(1) calls,
 * and asserts via std::abort(). That is correct for a standalone server whose
 * process IS the failure domain, but it is forbidden across the embeddable
 * realm boundary: the host is an
 * Android app process, and a bad DB must return an error code, not kill the
 * launcher.
 *
 * When POCKET_EMBEDDED is defined (only the libpocketrealm.so target defines
 * it; mangosd/realmd executables are unchanged), POCKET_FATAL(msg) throws a
 * pocket_realm::fatal_error carrying msg, which the C-ABI facade catches at
 * the boundary and converts to REALM_E_FATAL_STARTUP / REALM_E_BLOCKED_ON_
 * CLIENT_DATA. The raw exit(1) startup sites are rewritten to POCKET_FATAL so
 * the host process survives. See docs/patches/native-source-patches.md.
 *
 * Note: this header deliberately does NOT define pocket_realm::fatal_error
 * (that lives in the runtime, to keep upstream dependency-free). It forward-
 * declares only the throw helper the macro calls.
 */
#ifdef POCKET_EMBEDDED
namespace pocket_realm { namespace embed {
    [[noreturn]] void throw_fatal(const char* msg);
} }
// Build a message at the call site and hand it to the helper. The helper, not
// this macro, owns the throw so upstream translation units need no exception
// headers.
#define POCKET_FATAL(msg) pocket_realm::embed::throw_fatal(msg)
#else
#define POCKET_FATAL(msg) ::exit(1)
#endif

// Just warn.
#define WPWarning(CONDITION) \
if (!(CONDITION)) \
{ \
    printf("%s:%i: Warning: Assertion in %s failed: %s",\
        __FILE__, __LINE__, __FUNCTION__, STRINGIZE(CONDITION)); \
}

#ifdef MANGOS_DEBUG
#  define MANGOS_ASSERT WPError
#else
#  define MANGOS_ASSERT WPError                             // Error even if in release mode.
#endif

#endif
