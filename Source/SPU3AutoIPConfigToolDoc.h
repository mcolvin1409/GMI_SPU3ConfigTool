
// ***********************************************************************
// Author           : Mark C
// Created          : 15-May-2018
//
// ***********************************************************************
// <copyright file="SPU3AutoIPConfigToolDoc.h" company="AVT">
//     Copyright (c) AVT. All rights reserved.
// </copyright>
// ***********************************************************************

// SPU3AutoIPConfigToolDoc.h : interface of the CSPU3AutoIPConfigToolDoc class
//

#pragma once


class CSPU3AutoIPConfigToolDoc : public CDocument
{
protected: // create from serialization only
	CSPU3AutoIPConfigToolDoc();
	DECLARE_DYNCREATE(CSPU3AutoIPConfigToolDoc)

// Attributes
public:

// Operations
public:

// Overrides
public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
#ifdef SHARED_HANDLERS
	virtual void InitializeSearchContent();
	virtual void OnDrawThumbnail(CDC& dc, LPRECT lprcBounds);
#endif // SHARED_HANDLERS

// Implementation
public:
	virtual ~CSPU3AutoIPConfigToolDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()

#ifdef SHARED_HANDLERS
	// Helper function that sets search content for a Search Handler
	void SetSearchContent(const CString& value);
#endif // SHARED_HANDLERS
};
