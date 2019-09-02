#pragma once
#include <QSharedPointer>
#include "p2token.h"

class P2DocOpcodeClass
{
public:
    explicit P2DocOpcodeClass(P2MatchMask mm = P2MatchMask(), const char* pat = nullptr, const char* _func = nullptr);

    bool isDefined() const;
    P2MatchMask matchmask() const;
    const char* func() const;
    const QString pattern() const;
    const QString brief() const;
    const QString instr() const;
    const QStringList descr() const;
    p2_token_e token() const;

    void set_brief(const char* brief = nullptr);
    void set_instr(const char* instr = nullptr);
    void set_descr(QList<const char*> descr = QList<const char*>());
    void add_descr(const char* descr = nullptr);
    void set_token(p2_token_e token);

private:
    P2MatchMask m_matchmask;            //!< mask and match pair
    const char * m_func;                //!< name of the fuction which created this
    const char * m_pattern;             //!< opcode pattern
    const char * m_brief;               //!< opcode brief description
    const char * m_instr;               //!< opcode instruction example
    QList<const char *> m_descr;        //!< opcode description
    p2_token_e m_token;                 //!< opcode instruction token
};

typedef QSharedPointer<P2DocOpcodeClass> P2DocOpcode;

