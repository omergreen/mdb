/*
 * Copyright (c) 2016-2019 Matt Borgerson
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "gdbstub.h"
#include <target/interface.h>
#include <core/log.h>
#include <core/state.h>
#include <machine/arch/registers_struct.h>
#include <stddef.h>
#include <core/core.h>
#include "breakpoint.h"

/*****************************************************************************
 * Types
 ****************************************************************************/

typedef int (*dbg_enc_func)(char *buf, size_t buf_len, const char *data, size_t data_len);
typedef int (*dbg_dec_func)(const char *buf, size_t buf_len, char *data, size_t data_len);

/*****************************************************************************
 * Const Data
 ****************************************************************************/

const char digits[] = "0123456789abcdef";

/*****************************************************************************
 * Prototypes
 ****************************************************************************/

/* String processing helper functions */
static int dbg_is_printable_char(char ch);
static char dbg_get_digit(int val);
static int dbg_get_val(char digit, int base);
static int dbg_strtol(const char *str, size_t len, int base, const char **endptr);

/* Packet functions */
static int dbg_send_packet(const char *pkt, size_t pkt_len);
static int dbg_recv_packet(char *pkt_buf, size_t pkt_buf_len, size_t *pkt_len);
static int dbg_checksum(const char *buf, size_t len);
static int dbg_recv_ack(void);

/* Data encoding/decoding */
static int dbg_enc_hex(char *buf, size_t buf_len, const char *data, size_t data_len);
static int dbg_dec_hex(const char *buf, size_t buf_len, char *data, size_t data_len);
static int dbg_enc_bin(char *buf, size_t buf_len, const char *data, size_t data_len);
static int dbg_dec_bin(const char *buf, size_t buf_len, char *data, size_t data_len);

/* Packet creation helpers */
static int dbg_send_ok_packet();
static int dbg_send_conmsg_packet(char *buf, size_t buf_len, const char *msg);
static int dbg_send_signal_packet(char *buf, size_t buf_len, char signal);
static int dbg_send_error_packet(char *buf, size_t buf_len, char error);

/* Command functions */
static int dbg_mem_read(char *buf, size_t buf_len, address addr, size_t len, dbg_enc_func enc);
static int dbg_mem_write(const char *buf, size_t buf_len, address addr, size_t len, dbg_dec_func dec);
static int dbg_continue(void);
static int dbg_step(void);

static int dbg_sys_getc();
static int dbg_sys_putchar(int ch);


bool no_ack_mode = false;

/*****************************************************************************
 * String Processing Helper Functions
 ****************************************************************************/

/*
 * Get integer value for a string representation.
 *
 * If the string starts with + or -, it will be signed accordingly.
 *
 * If base == 0, the base will be determined:
 *   base 16 if the string starts with 0x or 0X,
 *   base 10 otherwise
 *
 * If endptr is specified, it will point to the last non-digit in the
 * string. If there are no digits in the string, it will be set to NULL.
 */
static int dbg_strtol(const char *str, size_t len, int base, const char **endptr)
{
	size_t pos;
	int sign, tmp, value, valid;

	value = 0;
	pos   = 0;
	sign  = 1;
	valid = 0;

	if (endptr) {
		*endptr = NULL;
	}

	if (len < 1) {
		return 0;
	}

	/* Detect negative numbers */
	if (str[pos] == '-') {
		sign = -1;
		pos += 1;
	} else if (str[pos] == '+') {
		sign = 1;
		pos += 1;
	}

	/* Detect '0x' hex prefix */
	if ((pos + 2 < len) && (str[pos] == '0') &&
		((str[pos+1] == 'x') || (str[pos+1] == 'X'))) {
		base = 16;
		pos += 2;
	}

	if (base == 0) {
		base = 10;
	}

	for (; (pos < len) && (str[pos] != '\x00'); pos++) {
		tmp = dbg_get_val(str[pos], base);
		if (tmp == EOF) {
			break;
		}

		value = value*base + tmp;
		valid = 1; /* At least one digit is valid */
	}

	if (!valid) {
		return 0;
	}

	if (endptr) {
		*endptr = str+pos;
	}

	value *= sign;

	return value;
}

/*
 * Get the corresponding ASCII hex digit character for a value.
 */
static char dbg_get_digit(int val)
{
	if ((val >= 0) && (val <= 0xf)) {
		return digits[val];
	} else {
		return EOF;
	}
}

/*
 * Get the corresponding value for a ASCII digit character.
 *
 * Supports bases 2-16.
 */
static int dbg_get_val(char digit, int base)
{
	int value;

	if ((digit >= '0') && (digit <= '9')) {
		value = digit-'0';
	} else if ((digit >= 'a') && (digit <= 'f')) {
		value = digit-'a'+0xa;
	} else if ((digit >= 'A') && (digit <= 'F')) {
		value = digit-'A'+0xa;
	} else {
		return EOF;
	}

	return (value < base) ? value : EOF;
}

/*
 * Determine if this is a printable ASCII character.
 */
static int dbg_is_printable_char(char ch)
{
	return (ch >= 0x20 && ch <= 0x7e);
}

/*****************************************************************************
 * Packet Functions
 ****************************************************************************/

/*
 * Receive a packet acknowledgment
 *
 * Returns:
 *    0   if an ACK (+) was received
 *    1   if a NACK (-) was received
 *    EOF otherwise
 */
static int dbg_recv_ack(void)
{
	int response;

    if (no_ack_mode) return 0;

	/* Wait for packet ack */
	switch (response = dbg_sys_getc()) {
	case '+':
		/* Packet acknowledged */
		return 0;
	case '-':
		/* Packet negative acknowledged */
		return 1;
	default:
		/* Bad response! */
		DEBUG("received bad packet response: 0x%2x", response);
		return EOF;
	}
}

/*
 * Calculate 8-bit checksum of a buffer.
 *
 * Returns:
 *    8-bit checksum.
 */
static int dbg_checksum(const char *buf, size_t len)
{
	unsigned char csum;

	csum = 0;

	while (len--) {
		csum += *buf++;
	}

	return csum;
}

/*
 * Transmits a packet of data.
 * Packets are of the form: $<packet-data>#<checksum>
 *
 * Returns:
 *    0   if the packet was transmitted and acknowledged
 *    1   if the packet was transmitted but not acknowledged
 *    EOF otherwise
 */
static int dbg_send_packet(const char *pkt_data, size_t pkt_len)
{
	char buf[3];
	char csum;

	/* Send packet start */
	if (dbg_sys_putchar('$') == EOF) {
		return EOF;
	}

#if DEBUGMODE
	{
		size_t p;
		target_log("-> ");
		for (p = 0; p < pkt_len; p++) {
			if (dbg_is_printable_char(pkt_data[p])) {
				target_log("%c", pkt_data[p]);
			} else {
				target_log("\\x%02x", pkt_data[p]&0xff);
			}
		}
		target_log("\n");
	}
#endif

	/* Send packet data */
	if (target_send(pkt_data, pkt_len) == EOF) {
		return EOF;
	}

	/* Send the checksum */
	buf[0] = '#';
	csum = dbg_checksum(pkt_data, pkt_len);
	if ((dbg_enc_hex(buf+1, sizeof(buf)-1, &csum, 1) == EOF) ||
		(target_send(buf, sizeof(buf)) == EOF)) {
		return EOF;
	}

	return dbg_recv_ack();
}

/*
 * Receives a packet of data, assuming a 7-bit clean connection.
 *
 * Returns:
 *    0   if the packet was received
 *    EOF otherwise
 */
static int dbg_recv_packet(char *pkt_buf, size_t pkt_buf_len, size_t *pkt_len)
{
	int data;
	char expected_csum, actual_csum;
	char buf[2];

	/* Wait for packet start */
	actual_csum = 0;

	while (1) {
		data = dbg_sys_getc();
		if (data == '$') {
			/* Detected start of packet. */
			break;
		}
	}

	/* Read until checksum */
	*pkt_len = 0;
	while (1) {
		data = dbg_sys_getc();

		if (data == EOF) {
			/* Error receiving character */
			return EOF;
		} else if (data == '#') {
			/* End of packet */
			break;
		} else {
			/* Check for space */
			if (*pkt_len >= pkt_buf_len) {
				DEBUG("packet buffer overflow");
				return EOF;
			}

			/* Store character and update checksum */
			pkt_buf[(*pkt_len)++] = (char) data;
		}
	}

#if DEBUGMODE
	{
		size_t p;
		target_log("<- ");
		for (p = 0; p < *pkt_len; p++) {
			if (dbg_is_printable_char(pkt_buf[p])) {
				target_log("%c", pkt_buf[p]);
			} else {
				target_log("\\x%02x", pkt_buf[p] & 0xff);
			}
		}
		target_log("\n");
	}
#endif

	/* Receive the checksum */
	if ((target_recv(buf, 2) == EOF) ||
		(dbg_dec_hex(buf, 2, &expected_csum, 1) == EOF)) {
		return EOF;
	}

	/* Verify checksum */
	actual_csum = dbg_checksum(pkt_buf, *pkt_len);
	if (actual_csum != expected_csum) {
		/* Send packet nack */
		DEBUG("received packet with bad checksum");
        if (!no_ack_mode) {
            dbg_sys_putchar('-');
        }
		return EOF;
	}

	/* Send packet ack */
    if (!no_ack_mode) {
        dbg_sys_putchar('+');
    }
	return 0;
}

/*****************************************************************************
 * Data Encoding/Decoding
 ****************************************************************************/

/*
 * Encode data to its hex-value representation in a buffer.
 *
 * Returns:
 *    0+  number of bytes written to buf
 *    EOF if the buffer is too small
 */
static int dbg_enc_hex(char *buf, size_t buf_len, const char *data, size_t data_len)
{
	size_t pos;

	if (buf_len < data_len*2) {
		/* Buffer too small */
		return EOF;
	}

	for (pos = 0; pos < data_len; pos++) {
		*buf++ = dbg_get_digit((data[pos] >> 4) & 0xf);
		*buf++ = dbg_get_digit((data[pos]     ) & 0xf);
	}

	return data_len*2;
}

/*
 * Decode data from its hex-value representation to a buffer.
 *
 * Returns:
 *    0   if successful
 *    EOF if the buffer is too small
 */
static int dbg_dec_hex(const char *buf, size_t buf_len, char *data, size_t data_len)
{
	size_t pos;
	int tmp;

	if (buf_len != data_len*2) {
		/* Buffer too small */
		return EOF;
	}

	for (pos = 0; pos < data_len; pos++) {
		/* Decode high nibble */
		tmp = dbg_get_val(*buf++, 16);
		if (tmp == EOF) {
			/* Buffer contained junk. */
			assert(0);
			return EOF;
		}

		data[pos] = tmp << 4;

		/* Decode low nibble */
		tmp = dbg_get_val(*buf++, 16);
		if (tmp == EOF) {
			/* Buffer contained junk. */
			assert(0);
			return EOF;
		}
		data[pos] |= tmp;
	}

	return 0;
}

/*
 * Encode data to its binary representation in a buffer.
 *
 * Returns:
 *    0+  number of bytes written to buf
 *    EOF if the buffer is too small
 */
static int dbg_enc_bin(char *buf, size_t buf_len, const char *data, size_t data_len)
{
	size_t buf_pos, data_pos;

	for (buf_pos = 0, data_pos = 0; data_pos < data_len; data_pos++) {
		if (data[data_pos] == '$' ||
			data[data_pos] == '#' ||
			data[data_pos] == '}' ||
			data[data_pos] == '*') {
			if (buf_pos+1 >= buf_len) {
				assert(0);
				return EOF;
			}
			buf[buf_pos++] = '}';
			buf[buf_pos++] = data[data_pos] ^ 0x20;
		} else {
			if (buf_pos >= buf_len) {
				assert(0);
				return EOF;
			}
			buf[buf_pos++] = data[data_pos];
		}
	}

	return buf_pos;
}

/*
 * Decode data from its bin-value representation to a buffer.
 *
 * Returns:
 *    0+  if successful, number of bytes decoded
 *    EOF if the buffer is too small
 */
static int dbg_dec_bin(const char *buf, size_t buf_len, char *data, size_t data_len)
{
	size_t buf_pos, data_pos;

	for (buf_pos = 0, data_pos = 0; buf_pos < buf_len; buf_pos++) {
		if (data_pos >= data_len) {
			/* Output buffer overflow */
			assert(0);
			return EOF;
		}
		if (buf[buf_pos] == '}') {
			/* The next byte is escaped! */
			if (buf_pos+1 >= buf_len) {
				/* There's an escape character, but no escaped character
				 * following the escape character. */
				assert(0);
				return EOF;
			}
			buf_pos += 1;
			data[data_pos++] = buf[buf_pos] ^ 0x20;
		} else {
			data[data_pos++] = buf[buf_pos];
		}
	}

	return data_pos;
}

/*****************************************************************************
 * Command Functions
 ****************************************************************************/

/*
 * Read from memory and encode into buf.
 *
 * Returns:
 *    0+  number of bytes written to buf
 *    EOF if the buffer is too small
 */
static int dbg_mem_read(char *buf, size_t buf_len, address addr, size_t len, dbg_enc_func enc)
{
    return enc(buf, buf_len, (char *)addr, len);
	/* char data[64]; */
	/* size_t pos; */

	/* if (len > sizeof(data)) { */
	/* 	return EOF; */
	/* } */

	/* /1* Read from system memory *1/ */
	/* for (pos = 0; pos < len; pos++) { */

	/* 	if (dbg_sys_mem_readb(addr+pos, &data[pos])) { */
	/* 		/1* Failed to read *1/ */
	/* 		return EOF; */
	/* 	} */
	/* } */

	/* /1* Encode data *1/ */
	/* return enc(buf, buf_len, data, len); */
}

/*
 * Write to memory from encoded buf.
 */
static int dbg_mem_write(const char *buf, size_t buf_len, address addr, size_t len, dbg_dec_func dec)
{
	if (dec(buf, buf_len, (char *)addr, len) == EOF) {
		return EOF;
	}
    return 0;
	/* char data[64]; */
	/* size_t pos; */

	/* if (len > sizeof(data)) { */
	/* 	return EOF; */
	/* } */

	/* /1* Decode data *1/ */
	/* if (dec(buf, buf_len, data, len) == EOF) { */
	/* 	return EOF; */
	/* } */

	/* /1* Write to system memory *1/ */
	/* for (pos = 0; pos < len; pos++) { */
	/* 	if (dbg_sys_mem_writeb(addr+pos, data[pos])) { */
	/* 		/1* Failed to write *1/ */
	/* 		return EOF; */
	/* 	} */
	/* } */

	/* return 0; */
}

/*****************************************************************************
 * Packet Creation Helpers
 ****************************************************************************/

/*
 * Send OK packet
 */
static int dbg_send_ok_packet()
{
	return dbg_send_packet("OK", 2);
}

/*
 * Send a message to the debugging console (via O XX... packet)
 */
static int dbg_send_conmsg_packet(char *buf, size_t buf_len, const char *msg)
{
	size_t size;
	int status;

	if (buf_len < 2) {
		/* Buffer too small */
		return EOF;
	}

	buf[0] = 'O';
	status = dbg_enc_hex(&buf[1], buf_len-1, msg, strlen(msg));
	if (status == EOF) {
		return EOF;
	}
	size = 1 + status;
	return dbg_send_packet(buf, size);
}

/*
 * Send a signal packet (S AA).
 */
static int dbg_send_signal_packet(char *buf, size_t buf_len, char signal)
{
	size_t size;
	int status;

	if (buf_len < 4) {
		/* Buffer too small */
		return EOF;
	}

	buf[0] = 'S';
	status = dbg_enc_hex(&buf[1], buf_len-1, &signal, 1);
	if (status == EOF) {
		return EOF;
	}
	size = 1 + status;
	return dbg_send_packet(buf, size);
}

/*
 * Send a error packet (E AA).
 */
static int dbg_send_error_packet(char *buf, size_t buf_len, char error)
{
	size_t size;
	int status;

	if (buf_len < 4) {
		/* Buffer too small */
		return EOF;
	}

	buf[0] = 'E';
	status = dbg_enc_hex(&buf[1], buf_len-1, &error, 1);
	if (status == EOF) {
		return EOF;
	}
	size = 1 + status;
	return dbg_send_packet(buf, size);
}

/*****************************************************************************
 * Communication Functions
 ****************************************************************************/
static int dbg_sys_getc() {
    char c;
    if (target_recv(&c, 1) != 1) {
        ERROR("can't getc");
        return EOF;
    }

    return c;
}
static int dbg_sys_putchar(int ch) {
    char c = ch;
    if (target_send(&c, 1) != 1) {
        ERROR("can't putchar");
        return EOF;
    }
}

bool starts_with_token(char *s1, char *token) {
    return strncmp(s1, token, strlen(token)) == 0;
}

extern char target_xml_start[] asm("_binary_gdb_target_description_xml_start");
extern char target_xml_end[] asm("_binary_gdb_target_description_xml_end");
bool send_target_xml(int offset, int length) {
    int xml_length = target_xml_end - target_xml_start;
    if (offset >= xml_length) {
        dbg_send_packet("l", 1);
        return true;
    }

    if (offset + length > xml_length) {
        length = xml_length - offset;
    }

    char packet[1 + length]; // don't care about vulnerabilities
    packet[0] = 'm';
    memcpy(packet + 1, target_xml_start, length);
    
    dbg_send_packet(packet, sizeof(packet));
    return true;
}

/*****************************************************************************
 * Main Loop
 ****************************************************************************/

/*
 * Main debug loop. Handles commands.
 */
enum action gdbstub()
{
	address     addr;
	char        pkt_buf[256];
	int         status;
	size_t      length;
	size_t      pkt_len;
	const char *ptr_next;

    int signum = 5; // TODO: ??

	dbg_send_signal_packet(pkt_buf, sizeof(pkt_buf), signum);


	while (1) {
		/* Receive the next packet */
		status = dbg_recv_packet(pkt_buf, sizeof(pkt_buf), &pkt_len);
		if (status == EOF) {
			break;
		}

		if (pkt_len == 0) {
			/* Received empty packet.. */
			continue;
		}

		ptr_next = pkt_buf;

		/*
		 * Handle one letter commands
		 */
		switch (pkt_buf[0]) {

		/* Calculate remaining space in packet from ptr_next position. */
		#define token_remaining_buf (pkt_len-(ptr_next-pkt_buf))

		/* Expecting a seperator. If not present, go to error */
		#define token_expect_seperator(c) \
			{ \
				if (!ptr_next || *ptr_next != c) { \
					goto error; \
				} else { \
					ptr_next += 1; \
				} \
			}

		/* Expecting an integer argument. If not present, go to error */
		#define token_expect_integer_arg(arg) \
			{ \
				arg = dbg_strtol(ptr_next, token_remaining_buf, \
				                 16, &ptr_next); \
				if (!ptr_next) { \
					goto error; \
				} \
			}

		/*
		 * Read Registers
		 * Command Format: g
		 */
		case 'g':
			/* Encode registers */
			status = dbg_enc_hex(pkt_buf, sizeof(pkt_buf),
			                     (char *)&g_state.regs,
			                     sizeof(g_state.regs));
			if (status == EOF) {
				goto error;
			}
			pkt_len = status;
			dbg_send_packet(pkt_buf, pkt_len);
			break;
		
		/*
		 * Write Registers
		 * Command Format: G XX...
		 */
		case 'G':
			status = dbg_dec_hex(pkt_buf+1, pkt_len-1,
                                 (char *)&g_state.regs,
			                     sizeof(g_state.regs));
			if (status == EOF) {
				goto error;
			}
			dbg_send_ok_packet();
			break;

		/*
		 * Read a Register
		 * Command Format: p n
		 */
		case 'p':
			ptr_next += 1;
			token_expect_integer_arg(addr);

			if (addr >= REGISTERS_LENGTH) {
				goto error;
			}

			/* Read Register */
			status = dbg_enc_hex(pkt_buf, sizeof(pkt_buf),
                                 (char *)&(((unsigned long *)&g_state.regs)[addr]),
			                     4);
			if (status == EOF) {
				goto error;
			}
			dbg_send_packet(pkt_buf, status);
			break;
		
		/*
		 * Write a Register
		 * Command Format: P n...=r...
		 */
		case 'P':
			ptr_next += 1;
			token_expect_integer_arg(addr);
			token_expect_seperator('=');

			if (addr < REGISTERS_LENGTH) {
				status = dbg_dec_hex(ptr_next, token_remaining_buf,
				                     (char *)&(((unsigned long *)&g_state.regs)[addr]),
				                     4);
				if (status == EOF) {
					goto error;
				}
			}
			dbg_send_ok_packet();
			break;
		
		/*
		 * Read Memory
		 * Command Format: m addr,length
		 */
		case 'm':
			ptr_next += 1;
			token_expect_integer_arg(addr);
			token_expect_seperator(',');
			token_expect_integer_arg(length);

			/* Read Memory */
			status = dbg_mem_read(pkt_buf, sizeof(pkt_buf),
			                      addr, length, dbg_enc_hex);
			if (status == EOF) {
				goto error;
			}
			dbg_send_packet(pkt_buf, status);
			break;
		
		/*
		 * Write Memory
		 * Command Format: M addr,length:XX..
		 */
		case 'M':
			ptr_next += 1;
			token_expect_integer_arg(addr);
			token_expect_seperator(',');
			token_expect_integer_arg(length);
			token_expect_seperator(':');

			/* Write Memory */
			status = dbg_mem_write(ptr_next, token_remaining_buf,
			                       addr, length, dbg_dec_hex);
			if (status == EOF) {
				goto error;
			}
			dbg_send_ok_packet();
			break;

		/*
		 * Write Memory (Binary)
		 * Command Format: X addr,length:XX..
		 */
		case 'X':
			ptr_next += 1;
			token_expect_integer_arg(addr);
			token_expect_seperator(',');
			token_expect_integer_arg(length);
			token_expect_seperator(':');

			/* Write Memory */
			status = dbg_mem_write(ptr_next, token_remaining_buf,
			                       addr, length, dbg_dec_bin);
			if (status == EOF) {
				goto error;
			}
			dbg_send_ok_packet();
			break;

		/* 
		 * Continue
		 * Command Format: c [addr]
		 */
		case 'c':
            return ACTION_CONTINUE;

		/*
		 * Single-step
		 * Command Format: s [addr]
		 */
		case 's':
            return ACTION_SINGLE_STEP;

		case '?':
			dbg_send_signal_packet(pkt_buf, sizeof(pkt_buf), signum);
			break;

        // From here, our own stuff
        
        case 'q':
            if (starts_with_token(pkt_buf + 1, "Supported")) {
                // throw out the packet for now, but answer that we support target description
                char packet[] = "PacketSize=1000;qXfer:features:read+;QStartNoAckMode+"; // copied from qemu's response
                dbg_send_packet(packet, strlen(packet));
            }
            else if (starts_with_token(pkt_buf + 1, "Xfer:features:read:target.xml:")) {
                ptr_next += strlen("qXfer:features:read:target.xml:");
                int xml_offset, xml_length;
                token_expect_integer_arg(xml_offset);
                token_expect_seperator(',')
                token_expect_integer_arg(xml_length);
                if (!send_target_xml(xml_offset, xml_length)) {
                    goto error;
                }
            }
            else if (starts_with_token(pkt_buf + 1, "Attached")) {
                dbg_send_packet("0", 1); // attached to existing process
            }
            else if (starts_with_token(pkt_buf + 1, "fThreadInfo")) {
                dbg_send_packet("m00", 3); // send thread id as 0
            }
            else if (starts_with_token(pkt_buf + 1, "sThreadInfo")) {
                dbg_send_packet("l", 1); // we don't have anymore thread info to send
            }
            else if (pkt_buf[1] == 'C') { // Return the current thread ID.
                dbg_send_packet("QC00", 4);
            }
            else {
                // unknown query
                dbg_send_packet(NULL, 0);
            }
            break;

        case 'Q':
            if (starts_with_token(pkt_buf + 1, "StartNoAckMode")) {
                dbg_send_ok_packet();
                no_ack_mode = true;
            }
            else {
                dbg_send_packet(NULL, 0);
            }
            break;

        /* case 'v': */
        /*     if (starts_with_token(pkt_buf + 1, "Cont")) { */
        /*         if (pkt_buf[5] == '?') { */
        /*             // answer that we support c and s */
        /*             char packet[] = "vCont;c;s"; */
        /*             dbg_send_packet(packet, strlen(packet)); */
        /*         } */
        /*         else if (pkt_buf[5] == ';') { */

        /*         } */
        /*         else { */
        /*             dbg_send_error_packet(pkt_buf, sizeof(pkt_buf), 1); */
        /*         } */
        /*     } */
        /*     else { */
        /*         dbg_send_packet(NULL, 0); */
        /*     } */
        /*     break; */

        case 'H': // Set thread for subsequent operations
            dbg_send_ok_packet();
            break;

        case 'Z': // set breakpoint
        case 'z': // unset breakpoint
        {
            int type;
            unsigned int addr;
            int kind;

            ptr_next += 1;
			token_expect_integer_arg(type);
			token_expect_seperator(',');
			token_expect_integer_arg(addr);
			token_expect_seperator(',');
			token_expect_integer_arg(kind);

            if (type != 0) {
                dbg_send_packet(NULL, 0);
            }
            if (kind != 4) {
                dbg_send_packet(NULL, 0);
            }

            if (pkt_buf[0] == 'Z') {
                if (breakpoint_exists(addr)) {
                    dbg_send_error_packet(pkt_buf, sizeof(pkt_buf), 1);
                }
                else {
                    breakpoint_add(addr, false);
                }
            }
            else {
                if (!breakpoint_exists(addr)) {
                    dbg_send_error_packet(pkt_buf, sizeof(pkt_buf), 1);
                }
                else {
                    breakpoint_remove(addr);
                }
            }

            dbg_send_ok_packet();

            break;
        }


		/*
		 * Unsupported Command
		 */
		default:
			dbg_send_packet(NULL, 0);
		}

		continue;

	error:
		dbg_send_error_packet(pkt_buf, sizeof(pkt_buf), 0x00);

		#undef token_remaining_buf
		#undef token_expect_seperator
		#undef token_expect_integer_arg
	}

	return ACTION_CONTINUE;
}

