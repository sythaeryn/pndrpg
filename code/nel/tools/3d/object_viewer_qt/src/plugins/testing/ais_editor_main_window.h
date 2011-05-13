#ifndef AIS_EDITOR_MAIN_WINDOW_H
#define AIS_EDITOR_MAIN_WINDOW_H

#include <QMainWindow>

#include "ais_text_editor.h"

namespace Ui {
    class AisEditorMainWindow;
}

class AisEditorMainWindow : public QMainWindow
{
	Q_OBJECT

public:
    explicit AisEditorMainWindow(QWidget *parent = 0);
    ~AisEditorMainWindow();

private:
    Ui::AisEditorMainWindow *ui;
	CAisTextEdit *m_textEditor;
};

#endif // AIS_EDITOR_MAIN_WINDOW_H