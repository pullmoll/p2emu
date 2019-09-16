#pragma once
#include <QDomDocument>

inline QDomElement p2_html(QDomDocument& doc, const char* element) { return doc.createElement(QString::fromLatin1(element)); }
inline QDomText p2_text(QDomDocument& doc, const QString& text) { return doc.createTextNode(text); }

static const QLatin1String attr_style("style");
static const QLatin1String attr_style_nowrap("white-space:nowrap;");
static const QLatin1String attr_style_left("text-align:left;");
static const QLatin1String attr_style_padding("padding:0px 4px 0px 4px;");

static const QLatin1String attr_width("width");
static const QLatin1String attr_10percent("10%");
static const QLatin1String attr_95percent("95%");
