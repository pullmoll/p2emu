#pragma once
#include <QDomDocument>

inline QDomElement html(QDomDocument& doc, const char* element) { return doc.createElement(QString::fromLatin1(element)); }
inline QDomText doc_text(QDomDocument& doc, const QString& text) { return doc.createTextNode(text); }
