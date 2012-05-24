// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
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

#if !defined(AFX_FOG_DLG_H__5B26E605_00F0_42FD_8893_D1A20AF132F5__INCLUDED_)
#define AFX_FOG_DLG_H__5B26E605_00F0_42FD_8893_D1A20AF132F5__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// fog_dlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CFogDlg dialog

class CFogDlg : public CDialog
{
// Construction
public:
	CFogDlg(CWnd* pParent = NULL);   // standard constructor

	float		getFogStart() const { return m_FogStart; }
	float		getFogEnd() const { return m_FogEnd; }

	void		setFogStart(float fogStart) { m_FogStart = fogStart; }
	void		setFogEnd(float fogEnd) { m_FogEnd = fogEnd; }





// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFogDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CFogDlg)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	// Dialog Data
	//{{AFX_DATA(CFogDlg)
	enum { IDD = IDD_SETUP_FOG };
	float	m_FogStart;
	float	m_FogEnd;
	//}}AFX_DATA	
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FOG_DLG_H__5B26E605_00F0_42FD_8893_D1A20AF132F5__INCLUDED_)
