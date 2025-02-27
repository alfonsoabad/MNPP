/* Copyright (C) 2003 MySQL AB

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; version 2 of the License.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA */

#ifndef NDB_VERSION_H
#define NDB_VERSION_H

#include <ndb_types.h>

/* NDB build version */
#define NDB_VERSION_BUILD 59

/* NDB major version */
#define NDB_VERSION_MAJOR 5

/* NDB minor version */
#define NDB_VERSION_MINOR 1

/* NDB status version */
#define NDB_VERSION_STATUS ""


#define NDB_MAKE_VERSION(A,B,C) (((A) << 16) | ((B) << 8)  | ((C) << 0))

#define NDB_VERSION_D NDB_MAKE_VERSION(NDB_VERSION_MAJOR, NDB_VERSION_MINOR, NDB_VERSION_BUILD)
#define NDB_VERSION_STRING_BUF_SZ 100
#ifdef __cplusplus
extern "C" {
#endif

void ndbPrintVersion();

Uint32 ndbMakeVersion(Uint32 major, Uint32 minor, Uint32 build);

Uint32 ndbGetMajor(Uint32 version);
  
Uint32 ndbGetMinor(Uint32 version);
  
Uint32 ndbGetBuild(Uint32 version);

const char* ndbGetVersionString(Uint32 version, const char * status,
                                char *buf, unsigned sz);
const char* ndbGetOwnVersionString();

Uint32 ndbGetOwnVersion();

#ifdef __cplusplus
}
#endif

#define NDB_VERSION_STRING ndbGetOwnVersionString()

#define NDB_VERSION ndbGetOwnVersion()

/**
 * Version id 
 *
 *  Used by transporter and when communicating with
 *     managment server
 */
/*#define NDB_VERSION_ID 0*/

/**
 * From which version do we support rowid
 */ 
#define NDBD_ROWID_VERSION (NDB_MAKE_VERSION(5,1,6))
#define NDBD_INCL_NODECONF_VERSION_4 NDB_MAKE_VERSION(4,1,17)
#define NDBD_INCL_NODECONF_VERSION_5 NDB_MAKE_VERSION(5,0,18)
#define NDBD_FRAGID_VERSION (NDB_MAKE_VERSION(5,1,6))
#define NDBD_DICT_LOCK_VERSION_5 NDB_MAKE_VERSION(5,0,23)
#define NDBD_DICT_LOCK_VERSION_5_1 NDB_MAKE_VERSION(5,1,12)

#define NDBD_UPDATE_FRAG_DIST_KEY_50 NDB_MAKE_VERSION(5,0,26)
#define NDBD_UPDATE_FRAG_DIST_KEY_51 NDB_MAKE_VERSION(5,1,12)

#define NDBD_QMGR_SINGLEUSER_VERSION_5 NDB_MAKE_VERSION(5,0,25)

#define NDBD_NODE_VERSION_REP NDB_MAKE_VERSION(6,1,1)

#define NDBD_PREPARE_COPY_FRAG_VERSION NDB_MAKE_VERSION(6,2,1)
#define NDBD_PREPARE_COPY_FRAG_V2_51 NDB_MAKE_VERSION(5,1,23)
#define NDBD_PREPARE_COPY_FRAG_V2_62 NDB_MAKE_VERSION(6,2,8)
#define NDBD_PREPARE_COPY_FRAG_V2_63 NDB_MAKE_VERSION(6,3,6)

/**
 * 0 = NO PREP COPY FRAG SUPPORT
 * 1 = NO MAX PAGE SUPPORT
 * 2 = LATEST VERSION
 */
static
inline
int
ndb_check_prep_copy_frag_version(Uint32 version)
{	
  const Uint32 major = (version >> 16) & 0xFF;
  const Uint32 minor = (version >>  8) & 0xFF;

  if (version == NDB_VERSION_D)
    return 2;

  if (major >= 6)
  {
    if (minor == 2)
    {
      if (version >= NDBD_PREPARE_COPY_FRAG_V2_62)
        return 2;
      if (version >= NDBD_PREPARE_COPY_FRAG_VERSION)
        return 1;
      return 0;
    }
    else if (minor == 3)
    {
      if (version >= NDBD_PREPARE_COPY_FRAG_V2_63)
        return 2;
      return 1;
    }
    return 2;
  }
  else if (major == 5 && minor == 1)
  {
    if (version >= NDBD_PREPARE_COPY_FRAG_V2_51)
      return 2;
  }
    
  return 0;
}

#endif
 
