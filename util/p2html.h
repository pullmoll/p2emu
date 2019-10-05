/****************************************************************************
 *
 * P2 emulator simple QDomElement and QDomText construction
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
#pragma once
#include <QDomDocument>

inline QDomElement p2_html(QDomDocument& doc, const char* element) { return doc.createElement(QString::fromLatin1(element)); }
inline QDomText p2_text(QDomDocument& doc, const QString& text) { return doc.createTextNode(text); }

inline QDomElement p2_head(QDomDocument& doc) { return p2_html(doc, "head"); }
inline QDomElement p2_title(QDomDocument& doc) { return p2_html(doc, "title"); }
inline QDomElement p2_body(QDomDocument& doc) { return p2_html(doc, "body"); }
inline QDomElement p2_table(QDomDocument& doc) { return p2_html(doc, "table"); }
inline QDomElement p2_tr(QDomDocument& doc) { return p2_html(doc, "tr"); }
inline QDomElement p2_th(QDomDocument& doc) { return p2_html(doc, "th"); }
inline QDomElement p2_td(QDomDocument& doc) { return p2_html(doc, "td"); }
inline QDomElement p2_tt(QDomDocument& doc) { return p2_html(doc, "tt"); }
inline QDomElement p2_hr(QDomDocument& doc) { return p2_html(doc, "hr"); }

static const QLatin1String attr_style("style");
static const QLatin1String attr_style_nowrap("white-space:nowrap;");
static const QLatin1String attr_style_left("text-align:left;");
static const QLatin1String attr_style_padding("padding:0 0.25em 0 0.25em;");

static const QLatin1String attr_width("width");
static const QLatin1String attr_10percent("10%");
static const QLatin1String attr_95percent("95%");
