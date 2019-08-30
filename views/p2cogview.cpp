/****************************************************************************
 *
 * P2 emulator Cog view widget implementation
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
#include "p2cogview.h"
#include "ui_p2cogview.h"
#include "p2cog.h"
#include "p2dasm.h"
#include "p2dasmmodel.h"

static QString bin(p2_LONG val, int digits = 1)
{
    return QString("%1").arg(val, digits, 2, QChar('0'));
}

static QString hex(p2_LONG val, int digits = 8)
{
    return QString("%1").arg(val, digits, 16, QChar('0'));
}

P2CogView::P2CogView(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::P2CogView)
    , m_cog(nullptr)
    , m_dasm(nullptr)
    , m_model(nullptr)
{
    ui->setupUi(this);
}

P2CogView::~P2CogView()
{
    delete ui;
}

void P2CogView::updateView()
{
    ui->group_PC->setTitle(QString("COG ID #%1").arg(m_cog->rd_ID()));
    const p2_LONG PC = m_cog->rd_PC();
    ui->le_PC->setText(hex(PC, 6));

    P2Opcode IR(m_cog->rd_mem(PC));
    QVariant D_aug = m_cog->rd_D_aug();
    QVariant S_aug = m_cog->rd_S_aug();
    p2_LONG D = D_aug.isValid() ? D_aug.toUInt() | IR.u.op.dst : IR.u.op.dst;
    p2_LONG S = S_aug.isValid() ? S_aug.toUInt() | IR.u.op.src : IR.u.op.src;
    ui->le_DST->setText(hex(D, 3));
    ui->le_SRC->setText(hex(S, 3));

    ui->le_IR->setText(QString("%1_%2_%3%4%5_%6_%8")
                       .arg(bin(IR.u.op.cond, 4))
                       .arg(bin(IR.u.op.inst, 7))
                       .arg(bin(IR.u.op.wc, 1))
                       .arg(bin(IR.u.op.wz, 1))
                       .arg(bin(IR.u.op.im, 1))
                       .arg(bin(IR.u.op.dst, 9))
                       .arg(bin(IR.u.op.src, 9)));
    ui->cb_C->setChecked(m_cog->rd_C());
    ui->cb_Z->setChecked(m_cog->rd_Z());

    const p2_flags_t flags = m_cog->rd_FLAGS();
    ui->cb_INT->setChecked(flags.f_INT);            // INT interrupt flag
    ui->cb_CT1->setChecked(flags.f_CT1);            // CT1 counter flag
    ui->cb_CT2->setChecked(flags.f_CT2);            // CT2 counter flag
    ui->cb_CT3->setChecked(flags.f_CT3);            // CT3 counter flag
    ui->cb_SE1->setChecked(flags.f_SE1);            // SE1 event flag
    ui->cb_SE2->setChecked(flags.f_SE2);            // SE2 event flag
    ui->cb_SE3->setChecked(flags.f_SE3);            // SE3 event flag
    ui->cb_SE4->setChecked(flags.f_SE4);            // SE4 event flag
    ui->cb_PAT->setChecked(flags.f_PAT);            // PAT pattern matching flag
    ui->cb_FBW->setChecked(flags.f_FBW);            // FBW flag
    ui->cb_XMT->setChecked(flags.f_XMT);            // XMT flag
    ui->cb_XFI->setChecked(flags.f_XFI);            // XFI flag
    ui->cb_XRO->setChecked(flags.f_XRO);            // XRO flag
    ui->cb_XRL->setChecked(flags.f_XRL);            // XRL flag
    ui->cb_ATN->setChecked(flags.f_ATN);            // ATN COG attention flag
    ui->cb_QMT->setChecked(flags.f_QMT);            // QMT Q empty flag

    ui->le_CT1->setText(hex(m_cog->rd_CT1()));
    ui->le_CT2->setText(hex(m_cog->rd_CT2()));
    ui->le_CT3->setText(hex(m_cog->rd_CT3()));

    p2_pat_t pat = m_cog->rd_PAT();
    switch (pat.mode) {
    case p2_PAT_NONE:
        ui->le_PAT_mode->setText(tr("OFF"));
        break;
    case p2_PAT_PA_EQ:
        ui->le_PAT_mode->setText(tr("PA == match"));
        break;
    case p2_PAT_PA_NE:
        ui->le_PAT_mode->setText(tr("PA != match"));
        break;
    case p2_PAT_PB_EQ:
        ui->le_PAT_mode->setText(tr("PB == match"));
        break;
    case p2_PAT_PB_NE:
        ui->le_PAT_mode->setText(tr("PB != match"));
        break;
    }
    ui->le_PAT_mask->setText(QString("%1").arg(pat.mask, 8, 16, QChar('0')));
    ui->le_PAT_match->setText(QString("%1").arg(pat.match, 8, 16, QChar('0')));

    p2_pin_t pin = m_cog->rd_PIN();
    switch (pin.mode) {
    case p2_PIN_NONE:
        ui->le_PIN_mode->setText(tr("OFF"));
        break;
    case p2_PIN_CHANGED_LO:
        ui->le_PIN_mode->setText(tr("Falling edge"));
        break;
    case p2_PIN_CHANGED_HI:
        ui->le_PIN_mode->setText(tr("Rising edge"));
        break;
    case p2_PIN_CHANGED:
        ui->le_PIN_mode->setText(tr("Any edge"));
        break;
    }
    ui->le_PIN_edge->setText(hex(pin.edge, 2));
    ui->le_PIN_num->setText(hex(pin.num, 2));

    const int row = static_cast<int>(PC < 0x00400 ? PC : PC / 4);
    m_model->invalidate();
    ui->tvDasm->selectRow(row);
}

void P2CogView::setCog(const P2Cog* cog)
{
    m_cog = cog;
    m_dasm = new P2Dasm(cog);
    m_dasm->setLowercase(true);
    m_model = new P2DasmModel(m_dasm);
    m_model->setOpcodeFormat(fmt_hex);
    ui->tvDasm->setModel(m_model);
    // Set column sizes
    for (int column = 0; column < m_model->columnCount(); column++) {
        QSize size = m_model->sizeHint(static_cast<P2DasmModel::column_e>(column));
        ui->tvDasm->setColumnWidth(column, size.width());
    }
    ui->tvDasm->setColumnHidden(P2DasmModel::c_Description, true);
    updateView();
}
