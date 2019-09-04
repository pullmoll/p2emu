#pragma once
#include <QSharedPointer>
#include "p2token.h"

typedef QPair<p2_token_e,bool> P2DocParam;
typedef QList<P2DocParam> P2DocParams;

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
    const P2DocParams& params() const;

    void set_brief(const char* brief = nullptr);
    void set_instr(const char* instr = nullptr);
    void set_descr(QList<const char*> descr = QList<const char*>());
    void add_descr(const char* descr = nullptr);
    void set_token(p2_token_e token);
    void set_params(p2_token_e token, bool mandatory = true);
    void add_param(p2_token_e token, bool mandatory = true);

private:
    P2MatchMask m_matchmask;            //!< mask and match pair
    const char * m_func;                //!< name of the fuction which created this
    const char * m_pattern;             //!< opcode pattern
    const char * m_brief;               //!< opcode brief description
    const char * m_instr;               //!< opcode instruction example
    QList<const char *> m_descr;        //!< opcode description
    p2_token_e m_token;                 //!< opcode instruction token
    P2DocParams m_params;               //!< opcode parameters (may be optional)
};

typedef QSharedPointer<P2DocOpcodeClass> P2DocOpcode;

