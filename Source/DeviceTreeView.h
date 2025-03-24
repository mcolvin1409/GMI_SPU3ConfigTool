
// ***********************************************************************
// Author           : Mark C
// Created          : 15-May-2018
//
// ***********************************************************************
// <copyright file="DeviceTreeView.h" company="AVT">
//     Copyright (c) AVT. All rights reserved.
// </copyright>
// ***********************************************************************

#pragma once

#include "ViewTree.h"
#include <map>

using namespace std;

class CTreeToolBar : public CMFCToolBar
{
	virtual void OnUpdateCmdUI(CFrameWnd* /*pTarget*/, BOOL bDisableIfNoHndler)
	{
		CMFCToolBar::OnUpdateCmdUI((CFrameWnd*) GetOwner(), bDisableIfNoHndler);
	}

	virtual BOOL AllowShowOnList() const { return FALSE; }
};

class CDeviceTreeView : public CDockablePane
{
public:
	CDeviceTreeView();
	virtual ~CDeviceTreeView();

	void AdjustLayout();
	void OnChangeVisualStyle();
	void Refresh();
	CString GetMACAddress( HTREEITEM hItem );
	CString GetIPAddress( HTREEITEM hItem );
	DWORD GetItemData( HTREEITEM hItem );
	HTREEITEM GetSelectedTreeItem();
	void SelectFirstDevice();
	void GetTreeItemToDeviceMap( std::map< HTREEITEM, std::pair< CString, CString > >& treeItemToDeviceMap );

private:
	CTreeToolBar m_wndToolBar;
	CViewTree m_wndDeviceTreeView;
	CImageList m_treeViewImages;	
	std::map< HTREEITEM, std::pair< CString, CString > > m_treeItemToDeviceMap;

private:	
	void HandleFirstDeviceConfigRecord();
	void AddNoDevicesFoundNode();
	bool IsDuplicateDevice( CString& MACAddress );		

// Overrides
private:
	virtual BOOL PreTranslateMessage(MSG* pMsg);

protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnRefresh();
	afx_msg void OnPaint();
	afx_msg void OnSetFocus(CWnd* pOldWnd);	
	afx_msg LRESULT OnAddDeviceToTree( WPARAM wParam, LPARAM lParam );
	afx_msg void OnTreeItemSelectionChanged( NMHDR* pNMHDR, LRESULT* pResult );
	afx_msg LRESULT OnUpdateTreeItemImage( WPARAM wParam, LPARAM lParam );
	afx_msg LRESULT OnUpdateTreeItemData( WPARAM wParam, LPARAM lParam );

	DECLARE_MESSAGE_MAP()
};

