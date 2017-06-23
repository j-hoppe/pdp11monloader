/* error.c: global error & info handling
 *
 *  Copyright (c) 2017, Joerg Hoppe
 *  j_hoppe@t-online.de, www.retrocmp.com
 *
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *  - Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 *  - Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 *  - Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *  HOLDERS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 *  TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 *  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 *  LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 *  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 *  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *
 *  29-Jan-2017  JH  created
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <assert.h>

#include "main.h"  // opt_*
#include "utils.h"
#include "error.h"  // own

FILE *ferr = NULL; // variable error stream

// global last raised error
int error_code;
// a stack of messages, for several caller infos
//int error_trace_level;
//char error_message[ERROR_MAX_TRACE_LEVEL + 1][1024];

void error_clear() {
	error_code = 0;
//	error_trace_level = 0;  //
}

// raise an error.
// can be used as "return error_set(...);"
//  text is pushed onto an message stack
int error_set(int code, char *fmt, ...) {
	char buffer[1024];
	va_list args;
//	assert(error_trace_level < ERROR_MAX_TRACE_LEVEL) ;
	error_code = code;
	if (code == ERROR_OK)
		error_clear();
	else if (fmt && strlen(fmt)) {
		va_start(args, fmt);
		vsprintf(buffer, fmt, args);
		va_end(args);
		// strcpy(error_message[error_trace_level++], buffer);
		error(buffer);
	}
	return error_code;
}

// print an info message and return
void info(char *fmt, ...) {
	va_list args;
	va_start(args, fmt);
	vfprintf(ferr, fmt, args);
	fprintf(ferr, "\n");
	va_end(args);
}

// print an warning message and return
void warning(char *fmt, ...) {
	va_list args;
	va_start(args, fmt);
	fprintf(ferr, "[Warning]  ");
	vfprintf(ferr, fmt, args);
	fprintf(ferr, "\n");
	va_end(args);
}

// print an error message and return
void error(char *fmt, ...) {
	va_list args;
	va_start(args, fmt);
	fprintf(ferr, "[ERROR]  ");
	vfprintf(ferr, fmt, args);
	fprintf(ferr, "\n");
	va_end(args);
}

// print an error message and die
void fatal(char *fmt, ...) {
	va_list args;
	va_start(args, fmt);
	fprintf(ferr, "[FATAL]  ");
	vfprintf(ferr, fmt, args);
	fprintf(ferr, "\n");
	va_end(args);
	exit(EXIT_FAILURE);
}

