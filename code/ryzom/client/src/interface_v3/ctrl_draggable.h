#ifndef CTRL_DRAGGABLE_H
#define CTRL_DRAGGABLE_H

#include "ctrl_base.h"

class CCtrlDraggable : public CCtrlBase
{
public:
	CCtrlDraggable( const TCtorParam &param );
	virtual ~CCtrlDraggable(){};

	static CCtrlDraggable *getDraggedSheet(){ return _LastDraggedSheet; }
	bool isDragged() const{ return dragged; }
	void setDragged( bool dragged ){ this->dragged = dragged; }
	bool isDraggable() const{ return draggable; }
	void setDraggable( bool draggable ){ this->draggable = draggable; }
	
	void abortDragging()
	{
		dragged = false;
		_LastDraggedSheet = NULL;
	}


	REFLECT_EXPORT_START(CCtrlDraggable, CCtrlBase)
		REFLECT_BOOL("dragable", isDraggable, setDraggable);
	REFLECT_EXPORT_END

protected:
	static void setDraggedSheet( CCtrlDraggable *draggable ){ _LastDraggedSheet = draggable; }

private:
	static CCtrlDraggable *_LastDraggedSheet;
	bool dragged;
	bool draggable;
};

#endif
