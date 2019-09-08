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
const QString template_str_opcode_doc = QStringLiteral("[EEEE OOOOOOO CZI DDDDDDDDD SSSSSSSSS] OPCODE {#}D,{#}S,#N {ANDC,ANDZ} xxxxx ");

const QString template_str_tokens = QStringLiteral("T");
const QString template_str_symbols = QStringLiteral("S");
const QString template_str_errors = QStringLiteral("😞");
const QString template_str_instruction = QStringLiteral(" label_name IF_NC_AND_NZ  INSTRUCTION #$1ff,#$1ff,#7 XORCZ ");
const QString template_str_description = QStringLiteral(" Some description string... ");

const QString key_tv_asm = QStringLiteral("tvAsm");

void trait_set(p2_traits_e& traits, const p2_traits_e set)
{
    traits = static_cast<p2_traits_e>(traits | set);
}

void trait_clr(p2_traits_e& traits, const p2_traits_e clr)
{
    traits = static_cast<p2_traits_e>(traits & ~clr);
}

bool has_trait(const p2_traits_e traits, const p2_traits_e has)
{
    return traits & has ? true : false;
}
