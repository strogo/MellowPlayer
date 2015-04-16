/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Copyright (C) 2013 Ivan Komissarov.
** Contact: http://www.qt-project.org/legal
**
** This file is part of the QtWidgets module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "keysequenceedit.h"

#if QT_VERSION >= 0x050000

KeySequenceEdit::KeySequenceEdit(QWidget *parent):
    QKeySequenceEdit(parent)
{

}

#else

#include "keysequenceedit.h"
#include "keysequenceedit_p.h"

#include "qboxlayout.h"
#include "qlineedit.h"


void KeySequenceEditPrivate::init()
{
    Q_Q(KeySequenceEdit);

    lineEdit = new QLineEdit(q);
    lineEdit->setObjectName("qt_keysequenceedit_lineedit");
    keyNum = 0;
    prevKey = -1;
    releaseTimer = 0;

    QVBoxLayout *layout = new QVBoxLayout(q);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(lineEdit);

    key[0] = key[1] = key[2] = key[3] = 0;

    lineEdit->setFocusProxy(q);
    lineEdit->installEventFilter(q);
    resetState();

    q->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    q->setFocusPolicy(Qt::StrongFocus);
    q->setAttribute(Qt::WA_MacShowFocusRect, true);
    q->setAttribute(Qt::WA_InputMethodEnabled, false);

    // TODO: add clear button
}

int KeySequenceEditPrivate::translateModifiers(Qt::KeyboardModifiers state, const QString &text)
{
    int result = 0;
    // The shift modifier only counts when it is not used to type a symbol
    // that is only reachable using the shift key anyway
    if ((state & Qt::ShiftModifier) && (text.isEmpty() ||
                                        !text.at(0).isPrint() ||
                                        text.at(0).isLetterOrNumber() ||
                                        text.at(0).isSpace()))
        result |= Qt::SHIFT;

    if (state & Qt::ControlModifier)
        result |= Qt::CTRL;
    if (state & Qt::MetaModifier)
        result |= Qt::META;
    if (state & Qt::AltModifier)
        result |= Qt::ALT;
    return result;
}

void KeySequenceEditPrivate::resetState()
{
    Q_Q(KeySequenceEdit);

    if (releaseTimer) {
        q->killTimer(releaseTimer);
        releaseTimer = 0;
    }
    prevKey = -1;
    lineEdit->setText(keySequence.toString(QKeySequence::NativeText));
    lineEdit->setPlaceholderText(KeySequenceEdit::tr("Press shortcut"));
}

void KeySequenceEditPrivate::finishEditing()
{
    Q_Q(KeySequenceEdit);

    resetState();
    emit q->keySequenceChanged(keySequence);
    emit q->editingFinished();
}

/*!
    \class KeySequenceEdit
    \brief The KeySequenceEdit widget allows to input a QKeySequence.

    \inmodule QtWidgets

    \since 5.2

    This widget lets the user choose a QKeySequence, which is usually used as
    a shortcut. The recording is initiated when the widget receives the focus
    and ends one second after the user releases the last key.

    \sa KeySequenceEdit::keySequence
*/

/*!
    Constructs a KeySequenceEdit widget with the given \a parent.
*/
KeySequenceEdit::KeySequenceEdit(QWidget *parent) :
    QWidget(*new KeySequenceEditPrivate, parent, 0)
{
    Q_D(KeySequenceEdit);
    d->init();
}

/*!
    Constructs a KeySequenceEdit widget with the given \a keySequence and \a parent.
*/
KeySequenceEdit::KeySequenceEdit(const QKeySequence &keySequence, QWidget *parent) :
    QWidget(*new KeySequenceEditPrivate, parent, 0)
{
    Q_D(KeySequenceEdit);
    d->init();
    setKeySequence(keySequence);
}

/*!
    \internal
*/
KeySequenceEdit::KeySequenceEdit(KeySequenceEditPrivate &dd, QWidget *parent, Qt::WindowFlags f) :
    QWidget(dd, parent, f)
{
    Q_D(KeySequenceEdit);
    d->init();
}

/*!
    Destroys the KeySequenceEdit object.
*/
KeySequenceEdit::~KeySequenceEdit()
{
}

/*!
    \property KeySequenceEdit::keySequence

    \brief This property contains the currently chosen key sequence.

    The shortcut can be changed by the user or via setter function.
*/
QKeySequence KeySequenceEdit::keySequence() const
{
    Q_D(const KeySequenceEdit);

    return d->keySequence;
}

void KeySequenceEdit::setKeySequence(const QKeySequence &keySequence)
{
    Q_D(KeySequenceEdit);

    d->resetState();

    if (d->keySequence == keySequence)
        return;

    d->keySequence = keySequence;

    d->key[0] = d->key[1] = d->key[2] = d->key[3] = 0;
    d->keyNum = keySequence.count();
    for (int i = 0; i < d->keyNum; ++i)
        d->key[i] = keySequence[i];

    d->lineEdit->setText(keySequence.toString(QKeySequence::NativeText));

    emit keySequenceChanged(keySequence);
}

/*!
    \fn void KeySequenceEdit::editingFinished()

    This signal is emitted when the user finishes entering the shortcut.

    \note there is a one second delay before releasing the last key and
    emitting this signal.
*/

/*!
    \brief Clears the current key sequence.
*/
void KeySequenceEdit::clear()
{
    setKeySequence(QKeySequence());
}

/*!
    \reimp
*/
bool KeySequenceEdit::event(QEvent *e)
{
    switch (e->type()) {
    case QEvent::Shortcut:
        return true;
    case QEvent::ShortcutOverride:
        e->accept();
        return true;
    default :
        break;
    }

    return QWidget::event(e);
}

/*!
    \reimp
*/
void KeySequenceEdit::keyPressEvent(QKeyEvent *e)
{
    Q_D(KeySequenceEdit);

    int nextKey = e->key();

    if (d->prevKey == -1) {
        clear();
        d->prevKey = nextKey;
    }

    d->lineEdit->setPlaceholderText(QString());
    if (nextKey == Qt::Key_Control
            || nextKey == Qt::Key_Shift
            || nextKey == Qt::Key_Meta
            || nextKey == Qt::Key_Alt) {
        return;
    }

    QString selectedText = d->lineEdit->selectedText();
    if (!selectedText.isEmpty() && selectedText == d->lineEdit->text()) {
        clear();
        if (nextKey == Qt::Key_Backspace)
            return;
    }

    if (d->keyNum >= MaxKeyCount)
        return;

    nextKey |= d->translateModifiers(e->modifiers(), e->text());

    d->key[d->keyNum] = nextKey;
    d->keyNum++;

    QKeySequence key(d->key[0], d->key[1], d->key[2], d->key[3]);
    d->keySequence = key;
    QString text = key.toString(QKeySequence::NativeText);
    if (d->keyNum < MaxKeyCount) {
        //: This text is an "unfinished" shortcut, expands like "Ctrl+A, ..."
        text = tr("%1, ...").arg(text);
    }
    d->lineEdit->setText(text);
    e->accept();
}

/*!
    \reimp
*/
void KeySequenceEdit::keyReleaseEvent(QKeyEvent *e)
{
    Q_D(KeySequenceEdit);

    if (d->prevKey == e->key()) {
        if (d->keyNum < MaxKeyCount)
            d->releaseTimer = startTimer(1000);
        else
            d->finishEditing();
    }
    e->accept();
}

/*!
    \reimp
*/
void KeySequenceEdit::timerEvent(QTimerEvent *e)
{
    Q_D(KeySequenceEdit);
    if (e->timerId() == d->releaseTimer) {
        d->finishEditing();
        return;
    }

    QWidget::timerEvent(e);
}

#endif // QT_VERSION >= 0x050000
