#include "ais_editor_main_window.h"
#include "ui_ais_editor_main_window.h"

AisEditorMainWindow::AisEditorMainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::AisEditorMainWindow)
{
    ui->setupUi(this);
	 m_textEditor = new CAisTextEdit(this);
	 this->setCentralWidget(m_textEditor);
}

AisEditorMainWindow::~AisEditorMainWindow() 
{
	delete ui;
	delete m_textEditor;
}