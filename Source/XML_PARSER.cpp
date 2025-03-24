// ***********************************************************************
// Author           : Mark C
// Created          : 15-May-2018
//
// ***********************************************************************
// <copyright file="XmlParser.cpp" company="AVT">
//     Copyright (c) AVT. All rights reserved.
// </copyright>
// ***********************************************************************

#include "stdafx.h"

#include <atlbase.h>
using namespace std;

#include "MigrationConstants.h"
#include "XML_PARSER.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// STYLE SHEET CONTENT TO CREATE XML FILE WITH PROPER INTENTATIONS
const tstring XLS_Buffer = TEXT("<?xml version=\"1.0\" encoding=\"UTF-8\"?> \
	<xsl:stylesheet version=\"1.0\" xmlns:xsl=\"http://www.w3.org/1999/XSL/Transform\"> \
	<xsl:output method=\"xml\" indent=\"yes\" encoding=\"UTF-8\" /> \
	<xsl:template match=\"@* | node()\"> \
	<xsl:copy> \
	<xsl:apply-templates select=\"@* | node()\" /> \
	</xsl:copy> \
	</xsl:template> \
	</xsl:stylesheet> ");


/**FDOC*************************************************************************
 *
 *  Function:       CXmlParser::dump_com_error
 *
 *  Author:         F.S.Monica				08-Nov-2005
 *
 *  Description:    To retrieve the error message from _com_error object
 *
 *  Parameters:     e : object contains the error message
 *
 *  Returns:        <None>
 *
 *  Side effects:   <None>
 *
 *  History:		
 *		Jan-2014,Chetan, Modified for UNICODE Support, WI:Mercury Server - Unicode enhancement
 *
 *********************************************************************FDOCEND**/
void CXmlParser::dump_com_error(_com_error &e)
{
	m_strLastError = (CString)(const TCHAR *) ("Error Message = " + _bstr_t (e.ErrorMessage()) + "\nError Description = " + _bstr_t (e.Description()));
}

/**FDOC*************************************************************************
 *
 *  Function:       CXmlParser::CXmlParser
 *
 *  Author:         F.S.Monica				08-Nov-2005
 *
 *  Description:    Constructor - to initialize the CXmlParser class members
 *
 *  Parameters:     <None>
 *
 *  Returns:        <None>
 *
 *  Side effects:   <None>
 *
 *  History:		
 *
 *********************************************************************FDOCEND**/
CXmlParser::CXmlParser()
{
	// Init our members
	this->m_aystrAttribNames.RemoveAll ();
	this->m_aystrAttribValues.RemoveAll ();
	this->m_strTag.Empty ();
	this->m_strName.Empty ();
	this->m_strChainTag.Empty ();
	this->m_strTextField.Empty ();
	this->m_strLastError.Empty ();

	// Init MSXML members
	//
	m_pCurrentNode = NULL;
	m_plDomDocument = NULL;
	m_pDocRoot = NULL;

	m_iAttrib_Index = -1;
}

/**FDOC*************************************************************************
 *
 *  Function:       CXmlParser::~CXmlParser
 *
 *  Author:         F.S.Monica				08-Nov-2005
 *
 *  Description:    Destructor - clears the IXMLDOMDocument pointer
 *
 *  Parameters:     <None>
 *
 *  Returns:        <None>
 *
 *  Side effects:   <None>
 *
 *  History:		
 *
 *********************************************************************FDOCEND**/
CXmlParser::~CXmlParser()
{
	// Free resource
	this->Reset_XML_Document();

	if (m_plDomDocument)
		m_plDomDocument.Release ();
}

/**FDOC*************************************************************************
 *
 *  Function:       CXmlParser::Init_MSXML
 *
 *  Author:         F.S.Monica				08-Nov-2005
 *
 *  Description:    Instantiate an XML document
 *
 *  Parameters:     <None>
 *
 *  Returns:        returns true if XML document intitialization is successful.
 *
 *  Side effects:   <None>
 *
 *  History:		
 *
 *********************************************************************FDOCEND**/
bool CXmlParser::Init_MSXML()
{
	m_strLastError.Empty ();

	// -- Init OLE Object Comunication for MSXML (only once time)--
	static bool ole_initialized = false;
	if(!ole_initialized)
	{
		::AfxOleInit();
		ole_initialized = true;
	}

	// -- Instantiate an XML document --
	if (m_plDomDocument)
		m_plDomDocument.Release ();

	if (m_plDomDocument == NULL)
	{
		HRESULT hr = m_plDomDocument.CreateInstance( MSXML2::CLSID_DOMDocument);
		if (FAILED(hr))
		{
			_com_error er(hr);
			dump_com_error (er);
			return false;
		}
	}

	return true;
}

/**FDOC*************************************************************************
 *
 *  Function:       CXmlParser::Load_XML_Document
 *
 *  Author:         F.S.Monica				08-Nov-2005
 *
 *  Description:    Loads a XML document from file
 *
 *  Parameters:     strFileName : XML file name with path
 *
 *  Returns:        returns true if XML file has been loaded successfully
 *
 *  Side effects:   <None>
 *
 *  History:		25-JUL-11 BEttinger, MT#16998, fixed erroneous code				
 *		Jan-2014,Chetan, Modified for UNICODE Support, WI:Mercury Server - Unicode enhancement
 *
 *********************************************************************FDOCEND**/
bool CXmlParser::Load_XML_Document(LPCTSTR strFileName)
{
	// Reset Document
	this->Reset_XML_Document();

	// Call the IXMLDOMDocumentPtr's load function to load the XML document
	VARIANT_BOOL vResult;
	m_plDomDocument->async = VARIANT_FALSE;
	vResult = m_plDomDocument->load((const TCHAR*)strFileName);

	if(vResult == VARIANT_TRUE)//success
	{
		// Now that the document is loaded, we need to initialize the root pointer
		m_pDocRoot = m_plDomDocument->documentElement;

		this->Go_to_Root();
		return true;
	}
	else //vResult == VARIANT_FALSE or not a valid value
	{
		return false;// XML document is not loaded
	}
}

/**FDOC*************************************************************************
 *
 *  Function:       CXmlParser::Load_XML_From_Buffer
 *
 *  Author:         F.S.Monica				08-Nov-2005
 *
 *  Description:    Loads a XML document from Buffer
 *
 *  Parameters:     strSource - Buffer which hold the XML nodes & data
 *
 *  Returns:        <None>
 *
 *  Side effects:   <None>
 *
 *  History:		
 *
 *********************************************************************FDOCEND**/
void CXmlParser::Load_XML_From_Buffer(LPCTSTR strSource)
{
	// Reset the document
	this->Reset_XML_Document();

	// Load from Buffer
	m_plDomDocument->loadXML(strSource);
	m_pDocRoot = m_plDomDocument->documentElement;

	// Start the Parsing
	Parse_Objects(m_plDomDocument);

	// Move to the root node
	this->Go_to_Root();
}

/**FDOC*************************************************************************
 *
 *  Function:       CXmlParser::Get_XML
 *
 *  Author:         F.S.Monica				08-Nov-2005
 *
 *  Description:    Get the XML Representation of the current node
 *
 *  Parameters:     strBuffer [out] - filled with current node details
 *
 *  Returns:        <None>
 *
 *  Side effects:   <None>
 *
 *  History:		
 *
 *********************************************************************FDOCEND**/
void CXmlParser::Get_XML (CString& strBuffer)
{       
	if (this->m_pCurrentNode == this->m_pDocRoot)
		this->Get_XML_Document (strBuffer);
	else
	{
		BSTR  bstr_xml;
		this->m_pCurrentNode->get_xml (&bstr_xml);

		strBuffer = bstr_xml;
	}
}

/**FDOC*************************************************************************
 *
 *  Function:       CXmlParser::Get_XML_Document
 *
 *  Author:         F.S.Monica				08-Nov-2005
 *
 *  Description:    Get the XML Representation of the entire document
 *
 *  Parameters:     strBuffer [out] - filled with Contents of XML document
 *
 *  Returns:        <None>
 *
 *  Side effects:   <None>
 *
 *  History:		
 *
 *********************************************************************FDOCEND**/
void CXmlParser::Get_XML_Document (CString& strBuffer)
{
	BSTR bstr_xml;
	this->m_plDomDocument->get_xml (&bstr_xml);

	strBuffer = bstr_xml;
}

/**FDOC*************************************************************************
 *
 *  Function:       CXmlParser::Parse_XML_Document
 *
 *  Author:         F.S.Monica				08-Nov-2005
 *
 *  Description:    this method will be redefined in derived class (Not used)
 *
 *  Parameters:     <None>
 *
 *  Returns:        <None>
 *
 *  Side effects:   <None>
 *
 *  History:		
 *
 *********************************************************************FDOCEND**/
void CXmlParser::Parse_XML_Document()
{ /* Nothing to do , this method will be redefined in derived class */ }

/**FDOC*************************************************************************
 *
 *  Function:       CXmlParser::Parse_Objects
 *
 *  Author:         F.S.Monica				08-Nov-2005
 *
 *  Description:    Internal Methods, Parse the Tree nodes
 *
 *  Parameters:     pObject - pointer to the current node
 *
 *  Returns:        <None>
 *
 *  Side effects:   <None>
 *
 *  History:		
 *
 *********************************************************************FDOCEND**/
void CXmlParser::Parse_Objects( MSXML2::IXMLDOMNodePtr pObject )
{
	// Child node
	MSXML2::IXMLDOMNodePtr pChild;

	// Grab Informations from the pObject node
	this->Grab_Node_Informations(pObject);

	// Update "CurrentNode"
	this->m_pCurrentNode = pObject;

	// Call User Parse Handling for let him what he want.
	this->Parse_XML_Document();

	if(Is_MSXML_Node(pObject) != MSXML2::NODE_ELEMENT)
		return;

	// add the ChainTag
	this->ChainTag_Add(this->m_strTag);

	for (pChild = pObject->firstChild;  NULL != pChild;  pChild = pChild->nextSibling)
	{
		// Parse Child nodes
		this->Parse_Objects(pChild);
	}

	// Remove Current Tag from ChainTag
	ChainTag_Remove(1);
}

/**FDOC*************************************************************************
 *
 *  Function:       CXmlParser::Is_Tag
 *
 *  Author:         F.S.Monica				08-Nov-2005
 *
 *  Description:    Return true if it's equal to the Current Tag (including < & >)
 *
 *  Parameters:     strTag - tag
 *
 *  Returns:        Return true if tag names are matching
 *
 *  Side effects:   <None>
 *
 *  History:		
 *
 *********************************************************************FDOCEND**/
bool CXmlParser::Is_Tag (LPCTSTR strTag)
{
	return (this->Get_CurrentTag ().CompareNoCase (strTag) == 0);
}

/**FDOC*************************************************************************
 *
 *  Function:       CXmlParser::Is_Tag
 *
 *  Author:         F.S.Monica				08-Nov-2005
 *
 *  Description:    Return true if it's equal to the Current Tag name
 *
 *  Parameters:     strTagName - tag name
 *
 *  Returns:        Return true if tag names are matching
 *
 *  Side effects:   <None>
 *
 *  History:		
 *
 *********************************************************************FDOCEND**/
bool CXmlParser::Is_Tag_Name (LPCTSTR strTagName)
{
	return (this->Get_CurrentName ().CompareNoCase (strTagName) == 0);
}

/**FDOC*************************************************************************
 *
 *  Function:       CXmlParser::Is_TextNode
 *
 *  Author:         F.S.Monica				08-Nov-2005
 *
 *  Description:    Checks whether it's a TEXT node or not
 *
 *  Parameters:     <None>
 *
 *  Returns:        Return true if it's a TEXT node 
 *
 *  Side effects:   <None>
 *
 *  History:		
 *		Jan-2014,Chetan, Modified for UNICODE Support, WI:Mercury Server - Unicode enhancement
 *
 *********************************************************************FDOCEND**/
bool CXmlParser::Is_TextNode ()
{
	return (this->Is_Tag (TEXT("<#TEXT#>")));
}

/**FDOC*************************************************************************
 *
 *  Function:       CXmlParser::
 *
 *  Author:         F.S.Monica				08-Nov-2005
 *
 *  Description:    Checks whether it is CDATA section or not
 *
 *  Parameters:     <None>
 *
 *  Returns:        Return true if it's a CDATA section 
 *
 *  Side effects:   <None>
 *
 *  History:		
 *		Jan-2014,Chetan, Modified for UNICODE Support, WI:Mercury Server - Unicode enhancement
 *
 *********************************************************************FDOCEND**/
bool CXmlParser::Is_CDataSection ()
{
	return (this->Is_Tag (TEXT("<#CDATA#>")));
}

/**FDOC*************************************************************************
 *
 *  Function:       CXmlParser::Get_CurrentTag
 *
 *  Author:         F.S.Monica				08-Nov-2005
 *
 *  Description:    Get the Current Tag value  (with "<>")
 *
 *  Parameters:     <None>
 *
 *  Returns:        Current Tag value  (with "<>")
 *
 *  Side effects:   <None>
 *
 *  History:		
 *
 *********************************************************************FDOCEND**/
CString& CXmlParser::Get_CurrentTag ()
{
	return (this->m_strTag);
}

/**FDOC*************************************************************************
 *
 *  Function:       CXmlParser::
 *
 *  Author:         F.S.Monica				08-Nov-2005
 *
 *  Description:    Get the Current Name value (without "<>")
 *
 *  Parameters:     <None>
 *
 *  Returns:        the Current Name value (without "<>")
 *
 *  Side effects:   <None>
 *
 *  History:		
 *
 *********************************************************************FDOCEND**/
CString& CXmlParser::Get_CurrentName ()
{
	return (this->m_strName);
}

/**FDOC*************************************************************************
 *
 *  Function:       CXmlParser::
 *
 *  Author:         F.S.Monica				08-Nov-2005
 *
 *  Description:    Test if a Tag is the Root
 *
 *  Parameters:     <None>
 *
 *  Returns:        Returns true if current node is root node
 *
 *  Side effects:   <None>
 *
 *  History:		
 *
 *********************************************************************FDOCEND**/
bool CXmlParser::Is_Root ()
{
	return (this->m_strChainTag.IsEmpty () != 0);
}

/**FDOC*************************************************************************
 *
 *  Function:       CXmlParser::Is_Child_of
 *
 *  Author:         F.S.Monica				08-Nov-2005
 *
 *  Description:    Test if a chain Tag is the parent of current Tag
 *
 *  Parameters:     strParent_chain - parent tag to be tested
 *
 *  Returns:        return true if a chain Tag is the parent of current Tag
 *
 *  Side effects:   <None>
 *
 *  History:		
 *		Jan-2014,Chetan, Modified for UNICODE Support, WI:Mercury Server - Unicode enhancement
 *
 *********************************************************************FDOCEND**/
bool CXmlParser::Is_Child_of(LPCTSTR strParent_chain)
{
	int iPos = this->m_strChainTag.Find (strParent_chain);
	int iLen = _tcslen(strParent_chain);

	if(iPos != -1)
	{       
		// look if it's the end of this chainTag
		return ( this->m_strChainTag.GetLength() == iPos + iLen );
	}

	return false;
}

/**FDOC*************************************************************************
 *
 *  Function:       CXmlParser::Get_TextValue
 *
 *  Author:         F.S.Monica				08-Nov-2005
 *
 *  Description:    Get the Text Value when it's a TEXT or CDATA node
 *
 *  Parameters:     <None>
 *
 *  Returns:        return the text value
 *
 *  Side effects:   <None>
 *
 *  History:		
 *
 *********************************************************************FDOCEND**/
CString& CXmlParser::Get_TextValue()
{
	// Now you can handle the text value on the real node directly if you want
	if((Is_MSXML_Node (this->m_pCurrentNode) == MSXML2::NODE_TEXT) || (Is_MSXML_Node (this->m_pCurrentNode) == MSXML2::NODE_CDATA_SECTION))
		return this->m_strTextField;

	// We must find his text value
	CString strTextValue;

	// Find if a CHILD TEXT NODE exist or not
	if (m_pCurrentNode == NULL)
	{
		if (this->m_pDocRoot != NULL)
			this->m_pCurrentNode = this->m_pDocRoot;
		else
		{
			// We can't set the text
			this->m_strTextField = "";
			return this->m_strTextField;
		}
	}

	// Find it now
	if(this->m_pCurrentNode->hasChildNodes ())
	{
		MSXML2::IXMLDOMNodePtr pChild;
		for(pChild = this->m_pCurrentNode->firstChild; pChild != NULL; pChild = pChild->nextSibling)
		{
			// Find it's a NODE TEXT
			if(this->Is_MSXML_Node (pChild) == MSXML2::NODE_TEXT)
			{
				// Take informations from this Text Node
				this->Grab_Node_Informations (pChild);
				strTextValue = this->m_strTextField;
				this->Grab_Node_Informations (this->m_pCurrentNode); // it remove this->m_strTextField
				this->m_strTextField = strTextValue;
				return (this->m_strTextField);
			}
		}
	}

	this->m_strTextField = "";
	return (this->m_strTextField);
}

/**FDOC*************************************************************************
 *
 *  Function:       CXmlParser::Is_MSXML_Node
 *
 *  Author:         F.S.Monica				08-Nov-2005
 *
 *  Description:    To check the type of node
 *
 *  Parameters:     Node to be tested
 *
 *  Returns:        Return NULL,NODE_ELEMENT,NODE_TEXT or NODE_CDATA_SECTION
 *
 *  Side effects:   <None>
 *
 *  History:		
 *
 *********************************************************************FDOCEND**/
HRESULT CXmlParser::Is_MSXML_Node(MSXML2::IXMLDOMNodePtr pChild)
{
	if(pChild == NULL)
		return NULL;

	// I handle only few node type other are ignored and are considered as NODE_ELEMENT
	//
	// Handled Node type by this wrapper:
	//      - NODE_ELEMENT
	//      - NODE_TEXT
	//      - NODE_CDATA_SECTION
	//      - NODE_PROCESSING_INSTRUCTION

	if(pChild->nodeType == MSXML2::NODE_TEXT)
		return MSXML2::NODE_TEXT;

	if(pChild->nodeType == MSXML2::NODE_CDATA_SECTION)
		return MSXML2::NODE_CDATA_SECTION;

	if(pChild->nodeType == MSXML2::NODE_PROCESSING_INSTRUCTION )
		return MSXML2::NODE_PROCESSING_INSTRUCTION;

	return MSXML2::NODE_ELEMENT;
}

/**FDOC*************************************************************************
 *
 *  Function:       CXmlParser::Grab_Node_Informations
 *
 *  Author:         F.S.Monica				08-Nov-2005
 *
 *  Description:    Grab all eficients informations about a MSXML Node
 *
 *  Parameters:     <None>
 *
 *  Returns:        <None>
 *
 *  Side effects:   <None>
 *
 *  History:		
 *		Jan-2014,Chetan, Modified for UNICODE Support, WI:Mercury Server - Unicode enhancement
 *
 *********************************************************************FDOCEND**/
void CXmlParser::Grab_Node_Informations(MSXML2::IXMLDOMNodePtr pChild)
{
	if(this->Is_MSXML_Node(pChild) == MSXML2::NODE_ELEMENT)
	{
		// It's a node

		// Tag Update
		m_strTag.Format(TEXT("<%s>"),(const TCHAR*)(pChild->nodeName));
		m_strName = (const TCHAR*)(pChild->nodeName);

		// TextField no significant value
		m_strTextField.Empty();

		// Update Attribute List
		this->m_aystrAttribNames.RemoveAll();
		this->m_aystrAttribValues.RemoveAll();

		MSXML2::IXMLDOMNamedNodeMapPtr pAttribs = pChild->Getattributes();
		if(pAttribs != NULL)
		{
			long nAttriCnt = pAttribs->Getlength();
			// Iterate over the attributes
			for(int i = 0; i < nAttriCnt; ++i)
			{
				MSXML2::IXMLDOMNodePtr pAttrib = pAttribs->Getitem(i);
				if(pAttrib != NULL)
				{
					this->m_aystrAttribNames.Add ((CString)(const TCHAR *) pAttrib->GetnodeName());

					_variant_t vVal = pAttrib->GetnodeValue ( );
					this->m_aystrAttribValues.Add ((CString)(const TCHAR *) _bstr_t(vVal));
				}
			}
		}
	}
	else if (this->Is_MSXML_Node(pChild) == MSXML2::NODE_TEXT)
	{
		// Tag is #TEXT#
		m_strTag.Empty();
		m_strTag = "<#TEXT#>";
		m_strName.Empty();
		m_strName = "#TEXT#";

		// TextField Update
		m_strTextField = (const TCHAR*)(pChild->text);

		// Update Attribute List have no means
		this->m_aystrAttribNames.RemoveAll();
		this->m_aystrAttribValues.RemoveAll();
	}
	else if (this->Is_MSXML_Node(pChild) == MSXML2::NODE_CDATA_SECTION)
	{
		// Tag is #CDATA#
		m_strTag.Empty();
		m_strTag = "<#CDATA#>";
		m_strName.Empty();
		m_strName = "#CDATA#";

		// TextField Update
		m_strTextField = (const TCHAR*)(pChild->text);

		// Update Attribute List have no means
		this->m_aystrAttribNames.RemoveAll();
		this->m_aystrAttribValues.RemoveAll();
	}
	else if (this->Is_MSXML_Node(pChild) == MSXML2::NODE_PROCESSING_INSTRUCTION)
	{
		/* Do nothing here */
	} 

	return;
}

/**FDOC*************************************************************************
 *
 *  Function:       CXmlParser::ChainTag_Add
 *
 *  Author:         F.S.Monica				08-Nov-2005
 *
 *  Description:    Add a tag to the Chain array
 *
 *  Parameters:     strVal : Tag to be added into the chain array
 *
 *  Returns:        <None>
 *
 *  Side effects:   <None>
 *
 *  History:		
 *
 *********************************************************************FDOCEND**/
void CXmlParser::ChainTag_Add(CString& strVal)
{
	// Add a tag to the ChainTag
	if(!m_strChainTag.IsEmpty())
		m_strChainTag += strVal;
	else
		m_strChainTag = strVal;
}

/**FDOC*************************************************************************
 *
 *  Function:       CXmlParser::ChainTag_Remove
 *
 *  Author:         F.S.Monica				08-Nov-2005
 *
 *  Description:    delete a tag from the Chain array
 *
 *  Parameters:     <None>
 *
 *  Returns:        iNumber - Tag number to be deleted
 *
 *  Side effects:   <None>
 *
 *  History:		
 *
 *********************************************************************FDOCEND**/
void CXmlParser::ChainTag_Remove(int iNumber)
{
	// Remove the n tag to the ChainTag
	for(int bcl = 0; bcl < iNumber; bcl ++)
	{
		int iPos = m_strChainTag.ReverseFind('<');
		if(iPos == -1)
			m_strChainTag.Empty();
		else
			m_strChainTag = m_strChainTag.Left(iPos);
	}
}

/**FDOC*************************************************************************
 *
 *  Function:       CXmlParser::Get_Attribute_Count
 *
 *  Author:         F.S.Monica				08-Nov-2005
 *
 *  Description:    To get the number of attributes for the current node
 *
 *  Parameters:     <None>
 *
 *  Returns:        Return the number of attributes for the current node
 *
 *  Side effects:   <None>
 *
 *  History:		
 *
 *********************************************************************FDOCEND**/
int CXmlParser::Get_Attribute_Count()
{
	return this->m_aystrAttribNames.GetSize();
}

/**FDOC*************************************************************************
 *
 *  Function:       CXmlParser::Get_Attribute_Name
 *
 *  Author:         F.S.Monica				08-Nov-2005
 *
 *  Description:    To get the attribute name for the nth attribute
 *
 *  Parameters:     iIndex - attribute Index
 *
 *  Returns:        Return the attribute name for the nth attribute
 *
 *  Side effects:   <None>
 *
 *  History:		
 *
 *********************************************************************FDOCEND**/
CString& CXmlParser::Get_Attribute_Name(int iIndex)
{
	if((iIndex < 0) || (iIndex > this->Get_Attribute_Count()))
	{
		m_strTmp.Empty();
		return m_strTmp;
	}

	return this->m_aystrAttribNames[iIndex]; 
}

/**FDOC*************************************************************************
 *
 *  Function:       CXmlParser::Get_Attribute_Value
 *
 *  Author:         F.S.Monica				08-Nov-2005
 *
 *  Description:    Get the attribute value for the nth attribute
 *
 *  Parameters:     iIndex - attribute Index
 *
 *  Returns:        Return the attribute value for the nth attribute
 *
 *  Side effects:   <None>
 *
 *  History:		
 *
 *********************************************************************FDOCEND**/
CString& CXmlParser::Get_Attribute_Value(int iIndex)
{
	if(iIndex < 0 || iIndex > this->Get_Attribute_Count())
	{   
		m_strTmp.Empty();
		return m_strTmp;
	}

	m_iAttrib_Index = iIndex;
	return this->m_aystrAttribValues[m_iAttrib_Index];
}

/**FDOC*************************************************************************
 *
 *  Function:       CXmlParser::Is_Having_Attribute
 *
 *  Author:         F.S.Monica				08-Nov-2005
 *
 *  Description:    Checks whether the current node have the specified attribute defined
 *
 *  Parameters:     strName - Attribute name
 *
 *  Returns:        Return true if current node have the specified attribute defined
 *
 *  Side effects:   <None>
 *
 *  History:		
 *
 *********************************************************************FDOCEND**/
bool CXmlParser::Is_Having_Attribute(LPCTSTR strName)
{
	// Create the CString strName Object
	CString strAttribName = strName;

	// Clear attribute index
	m_iAttrib_Index = -1;

	int bcl;
	for(bcl = 0; bcl < this->Get_Attribute_Count(); bcl++)
	{
		// Check if the name is equal
		if(this->m_aystrAttribNames[bcl].CompareNoCase (strAttribName) == 0)
		{
			// set index fot let user to retrieve value with "Get_Attribute_Value()" method
			m_iAttrib_Index = bcl;
			return true;
		}
	}
	return false;
}

/**FDOC*************************************************************************
 *
 *  Function:       CXmlParser::Get_Attribute_Value
 *
 *  Author:         F.S.Monica				08-Nov-2005
 *
 *  Description:    Return the attribute value selected by "Is_Having_Attribute()"
 *
 *  Parameters:     <None>
 *
 *  Returns:        Return the value of the attribute 
 *
 *  Side effects:   <None>
 *
 *  History:		
 *
 *********************************************************************FDOCEND**/
CString & CXmlParser::Get_Attribute_Value()
{
	if(m_iAttrib_Index != -1)
		return this->m_aystrAttribValues[m_iAttrib_Index];

	// We can't retrieve a Attribute values
	m_strTmp.Empty();
	return m_strTmp;
}

/**FDOC*************************************************************************
 *
 *  Function:       CXmlParser::Get_Attribute_Value
 *
 *  Author:         F.S.Monica				08-Nov-2005
 *
 *  Description:    To get the value of a specified attribute
 *
 *  Parameters:     strName [in] - Attribute name
 *					strValue [out] - Attribute's value
 *
 *  Returns:        Returns true if the specified attribute is found
 *
 *  Side effects:   <None>
 *
 *  History:		
 *
 *********************************************************************FDOCEND**/
bool CXmlParser::Get_Attribute_Value (LPCTSTR strName, CString &strValue)
{
	bool bResult = false;
	if (Is_Having_Attribute (strName))
	{
		strValue = Get_Attribute_Value ();
		bResult = true;
	}
	return bResult;
}

/**FDOC*************************************************************************
 *
 *  Function:       CXmlParser::_add_lastchild
 *
 *  Author:         F.S.Monica				08-Nov-2005
 *
 *  Description:    Add a node as last chid to the current node
 *
 *  Parameters:     pNewNode [in] - new node details
 *
 *  Returns:        true - if successful
 *
 *  Side effects:   <None>
 *
 *  History:		
 *		Jan-2014,Chetan, Modified for UNICODE Support, WI:Mercury Server - Unicode enhancement
 *
 *********************************************************************FDOCEND**/
bool CXmlParser::_add_lastchild(MSXML2::IXMLDOMNodePtr pNewNode)
{
	// Attach the Node to the document
	if(m_pCurrentNode != NULL)
	{
		if(Is_MSXML_Node(this->m_pCurrentNode) != MSXML2::NODE_ELEMENT)
			return false;
		this->ChainTag_Add( this->Get_CurrentTag() );
		pNewNode = m_pCurrentNode->appendChild(pNewNode);
	}
	else if(m_pDocRoot != NULL)
	{
		if(Is_MSXML_Node(this->m_pDocRoot) != MSXML2::NODE_ELEMENT)
			return false;
		this->ChainTag_Add( (CString)(const TCHAR*)m_pDocRoot->nodeName );
		pNewNode = m_pDocRoot->appendChild(pNewNode);
	}
	else
	{
		this->m_strChainTag.Empty();
		m_pDocRoot = pNewNode;
		m_plDomDocument->documentElement = m_pDocRoot;
	}

	// Update Current Node (cast operation)
	m_pCurrentNode = pNewNode;

	// Update information for this Node
	this->Grab_Node_Informations(m_pCurrentNode);

	return true;
}

/**FDOC*************************************************************************
 *
 *  Function:       CXmlParser::_add_firstchild
 *
 *  Author:         F.S.Monica				08-Nov-2005
 *
 *  Description:    Add a node as first child to the current node
 *
 *  Parameters:     pNewNode [in] - new node details
 *
 *  Returns:        true - if successful
 *
 *  Side effects:   <None>
 *
 *  History:		
 *		Jan-2014,Chetan, Modified for UNICODE Support, WI:Mercury Server - Unicode enhancement
 *
 *********************************************************************FDOCEND**/
bool CXmlParser::_add_firstchild(MSXML2::IXMLDOMNodePtr pNewNode)
{
	// Create Reference Node for the Insertion
	_variant_t NodeRef = (IUnknown *)m_pCurrentNode->firstChild;

	// Attach the Node to the document
	if(m_pCurrentNode != NULL)
	{
		if(Is_MSXML_Node(this->m_pCurrentNode) != MSXML2::NODE_ELEMENT)
			return false;
		this->ChainTag_Add( this->Get_CurrentTag() );
		pNewNode = m_pCurrentNode->insertBefore(pNewNode,NodeRef);
	}
	else if(m_pDocRoot != NULL)
	{
		if(Is_MSXML_Node(this->m_pDocRoot) != MSXML2::NODE_ELEMENT)
			return false;
		this->ChainTag_Add( (CString)(const TCHAR*)m_pDocRoot->nodeName );
		pNewNode = m_pDocRoot->insertBefore(pNewNode,NodeRef);
	}
	else
	{
		this->m_strChainTag.Empty();
		m_pDocRoot = pNewNode;
		m_plDomDocument->documentElement = m_pDocRoot;
	}

	// Update Current Node (cast operation)
	m_pCurrentNode = pNewNode;

	// Update information for this Node
	this->Grab_Node_Informations(m_pCurrentNode);

	return true;
}

/**FDOC*************************************************************************
 *
 *  Function:       CXmlParser::_add_before
 *
 *  Author:         F.S.Monica				08-Nov-2005
 *
 *  Description:    Add a node just before the current node
 *
 *  Parameters:     pNewNode - new node details
 *
 *  Returns:        true - if successful
 *
 *  Side effects:   <None>
 *
 *  History:		
 *
 *********************************************************************FDOCEND**/
bool CXmlParser::_add_before(MSXML2::IXMLDOMNodePtr pNewNode)
{
	if(this->Is_Root())
	{
		return false;
	}

	// Create Reference Node for the Insertion
	MSXML2::IXMLDOMNodePtr pParent;
	this->m_pCurrentNode->get_parentNode(&pParent);
	_variant_t NodeRef = (IUnknown *)m_pCurrentNode;


	// Attach the Node to the document
	if(m_pCurrentNode != NULL)
		pNewNode = pParent->insertBefore(pNewNode,NodeRef);
	else if(m_pDocRoot != NULL)
		pNewNode = m_pDocRoot->insertBefore(pNewNode,NodeRef);
	else
	{
		m_pDocRoot = pNewNode;
		m_plDomDocument->documentElement = m_pDocRoot;
	}

	// Update Current Node (cast operation)
	m_pCurrentNode = pNewNode;

	// Update information for this Node
	this->Grab_Node_Informations(m_pCurrentNode);

	return true;
}

/**FDOC*************************************************************************
 *
 *  Function:       CXmlParser::_add_after
 *
 *  Author:         F.S.Monica				08-Nov-2005
 *
 *  Description:    Adds a node juszt after the current node
 *
 *  Parameters:     pNewNode - new node details
 *
 *  Returns:        true - if successful
 *
 *  Side effects:   <None>
 *
 *  History:		
 *
 *********************************************************************FDOCEND**/
bool CXmlParser::_add_after(MSXML2::IXMLDOMNodePtr pNewNode)
{
	// If m_pCurrentNode->NextSibling == NULL then must call Add_LastChildNode on Parent Node
	// Because we can't use InsertBefore on a NULL Reference ;o)
	//
	// We are sure that a Parent node exist because before we control that we aren't not on root node.
	//
	if(m_pCurrentNode->nextSibling == NULL)
	{
		// Get Parent Node
		MSXML2::IXMLDOMNodePtr pParent;
		this->m_pCurrentNode->get_parentNode (&pParent);

		// Set Parent node as Current Node
		this->m_pCurrentNode = pParent;
		this->Grab_Node_Informations (pParent);

		// Add Node as Last Child Node
		return (this->_add_lastchild (pNewNode));
	}

	// Create Reference Node for the Insertion
	MSXML2::IXMLDOMNodePtr pParent;
	this->m_pCurrentNode->get_parentNode (&pParent);
	_variant_t NodeRef = (IUnknown *)m_pCurrentNode->nextSibling;

	// Attach the Node to the document
	if(m_pCurrentNode != NULL)
		pNewNode = pParent->insertBefore (pNewNode, NodeRef);
	else if(m_pDocRoot != NULL)
		pNewNode = m_pDocRoot->insertBefore (pNewNode, NodeRef);
	else
	{
		m_pDocRoot = pNewNode;
		m_plDomDocument->documentElement = m_pDocRoot;
	}

	// Update Current Node (cast operation)
	m_pCurrentNode = pNewNode;

	// Update information for this Node
	this->Grab_Node_Informations (m_pCurrentNode);

	return true;
}

/**FDOC*************************************************************************
 *
 *  Function:       CXmlParser::Add_LastChildCData
 *
 *  Author:         F.S.Monica				08-Nov-2005
 *
 *  Description:    Add a new CDATA Section at the End of Current Node
 *
 *  Parameters:     strData - data to be added
 *
 *  Returns:        true - if successful
 *
 *  Side effects:   <None>
 *
 *  History:		
 *
 *********************************************************************FDOCEND**/
bool CXmlParser::Add_LastChildCData(LPCTSTR strData)
{
	// Nodes members
	MSXML2::IXMLDOMNodePtr pNewNode;

	CComVariant vType(MSXML2::NODE_CDATA_SECTION);
	pNewNode = m_plDomDocument->createNode(vType, "", "");

	// Attach the Node to the document
	bool Result = this->_add_lastchild(pNewNode);
	this->Set_TextValue(strData);

	return Result;
}

/**FDOC*************************************************************************
 *
 *  Function:       CXmlParser::Add_FirstChildCData
 *
 *  Author:         F.S.Monica				08-Nov-2005
 *
 *  Description:    Add a new CDATA Section at the Begining of Current Node
 *
 *  Parameters:     strData - data to be added
 *
 *  Returns:        true - if successful
 *
 *  Side effects:   <None>
 *
 *  History:		
 *
 *********************************************************************FDOCEND**/
bool CXmlParser::Add_FirstChildCData(LPCTSTR strData)
{
	// Nodes members
	MSXML2::IXMLDOMNodePtr pNewNode;

	// If no child then use Add_LastChildCData or CurrentElement node not set yet
	if(this->m_pCurrentNode != NULL)
	{
		if( !this->m_pCurrentNode->hasChildNodes() )
			return (this->Add_LastChildCData(strData));
	}
	else
		return (this->Add_LastChildCData(strData));

	// Create the NODE
	CComVariant vType(MSXML2::NODE_CDATA_SECTION);
	pNewNode = m_plDomDocument->createNode(vType, "", "");


	bool Result = this->_add_firstchild(pNewNode);
	this->Set_TextValue(strData);

	return Result;
}

/**FDOC*************************************************************************
 *
 *  Function:       CXmlParser::Add_CDataBefore
 *
 *  Author:         F.S.Monica				08-Nov-2005
 *
 *  Description:    Add a new CDATA Section (same level) before the Current Node
 *
 *  Parameters:     strData - data to be added
 *
 *  Returns:        true - if successful
 *
 *  Side effects:   <None>
 *
 *  History:		
 *
 *********************************************************************FDOCEND**/
bool CXmlParser::Add_CDataBefore(LPCTSTR strData)
{
	MSXML2::IXMLDOMNodePtr pNewNode;

	// Can't use this function on the root node
	if(this->Is_Root())
		return false;

	// Create the NODE
	CComVariant vType(MSXML2::NODE_CDATA_SECTION);
	pNewNode = m_plDomDocument->createNode(vType, "", "");

	bool Result = this->_add_before(pNewNode);
	this->Set_TextValue(strData);

	return Result;
}

/**FDOC*************************************************************************
 *
 *  Function:       CXmlParser::Add_CDataAfter
 *
 *  Author:         F.S.Monica				08-Nov-2005
 *
 *  Description:    Add a new CDATA Section (same levelà after the Current Node
 *
 *  Parameters:     strData - data to be added
 *
 *  Returns:        true - if successful
 *
 *  Side effects:   <None>
 *
 *  History:		
 *
 *********************************************************************FDOCEND**/
bool CXmlParser::Add_CDataAfter(LPCTSTR strData)
{
	// Nodes members
	MSXML2::IXMLDOMNodePtr pNewNode;

	// Can't use this function on the root node
	if(this->Is_Root())
	{
		return false;
	}

	// Create the NODE
	CComVariant vType(MSXML2::NODE_CDATA_SECTION);
	pNewNode = m_plDomDocument->createNode(vType, "", "");

	bool Result = this->_add_after(pNewNode);
	this->Set_TextValue(strData);

	return Result;
}

/**FDOC*************************************************************************
 *
 *  Function:       CXmlParser::Add_LastChildNode
 *
 *  Author:         F.S.Monica				08-Nov-2005
 *
 *  Description:    Add a new CHILD NODE at the End of Current Node
 *
 *  Parameters:     strName - node name to be added
 *
 *  Returns:        true - if successful
 *
 *  Side effects:   <None>
 *
 *  History:		
 *
 *********************************************************************FDOCEND**/
bool CXmlParser::Add_LastChildNode (LPCTSTR strName)
{
	// Nodes members
	MSXML2::IXMLDOMNodePtr pNewNode;

	// Create NODE TYPE
	CComVariant vType (MSXML2::NODE_ELEMENT);

	// Create the NODE
	pNewNode = m_plDomDocument->createNode (vType, strName, "");

	// Attach the Node to the document
	return this->_add_lastchild (pNewNode);
}

/**FDOC*************************************************************************
 *
 *  Function:       CXmlParser::Add_NodeBefore
 *
 *  Author:         F.S.Monica				08-Nov-2005
 *
 *  Description:    Add a new NODE (same level) before the Current Node
 *
 *  Parameters:     strName - node name to be added
 *
 *  Returns:        true - if successful
 *
 *  Side effects:   <None>
 *
 *  History:		
 *
 *********************************************************************FDOCEND**/
bool CXmlParser::Add_NodeBefore(LPCTSTR strName)
{
	MSXML2::IXMLDOMNodePtr pNewNode;

	// Can't use this function on the root node
	if(this->Is_Root())
		return false;
	// Create NODE TYPE
	CComVariant vType(MSXML2::NODE_ELEMENT);

	// Create the NODE
	pNewNode = m_plDomDocument->createNode(vType, strName, "");

	return this->_add_before(pNewNode);
}

/**FDOC*************************************************************************
 *
 *  Function:       CXmlParser::Add_NodeAfter
 *
 *  Author:         F.S.Monica				08-Nov-2005
 *
 *  Description:    Add a new NODE (same level) after the Current Node
 *
 *  Parameters:     strName - node name to be added
 *
 *  Returns:        true - if successful
 *
 *  Side effects:   <None>
 *
 *  History:		
 *
 *********************************************************************FDOCEND**/
bool CXmlParser::Add_NodeAfter(LPCTSTR strName)
{
	MSXML2::IXMLDOMNodePtr pNewNode;

	// Can't use this function on the root node
	if(this->Is_Root())
	{
		return false;
	}

	// Create NODE TYPE
	CComVariant vType (MSXML2::NODE_ELEMENT);

	// Create the NODE
	pNewNode = m_plDomDocument->createNode (vType, strName, "");

	return this->_add_after (pNewNode);
}

/**FDOC*************************************************************************
 *
 *  Function:       CXmlParser::Add_FirstChildNode
 *
 *  Author:         F.S.Monica				08-Nov-2005
 *
 *  Description:    Add a new CHILD NODE at the Begining of Current Node
 *
 *  Parameters:     strName - node name to be added
 *
 *  Returns:        true - if successful
 *
 *  Side effects:   <None>
 *
 *  History:		
 *
 *********************************************************************FDOCEND**/
bool CXmlParser::Add_FirstChildNode(LPCTSTR strName)
{
	MSXML2::IXMLDOMNodePtr pNewNode;

	// If no child then use Add_LastChildNode or CurrentElement node not set yet
	if(m_pCurrentNode != NULL)
	{
		if( !this->m_pCurrentNode->hasChildNodes() )
			return (this->Add_LastChildNode (strName));
	}
	else
		return (this->Add_LastChildNode (strName));

	// Create NODE TYPE
	CComVariant vType(MSXML2::NODE_ELEMENT);

	// Create the NODE
	pNewNode = m_plDomDocument->createNode (vType, strName, "");

	return this->_add_firstchild (pNewNode);
}

/**FDOC*************************************************************************
 *
 *  Function:       CXmlParser::Set_Attribute
 *
 *  Author:         F.S.Monica				08-Nov-2005
 *
 *  Description:    Set an attribute to the Current Node
 *
 *  Parameters:     strAttribName - Attribute name
 *					strAttribValue - Attribute value
 *
 *  Returns:        true - if successful
 *
 *  Side effects:   <None>
 *
 *  History:		
 *
 *********************************************************************FDOCEND**/
bool CXmlParser::Set_Attribute(LPCTSTR strAttribName, LPCTSTR strAttribValue)
{
	// Nodes members
	MSXML2::IXMLDOMElementPtr pCurrentElementNode = m_pCurrentNode;

	// Create Attribute variable
	CComVariant vAttribute(strAttribValue);

	// Set the new attribute
	if(m_pCurrentNode != NULL)
	{
		pCurrentElementNode->setAttribute( strAttribName, vAttribute );
		m_pCurrentNode = pCurrentElementNode;
		this->Grab_Node_Informations(m_pCurrentNode);
	}
	else
	{
		m_pDocRoot->setAttribute( strAttribName, vAttribute );
		this->Grab_Node_Informations(m_pDocRoot);
	}

	return true;
}

/**FDOC*************************************************************************
 *
 *  Function:       CXmlParser::Set_Attribute
 *
 *  Author:         F.S.Monica				08-Nov-2005
 *
 *  Description:    Set an attribute to the Current Node
 *
 *  Parameters:     strAttribName - Attribute name
 *					strAttribValue - Attribute value
 *
 *  Returns:        true - if successful
 *
 *  Side effects:   <None>
 *
 *  History:		
 *
 *********************************************************************FDOCEND**/
bool CXmlParser::Set_Attribute(LPCTSTR strAttribName, CComVariant strAttribValue)
{
	// Nodes members
	MSXML2::IXMLDOMElementPtr pCurrentElementNode = m_pCurrentNode;

	// Create Attribute variable
	CComVariant vAttribute(strAttribValue);

	// Set the new attribute
	if(m_pCurrentNode != NULL)
	{
		pCurrentElementNode->setAttribute (strAttribName, vAttribute);
		m_pCurrentNode = pCurrentElementNode;
		this->Grab_Node_Informations (m_pCurrentNode);
	}
	else
	{
		m_pDocRoot->setAttribute (strAttribName, vAttribute);
		this->Grab_Node_Informations (m_pDocRoot);
	}

	return true;
}

/**FDOC*************************************************************************
 *
 *  Function:       CXmlParser::Remove_Attribute
 *
 *  Author:         F.S.Monica				08-Nov-2005
 *
 *  Description:    Remove an attribute the the Current Node
 *
 *  Parameters:     strAttribName - Attribute name
 *
 *  Returns:        true - if successful
 *
 *  Side effects:   <None>
 *
 *  History:		
 *
 *********************************************************************FDOCEND**/
bool CXmlParser::Remove_Attribute(LPCTSTR strAttribName)
{
	if(m_pCurrentNode == NULL)
	{
		if(this->m_pDocRoot != NULL)
			this->m_pCurrentNode = this->m_pDocRoot;
		else
		{
			return false;
		}
	}

	MSXML2::IXMLDOMElementPtr pCurrentElementNode = m_pCurrentNode;
	pCurrentElementNode->removeAttribute (strAttribName);
	m_pCurrentNode = pCurrentElementNode;

	return true;
}

/**FDOC*************************************************************************
 *
 *  Function:       CXmlParser::Set_TextValue
 *
 *  Author:         F.S.Monica				08-Nov-2005
 *
 *  Description:    Set a Text to the Current Node
 *
 *  Parameters:     strTextValue [in] - text value
 *
 *  Returns:        true - if successful
 *
 *  Side effects:   <None>
 *
 *  History:		
 *
 *********************************************************************FDOCEND**/
bool CXmlParser::Set_TextValue(LPCTSTR strTextValue)
{
	MSXML2::IXMLDOMNodePtr pNewNode;

	// Find if a CHILD TEXT NODE exist or not
	if(m_pCurrentNode == NULL)
	{
		if(this->m_pDocRoot != NULL)
			this->m_pCurrentNode = this->m_pDocRoot;
		else
		{
			return false;
		}
	}

	if(this->Is_MSXML_Node(this->m_pCurrentNode) == MSXML2::NODE_CDATA_SECTION)
	{
		CComVariant sValue(strTextValue);
		this->m_pCurrentNode->put_nodeValue(sValue);

		return true;
	}

	// Rq: a NODE_CDATA_SECTION can't have any childs
	if(this->m_pCurrentNode->hasChildNodes())
	{
		MSXML2::IXMLDOMNodePtr pChild;
		for(pChild = this->m_pCurrentNode->firstChild; pChild != NULL; pChild = pChild->nextSibling)
		{
			// Find it's a NODE TEXT
			if(this->Is_MSXML_Node(pChild) == MSXML2::NODE_TEXT)
			{
				// A Text Node is found, Replace it now!!
				CComVariant sValue(strTextValue);
				pChild->put_nodeValue(sValue);

				return true;
			}
		}
	}

	// No previous Text was defined before, we can add it.
	if(this->Is_MSXML_Node(this->m_pCurrentNode) == MSXML2::NODE_ELEMENT)
	{
		// Create NODE TEXT type
		m_pCurrentNode->text = strTextValue;
	}

	return true;
}

/**FDOC*************************************************************************
 *
 *  Function:       CXmlParser::Save_XML_Document
 *
 *  Author:         F.S.Monica				08-Nov-2005
 *
 *  Description:    Save the Current XML Document into a file
 *
 *  Parameters:     strFileName - file name to save XML data
 *
 *  Returns:        true - if successful
 *
 *  Side effects:   <None>
 *
 *  History:		20-Dec-2006 ABaranwal, Modified to return false if saving fails
 *
 *********************************************************************FDOCEND**/
bool CXmlParser::Save_XML_Document(LPCTSTR strFileName)
{
	// Format the output : XML document 
	MSXML2::IXMLDOMDocumentPtr pXSLIndenter (MSXML2::CLSID_DOMDocument);
	HRESULT hr = S_OK;

	pXSLIndenter->async = VARIANT_FALSE;

	hr = pXSLIndenter->loadXML (XLS_Buffer.c_str());

	if (hr == VARIANT_TRUE)
	{
		IDispatch* pDisp;
		HRESULT _hr = m_plDomDocument->QueryInterface (IID_IDispatch, (void **)&pDisp);
		ASSERT (SUCCEEDED(_hr));
		VARIANT vObject;
		vObject.vt = VT_DISPATCH; 
		vObject.pdispVal = pDisp;

		hr = m_plDomDocument->transformNodeToObject (pXSLIndenter, &vObject);
		SAFERELEASE (pDisp);
	}

	// Save the XML document
	try
	{
		::SetFileAttributes (strFileName, FILE_ATTRIBUTE_NORMAL);
		hr = m_plDomDocument->save(strFileName);
#if !defined _DEBUG
		::SetFileAttributes (strFileName, FILE_ATTRIBUTE_READONLY);
#endif
	}
	catch(...)
	{
		//Saving Failed
		return false;
	}

	// Return
	if (!SUCCEEDED(hr))
		return false;

	return true;
}

/**FDOC*************************************************************************
 *
 *  Function:       CXmlParser::Reset_XML_Document
 *
 *  Author:         F.S.Monica				08-Nov-2005
 *
 *  Description:    Clear the XML Document
 *
 *  Parameters:     <None>
 *
 *  Returns:        <None>
 *
 *  Side effects:   <None>
 *
 *  History:		
 *
 *********************************************************************FDOCEND**/
void CXmlParser::Reset_XML_Document()
{
	// Init
	this->Init_MSXML ();
//	m_plDomDocument->loadXML ("");
	m_pDocRoot = m_plDomDocument->documentElement;
	m_pCurrentNode = NULL;

	// Init our members
	this->m_aystrAttribNames.RemoveAll();
	this->m_aystrAttribValues.RemoveAll();
	this->m_strTag.Empty();
	this->m_strName.Empty();
	this->m_strChainTag.Empty();
	this->m_strTextField.Empty();
	m_iAttrib_Index = -1;
}

/**FDOC*************************************************************************
 *
 *  Function:       CXmlParser::Go_to_Root
 *
 *  Author:         F.S.Monica				08-Nov-2005
 *
 *  Description:    Go to the Root node
 *
 *  Parameters:     <None>
 *
 *  Returns:        <None>
 *
 *  Side effects:   <None>
 *
 *  History:		
 *
 *********************************************************************FDOCEND**/
void CXmlParser::Go_to_Root()
{ 
	m_iAttrib_Index = -1;
	this->m_pCurrentNode = this->m_pDocRoot;
	this->m_strChainTag.Empty();
}

/**FDOC*************************************************************************
 *
 *  Function:       CXmlParser::Go_to_Child
 *
 *  Author:         F.S.Monica				08-Nov-2005
 *
 *  Description:    Go to the first child of the current node
 *
 *  Parameters:     <None>
 *
 *  Returns:        true - if child node found
 *
 *  Side effects:   <None>
 *
 *  History:		
 *
 *********************************************************************FDOCEND**/
bool CXmlParser::Go_to_Child()
{
	// Child node
	MSXML2::IXMLDOMNodePtr pChild;

	if(Is_MSXML_Node(this->m_pCurrentNode) != MSXML2::NODE_ELEMENT)
		return false;

	pChild = m_pCurrentNode->firstChild;
	if(pChild == NULL)
		return false;

	// Grab Information from Child node
	m_iAttrib_Index = -1;
	ChainTag_Add(this->Get_CurrentTag());
	this->Grab_Node_Informations(pChild);

	this->m_pCurrentNode = pChild;
	return true;
}

/**FDOC*************************************************************************
 *
 *  Function:       CXmlParser::Go_to_Parent
 *
 *  Author:         F.S.Monica				08-Nov-2005
 *
 *  Description:    Go to the parent node of the current node
 *
 *  Parameters:     <None>
 *
 *  Returns:        returns true if current node is not root node
 *
 *  Side effects:   <None>
 *
 *  History:		
 *
 *********************************************************************FDOCEND**/
bool CXmlParser::Go_to_Parent()
{
	// Parent node
	MSXML2::IXMLDOMNodePtr pParent = NULL;

	if(this->m_pCurrentNode == this->m_pDocRoot)
		return false;

	this->m_pCurrentNode->get_parentNode(&pParent);
	this->m_pCurrentNode = pParent;

	m_iAttrib_Index = -1;
	this->ChainTag_Remove(1);
	this->Grab_Node_Informations(this->m_pCurrentNode);
	return true;
}

/**FDOC*************************************************************************
 *
 *  Function:       CXmlParser::Go_Forward
 *
 *  Author:         F.S.Monica				08-Nov-2005
 *
 *  Description:    Go to the next node of same level as the current node
 *
 *  Parameters:     <None>
 *
 *  Returns:        true - if next node found in the same level
 *
 *  Side effects:   <None>
 *
 *  History:		
 *
 *********************************************************************FDOCEND**/
bool CXmlParser::Go_Forward()
{
	// Sibling node
	MSXML2::IXMLDOMNodePtr pSibling = NULL;

	if((Is_MSXML_Node(this->m_pCurrentNode) != MSXML2::NODE_ELEMENT) && (Is_MSXML_Node(this->m_pCurrentNode) != MSXML2::NODE_CDATA_SECTION))
		return false;

	pSibling = m_pCurrentNode->nextSibling;
	if(pSibling == NULL)
		return false;

	m_pCurrentNode = pSibling;

	// Grab Information from Sibling node
	m_iAttrib_Index = -1;
	this->Grab_Node_Informations(m_pCurrentNode);
	return true;
}

/**FDOC*************************************************************************
 *
 *  Function:       CXmlParser::Go_Backward
 *
 *  Author:         F.S.Monica				08-Nov-2005
 *
 *  Description:    Go to the previous node of same level as the current node
 *
 *  Parameters:     <None>
 *
 *  Returns:        true - if previous node found in the same level
 *
 *  Side effects:   <None>
 *
 *  History:		
 *
 *********************************************************************FDOCEND**/
bool CXmlParser::Go_Backward()
{
	// Sibling node
	MSXML2::IXMLDOMNodePtr pSibling;

	if((Is_MSXML_Node(this->m_pCurrentNode) != MSXML2::NODE_ELEMENT) && (Is_MSXML_Node(this->m_pCurrentNode) != MSXML2::NODE_CDATA_SECTION))
		return false;

	pSibling = m_pCurrentNode->previousSibling;
	if(pSibling == NULL)
		return false;

	m_pCurrentNode = pSibling;

	// Grab Information from Sibling node
	m_iAttrib_Index = -1;
	this->Grab_Node_Informations(pSibling);
	return true;
}

/**FDOC*************************************************************************
 *
 *  Function:       CXmlParser::Go_to_Child
 *
 *  Author:         F.S.Monica				08-Nov-2005
 *
 *  Description:    Go to a Direct Child Node
 *
 *  Parameters:     strNodeName - node name
 *
 *  Returns:        true - if successful
 *
 *  Side effects:   <None>
 *
 *  History:		
 *
 *********************************************************************FDOCEND**/
bool CXmlParser::Go_to_Child(LPCTSTR strNodeName)
{
	// Child node
	MSXML2::IXMLDOMNodePtr pChild;

	if(Is_MSXML_Node(this->m_pCurrentNode) != MSXML2::NODE_ELEMENT)
	return false;

	ChainTag_Add(this->Get_CurrentTag());

	for (pChild = m_pCurrentNode->firstChild;  NULL != pChild;  pChild = pChild->nextSibling)
	{
		// Grab Information from Child node
		this->Grab_Node_Informations(pChild);

		if(this->Get_CurrentName().CompareNoCase(strNodeName) == 0)
		{
			// Update new Position
			m_iAttrib_Index = -1;
			this->m_pCurrentNode = pChild;
			return true;
		}
	}

	// Node Not Found (Restore Node informations)
	this->ChainTag_Remove(1);
	this->Grab_Node_Informations(this->m_pCurrentNode);
	return false;
}

/**FDOC*************************************************************************
 *
 *  Function:       CXmlParser::Go_to_Descendant
 *
 *  Author:         F.S.Monica				08-Nov-2005
 *
 *  Description:    Go to a Descendant Node (Child, Little Child, Etc...)
 *
 *  Parameters:     strNodeName - node name
 *
 *  Returns:        true - if successful
 *
 *  Side effects:   <None>
 *
 *  History:		
 *
 *********************************************************************FDOCEND**/
bool CXmlParser::Go_to_Descendant(LPCTSTR strNodeName)
{
	// Child node
	MSXML2::IXMLDOMNodePtr pChild;

	// Current Node before the call method
	MSXML2::IXMLDOMElementPtr pCurrent = this->m_pCurrentNode;

	if(Is_MSXML_Node(this->m_pCurrentNode) != MSXML2::NODE_ELEMENT)
		return false;

	ChainTag_Add(this->Get_CurrentTag());

	for (pChild = m_pCurrentNode->firstChild;  NULL != pChild;  pChild = pChild->nextSibling)
	{
		// Grab Information from Child node
		this->Grab_Node_Informations(pChild);

		if(this->Get_CurrentName().CompareNoCase(strNodeName) == 0)
		{
			// Update new Position
			m_iAttrib_Index = -1;
			this->m_pCurrentNode = pChild;
			return true;
		}

		// Try to go into Childs of this Child
		this->m_pCurrentNode = pChild;
		if( this->Go_to_Descendant(strNodeName) )
		{
			// We find the approriate node
			// All is set, we can return
			return true;
		}

		// Restore Current node
		this->m_pCurrentNode = pCurrent;
	}

	// Node Not Found (Restore Node informations)
	this->ChainTag_Remove(1);
	this->Grab_Node_Informations(this->m_pCurrentNode);
	return false;
}

/**FDOC*************************************************************************
 *
 *  Function:       CXmlParser::Go_to_Parent
 *
 *  Author:         F.S.Monica				08-Nov-2005
 *
 *  Description:    Go to a Parent Node
 *
 *  Parameters:     strNodeName - node name
 *
 *  Returns:        true - if successful
 *
 *  Side effects:   <None>
 *
 *  History:		
 *
 *********************************************************************FDOCEND**/
bool CXmlParser::Go_to_Parent(LPCTSTR strNodeName)
{
	// Parent node
	MSXML2::IXMLDOMNodePtr pParent = NULL;
	MSXML2::IXMLDOMNodePtr oldCurrent = this->m_pCurrentNode;

	if(this->m_pCurrentNode == this->m_pDocRoot)
		return false;

	CString strOldChainTag = this->m_strChainTag;

	this->m_pCurrentNode->get_parentNode(&pParent);

	while (true)
	{
		this->m_pCurrentNode = pParent;
		this->ChainTag_Remove(1);
		this->Grab_Node_Informations(this->m_pCurrentNode);
		if(this->Get_CurrentName().CompareNoCase (strNodeName) == 0)
		{
			m_iAttrib_Index = -1;
			return true;
		}

		if(this->m_pCurrentNode == this->m_pDocRoot)
			break;

		this->m_pCurrentNode->get_parentNode(&pParent);
	}

	// Parent not found
	this->m_pCurrentNode = oldCurrent;
	this->m_strChainTag = strOldChainTag;
	this->Grab_Node_Informations(this->m_pCurrentNode);
	return false;
}

/**FDOC*************************************************************************
 *
 *  Function:       CXmlParser::Go_Forward
 *
 *  Author:         F.S.Monica				08-Nov-2005
 *
 *  Description:    Go to a Node attached at the same Node than the Current Node (Forward sens)
 *
 *  Parameters:     strNodeName - node name
 *
 *  Returns:        true - if successful
 *
 *  Side effects:   <None>
 *
 *  History:		
 *
 *********************************************************************FDOCEND**/
bool CXmlParser::Go_Forward(LPCTSTR strNodeName)
{
	// Sibling node
	MSXML2::IXMLDOMNodePtr pSibling = NULL;

	if((Is_MSXML_Node(this->m_pCurrentNode) != MSXML2::NODE_ELEMENT) && (Is_MSXML_Node(this->m_pCurrentNode) != MSXML2::NODE_CDATA_SECTION))
		return false;

	for (pSibling = m_pCurrentNode;  true;  pSibling = pSibling->nextSibling)
	{
		if(pSibling == NULL)
			break;

		// Grab Information from Sibling node
		this->Grab_Node_Informations (pSibling);

		if(this->Get_CurrentName().CompareNoCase (strNodeName) == 0)
		{
			// Update new Position
			m_iAttrib_Index = -1;
			this->m_pCurrentNode = pSibling;
			return true;
		}
	}

	// Node Not Found (Restore Node informations)
	this->Grab_Node_Informations (this->m_pCurrentNode);
	return false;
}

/**FDOC*************************************************************************
 *
 *  Function:       CXmlParser::Go_Backward
 *
 *  Author:         F.S.Monica				08-Nov-2005
 *
 *  Description:    Go to the previous node of same level as the current node
 *
 *  Parameters:     strNodeName - node name
 *
 *  Returns:        true - if successful
 *
 *  Side effects:   <None>
 *
 *  History:		
 *
 *********************************************************************FDOCEND**/
bool CXmlParser::Go_Backward(LPCTSTR strNodeName)
{
	// Sibling node
	MSXML2::IXMLDOMNodePtr pSibling;

	if((Is_MSXML_Node(this->m_pCurrentNode) != MSXML2::NODE_ELEMENT) && (Is_MSXML_Node(this->m_pCurrentNode) != MSXML2::NODE_CDATA_SECTION))
		return false;

	for (pSibling = m_pCurrentNode;  NULL != pSibling;  pSibling = pSibling->previousSibling)
	{
		// Grab Information from Sibling node
		this->Grab_Node_Informations(pSibling);

		if(this->Get_CurrentName().CompareNoCase (strNodeName) == 0)
		{
			// Update new Position
			m_iAttrib_Index = -1;
			this->m_pCurrentNode = pSibling;
			return true;
		}
	}

	// Node Not Found (Restore Node informations)
	this->Grab_Node_Informations(this->m_pCurrentNode);
	return false;
}

/**FDOC*************************************************************************
 *
 *  Function:       CXmlParser::Remove
 *
 *  Author:         F.S.Monica				08-Nov-2005
 *
 *  Description:    Remove the Current Node (Move to the Parent Node)
 *
 *  Parameters:     <None>
 *
 *  Returns:        true - if successful
 *
 *  Side effects:   <None>
 *
 *  History:		
 *
 *********************************************************************FDOCEND**/
bool CXmlParser::Remove()
{
	// Parent node
	MSXML2::IXMLDOMNodePtr pParent= NULL;

	if(m_pCurrentNode == NULL)
	{
		if (this->m_pDocRoot != NULL)
			this->m_pCurrentNode = this->m_pDocRoot;
		else
			return false;
	}

	if (this->m_pCurrentNode != this->m_pDocRoot)
	{
		this->m_pCurrentNode->get_parentNode (&pParent);
		pParent->removeChild (this->m_pCurrentNode);
		this->m_pCurrentNode = pParent;
		this->Grab_Node_Informations (this->m_pCurrentNode);
	}
	else
		this->Reset_XML_Document ();

	m_iAttrib_Index = -1;
	return true;
}

/**FDOC*************************************************************************
 *
 *  Function:       CXmlParser::RemoveChild
 *
 *  Author:         F.S.Monica				08-Nov-2005
 *
 *  Description:    Remove a Child Node     (No Move);
 *
 *  Parameters:     strNodeName - node name
 *
 *  Returns:        true - if successful
 *
 *  Side effects:   <None>
 *
 *  History:		
 *
 *********************************************************************FDOCEND**/
bool CXmlParser::RemoveChild(LPCTSTR strNodeName)
{
	// Find the Child
	if(Is_MSXML_Node (this->m_pCurrentNode) != MSXML2::NODE_ELEMENT)
		return false;

	if (this->Go_to_Child (strNodeName))
	{
		int iCur_attrib_index = m_iAttrib_Index;
		bool bResult = this->Remove ();

		m_iAttrib_Index = iCur_attrib_index;
		return bResult;
	}
	else
		return false;
}

/**FDOC*************************************************************************
 *
 *  Function:       CXmlParser::Set_Header
 *
 *  Author:         F.S.Monica				08-Nov-2005
 *
 *  Description:    To Set an header property like <? xml version="1.0"?>
 *
 *  Parameters:     strHeaderName - Name of the header
 *					strAttribName - Name of attribute
 *					strAttribValue - Value of attribute
 *
 *  Returns:        true - if successful
 *
 *  Side effects:   <None>
 *
 *  History:		
 *		Jan-2014,Chetan, Modified for UNICODE Support, WI:Mercury Server - Unicode enhancement
 *
 *********************************************************************FDOCEND**/
bool CXmlParser::Set_Header( LPCTSTR strHeaderName, LPCTSTR strAttribName, LPCTSTR strAttribValue )
{

	return false;
}

/**FDOC*************************************************************************
 *
 *  Function:       CXmlParser::Get_Header
 *
 *  Author:         F.S.Monica				08-Nov-2005
 *
 *  Description:    Return the header property like <? xml version="1.0"?>
 *
 *  Parameters:     strHeaderName [in] - Name of the header
 *					strAttribName [in] - Name of attribute
 *					strResult [out] - Value of attribute
 *
 *  Returns:        true - if successful
 *
 *  Side effects:   <None>
 *
 *  History:		
 *
 *********************************************************************FDOCEND**/
bool CXmlParser::Get_Header(LPCTSTR strHeaderName,LPCTSTR strAttribName,CString& strResult)
{
	MSXML2::IXMLDOMNodePtr pChild;                  // Working node
	strResult.Empty();

	pChild = m_plDomDocument->firstChild;   // Start at first document child

	if(pChild == NULL) 
	{
		return false;  
	}

	while(pChild != NULL)
	{
		if(pChild->nodeType != MSXML2::NODE_PROCESSING_INSTRUCTION)
			break;

		BSTR bstr;
		CString cstr;

		pChild->get_baseName (&bstr);
		cstr = bstr;
		if(cstr.CompareNoCase (strHeaderName) == 0) 
		{
			// Correct header, check the correct property
			pChild->get_text (&bstr);
			cstr = bstr;

			int iIndex = cstr.Find (strAttribName, 0);
			if(iIndex == -1)
				return false;

			int iStart, iEnd;
			iStart = cstr.Find ('"', iIndex);
			if(iStart == -1) 
			{
				return false;
			}

			iEnd = cstr.Find ('"', iStart + 1);
			if(iEnd == -1) 
			{
				return false;
			}

			strResult = cstr.Mid (iStart + 1 , iEnd - iStart - 1);
			return true;
		}

		pChild = pChild->nextSibling;   // Next Processing instruction node
	}
	return false;
}

/**FDOC*************************************************************************
 *
 *  Function:       CXmlParser::Add_Comment
 *
 *  Author:         F.S.Monica				08-Nov-2005
 *
 *  Description:    Add Comment node
 *
 *  Parameters:     strData - comment tstring
 *
 *  Returns:        true - if successful
 *
 *  Side effects:   <None>
 *
 *  History:		
 *
 *********************************************************************FDOCEND**/
bool CXmlParser::Add_Comment (LPCTSTR strData)
{
	// Nodes members
	MSXML2::IXMLDOMNodePtr pNewNode;

	// Create the NODE
	pNewNode = m_plDomDocument->createComment (strData);

	// Attach the Node to the document
	return m_pCurrentNode->appendChild (pNewNode);
}

/**FDOC*************************************************************************
 *
 *  Function:       CXmlParser::Add_LastChildAsComment
 *
 *  Author:         F.S.Monica				08-Nov-2005
 *
 *  Description:    Add Comment node as last child to the current node
 *
 *  Parameters:     strData - comment tstring
 *
 *  Returns:        true - if successful
 *
 *  Side effects:   <None>
 *
 *  History:		
 *
 *********************************************************************FDOCEND**/
bool CXmlParser::Add_LastChildAsComment (LPCTSTR strData)
{
	// Nodes members
	MSXML2::IXMLDOMNodePtr pNewNode;

	// Create the NODE
	pNewNode = m_plDomDocument->createComment (strData);

	// Attach the Node to the document
	return this->_add_lastchild (pNewNode);
}

/**FDOC*************************************************************************
 *
 *  Function:       CXmlParser::Add_FirstChildAsComment
 *
 *  Author:         F.S.Monica				08-Nov-2005
 *
 *  Description:    Add Comment node as first child to the current node
 *
 *  Parameters:     strData - comment tstring
 *
 *  Returns:        true - if successful
 *
 *  Side effects:   <None>
 *
 *  History:		
 *
 *********************************************************************FDOCEND**/
bool CXmlParser::Add_FirstChildAsComment (LPCTSTR strData)
{
	// Nodes members
	MSXML2::IXMLDOMNodePtr pNewNode;

	// Create the NODE
	pNewNode = m_plDomDocument->createComment (strData);

	// Attach the Node to the document
	return this->_add_firstchild (pNewNode);
}

/**FDOC*************************************************************************
 *
 *  Function:       CXmlParser::ParseSnippetIntoFragment
 *
 *  Author:         F.S.Monica				22-Feb-2006
 *
 *  Description:    Converts the New XML config tstring to DOM Document pointer
 *
 *  Parameters:     strNewXMLNode - XML tstring to be updated;
 *					pOwnerDoc - Pointer to the document to update
 *					pFragment - DOM Document fragment pointer
 *
 *  Returns:        S_OK - if successful
 *
 *  Side effects:   <None>
 *
 *  History:		
 *
 *********************************************************************FDOCEND**/
HRESULT CXmlParser::ParseSnippetIntoFragment (CString strNewXMLNode, MSXML2::IXMLDOMDocumentPtr pOwnerDoc, MSXML2::IXMLDOMDocumentFragmentPtr& pFragment)
{
	HRESULT hrResult = S_OK;
	CString strValue;

	MSXML2::IXMLDOMDocumentPtr pNewDoc = NULL;
	hrResult = pNewDoc.CreateInstance (MSXML2::CLSID_DOMDocument);
	ASSERT (SUCCEEDED(hrResult));

	pNewDoc->async = false;

	strValue = _T("<dummy>") + strNewXMLNode + _T("</dummy>");
	if (pNewDoc->loadXML ((bstr_t) strValue))
	{ 
		pFragment = pOwnerDoc->createDocumentFragment ();
		ASSERT (pFragment != NULL);

		MSXML2::IXMLDOMElementPtr pSubNodes = pNewDoc->documentElement;
		ASSERT (pSubNodes != NULL);

		while (pSubNodes->hasChildNodes ())
			pFragment->appendChild (pSubNodes->firstChild);

		return (S_OK);
	} 
	return (E_FAIL);
}

/**FDOC*************************************************************************
 *
 *  Function:       CXmlParser::UpdateNode
 *
 *  Author:         F.S.Monica				22-Feb-2006
 *
 *  Description:    Replace a Child Node with new node;
 *
 *  Parameters:     strNewXML - Modified Config data in XML data format
 *					strNodePath - Node Path
 *
 *  Returns:        true - if successful
 *
 *  Side effects:   <None>
 *
 *  History:		
 *					24-Mar-2006, Modified by F.S.Monica for the assignment# 5390
 *
 *********************************************************************FDOCEND**/
bool CXmlParser::UpdateXMLNode (CString strNewXML, CString strNodePath)
{
	this->Go_to_Root ();

	MSXML2::IXMLDOMDocumentFragmentPtr pFragment = NULL;

	HRESULT hrResult = ParseSnippetIntoFragment (strNewXML, m_plDomDocument, pFragment);
	if (hrResult == E_FAIL)
		return false;

	MSXML2::IXMLDOMNodeListPtr pNodes = NULL;
	if (strNodePath != "")
	{
		this->Go_to_Root ();
		pNodes = this->m_pCurrentNode->selectNodes (bstr_t(strNodePath));
	}
	else
	{
		CString strNodeName = (CString)(const TCHAR*) pFragment->firstChild->GetnodeName ();
		pNodes = m_plDomDocument->getElementsByTagName(_bstr_t(strNodeName)); 
	}

	if (pNodes == NULL)
		return false;

	MSXML2::IXMLDOMNodePtr pTargetNode = pNodes->item[0];
	if (pTargetNode == NULL)
		return false;

	if (pTargetNode == m_plDomDocument->documentElement)
		return false;

	MSXML2::IXMLDOMNodePtr pEditNode = NULL;
	pTargetNode->get_parentNode (&pEditNode);

	pEditNode->replaceChild (pFragment, pTargetNode);

	return true;
}
