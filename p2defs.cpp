/****************************************************************************
 *
 * Propeller2 constants, enumerations, opcode structure, helpers...
 *
 * Copyright (C) 2019 Jürgen Buchmüller <pullmoll@t-online.de>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 ****************************************************************************/
#include <QString>
#include "p2defs.h"
#include "p2token.h"

const QString template_str_origin = QStringLiteral("FFFFFF ");
const QString template_str_address = QStringLiteral("COG:1FF ");

const QString template_str_opcode_bin = QStringLiteral("EEEE OOOOOOO CZI DDDDDDDDD SSSSSSSSS ");
const QString template_str_opcode_byt = QStringLiteral("F FF FFF FFF FFF ");
const QString template_str_opcode_dec = QStringLiteral("999999999999 ");
const QString template_str_opcode_hex = QStringLiteral("FFFFFFFF ");

const QString template_str_tokens = QStringLiteral("T");
const QString template_str_symbols = QStringLiteral("S");
const QString template_str_errors = QStringLiteral("😞");
const QString template_str_instruction = QStringLiteral(" label_name IF_NC_AND_NZ  INSTRUCTION #$1ff,#$1ff,#7 XORCZ ");
const QString template_str_description = QStringLiteral(" Some description string... ");

const QString key_tv_asm = QStringLiteral("tvAsm");

QString format_opcode_bin(const p2_opcode_u& IR)
{
    return QString("%1 %2 %3%4%5 %6 %7")
            .arg(IR.op.cond, 4, 2, QChar('0'))
            .arg(IR.op.inst, 7, 2, QChar('0'))
            .arg(IR.op.wc,   1, 2, QChar('0'))
            .arg(IR.op.wz,   1, 2, QChar('0'))
            .arg(IR.op.im,   1, 2, QChar('0'))
            .arg(IR.op.dst,  9, 2, QChar('0'))
            .arg(IR.op.src,  9, 2, QChar('0'));
}

QString format_opcode_byt(const p2_opcode_u& IR)
{
    return QString("%1 %2 %3%4%5 %6 %7")
            .arg(IR.op.cond, 1, 16, QChar('0'))
            .arg(IR.op.inst, 2, 16, QChar('0'))
            .arg(IR.op.wc,   1, 16, QChar('0'))
            .arg(IR.op.wz,   1, 16, QChar('0'))
            .arg(IR.op.im,   1, 16, QChar('0'))
            .arg(IR.op.dst,  3, 16, QChar('0'))
            .arg(IR.op.src,  3, 16, QChar('0'));
}

QString format_opcode_dec(const p2_opcode_u& IR)
{
    // 4294967295
    return QString("%1").arg(IR.opcode, 10);
}

QString format_opcode_hex(const p2_opcode_u& IR)
{
    // FFFFFFFF
    return QString("%1").arg(IR.opcode, 8, 16, QChar('0'));
}

QString format_data_bin(const p2_opcode_u& IR, bool prefix)
{
    // 11111111_11111111_11111111_11111111
    return QString("%1%2 %3 %4 %5")
            .arg(prefix ? QStringLiteral("%") : QString())
            .arg((IR.opcode >> 24) & 0xff, 8, 2, QChar('0'))
            .arg((IR.opcode >> 16) & 0xff, 8, 2, QChar('0'))
            .arg((IR.opcode >>  8) & 0xff, 8, 2, QChar('0'))
            .arg((IR.opcode >>  0) & 0xff, 8, 2, QChar('0'));
}

QString format_data_byt(const p2_opcode_u& IR, bool prefix)
{
    // FF_FF_FF_FF
    return QString("%1%2 %3 %4 %5")
            .arg(prefix ? QStringLiteral("%%") : QString())
             .arg((IR.opcode >> 24) & 0xff, 2, 16, QChar('0'))
             .arg((IR.opcode >> 16) & 0xff, 2, 16, QChar('0'))
             .arg((IR.opcode >>  8) & 0xff, 2, 16, QChar('0'))
             .arg((IR.opcode >>  0) & 0xff, 2, 16, QChar('0'));
}

QString format_data_dec(const p2_opcode_u& IR)
{
    // 4294967295
    return QString("%1")
            .arg(IR.opcode, 0, 10);
}

QString format_data_hex(const p2_opcode_u& IR, bool prefix)
{
    // FFFFFFFF
    return QString("%1%2")
            .arg(prefix ? QStringLiteral("$") : QString())
            .arg(IR.opcode, 0, 16, QChar('0'));
}

QString format_data_bin(const p2_LONG data, bool prefix)
{
    p2_opcode_u IR = {data};
    return format_data_bin(IR, prefix);
}

QString format_data_byt(const p2_LONG data, bool prefix)
{
    p2_opcode_u IR = {data};
    return format_data_byt(IR, prefix);
}

QString format_data_dec(const p2_LONG data)
{
    p2_opcode_u IR = {data};
    return format_data_dec(IR);
}

QString format_data_hex(const p2_LONG data, bool prefix)
{
    p2_opcode_u IR = {data};
    return format_data_hex(IR, prefix);
}

QString format_opcode(const p2_opcode_u& IR, const p2_format_e fmt)
{
    switch (fmt) {
    case fmt_bin: return format_opcode_bin(IR);
    case fmt_byt: return format_opcode_byt(IR);
    case fmt_dec: return format_opcode_dec(IR);
    case fmt_hex: return format_opcode_hex(IR);
    }
    return QStringLiteral("<invalid format>");
}

QString format_data(const p2_opcode_u& IR, const p2_format_e fmt, bool prefix)
{
    switch (fmt) {
    case fmt_bin: return format_data_bin(IR, prefix);
    case fmt_byt: return format_data_byt(IR, prefix);
    case fmt_dec: return format_data_dec(IR);
    case fmt_hex: return format_data_hex(IR, prefix);
    }
    return QStringLiteral("<invalid format>");
}

QString format_data(const p2_LONG data, const p2_format_e fmt, bool prefix)
{
    p2_opcode_u IR = {data};
    return format_data(IR, fmt, prefix);
}
