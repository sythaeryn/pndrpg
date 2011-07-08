

#ifndef SOURCE_SELECTION_H
#define	SOURCE_SELECTION_H

#include <QtCore/QObject>
#include <QtGui/QDialog>
#include <QtCore/QString>
#include <QtGui/QListWidgetItem>
#include "ui_source_selection.h"
#include <map>

using namespace std;

namespace Plugin 
{
        class CSourceDialog : public QDialog
        {
               Q_OBJECT
        private:
            Ui::SourceSelectionDialog _ui;
            
        private Q_SLOTS:

		void OkButtonClicked();
	public:
		CSourceDialog(QWidget *parent = 0);
		~CSourceDialog(){}
                void setSourceOptions(map<QListWidgetItem*, int> options);
                QListWidgetItem *selected_item;

        };    
}


#endif	/* SOURCE_SELECTION_H */

