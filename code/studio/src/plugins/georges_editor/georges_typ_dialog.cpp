#include "georges_typ_dialog.h"
#include "georges.h"

class GeorgesTypDialogPvt
{
public:
	GeorgesTypDialogPvt()
	{
		typ = NULL;
	}

	~GeorgesTypDialogPvt()
	{
		delete typ;
		typ = NULL;
	}


	NLGEORGES::CType *typ;
};

GeorgesTypDialog::GeorgesTypDialog( QWidget *parent ) :
GeorgesDockWidget( parent )
{
	m_ui.setupUi( this );
	m_pvt = new GeorgesTypDialogPvt();
	setupConnections();
}

GeorgesTypDialog::~GeorgesTypDialog()
{
	delete m_pvt;
	m_pvt = NULL;
}


bool GeorgesTypDialog::load( const QString &fileName )
{
	GeorgesQt::CGeorges georges;
	NLGEORGES::UType *utyp = georges.loadFormType( fileName.toUtf8().constData() );
	if( utyp == NULL )
		return false;

	m_pvt->typ = dynamic_cast< NLGEORGES::CType* >( utyp );

	loadTyp();

	return true;
}


void GeorgesTypDialog::write()
{
}

void GeorgesTypDialog::onAddClicked()
{
}

void GeorgesTypDialog::onRemoveClicked()
{
}

void GeorgesTypDialog::setupConnections()
{
	connect( m_ui.addButton, SIGNAL( clicked( bool ) ), this, SLOT( onAddClicked() ) );
	connect( m_ui.removeButton, SIGNAL( clicked( bool ) ), this, SLOT( onRemoveClicked() ) );
}

void GeorgesTypDialog::log( const QString &msg )
{
	QString logMsg = buildLogMsg( msg );
	m_ui.logEdit->appendPlainText( logMsg );
}

void GeorgesTypDialog::loadTyp()
{
	m_ui.logEdit->setPlainText( m_pvt->typ->Header.Log.c_str() );
	m_ui.commentEdit->setPlainText( m_pvt->typ->Header.Comments.c_str() );

	std::vector< NLGEORGES::CType::CDefinition >::iterator itr = m_pvt->typ->Definitions.begin();
	while( itr != m_pvt->typ->Definitions.end() )
	{
		NLGEORGES::CType::CDefinition &def = *itr;

		QTreeWidgetItem *item = new QTreeWidgetItem();
		item->setText( 0, def.Label.c_str() );
		item->setText( 1, def.Value.c_str() );
		m_ui.tree->addTopLevelItem( item );

		++itr;
	}


}

