#include "ais_text_editor.h"

#include <QStringListModel>
#include <QKeyEvent>
#include <QScrollBar>
#include <QStandardItemModel>
#include <QColumnView>

CAisTextEdit::CAisTextEdit(QWidget *parent) : QTextEdit(parent)
{
	m_completer = new QCompleter(this);
	m_completer->setWidget(this);

	m_model = new QStandardItemModel(this);
	QStandardItem *func1 = CAisTextEdit::generateCompletionItem(QString("addHpUpTrigger(f,f)"),
		QString("use the addHpUpTrigger to get a trigger when bots hp goes above a limit."));
	QStandardItem *func2 = CAisTextEdit::generateCompletionItem(QString("addHpDownTrigger(f,f)"),
		QString("use the addHpDownTrigger to get a trigger when bots hp goes below a limit."));
	m_model->appendRow(func1);
	m_model->appendRow(func2);

    /*"addHpDownTrigger_ff_
    "delHpUpTrigger_ff_
    "delHpDownTrigger_ff_
    "addHpUpTrigger_fs_
    "addHpDownTrigger_fs_
    "delHpUpTrigger_fs_
    "delHpDownTrigger_fs_
    "addNamedEntityListener_ssf_
    "delNamedEntityListener_ssf_
    "addNamedEntityListener_sss_
    "delNamedEntityListener_sss_*/
	
	//words << "working" << "properly";
	//QStringListModel *model = new QStringListModel(words, m_completer);

	m_completer->setModel(m_model);
	m_completer->setCompletionMode(QCompleter::PopupCompletion);

	m_completerPopup = new QColumnView(this);
	m_completer->setPopup(m_completerPopup);
	//m_completerPopup->set

	connect(m_completer, SIGNAL(activated(QString)), this, SLOT(insertCompletion(QString)));
}

QStandardItem *CAisTextEdit::generateCompletionItem(const QString &function, const QString &functionDoc)
{
	QStandardItem *func1 = new QStandardItem(function);
	QStandardItem *func1Docs = new QStandardItem(functionDoc);
	func1->appendRow(func1Docs);
	return func1;
}

void CAisTextEdit::keyPressEvent(QKeyEvent *event)
{
	if(m_completer->popup()->isVisible())
	{
		switch(event->key())
		{
		case Qt::Key_Enter:
		case Qt::Key_Return:
		case Qt::Key_Escape:
		case Qt::Key_Tab:
			event->ignore();
			return;
		}
	}

	QTextEdit::keyPressEvent(event);

	const QString completionPrefix = textUnderCursor();
	static QString eow("~!@#$%^&*()_+{}|:\"<>,./;'[]\\-=");
	
	// Check to determine if we should show the completer popup.
	if(event->text().isEmpty() || completionPrefix.length() < 3 || eow.contains(event->text().right(1)))
	{
		m_completer->popup()->hide();
		return;
	}
	
	if(completionPrefix != m_completer->completionPrefix())
	{
		m_completer->setCompletionPrefix(completionPrefix);
		m_completer->popup()->setCurrentIndex(m_completer->completionModel()->index(0,0));
	}
	
	QRect cr = cursorRect();
	cr.setWidth(m_completer->popup()->sizeHint().width() +
		m_completer->popup()->verticalScrollBar()->sizeHint().width());
	m_completer->complete(cr);
}

QString CAisTextEdit::textUnderCursor() const
{
	QTextCursor cursor = textCursor();
	cursor.select(QTextCursor::WordUnderCursor);
	return cursor.selectedText();
}

void CAisTextEdit::insertCompletion(const QString &completion)
{
	QTextCursor cursor = textCursor();
	int extra = completion.length() - m_completer->completionPrefix().length();
	cursor.movePosition(QTextCursor::Left);
	cursor.movePosition(QTextCursor::EndOfWord);
	cursor.insertText(completion.right(extra));
	setTextCursor(cursor);
}