// ***********************************************************************
// Author           : Mark C
// Created          : 15-May-2018
//
// ***********************************************************************
// <copyright file="SPU3AutoIPConfigToolDoc.cpp" company="AVT">
//     Copyright (c) AVT. All rights reserved.
// </copyright>
// ***********************************************************************

// SPU3AutoIPConfigToolDoc.cpp : implementation of the CSPU3AutoIPConfigToolDoc class
//

#include "stdafx.h"
// SHARED_HANDLERS can be defined in an ATL project implementing preview, thumbnail
// and search filter handlers and allows sharing of document code with that project.
#ifndef SHARED_HANDLERS
#include "SPU3AutoIPConfigTool.h"
#endif

#include "SPU3AutoIPConfigToolDoc.h"

#include <propkey.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CSPU3AutoIPConfigToolDoc

IMPLEMENT_DYNCREATE( CSPU3AutoIPConfigToolDoc, CDocument )

BEGIN_MESSAGE_MAP( CSPU3AutoIPConfigToolDoc, CDocument )
END_MESSAGE_MAP()


// CSPU3AutoIPConfigToolDoc construction/destruction

/// <summary>
/// Initializes a new instance of the <see cref="CSPU3AutoIPConfigToolDoc"/> class.
/// </summary>
CSPU3AutoIPConfigToolDoc::CSPU3AutoIPConfigToolDoc()
{
	// TODO: add one-time construction code here

}

/// <summary>
/// Finalizes an instance of the <see cref="CSPU3AutoIPConfigToolDoc"/> class.
/// </summary>
CSPU3AutoIPConfigToolDoc::~CSPU3AutoIPConfigToolDoc()
{ }

/// <summary>
/// Called when [new document].
/// </summary>
/// <returns></returns>
BOOL CSPU3AutoIPConfigToolDoc::OnNewDocument()
{
	if( !CDocument::OnNewDocument() )
		return FALSE;

	// TODO: add reinitialization code here
	// (SDI documents will reuse this document)

	return TRUE;
}




// CSPU3AutoIPConfigToolDoc serialization

/// <summary>
/// Serializes the specified ar.
/// </summary>
/// <param name="ar">The ar.</param>
void CSPU3AutoIPConfigToolDoc::Serialize( CArchive& ar )
{
	if( ar.IsStoring() )
	{
		// TODO: add storing code here
	}
	else
	{
		// TODO: add loading code here
	}
}

#ifdef SHARED_HANDLERS

// Support for thumbnails
/// <summary>
/// Override this method in a derived class to draw content of thumbnail
/// </summary>
/// <param name="dc">A reference to a device context.</param>
/// <param name="lprcBounds">Specifies a bounding rectangle of area where the thumbnail should be drawn.</param>
void CSPU3AutoIPConfigToolDoc::OnDrawThumbnail( CDC& dc, LPRECT lprcBounds )
{
	// Modify this code to draw the document's data
	dc.FillSolidRect( lprcBounds, RGB( 255, 255, 255 ) );

	CString strText = _T( "TODO: implement thumbnail drawing here" );
	LOGFONT lf;

	CFont* pDefaultGUIFont = CFont::FromHandle( ( HFONT )GetStockObject( DEFAULT_GUI_FONT ) );
	pDefaultGUIFont->GetLogFont( &lf );
	lf.lfHeight = 36;

	CFont fontDraw;
	fontDraw.CreateFontIndirect( &lf );

	CFont* pOldFont = dc.SelectObject( &fontDraw );
	dc.DrawText( strText, lprcBounds, DT_CENTER | DT_WORDBREAK );
	dc.SelectObject( pOldFont );
}

// Support for Search Handlers
/// <summary>
/// Called to initialize search content for Search Handler.
/// </summary>
/// <remarks>
/// You should override this method in a derived class to initialize search content.
/// The content should be a string with parts delimited by ';'. For example, "point; rectangle; ole item".
/// </remarks>
/// search/organize/preview/thumbnail support - search and chunk management
void CSPU3AutoIPConfigToolDoc::InitializeSearchContent()
{
	CString strSearchContent;
	// Set search contents from document's data. 
	// The content parts should be separated by ";"

	// For example:  strSearchContent = _T("point;rectangle;circle;ole object;");
	SetSearchContent( strSearchContent );
}

/// <summary>
/// Sets the content of the search.
/// </summary>
/// <param name="value">The value.</param>
void CSPU3AutoIPConfigToolDoc::SetSearchContent( const CString& value )
{
	if( value.IsEmpty() )
	{
		RemoveChunk( PKEY_Search_Contents.fmtid, PKEY_Search_Contents.pid );
	}
	else
	{
		CMFCFilterChunkValueImpl *pChunk = NULL;
		ATLTRY( pChunk = new CMFCFilterChunkValueImpl );
		if( pChunk != NULL )
		{
			pChunk->SetTextValue( PKEY_Search_Contents, value, CHUNK_TEXT );
			SetChunkValue( pChunk );
		}
	}
}

#endif // SHARED_HANDLERS

// CSPU3AutoIPConfigToolDoc diagnostics

#ifdef _DEBUG
/// <summary>
/// Asserts the valid.
/// </summary>
void CSPU3AutoIPConfigToolDoc::AssertValid() const
{
	CDocument::AssertValid();
}

/// <summary>
/// Dumps the specified dc.
/// </summary>
/// <param name="dc">The dc.</param>
void CSPU3AutoIPConfigToolDoc::Dump( CDumpContext& dc ) const
{
	CDocument::Dump( dc );
}
#endif //_DEBUG


// CSPU3AutoIPConfigToolDoc commands
