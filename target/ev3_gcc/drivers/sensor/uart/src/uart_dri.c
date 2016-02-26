//#define DISABLE_PRU_UARTS

#include "am1808.h"
#include <stdarg.h>
#include <ctype.h>
#include "errno.h"
#include "csl.h"
#include "suart_err.h"
#include "target_config.h"
#include "kernel_cfg.h"

#include "driver_debug.h"
//#include "ev3api.h"

//#define DEBUG
//#define HIGHDEBUG
//#define DEBUG_D_UART_ERROR
// TODO: what if str contains "%" ?
//#define UartWrite(str) syslog_printf(str, NULL, target_fput_log)
//#define UartWrite(str) syslog_printf(str, NULL, ev3rt_console_log_putc)
#define UartWrite(str)

/**
 * Portable snprintf() implementation from Linux kernel
 */

#define ZEROPAD	1		/* pad with zero */
#define SIGN	2		/* unsigned/signed long */
#define PLUS	4		/* show plus */
#define SPACE	8		/* space if plus */
#define LEFT	16		/* left justified */
#define SMALL	32		/* use lowercase in hex (must be 32 == 0x20) */
#define SPECIAL	64		/* prefix hex with "0x", octal with "0" */


enum format_type {
	FORMAT_TYPE_NONE, /* Just a string part */
	FORMAT_TYPE_WIDTH,
	FORMAT_TYPE_PRECISION,
	FORMAT_TYPE_CHAR,
	FORMAT_TYPE_STR,
	FORMAT_TYPE_PTR,
	FORMAT_TYPE_PERCENT_CHAR,
	FORMAT_TYPE_INVALID,
	FORMAT_TYPE_LONG_LONG,
	FORMAT_TYPE_ULONG,
	FORMAT_TYPE_LONG,
	FORMAT_TYPE_UBYTE,
	FORMAT_TYPE_BYTE,
	FORMAT_TYPE_USHORT,
	FORMAT_TYPE_SHORT,
	FORMAT_TYPE_UINT,
	FORMAT_TYPE_INT,
	FORMAT_TYPE_SIZE_T,
	FORMAT_TYPE_PTRDIFF
};

struct printf_spec {
	u8	type;		/* format_type enum */
	u8	flags;		/* flags to number() */
	u8	base;		/* number base, 8, 10 or 16 only */
	u8	qualifier;	/* number qualifier, one of 'hHlLtzZ' */
	int16_t	field_width;	/* width of output field */
	int16_t	precision;	/* # of digits/chars */
};

static noinline_for_stack
int skip_atoi(const char **s)
{
	int i = 0;

	while (isdigit((uint8_t)**s))
		i = i*10 + *((*s)++) - '0';

	return i;
}

/* Formats correctly any integer in [0, 999999999] */
static noinline_for_stack
char *put_dec_full9(char *buf, unsigned q)
{
	unsigned r;

	/*
	 * Possible ways to approx. divide by 10
	 * (x * 0x1999999a) >> 32 x < 1073741829 (multiply must be 64-bit)
	 * (x * 0xcccd) >> 19     x <      81920 (x < 262149 when 64-bit mul)
	 * (x * 0x6667) >> 18     x <      43699
	 * (x * 0x3334) >> 17     x <      16389
	 * (x * 0x199a) >> 16     x <      16389
	 * (x * 0x0ccd) >> 15     x <      16389
	 * (x * 0x0667) >> 14     x <       2739
	 * (x * 0x0334) >> 13     x <       1029
	 * (x * 0x019a) >> 12     x <       1029
	 * (x * 0x00cd) >> 11     x <       1029 shorter code than * 0x67 (on i386)
	 * (x * 0x0067) >> 10     x <        179
	 * (x * 0x0034) >>  9     x <         69 same
	 * (x * 0x001a) >>  8     x <         69 same
	 * (x * 0x000d) >>  7     x <         69 same, shortest code (on i386)
	 * (x * 0x0007) >>  6     x <         19
	 * See <http://www.cs.uiowa.edu/~jones/bcd/divide.html>
	 */
	r      = (q * (uint64_t)0x1999999a) >> 32;
	*buf++ = (q - 10 * r) + '0'; /* 1 */
	q      = (r * (uint64_t)0x1999999a) >> 32;
	*buf++ = (r - 10 * q) + '0'; /* 2 */
	r      = (q * (uint64_t)0x1999999a) >> 32;
	*buf++ = (q - 10 * r) + '0'; /* 3 */
	q      = (r * (uint64_t)0x1999999a) >> 32;
	*buf++ = (r - 10 * q) + '0'; /* 4 */
	r      = (q * (uint64_t)0x1999999a) >> 32;
	*buf++ = (q - 10 * r) + '0'; /* 5 */
	/* Now value is under 10000, can avoid 64-bit multiply */
	q      = (r * 0x199a) >> 16;
	*buf++ = (r - 10 * q)  + '0'; /* 6 */
	r      = (q * 0xcd) >> 11;
	*buf++ = (q - 10 * r)  + '0'; /* 7 */
	q      = (r * 0xcd) >> 11;
	*buf++ = (r - 10 * q) + '0'; /* 8 */
	*buf++ = q + '0'; /* 9 */
	return buf;
}

/* Similar to above but do not pad with zeros.
 * Code can be easily arranged to print 9 digits too, but our callers
 * always call put_dec_full9() instead when the number has 9 decimal digits.
 */
static noinline_for_stack
char *put_dec_trunc8(char *buf, unsigned r)
{
	unsigned q;

	/* Copy of previous function's body with added early returns */
	while (r >= 10000) {
		q = r + '0';
		r  = (r * (uint64_t)0x1999999a) >> 32;
		*buf++ = q - 10*r;
	}

	q      = (r * 0x199a) >> 16;	/* r <= 9999 */
	*buf++ = (r - 10 * q)  + '0';
	if (q == 0)
		return buf;
	r      = (q * 0xcd) >> 11;	/* q <= 999 */
	*buf++ = (q - 10 * r)  + '0';
	if (r == 0)
		return buf;
	q      = (r * 0xcd) >> 11;	/* r <= 99 */
	*buf++ = (r - 10 * q) + '0';
	if (q == 0)
		return buf;
	*buf++ = q + '0';		 /* q <= 9 */
	return buf;
}

static
char *put_dec(char *buf, unsigned long long n)
{
	if (n >= 100*1000*1000) {
		while (n >= 1000*1000*1000)
			buf = put_dec_full9(buf, n / (1000*1000*1000)/*do_div(n, 1000*1000*1000)*/);
		if (n >= 100*1000*1000)
			return put_dec_full9(buf, n);
	}
	return put_dec_trunc8(buf, n);
}

static noinline_for_stack
char *number(char *buf, char *end, unsigned long long num,
	     struct printf_spec spec)
{
	/* we are called with base 8, 10 or 16, only, thus don't need "G..."  */
	static const char digits[16] = "0123456789ABCDEF"; /* "GHIJKLMNOPQRSTUVWXYZ"; */

	char tmp[66];
	char sign;
	char locase;
	int need_pfx = ((spec.flags & SPECIAL) && spec.base != 10);
	int i;
	bool is_zero = num == 0LL;

	/* locase = 0 or 0x20. ORing digits or letters with 'locase'
	 * produces same digits or (maybe lowercased) letters */
	locase = (spec.flags & SMALL);
	if (spec.flags & LEFT)
		spec.flags &= ~ZEROPAD;
	sign = 0;
	if (spec.flags & SIGN) {
		if ((signed long long)num < 0) {
			sign = '-';
			num = -(signed long long)num;
			spec.field_width--;
		} else if (spec.flags & PLUS) {
			sign = '+';
			spec.field_width--;
		} else if (spec.flags & SPACE) {
			sign = ' ';
			spec.field_width--;
		}
	}
	if (need_pfx) {
		if (spec.base == 16)
			spec.field_width -= 2;
		else if (!is_zero)
			spec.field_width--;
	}

	/* generate full string in tmp[], in reverse order */
	i = 0;
	if (num < spec.base)
		tmp[i++] = digits[num] | locase;
	/* Generic code, for any base:
	else do {
		tmp[i++] = (digits[do_div(num,base)] | locase);
	} while (num != 0);
	*/
	else if (spec.base != 10) { /* 8 or 16 */
		int mask = spec.base - 1;
		int shift = 3;

		if (spec.base == 16)
			shift = 4;
		do {
			tmp[i++] = (digits[((unsigned char)num) & mask] | locase);
			num >>= shift;
		} while (num);
	} else { /* base 10 */
		i = put_dec(tmp, num) - tmp;
	}

	/* printing 100 using %2d gives "100", not "00" */
	if (i > spec.precision)
		spec.precision = i;
	/* leading space padding */
	spec.field_width -= spec.precision;
	if (!(spec.flags & (ZEROPAD+LEFT))) {
		while (--spec.field_width >= 0) {
			if (buf < end)
				*buf = ' ';
			++buf;
		}
	}
	/* sign */
	if (sign) {
		if (buf < end)
			*buf = sign;
		++buf;
	}
	/* "0x" / "0" prefix */
	if (need_pfx) {
		if (spec.base == 16 || !is_zero) {
			if (buf < end)
				*buf = '0';
			++buf;
		}
		if (spec.base == 16) {
			if (buf < end)
				*buf = ('X' | locase);
			++buf;
		}
	}
	/* zero or space padding */
	if (!(spec.flags & LEFT)) {
		char c = (spec.flags & ZEROPAD) ? '0' : ' ';
		while (--spec.field_width >= 0) {
			if (buf < end)
				*buf = c;
			++buf;
		}
	}
	/* hmm even more zero padding? */
	while (i <= --spec.precision) {
		if (buf < end)
			*buf = '0';
		++buf;
	}
	/* actual digits of result */
	while (--i >= 0) {
		if (buf < end)
			*buf = tmp[i];
		++buf;
	}
	/* trailing space padding */
	while (--spec.field_width >= 0) {
		if (buf < end)
			*buf = ' ';
		++buf;
	}

	return buf;
}


static noinline_for_stack
char *string(char *buf, char *end, const char *s, struct printf_spec spec)
{
	int len, i;

	if ((unsigned long)s < PAGE_SIZE)
		s = "(null)";

	len = strnlen(s, spec.precision);

	if (!(spec.flags & LEFT)) {
		while (len < spec.field_width--) {
			if (buf < end)
				*buf = ' ';
			++buf;
		}
	}
	for (i = 0; i < len; ++i) {
		if (buf < end)
			*buf = *s;
		++buf; ++s;
	}
	while (len < spec.field_width--) {
		if (buf < end)
			*buf = ' ';
		++buf;
	}

	return buf;
}

/*
 * Helper function to decode printf style format.
 * Each call decode a token from the format and return the
 * number of characters read (or likely the delta where it wants
 * to go on the next call).
 * The decoded token is returned through the parameters
 *
 * 'h', 'l', or 'L' for integer fields
 * 'z' support added 23/7/1999 S.H.
 * 'z' changed to 'Z' --davidm 1/25/99
 * 't' added for ptrdiff_t
 *
 * @fmt: the format string
 * @type of the token returned
 * @flags: various flags such as +, -, # tokens..
 * @field_width: overwritten width
 * @base: base of the number (octal, hex, ...)
 * @precision: precision of a number
 * @qualifier: qualifier of a number (long, size_t, ...)
 */
static noinline_for_stack
int format_decode(const char *fmt, struct printf_spec *spec)
{
	const char *start = fmt;

	/* we finished early by reading the field width */
	if (spec->type == FORMAT_TYPE_WIDTH) {
		if (spec->field_width < 0) {
			spec->field_width = -spec->field_width;
			spec->flags |= LEFT;
		}
		spec->type = FORMAT_TYPE_NONE;
		goto precision;
	}

	/* we finished early by reading the precision */
	if (spec->type == FORMAT_TYPE_PRECISION) {
		if (spec->precision < 0)
			spec->precision = 0;

		spec->type = FORMAT_TYPE_NONE;
		goto qualifier;
	}

	/* By default */
	spec->type = FORMAT_TYPE_NONE;

	for (; *fmt ; ++fmt) {
		if (*fmt == '%')
			break;
	}

	/* Return the current non-format string */
	if (fmt != start || !*fmt)
		return fmt - start;

	/* Process flags */
	spec->flags = 0;

	while (1) { /* this also skips first '%' */
		bool found = true;

		++fmt;

		switch (*fmt) {
		case '-': spec->flags |= LEFT;    break;
		case '+': spec->flags |= PLUS;    break;
		case ' ': spec->flags |= SPACE;   break;
		case '#': spec->flags |= SPECIAL; break;
		case '0': spec->flags |= ZEROPAD; break;
		default:  found = false;
		}

		if (!found)
			break;
	}

	/* get field width */
	spec->field_width = -1;

	if (isdigit((uint8_t)*fmt))
		spec->field_width = skip_atoi(&fmt);
	else if (*fmt == '*') {
		/* it's the next argument */
		spec->type = FORMAT_TYPE_WIDTH;
		return ++fmt - start;
	}

precision:
	/* get the precision */
	spec->precision = -1;
	if (*fmt == '.') {
		++fmt;
		if (isdigit((uint8_t)*fmt)) {
			spec->precision = skip_atoi(&fmt);
			if (spec->precision < 0)
				spec->precision = 0;
		} else if (*fmt == '*') {
			/* it's the next argument */
			spec->type = FORMAT_TYPE_PRECISION;
			return ++fmt - start;
		}
	}

qualifier:
	/* get the conversion qualifier */
	spec->qualifier = -1;
	if (*fmt == 'h' || _tolower(*fmt) == 'l' ||
	    _tolower(*fmt) == 'z' || *fmt == 't') {
		spec->qualifier = *fmt++;
		if (unlikely(spec->qualifier == *fmt)) {
			if (spec->qualifier == 'l') {
				spec->qualifier = 'L';
				++fmt;
			} else if (spec->qualifier == 'h') {
				spec->qualifier = 'H';
				++fmt;
			}
		}
	}

	/* default base */
	spec->base = 10;
	switch (*fmt) {
	case 'c':
		spec->type = FORMAT_TYPE_CHAR;
		return ++fmt - start;

	case 's':
		spec->type = FORMAT_TYPE_STR;
		return ++fmt - start;

	case 'p':
		spec->type = FORMAT_TYPE_PTR;
		return fmt - start;
		/* skip alnum */

	case '%':
		spec->type = FORMAT_TYPE_PERCENT_CHAR;
		return ++fmt - start;

	/* integer number formats - set up the flags and "break" */
	case 'o':
		spec->base = 8;
		break;

	case 'x':
		spec->flags |= SMALL;

	case 'X':
		spec->base = 16;
		break;

	case 'd':
	case 'i':
		spec->flags |= SIGN;
	case 'u':
		break;

	case 'n':
		/*
		 * Since %n poses a greater security risk than utility, treat
		 * it as an invalid format specifier. Warn about its use so
		 * that new instances don't get added.
		 */
//		WARN_ONCE(1, "Please remove ignored %%n in '%s'\n", fmt);
		assert(false);
		/* Fall-through */

	default:
		spec->type = FORMAT_TYPE_INVALID;
		return fmt - start;
	}

	if (spec->qualifier == 'L')
		spec->type = FORMAT_TYPE_LONG_LONG;
	else if (spec->qualifier == 'l') {
		if (spec->flags & SIGN)
			spec->type = FORMAT_TYPE_LONG;
		else
			spec->type = FORMAT_TYPE_ULONG;
	} else if (_tolower(spec->qualifier) == 'z') {
		spec->type = FORMAT_TYPE_SIZE_T;
	} else if (spec->qualifier == 't') {
		spec->type = FORMAT_TYPE_PTRDIFF;
	} else if (spec->qualifier == 'H') {
		if (spec->flags & SIGN)
			spec->type = FORMAT_TYPE_BYTE;
		else
			spec->type = FORMAT_TYPE_UBYTE;
	} else if (spec->qualifier == 'h') {
		if (spec->flags & SIGN)
			spec->type = FORMAT_TYPE_SHORT;
		else
			spec->type = FORMAT_TYPE_USHORT;
	} else {
		if (spec->flags & SIGN)
			spec->type = FORMAT_TYPE_INT;
		else
			spec->type = FORMAT_TYPE_UINT;
	}

	return ++fmt - start;
}

/**
 * vsnprintf - Format a string and place it in a buffer
 * @buf: The buffer to place the result into
 * @size: The size of the buffer, including the trailing null space
 * @fmt: The format string to use
 * @args: Arguments for the format string
 *
 * This function follows C99 vsnprintf, but has some extensions:
 * %pS output the name of a text symbol with offset
 * %ps output the name of a text symbol without offset
 * %pF output the name of a function pointer with its offset
 * %pf output the name of a function pointer without its offset
 * %pB output the name of a backtrace symbol with its offset
 * %pR output the address range in a struct resource with decoded flags
 * %pr output the address range in a struct resource with raw flags
 * %pM output a 6-byte MAC address with colons
 * %pMR output a 6-byte MAC address with colons in reversed order
 * %pMF output a 6-byte MAC address with dashes
 * %pm output a 6-byte MAC address without colons
 * %pmR output a 6-byte MAC address without colons in reversed order
 * %pI4 print an IPv4 address without leading zeros
 * %pi4 print an IPv4 address with leading zeros
 * %pI6 print an IPv6 address with colons
 * %pi6 print an IPv6 address without colons
 * %pI6c print an IPv6 address as specified by RFC 5952
 * %pIS depending on sa_family of 'struct sockaddr *' print IPv4/IPv6 address
 * %piS depending on sa_family of 'struct sockaddr *' print IPv4/IPv6 address
 * %pU[bBlL] print a UUID/GUID in big or little endian using lower or upper
 *   case.
 * %*pE[achnops] print an escaped buffer
 * %*ph[CDN] a variable-length hex string with a separator (supports up to 64
 *           bytes of the input)
 * %n is ignored
 *
 * ** Please update Documentation/printk-formats.txt when making changes **
 *
 * The return value is the number of characters which would
 * be generated for the given input, excluding the trailing
 * '\0', as per ISO C99. If you want to have the exact
 * number of characters written into @buf as return value
 * (not including the trailing '\0'), use vscnprintf(). If the
 * return is greater than or equal to @size, the resulting
 * string is truncated.
 *
 * If you're not already dealing with a va_list consider using snprintf().
 */
#define vsnprintf vsnprintf_linux
static
int vsnprintf_linux(char *buf, size_t size, const char *fmt, va_list args)
{
	unsigned long long num;
	char *str, *end;
	struct printf_spec spec = {0};

	/* Reject out-of-range values early.  Large positive sizes are
	   used for unknown buffer sizes. */
	if (/*WARN_ON_ONCE*/((int) size < 0)) {
		assert(false);
		return 0;
	}


	str = buf;
	end = buf + size;

	/* Make sure end is always >= buf */
	if (end < buf) {
		end = ((void *)-1);
		size = end - buf;
	}

	while (*fmt) {
		const char *old_fmt = fmt;
		int read = format_decode(fmt, &spec);

		fmt += read;

		switch (spec.type) {
		case FORMAT_TYPE_NONE: {
			int copy = read;
			if (str < end) {
				if (copy > end - str)
					copy = end - str;
				memcpy(str, old_fmt, copy);
			}
			str += read;
			break;
		}

		case FORMAT_TYPE_WIDTH:
			spec.field_width = va_arg(args, int);
			break;

		case FORMAT_TYPE_PRECISION:
			spec.precision = va_arg(args, int);
			break;

		case FORMAT_TYPE_CHAR: {
			char c;

			if (!(spec.flags & LEFT)) {
				while (--spec.field_width > 0) {
					if (str < end)
						*str = ' ';
					++str;

				}
			}
			c = (unsigned char) va_arg(args, int);
			if (str < end)
				*str = c;
			++str;
			while (--spec.field_width > 0) {
				if (str < end)
					*str = ' ';
				++str;
			}
			break;
		}

		case FORMAT_TYPE_STR:
			str = string(str, end, va_arg(args, char *), spec);
			break;

		case FORMAT_TYPE_PTR:
#if 0
			str = pointer(fmt+1, str, end, va_arg(args, void *),
				      spec);
			while (isalnum(*fmt))
				fmt++;
#endif
			assert(false);
			break;

		case FORMAT_TYPE_PERCENT_CHAR:
			if (str < end)
				*str = '%';
			++str;
			break;

		case FORMAT_TYPE_INVALID:
			if (str < end)
				*str = '%';
			++str;
			break;

		default:
			switch (spec.type) {
			case FORMAT_TYPE_LONG_LONG:
				num = va_arg(args, long long);
				break;
			case FORMAT_TYPE_ULONG:
				num = va_arg(args, unsigned long);
				break;
			case FORMAT_TYPE_LONG:
				num = va_arg(args, long);
				break;
			case FORMAT_TYPE_SIZE_T:
				if (spec.flags & SIGN)
					num = va_arg(args, ssize_t);
				else
					num = va_arg(args, size_t);
				break;
			case FORMAT_TYPE_PTRDIFF:
				num = va_arg(args, ptrdiff_t);
				break;
			case FORMAT_TYPE_UBYTE:
				num = (unsigned char) va_arg(args, int);
				break;
			case FORMAT_TYPE_BYTE:
				num = (signed char) va_arg(args, int);
				break;
			case FORMAT_TYPE_USHORT:
				num = (unsigned short) va_arg(args, int);
				break;
			case FORMAT_TYPE_SHORT:
				num = (short) va_arg(args, int);
				break;
			case FORMAT_TYPE_INT:
				num = (int) va_arg(args, int);
				break;
			default:
				num = va_arg(args, unsigned int);
			}

			str = number(str, end, num, spec);
		}
	}

	if (size > 0) {
		if (str < end)
			*str = '\0';
		else
			end[-1] = '\0';
	}

	/* the trailing null byte doesn't count towards the total */
	return str-buf;

}

/**
 * snprintf - Format a string and place it in a buffer
 * @buf: The buffer to place the result into
 * @size: The size of the buffer, including the trailing null space
 * @fmt: The format string to use
 * @...: Arguments for the format string
 *
 * The return value is the number of characters which would be
 * generated for the given input, excluding the trailing null,
 * as per ISO C99.  If the return is greater than or equal to
 * @size, the resulting string is truncated.
 *
 * See the vsnprintf() documentation for format string extensions over C99.
 */
#define snprintf snprintf_linux
static
int snprintf_linux(char *buf, size_t size, const char *fmt, ...)
{
	va_list args;
	int i;

	va_start(args, fmt);
	i = vsnprintf(buf, size, fmt, args);
	va_end(args);

	return i;
}

//#define DEBUG
//#define DEBUG_TRACE_ANGLE
//#define DEBUG_TRACE_MODE_CHANGE
//#define DEBUG_D_UART_ERROR
//#define DISABLE_FAST_DATALOG_BUFFER
//#define DISABLE_UART_DATA_ERROR

//#define snprintf(str,size,format,...) printk(format, ##__VA_ARGS__);sprintf(str, format, ##__VA_ARGS__)

#define request_irq(irq, ...) (ena_int(irq) == E_OK ? 0 : 1)

static UART uart_sensor_values;
static volatile UART *pUart = &uart_sensor_values;

static uart_data_t driver_data_uart_sensor[TNUM_INPUT_PORT];

/*
 * Reuse of 'd_uart_mod.c' from LEGO MINDSTORMS EV3 source code
 */
#include "../d_uart/Linuxmod_AM1808/d_uart_mod.c"

static DEVCON devcon = {
    { CONN_NONE, CONN_NONE, CONN_NONE, CONN_NONE },
    { TYPE_UNKNOWN, TYPE_UNKNOWN, TYPE_UNKNOWN, TYPE_UNKNOWN },
    { MODE_NONE_UART_SENSOR, MODE_NONE_UART_SENSOR, MODE_NONE_UART_SENSOR, MODE_NONE_UART_SENSOR }
};

/**
 * Reset UART sensor automatically
 */
static
void uart_sensor_cyc(intptr_t unused) {
    for(uint32_t i = 0; i < 4; ++i) {
        pUart->Status[i] &= ~UART_PORT_CHANGED;
    }
}

static void initialize(intptr_t unused) {
    ER_ID ercd;

    /**
     * Register ISR for Port 1
     */
    if (!(*ev3rt_sensor_port_1_disabled)) {
        T_CISR port1_isr;
        port1_isr.isratr = TA_NULL;
        port1_isr.exinf  = INTNO_UART_PORT1;
        port1_isr.intno  = INTNO_UART_PORT1;
        port1_isr.isr    = (*ev3rt_sensor_port_1_disabled) ? uart_sio_isr : uart_sensor_isr;
        port1_isr.isrpri = TMIN_ISRPRI;
        ercd = acre_isr(&port1_isr);
        assert(ercd > 0);
    }

	UART0.PWREMU_MGMT = 0x6001;
	//UART0.PWREMU_MGMT = 0x6001; //sio_opn_por(1); /* Uart port 2 & enable irq(side effect)*/

	ModuleInit();

	T_HIRES_CCYC ccyc;
	ccyc.cycatr = TA_STA;
	ccyc.cychdr = uart_sensor_cyc;
	ccyc.cyctim = PERIOD_UART_SENSOR_CYC;
	ercd = acre_hires_cyc(&ccyc);
	assert(ercd > 0);

	for(int i = 0; i < TNUM_INPUT_PORT; ++i) {
	    driver_data_uart_sensor[i].actual = &(pUart->Actual[i]);
	    driver_data_uart_sensor[i].raw = pUart->Raw[i];
	    driver_data_uart_sensor[i].status = &(pUart->Status[i]);
	}
	global_brick_info.uart_sensors = driver_data_uart_sensor;
#if defined(DEBUG) || 1
    syslog(LOG_NOTICE, "uart_dri initialized.");
#endif
}

static void softreset(intptr_t unused) {
	for(int i = 0; i < 4; ++i)
		extsvc_uart_sensor_config(i, MODE_NONE_UART_SENSOR, 0, 0, 0, 0);
}

void initialize_uart_dri(intptr_t unused) {
	ev3_driver_t driver;
	driver.init_func = initialize;
	driver.softreset_func = softreset;
	SVC_PERROR(platform_register_driver(&driver));
}

void uart_sensor_isr(intptr_t intno) {
    // TODO: re-implement Uart1Interrupt() and Uart2Interrupt()
    switch(intno) {
    case INTNO_UART_PORT1:
        Uart1Interrupt(0, NULL);
        break;
    case INTNO_UART_PORT2:
        Uart2Interrupt(0, NULL);
        break;
    case INTNO_UART_PORT3:
        pru_suart_isr(1); // Magic number, port 3 uses SUART2
        break;
    case INTNO_UART_PORT4:
        pru_suart_isr(0); // Magic number, port 4 uses SUART1
        break;
    default:
        syslog(LOG_ERROR, "[uart dri] Invalid interrupt number %d", intno);
    }
}

/**
 * Implementation of extended service calls
 */

/**
 * Function to configure the mode of a UART sensor port.
 * @param port a sensor port
 * @param mode the target mode, or MODE_NONE_UART_SENSOR (disconnect mode)
 * @retval E_OK  success
 * @retval E_PAR invalid port number
 */
ER_UINT extsvc_uart_sensor_config(intptr_t port, intptr_t mode, intptr_t par3, intptr_t par4, intptr_t par5, ID cdmid) {
	ER_UINT ercd;

	CHECK_SENSOR_PORT(port);

	// TODO: check mode
	if(mode == MODE_NONE_UART_SENSOR) {
		devcon.Connection[port] = CONN_NONE;
		devcon.Mode[port] = 0;
		if (port != DEBUG_UART) { // Use as an I2C port if no UART sensor is connected
            extern void setup_i2c_port(); // TODO: extern from i2c_dri.c
			setup_i2c_port(port);
		}
	} else {
		devcon.Connection[port] = CONN_INPUT_UART;
		devcon.Mode[port] = mode;
	}

	Device1Ioctl(NULL, NULL, UART_SET_CONN, (unsigned long)&devcon);

	ercd = E_OK;

error_exit:
	return(ercd);
}

/**
 * For debug
 */

void dump_uart_sensor_types() {
	static const char *datafmtstr[] = { "DATA8", "DATA16", "DATA32", "DATAF" };

	for(int port = 0; port < INPUTS; ++port) {
		syslog(LOG_ERROR, "<Sensor Port %d>", port + 1);
		if(devcon.Connection[port] == CONN_NONE) {
			syslog(LOG_ERROR, "Status: Not connected");
			continue;
		}

		assert(devcon.Connection[port] == CONN_INPUT_UART);
		syslog(LOG_ERROR, "Status: Connected @ Mode %d", devcon.Mode[port]);

		for(int i = 0; i < MAX_DEVICE_MODES; ++i) {
			if(TypeData[port][i].Name[0] == '\0') break;
			syslog(LOG_ERROR, ">>>> Type Info of Mode %d <<<<", TypeData[port][i].Mode);
			syslog(LOG_ERROR, "Name:   %s", TypeData[port][i].Name);
			syslog(LOG_ERROR, "Mode:   %d", TypeData[port][i].Mode);
			syslog(LOG_ERROR, "RawMin: %d", (int)TypeData[port][i].RawMin); // TODO: float
			syslog(LOG_ERROR, "RawMax: %d", (int)TypeData[port][i].RawMax); // TODO: float
			syslog(LOG_ERROR, "PctMin: %d", (int)TypeData[port][i].PctMin); // TODO: float
			syslog(LOG_ERROR, "PctMax: %d", (int)TypeData[port][i].PctMax); // TODO: float
			syslog(LOG_ERROR, "SiMin:  %d", (int)TypeData[port][i].SiMin); // TODO: float
			syslog(LOG_ERROR, "SiMax:  %d", (int)TypeData[port][i].SiMax); // TODO: float
			syslog(LOG_ERROR, "Data Format:  %d * %s", TypeData[port][i].DataSets, datafmtstr[TypeData[port][i].Format]);
			syslog(LOG_ERROR, "Invalid Time: %d ms", TypeData[port][i].InvalidTime);
			syslog(LOG_ERROR, "SI Symbol:    %s", TypeData[port][i].Symbol);
		}
	}
}

/**
 * Legacy code
 */
#if 0


//void setOperatingMode(int port, int mode) {
//	devcon.Connection[port] = CONN_INPUT_UART;
//	devcon.Mode[port] = mode;
//	Device1Ioctl(NULL, NULL, UART_SET_CONN, &devcon);
//}
//
//int waitNonZeroStatus(int port, int time) { //TODO: time is not used by now
//	int status;
//	while((status = pUart->Status[port]) == 0);
//	return status;
//}
//
//#define DEBUG
//
//int uart_sensor_switch_mode(uint8_t port, uint8_t mode) {
//    if(devcon.Mode[port] == mode) return 0;
//#ifdef DEBUG
//    SYSTIM tim1, tim2;
//    get_tim(&tim1);
//#endif
//    setOperatingMode(port, mode);
//    int status = waitNonZeroStatus(port, 4000);
//#ifdef DEBUG
//    get_tim(&tim2);
//    printk("[d_uart]Switch mode on port %d takes %d ms\n", port, tim2 - tim1);
//#endif
//    return status;
//}

//unsigned short uart_get_short(int Port) {
//	unsigned short res = (UWORD)(*pUart).Raw[Port][pUart->Actual[Port]][0];
//	res |= (UWORD)(*pUart).Raw[Port][pUart->Actual[Port]][1];
//	return res;
//}

//void config_uart_sensor(uint8_t port, uint8_t mode) {
////    printk("Init status %d\n", pUart->Status[port]);
//
//    setOperatingMode(port, mode);
//
//    while(!(pUart->Status[port] & UART_DATA_READY));
//
//////    int retryCnt = 0;
////
////    int status = waitNonZeroStatus(port, 4000);
////
////    while((status & UART_PORT_CHANGED) != 0) {
////        printk("Start Init\n");
//////        retryCnt++;
//////        UARTCTL uc;
//////        uc.Port = Port;
//////        ioctl(d_uart, UART_CLEAR_CHANGED, &uc);
////        pUart->Status[port] &= ~UART_PORT_CHANGED;
//////        tslp_tsk(5); //usleep(5000);
////        status = waitNonZeroStatus(port, 4000);
////        printk("1st status after init %d\n", status);
////        if ((status & UART_DATA_READY) != 0 && (status & UART_PORT_CHANGED) == 0) {
////            setOperatingMode(port, mode);
////            status = waitNonZeroStatus(port, 4000);
////        }
////    }
////
////    // setMode
////    setOperatingMode(port, mode);
////    printk("Set mode %d\n", mode);
////    status = waitNonZeroStatus(port, 4000);
////    printk("Status is %d\n", status);
//
//}

// Debug Type Info
//#ifdef DEBUG
//#endif
#endif
