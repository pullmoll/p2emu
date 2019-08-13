/****************************************************************************
 *
 * Simple CSV reader / writer
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
#include <QFile>
#include <QTextCodec>
#include <QTextStream>
#include "csv.h"

CSV::CSV(const QString& filename, const char* codec, QChar delim, QChar quote)
    : m_filename(filename)
    , m_codec(codec)
    , m_delim(delim)
    , m_quote(quote)
{
}

/**
 * @brief Read a file with comma separated values (CSV).
 * @param records result is stored as an array of QStringLists per row
 * @param filename the input file name
 * @param codec the codec to use for the input stream
 * @param delim the column delimiter character to use
 * @param quote the field quote character to use
 * @return true on success, false on error
 */
bool CSV::read(QList<QStringList> &records)
{
    QFile csv(m_filename);
    records.clear();
    if (!csv.open(QIODevice::ReadOnly)) {
        qDebug("%s: can not open '%s' for input", __func__, qPrintable(m_filename));
        return false;
    }

    QTextStream stream(&csv);

    //
    // "ansi" signals codecForLocale() is requiered...
    //
    const QString scodec = QString::fromLatin1(m_codec);
    if (0 == scodec.compare(QStringLiteral("ansi"), Qt::CaseInsensitive))
        stream.setCodec(QTextCodec::codecForLocale());
    else
        stream.setCodec(m_codec);

    while (!stream.atEnd()) {
        QString line = stream.readLine().trimmed();
        if (line.isEmpty())
            continue;

        QStringList row;
        QString field;
        bool quoted = false;
        do {
            for (int i = 0; i < line.length(); i++) {
                QChar ch = line.at(i);
                if (quoted) {
                    // inside quote
                    if (m_quote == ch) {
                        if (i + 1 < line.length() && m_quote == line.at(i+1)) {
                            // quote character inside quoted field
                            field.append(ch);
                            i++;    // skip next character
                        } else {
                            // quoted field ends
                            quoted = false;
                        }
                    } else {
                        // field character inside quotes
                        field.append(ch);
                    }
                } else if (m_quote == ch) {
                    // quoted field start
                    quoted = true;
                } else if (m_delim == ch) {
                    // field delimiter
                    row.append(field);
                    field.clear();
                } else {
                    // field character
                    field.append(ch);
                }
            }
            if (quoted)
                line = stream.readLine().trimmed();
        } while (quoted);

        if (!field.isEmpty())
            row.append(field);
        records.append(row);
        row.clear();
    }
    return true;
}

/**
 * @brief Write a file with comma separated values (CSV).
 * @param records records to write - each row has a list of strings to write
 * @param filename the output file name
 * @param codec the codec to use for the output stream
 * @param delim the column delimiter character to use
 * @param quote the field quote character to use
 * @return true on success, false on error
 */
bool CSV::write(const QList<QStringList> &records)
{
    QFile csv(m_filename);
    if (!csv.open(QIODevice::WriteOnly)) {
        qDebug("%s: can not open '%s' for output", __func__, qPrintable(m_filename));
        return false;
    }
    QTextStream stream(&csv);
    stream.setCodec(m_codec);
    int maxcol = 0;
    foreach(const QStringList& row, records)
        maxcol = qMax(maxcol, row.count());
    foreach(const QStringList row, records) {
        QStringList out;
        foreach (QString field, row) {
            if (field.contains(m_quote)) {
                for (int i = 0; i < field.length(); i++) {
                    if (m_quote == field.at(i)) {
                        field.insert(i, m_quote);
                        i++;
                    }
                }
            }
            if (field.contains(m_delim) || field.contains(m_quote)) {
                field.insert(0, m_quote);
                field.append(m_quote);
            }
            out.append(field);
        }
        while (out.count() < maxcol)
            out.append(QString());
        stream << out.join(m_delim) << endl;
    }
    return true;
}
