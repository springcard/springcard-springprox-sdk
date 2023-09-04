/**** SpringProx.API/sprox_api.c
 *
 * NAME
 *   SpringProx.API :: Entry points
 *
 * DESCRIPTION
 *   Major entry points of the DLL
 *
 * HISTORY
 *
 *  JDA 23/06/2004 : created
 *  JDA 02/12/2005 : changed "#ifdef SPROX_MULTI_INSTANCE" to "#ifdef SPROX_API_REENTRANT"
 *
 **/

#include "sprox_api_i.h"

#ifndef SPROX_API_REENTRANT
static SPROX_CTX_ST  sprox_ctx_def;
SPROX_CTX_ST* sprox_ctx_glob = &sprox_ctx_def;
#endif

/**f* SpringProx.API/SPROX_GetLibrary
 *
 * NAME
 *   SPROX_GetLibrary
 *
 * DESCRIPTION
 *   Retrieve the SpringProxAPI version info
 *
 * INPUTS
 *   TCHAR *library     : buffer to receive the library info
 *   WORD  len          : character-size of the buffer
 *
 * RETURNS
 *   MI_OK              : success
 *
 * NOTES
 *   The function returns
 *   - "SPRINGCARD SPRINGPROX/CSB <level> <version>"
 *
 *   <level> shows library capabilities :
 *   - 1        basic firmware
 *
 *   <version> is the version field, formatted "M.mm" as follow :
 *   - M        is the major release number (1 to 2 digits)
 *   - m        is the minor release number (1 to 3 digits)
 *
 * VARIANTS
 *   SPROX_GetLibraryA : ASCII version
 *   SPROX_GetLibraryW : UNICODE version
 *
 **/
SPROX_API_FUNC(GetLibrary) (TCHAR* library, WORD len)
{
#ifdef UNICODE
	return SPROX_API_CALL(GetLibraryW) (library, len);
#else
	return SPROX_API_CALL(GetLibraryA) (library, len);
#endif
}

SPROX_API_FUNC(GetLibraryA) (char* library, WORD len)
{
	char buffer[64];

#ifndef SPROX_API_REENTRANT
	snprintf(buffer, sizeof(buffer), "SPRINGCARD SPRINGPROX API %s", sprox_library_revision);
#else
	snprintf(buffer, sizeof(buffer), "SPRINGCARD SPRINGPROX-EX API %s", sprox_library_revision);
#endif

	SPROX_Trace(TRACE_ALL, buffer);

	if (library == NULL) return MI_LIB_CALL_ERROR;
	if (len == 0) return MI_LIB_CALL_ERROR;

	strlcpy(library, buffer, len);
	library[len - 1] = '\0';
	return MI_OK;
}

SPROX_API_FUNC(GetLibraryW) (wchar_t* library, WORD len)
{
	char buffer[64];
	int i;
	SWORD rc;
	if (len > 64) len = 64;
	rc = SPROX_API_CALL(GetLibraryA) (buffer, len);
	if (rc != MI_OK) return rc;
	for (i = 0; i < len; i++) library[i] = buffer[i];
	return MI_OK;
}

#ifdef SPROX_API_REENTRANT

SPRINGPROX_LIB SPROX_INSTANCE SPRINGPROX_API SPROXx_CreateInstance(void)
{
	return calloc(1, sizeof(struct _SPROX_CTX_ST));
}

SPRINGPROX_LIB void SPRINGPROX_API SPROXx_DestroyInstance(SPROX_INSTANCE instance)
{
	SPROXx_ReaderClose(instance);
}

#else

#ifdef SPRINGPROX_EX_H

SPRINGPROX_LIB void SPRINGPROX_API SPROX_SelectInstance(SPROX_INSTANCE rInst)
{
	sprox_ctx_glob = (SPROX_CTX_ST*)rInst;
	if (sprox_ctx_glob == NULL)
		sprox_ctx_glob = &sprox_ctx_def;
}

#endif

#endif


