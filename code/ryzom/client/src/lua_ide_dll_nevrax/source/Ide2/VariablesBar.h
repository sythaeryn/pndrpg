// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

// VariablesBar.h: interface for the CVariablesBar class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_VARIABLESBAR_H__233488EE_2350_4228_B266_DA29C45596BA__INCLUDED_)
#define AFX_VARIABLESBAR_H__233488EE_2350_4228_B266_DA29C45596BA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CVariablesBar : public CCJControlBar  
{
public:
	void RemoveAll();
	void AddVariable(const char* szName, const char* szType, const char* szValue);
	CVariablesBar();
	virtual ~CVariablesBar();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CVariablesBar)
	//}}AFX_VIRTUAL

// Generated message map functions
protected:
	CCJListCtrl m_variables;

	//{{AFX_MSG(CVariablesBar)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif // !defined(AFX_VARIABLESBAR_H__233488EE_2350_4228_B266_DA29C45596BA__INCLUDED_)
