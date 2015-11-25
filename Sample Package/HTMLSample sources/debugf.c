/*    File: debugf.c        Description:        debugf is like printf.  Routines implemented herein provide a subset	of the standard C library's sprintf functionality.  The differences	with this implementation include:		it's small (around 2k once compiled)		it doesn't call any toolbox or os routines.  it's entirely self contained	and does not call any toolbox routines or stdclib routines.  The only call	made outside of this library is the DebugStr call inside of the debugf	routine.	it's safe to call at interrupt time (assuming there's the stack	space to do it).		debugf can be called anywhere it is safe to call DebugStr.    Copyright:        � Copyright 2000 Apple Computer, Inc. All rights reserved.        Disclaimer:        IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.        ("Apple") in consideration of your agreement to the following terms, and your        use, installation, modification or redistribution of this Apple software        constitutes acceptance of these terms.  If you do not agree with these terms,        please do not use, install, modify or redistribute this Apple software.        In consideration of your agreement to abide by the following terms, and subject        to these terms, Apple grants you a personal, non-exclusive license, under Apple�s        copyrights in this original Apple software (the "Apple Software"), to use,        reproduce, modify and redistribute the Apple Software, with or without        modifications, in source and/or binary forms; provided that if you redistribute        the Apple Software in its entirety and without modifications, you must retain        this notice and the following text and disclaimers in all such redistributions of        the Apple Software.  Neither the name, trademarks, service marks or logos of        Apple Computer, Inc. may be used to endorse or promote products derived from the        Apple Software without specific prior written permission from Apple.  Except as        expressly stated in this notice, no other rights or licenses, express or implied,        are granted by Apple herein, including but not limited to any patent rights that        may be infringed by your derivative works or by other works in which the Apple        Software may be incorporated.        The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO        WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED        WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR        PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE OR IN        COMBINATION WITH YOUR PRODUCTS.        IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR        CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE        GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)        ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION        OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER UNDER THEORY OF CONTRACT, TORT        (INCLUDING NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN        ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.    Change History (most recent first):        Wed, Feb 16, 2000 -- created*/#include "debugf.h"#include <Types.h>#include <StdArg.h>/* gotchas:	doesn't check for string buffer overflow.  There's an extra	768 bytes in string buffer to help avoid overflows,	but, 'use your head'*/int vsoutputf(char *s, const char *fmt, va_list args) {	char const *pull;	char *push, decb[32], *td;	unsigned char *rp;	const char *syms = "0123456789ABCDEF";	unsigned long value, i;	long base, digits;	Boolean twoscomp, hasDigits;	push = s;	pull = fmt;	while (*pull) {		if (*pull != '%') {			*push++ = *pull++;		} else {			pull++; /* skip the % */			if (*pull == '.') {				pull++; /* skip the . */				digits = 0;				while (*pull >= '0' && *pull <= '9')					digits = (digits * 10) + ((*pull++) - '0');				hasDigits = true;			} else {				digits = 0;				hasDigits = false;			}			base = 0;			switch(*pull) {				case 's':					pull++;  /* skip the s */					td = va_arg(args, char*);					while (*td) {						*push++ = *td++;						digits--;						if (hasDigits && digits <= 0) break;					}					break;				case 'p':					pull++;  /* skip the p */					rp = va_arg(args, unsigned char*);					for (i=0; i<*rp; i++) {						*push++ = (char) (rp[i+1]);						digits--;						if (hasDigits && digits <= 0) break;					}					break;				case 'c':  /* skip the c */					pull++;					*push++ =  (char) (va_arg(args, long));					digits--;					break;				case 'b': pull++; base = 2; twoscomp = false; break;				case 'n': pull++; base = 4; twoscomp = false; break;				case 'o': pull++; base = 8; twoscomp = false; break;				case 'd': pull++; base = 10; twoscomp = true; break;				case 'u': pull++; base = 10; twoscomp = false; break;				case 'x': pull++; base = 16; twoscomp = false; break;				default: digits = 0; base = 0; break;			}			if (base == 0) {				while (digits > 0) { *push++ = ' '; digits--; }			} else {				value = va_arg(args, unsigned long);				if (twoscomp && (value&0x80000000) != 0) {					*push++ = '-'; value = ((~value)+1);				}				td = decb;				do {	*td++ = syms[value%base];					digits--;					value /= base;				} while (value != 0);				while (digits > 0) { *push++ = '0'; digits--; }				while (td != decb) *push++ = *--td;			}		}	}	return push - s;}StringPtr PLoutputf(StringPtr p, const char *fmt, ...) {	va_list args;	va_start(args, fmt);	p[0] = vsoutputf((char*) (p+1), fmt, args);	va_end(args);	return p;}int soutputf(char *s, const char *fmt, ...) {	int result;	va_list args;	va_start(args, fmt);	result = vsoutputf(s, fmt, args);	s[result] = '\0';	va_end(args);	return result;}void debugf(char const *fmt, ...) {	char s[512];	va_list args;	va_start(args, fmt);	s[0] = vsoutputf(s+1, fmt, args);	DebugStr((StringPtr) s);	va_end(args);}