#ifndef AIS_TEXT_EDITOR_H
#define AIS_TEXT_EDITOR_H

#include <QObject>
#include <QTextEdit>
#include <QCompleter>
#include <QColumnView>
#include <QStandardItemModel>

class CAisTextEdit : public QTextEdit
{
	Q_OBJECT

public:
	CAisTextEdit(QWidget *parent = 0);

	static QStandardItem *generateCompletionItem(const QString &function, const QString &functionDoc);

private Q_SLOTS:
	void insertCompletion( const QString &completion);

private:
	void keyPressEvent(QKeyEvent *event);
	QString textUnderCursor() const;
	QCompleter *m_completer;
	QStandardItemModel *m_model;
	QColumnView *m_completerPopup;
};

#endif // AIS_TEXT_EDITOR_H