#pragma once
#include <QDomDocument>

inline QDomElement p2_html(QDomDocument& doc, const char* element) { return doc.createElement(QString::fromLatin1(element)); }
inline QDomText p2_text(QDomDocument& doc, const QString& text) { return doc.createTextNode(text); }
