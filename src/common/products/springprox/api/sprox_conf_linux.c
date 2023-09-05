/*
  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
  ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
  TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
  PARTICULAR PURPOSE.

  SpringProx API
  --------------

  Copyright (c) 2000-2008 SpringCard SAS, FRANCE - www.springcard.com

  config_linux.c
  --------------
  Where to store configuration settings under Win32

  revision :
  ----------

	JDA 10/01/2007 : created

*/
#include "sprox_api_i.h"

#ifdef __linux__

#include <ctype.h>

#define CONFIG_FILE  "/etc/springprox.conf"

static char device_name[48 + 1];

void LoadSettings(void)
{
	FILE* fp;
	char line[256];
	char* name, * value;
	int  trace_level = 0;
	char* trace_file = NULL;

	fp = fopen(CONFIG_FILE, "rt");
	if (fp == NULL) return;
	while (fgets(line, sizeof(line), fp) != NULL)
	{
		name = line;
		while (isspace(*name) && *name) name++;
		if (!isalpha(*name)) continue;
		name = strtok(name, "=");
		if (name == NULL) continue;
		value = strtok(NULL, "\r\n");
		if (value == NULL) continue;
		while (isspace(*value) && *value) value++;
		if (!*value) continue;

		if (!strcmp(name, "DeviceName"))
		{
			strncpy(device_name, value, sizeof(device_name));
			device_name[sizeof(device_name) - 1] = '\0';
		}
		else
			if (!strcmp(name, "DebugLevel"))
			{
				trace_level = atoi(value);
			}
			else
				if (!strcmp(name, "DebugFile"))
				{
					trace_file = strdup(value);
				}
	}
	fclose(fp);

	if (trace_file != NULL)
	{
		SPROX_TraceSetFile(trace_file);
		free(trace_file);
	}
	SPROX_TraceSetLevel(trace_level);
}

BOOL GetDefaultDevice(SPROX_CTX_ST* sprox_ctx)
{
	LoadSettings();
	if (strlen(device_name))
	{
		strcpy(sprox_ctx->com_name, device_name);
		return TRUE;
	}
	return FALSE;
}

BOOL LoadDefaultDevice(void)
{
	return FALSE;
}


#endif
