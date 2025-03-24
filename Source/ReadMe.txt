================================================================================
    MICROSOFT FOUNDATION CLASS LIBRARY : SPU3AutoIPConfigTool Project Overview
===============================================================================

The application wizard has created this SPU3AutoIPConfigTool application for
you.  This application not only demonstrates the basics of using the Microsoft
Foundation Classes but is also a starting point for writing your application.

This file contains a summary of what you will find in each of the files that
make up your SPU3AutoIPConfigTool application.

SPU3AutoIPConfigTool.vcxproj
    This is the main project file for VC++ projects generated using an application wizard.
    It contains information about the version of Visual C++ that generated the file, and
    information about the platforms, configurations, and project features selected with the
    application wizard.

SPU3AutoIPConfigTool.vcxproj.filters
    This is the filters file for VC++ projects generated using an Application Wizard. 
    It contains information about the assciation between the files in your project 
    and the filters. This association is used in the IDE to show grouping of files with
    similar extensions under a specific node (for e.g. ".cpp" files are associated with the
    "Source Files" filter).

SPU3AutoIPConfigTool.h
    This is the main header file for the application.  It includes other
    project specific headers (including Resource.h) and declares the
    CSPU3AutoIPConfigToolApp application class.

SPU3AutoIPConfigTool.cpp
    This is the main application source file that contains the application
    class CSPU3AutoIPConfigToolApp.

SPU3AutoIPConfigTool.rc
    This is a listing of all of the Microsoft Windows resources that the
    program uses.  It includes the icons, bitmaps, and cursors that are stored
    in the RES subdirectory.  This file can be directly edited in Microsoft
    Visual C++. Your project resources are in 1033.

res\SPU3AutoIPConfigTool.ico
    This is an icon file, which is used as the application's icon.  This
    icon is included by the main resource file SPU3AutoIPConfigTool.rc.

res\SPU3AutoIPConfigTool.rc2
    This file contains resources that are not edited by Microsoft
    Visual C++. You should place all resources not editable by
    the resource editor in this file.

/////////////////////////////////////////////////////////////////////////////

For the main frame window:
    The project includes a standard MFC interface.

MainFrm.h, MainFrm.cpp
    These files contain the frame class CMainFrame, which is derived from
    CFrameWnd and controls all SDI frame features.

res\Toolbar.bmp
    This bitmap file is used to create tiled images for the toolbar.
    The initial toolbar and status bar are constructed in the CMainFrame
    class. Edit this toolbar bitmap using the resource editor, and
    update the IDR_MAINFRAME TOOLBAR array in SPU3AutoIPConfigTool.rc to add
    toolbar buttons.
/////////////////////////////////////////////////////////////////////////////

The application wizard creates one document type and one view:

SPU3AutoIPConfigToolDoc.h, SPU3AutoIPConfigToolDoc.cpp - the document
    These files contain your CSPU3AutoIPConfigToolDoc class.  Edit these files to
    add your special document data and to implement file saving and loading
    (via CSPU3AutoIPConfigToolDoc::Serialize).

SPU3AutoIPConfigToolView.h, SPU3AutoIPConfigToolView.cpp - the view of the document
    These files contain your CSPU3AutoIPConfigToolView class.
    CSPU3AutoIPConfigToolView objects are used to view CSPU3AutoIPConfigToolDoc objects.




/////////////////////////////////////////////////////////////////////////////

Help Support:

hlp\SPU3AutoIPConfigTool.hhp
    This file is a help project file. It contains the data needed to
    compile the help files into a .chm file.

hlp\SPU3AutoIPConfigTool.hhc
    This file lists the contents of the help project.

hlp\SPU3AutoIPConfigTool.hhk
    This file contains an index of the help topics.

hlp\afxcore.htm
    This file contains the standard help topics for standard MFC
    commands and screen objects. Add your own help topics to this file.

makehtmlhelp.bat
    This file is used by the build system to compile the help files.

hlp\Images\*.gif
    These are bitmap files required by the standard help file topics for
    Microsoft Foundation Class Library standard commands.


/////////////////////////////////////////////////////////////////////////////

Other standard files:

StdAfx.h, StdAfx.cpp
    These files are used to build a precompiled header (PCH) file
    named SPU3AutoIPConfigTool.pch and a precompiled types file named StdAfx.obj.

Resource.h
    This is the standard header file, which defines new resource IDs.
    Microsoft Visual C++ reads and updates this file.

/////////////////////////////////////////////////////////////////////////////

Other notes:

The application wizard uses "TODO:" to indicate parts of the source code you
should add to or customize.

If your application uses MFC in a shared DLL, you will need
to redistribute the MFC DLLs. If your application is in a language
other than the operating system's locale, you will also have to
redistribute the corresponding localized resources MFC100XXX.DLL.
For more information on both of these topics, please see the section on
redistributing Visual C++ applications in MSDN documentation.

/////////////////////////////////////////////////////////////////////////////
