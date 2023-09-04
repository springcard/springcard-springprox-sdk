#include "sprox_api_i.h"

#include <stdarg.h>
#include <time.h>

static BYTE  trace_level = 0;
static TCHAR trace_file[MAX_PATH] = _T("");
static TCHAR last_trace_head = '\0';

const char* _TRACE(const TCHAR* s)
{
	static char st[MAX_PATH];
	int i;

	for (i = 0; s[i] != '\0'; i++)
		st[i] = (char)s[i];
	st[i] = '\0';

	return st;
}

SPRINGPROX_LIB void  SPRINGPROX_API SPROX_SetVerbose(BYTE level, const TCHAR* filename)
{
	SPROX_TraceSetLevel(level);
	SPROX_TraceSetFile(filename);
}

BYTE SPROX_TraceGetLevel(void)
{
	return trace_level;
}

void SPROX_TraceSetLevel(BYTE level)
{
	trace_level = level;
	SPROX_Trace(trace_level, "Trace level set to %02X", level);
}

void SPROX_TraceSetFile(const TCHAR* filename)
{
	if (filename != NULL)
	{
		SPROX_Trace(trace_level, "Trace file set to %s", filename);
		_tcscpy_s(trace_file, sizeof(trace_file) / sizeof(TCHAR), filename);
		SPROX_Trace(trace_level, "Trace file set to %s", trace_file);
	}
	else
		trace_file[0] = '\0';
}

void SPROX_Trace(BYTE level, const char* fmt, ...)
{
#ifdef _UNICODE
	TCHAR   line_u[MAX_PATH];
	size_t  i;
#endif
	va_list arg_ptr;
	char    line_a[MAX_PATH];
	FILE* file_out;
	TCHAR* line;
#ifndef UNDER_CE
	clock_t c;
#endif

	if (!(level & trace_level)) return; /* No need to trace this */
	if (trace_file == NULL)
	{
		/* Output to console */
		file_out = stdout;
	}
	else
	{
		if (!strlen(trace_file)) return FALSE;
		if (!_tcsncmp(trace_file, _T("DLG"), 3))
		{
			/* Output to message box */
			file_out = NULL;
		}
		else if (!_tcsncmp(trace_file, _T("CON"), 3) || !_tcsncmp(trace_file, _T("stdout"), 6))
		{
			/* Output to console */
			file_out = stdout;
		}
		else if (!_tcsncmp(trace_file, _T("ERR"), 3) || !_tcsncmp(trace_file, _T("stderr"), 6))
		{
			/* Output to console */
			file_out = stderr;
		}
		else
		{
			/* Output to file */
			errno_t err = _tfopen_s(&file_out, trace_file, _T("at+"));
			if (err)
				return;
		}
	}

	/* Prepare the line */
	if (fmt != NULL)
	{
		va_start(arg_ptr, fmt);
		vsnprintf(line_a, sizeof(line_a), fmt, arg_ptr);
		va_end(arg_ptr);
	}

#ifdef _UNICODE
	/* Convert the line to unicode if needed */
	for (i = 0; i <= strlen(line_a); i++)
		line_u[i] = line_a[i];
	line = line_u;
#else
	line = line_a;
#endif

	if (file_out == NULL)
	{
#ifdef WIN32
		/* Message Box */
		if (fmt != NULL)
		{
			MessageBox(0, line, _T("SpringProx API"), MB_APPLMODAL + MB_TOPMOST);
		}
#endif
		return;
	}

	if (fmt == NULL)
	{
		_ftprintf(file_out, _T("\n"));
		last_trace_head = '\0';
	}
	else
	{
		switch (line[0])
		{
		case '+':
		case '-':
			if (last_trace_head != line[0])
			{
				_ftprintf(file_out, _T("\n"));
				last_trace_head = line[0];
			}
			break;

		case '>':
		case ':':
		case '.':
			break;

		case '_':
			line++;
			break;

		default:
#ifndef UNDER_CE
			c = clock();
			c *= 1000;
			c /= CLOCKS_PER_SEC;
			_ftprintf(file_out, _T("\n%03ld.%03ld\t"), c / 1000, c % 1000);
#else
			_ftprintf(file_out, _T("\n"));
#endif
			last_trace_head = '\0';
			break;
		}
		_ftprintf(file_out, _T("%s"), line);
	}

	if ((file_out != stdout) && (file_out != stderr))
		fclose(file_out);
	else
		fflush(file_out);
}

