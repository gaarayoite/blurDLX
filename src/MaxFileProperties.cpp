/*!
	\file		MaxFileProperties.cpp

	\remarks	Maxscript extension to have access to max file OLE container properties
	
	\author		Diego Garcia Huerta
	\author		Email: diego@blur.com
	\author		Company: Blur Studio
	\date		05/01/07

	\history
				- version 1.0 DGH 05/03/07: Created

	\note
				Copyright (c) 2006, Blur Studio Inc.
				All rights reserved.

				Redistribution and use in source and binary forms, with or without 
				modification, are permitted provided that the following conditions 
				are met:

					* Redistributions of source code must retain the above copyright 
					notice, this list of conditions and the following disclaimer.
					* Redistributions in binary form must reproduce the above 
					copyright notice, this list of conditions and the following 
					disclaimer in the documentation and/or other materials provided 
					with the distribution.
					* Neither the name of the Blur Studio Inc. nor the names of its 
					contributors may be used to endorse or promote products derived 
					from this software without specific prior written permission.

				THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
				"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
				LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS 
				FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE 
				COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, 
				INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, 
				BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
				LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER 
				CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT 
				LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN 
				ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
				POSSIBILITY OF SUCH DAMAGE.
*/

#ifdef __MAXSCRIPT_2012__
#include "maxscript\maxscript.h"
#include "maxscript\UI\rollouts.h"
#include "maxscript\foundation\numbers.h"
#include "maxscript\foundation\3dmath.h"
#include "maxscript\maxwrapper\mxsobjects.h"
#include "maxscript\maxwrapper\maxclasses.h"
#include "maxscript\compiler\parser.h"
extern TCHAR* GetString(int id);
#else
#include "MAXScrpt.h"
#include "Rollouts.h"
#include "Numbers.h"
#include "3DMath.h"
#include "MAXObj.h"
#include "MAXclses.h"
#include "Parser.h"
#endif


#ifdef ScripterExport
	#undef ScripterExport
#endif
#define ScripterExport __declspec( dllexport )

#include "MaxFileProperties.h"
#include "Resource.h"

#ifdef __MAXSCRIPT_2012__
#include "maxscript\macros\define_external_functions.h"
#else
#include "defextfn.h"
#endif
	def_name ( maxfileproperties )

#ifdef __MAXSCRIPT_2012__
#include "maxscript\macros\define_instantiation_functions.h"
#else
#include "definsfn.h"
#endif
	def_name ( summaryInfo )
	def_name ( documentSummaryInfo )
	def_name ( userDefinedProperties )
	def_name ( general )
	def_name ( meshTotals )
	def_name ( sceneTotals )
	def_name ( externalDependences )
//	def_name ( objects )
	def_name ( materials )
	def_name ( usedPlugins )
	def_name ( renderData )

//	def_name ( title )
	def_name ( subject )
	def_name ( author )
	def_name ( keywords )
	def_name ( comments )

	def_name ( manager )
	def_name ( company )
//	def_name ( category )
	

#define PROPSET_SUMINFO			0x000000ff
#define PROPSET_DOCSUMINFO		0x0000ff00
#define PROPSET_USERDEF			0x00ff0000

#define ALL_PROPERTIES			0xffffffff	// All props
#define TITLE_PROP				0x00000001	// Summary Info
#define SUBJECT_PROP			0x00000002	// Summary Info
#define AUTHOR_PROP				0x00000004	// Summary Info
#define KEYWORDS_PROP			0x00000008	// Summary Info
#define COMMENTS_PROP			0x00000010	// Summary Info
#define MANAGER_PROP			0x00000100	// Document Summary Info
#define COMPANY_PROP			0x00000200	// Document Summary Info
#define CATEGORY_PROP			0x00000400	// Document Summary Info
#define EXT_DEPEND_PROP			0x00000800	// Document Summary Info
#define PLUGINS_PROP			0x00001000	// Document Summary Info
#define OBJECTS_PROP			0x00002000	// Document Summary Info
#define MATERIALS_PROP			0x00004000	// Document Summary Info
#define USER_PROP				0x00010000	// User Defined Properties

#define PID_TITLE				0x00000002
#define PID_SUBJECT				0x00000003
#define PID_AUTHOR				0x00000004
#define PID_KEYWORDS			0x00000005
#define PID_COMMENTS			0x00000006

#define PID_MANAGER				0x0000000E
#define PID_COMPANY				0x0000000F
#define PID_CATEGORY			0x00000002
#define PID_HEADINGPAIR			0x0000000C
#define PID_DOCPARTS			0x0000000D

void TypeNameFromVariant(PROPVARIANT* pProp, char* szString, int bufSize)
{
switch (pProp->vt) {
	case VT_LPWSTR:
	case VT_LPSTR:
		strcpy(szString, GetString(IDS_TYPE_TEXT));
		break;
	case VT_I4:
	case VT_R4:
	case VT_R8:
		strcpy(szString, GetString(IDS_TYPE_NUMBER));
		break;
	case VT_BOOL:
		strcpy(szString, GetString(IDS_TYPE_BOOL));
		break;
	case VT_FILETIME:
		strcpy(szString, GetString(IDS_TYPE_DATE));
		break;
	default:
		strcpy(szString, "");
		break;
	}
}

void VariantToString(PROPVARIANT* pProp, char* szString, int bufSize)
{
switch (pProp->vt) {
	case VT_LPWSTR:
		wcstombs(szString, pProp->pwszVal, bufSize);
		break;
	case VT_LPSTR:
		strcpy(szString, pProp->pszVal);
		break;
	case VT_I4:
		sprintf(szString, "%ld", pProp->lVal);
		break;
	case VT_R4:
		sprintf(szString, "%f", pProp->fltVal);
		break;
	case VT_R8:
		sprintf(szString, "%lf", pProp->dblVal);
		break;
	case VT_BOOL:
		sprintf(szString, "%s", pProp->boolVal ? GetString(IDS_VAL_YES) : GetString(IDS_VAL_NO));
		break;
	case VT_FILETIME:
		SYSTEMTIME sysTime;
		FileTimeToSystemTime(&pProp->filetime, &sysTime);
		GetDateFormat(LOCALE_SYSTEM_DEFAULT,
						DATE_SHORTDATE,
						&sysTime,
						NULL,
						szString,
						bufSize);
		break;
	default:
		strcpy(szString, "");
		break;
	}
}

// ============================================================================

visible_class_instance(MaxFileProperties, "MaxFileProperties");

// ============================================================================
MaxFileProperties::MaxFileProperties()
{
	TSTR m_filename = _T("");
	TSTR m_title	= _T("");
	TSTR m_subject	= _T("");
	TSTR m_author	= _T("");
	TSTR m_keywords = _T("");
	TSTR m_comments = _T("");

	TSTR m_manager	= _T("");
	TSTR m_company	= _T("");
	TSTR m_category = _T("");

	m_summaryInfo			= new Array(0);
	m_documentSummaryInfo	= new Array(0);
	m_general				= new Array(0);
	m_meshTotals			= new Array(0);
	m_sceneTotals			= new Array(0);
	m_externalDependences	= new Array(0);
	m_objects				= new Array(0);
	m_materials				= new Array(0);
	m_usedPlugins			= new Array(0);
	m_renderData			= new Array(0); 
	m_userDefinedProperties = new Array(0); 
}

// ============================================================================
void MaxFileProperties::sprin1(CharStream* s)
{
	s->printf(_T("<MaxFileProperties:%s>"), (TCHAR*)this->m_filename);
}


// ============================================================================
void MaxFileProperties::gc_trace()
{
	Value::gc_trace();

	if (m_summaryInfo && m_summaryInfo->is_not_marked())
		m_summaryInfo->gc_trace();
	if (m_documentSummaryInfo && m_documentSummaryInfo->is_not_marked())
		m_documentSummaryInfo->gc_trace();
	if (m_general && m_general->is_not_marked())
		m_general->gc_trace();
	if (m_meshTotals && m_meshTotals->is_not_marked())
		m_meshTotals->gc_trace();
	if (m_sceneTotals && m_sceneTotals->is_not_marked())
		m_sceneTotals->gc_trace();
	if (m_externalDependences && m_externalDependences->is_not_marked())
		m_externalDependences->gc_trace();
	if (m_objects && m_objects->is_not_marked())
		m_objects->gc_trace();
	if (m_materials && m_materials->is_not_marked())
		m_materials->gc_trace();
	if (m_usedPlugins && m_usedPlugins->is_not_marked())
		m_usedPlugins->gc_trace();
	if (m_renderData && m_renderData->is_not_marked())
		m_renderData->gc_trace();

}

Value*	MaxFileProperties::set_property(Value** arg_list, int count)
{
	Value* val = arg_list[0];
	Value* prop = arg_list[1];

	return &undefined;
}

Value* MaxFileProperties::get_property(Value** arg_list, int count)
{
	Value* prop = arg_list[0];

	if (prop == n_filename)
		return new String(this->m_filename);
	else if (prop == n_summaryInfo)
		return this->m_summaryInfo;
	else if (prop == n_documentSummaryInfo)
		return this->m_documentSummaryInfo;
	else if (prop == n_userDefinedProperties)
		return this->m_userDefinedProperties;
	else if (prop == n_general)
		return this->m_general;
	else if (prop == n_meshTotals)
		return this->m_meshTotals;
	else if (prop == n_sceneTotals)
		return this->m_sceneTotals;
	else if (prop == n_externalDependences)
		return this->m_externalDependences;
	else if (prop == n_objects)
		return this->m_objects;
	else if (prop == n_materials)
		return this->m_materials;
	else if (prop == n_usedPlugins)
		return this->m_usedPlugins;
	else if (prop == n_renderData)
		return this->m_renderData;

	else if (prop == n_title)
		return new String(this->m_title);
	else if (prop == n_subject)
		return new String(this->m_subject);
	else if (prop == n_author)
		return new String(this->m_author);
	else if (prop == n_keywords)
		return new String(this->m_keywords);
	else if (prop == n_comments)
		return new String(this->m_comments);

	else if (prop == n_manager)
		return new String(this->m_manager);
	else if (prop == n_company)
		return new String(this->m_company);
	else if (prop == n_category)
		return new String(this->m_category);

	return &undefined;
}

Value* MaxFileProperties::show_props_vf(Value** arg_list, int count)
{

	return &ok;
}


Value* MaxFileProperties::get_props_vf(Value** arg_list, int count)
{
	Array* propNames = new Array(0);
	propNames->append(n_summaryInfo);
	propNames->append(n_title);
	propNames->append(n_subject);
	propNames->append(n_author);
	propNames->append(n_keywords);
	propNames->append(n_comments);
	propNames->append(n_documentSummaryInfo);
	propNames->append(n_manager);
	propNames->append(n_company);
	propNames->append(n_category);
	propNames->append(n_general);
	propNames->append(n_meshTotals);
	propNames->append(n_sceneTotals);
	propNames->append(n_externalDependences);
	propNames->append(n_objects);
	propNames->append(n_materials);
	propNames->append(n_usedPlugins);
	propNames->append(n_renderData);
	propNames->append(n_userDefinedProperties);

	return propNames;
}


//getmaxfileproperties "C:/temp/temp.max"
def_struct_primitive( getmaxfileproperties, blurUtil, "getmaxfileproperties");

Value* getmaxfileproperties_cf(Value** arg_list, int count)
{
	MaxFileProperties *mProps = new MaxFileProperties();

	LPSTORAGE				pStorage = NULL;
	IPropertySetStorage*	pPropertySetStorage = NULL;
	IPropertyStorage*		pSummaryInfoStorage = NULL;
	IPropertyStorage*		pDocumentSummaryInfoStorage = NULL;
	IPropertyStorage*		pUserDefinedPropertyStorage = NULL;
	wchar_t					wfilename[_MAX_PATH];
	char					szBuf[256];
	TCHAR*					filename = arg_list[0]->to_string();

	mProps->m_filename = filename;

	MultiByteToWideChar(CP_ACP, 0, filename, -1, wfilename, _MAX_PATH);
	HRESULT	res = StgOpenStorage(wfilename, (LPSTORAGE)0, STGM_DIRECT|STGM_READ|STGM_SHARE_EXCLUSIVE,	NULL,0,&pStorage);
	if (res!=S_OK) 
	{
		return mProps;
	}

	// Get the Storage interface
	if (S_OK != pStorage->QueryInterface(IID_IPropertySetStorage, (void**)&pPropertySetStorage)) 
	{
		pStorage->Release();
		return mProps;
	}

	// Get the SummaryInfo property set interface
	if (S_OK == pPropertySetStorage->Open(FMTID_SummaryInformation, STGM_READ|STGM_SHARE_EXCLUSIVE, &pSummaryInfoStorage)) 
	{
		BOOL bFound = FALSE;

		PROPSPEC	PropSpec[5];
		PROPVARIANT	PropVar[5];

		PropSpec[0].ulKind = PRSPEC_PROPID;
		PropSpec[0].propid = PID_TITLE;

		PropSpec[1].ulKind = PRSPEC_PROPID;
		PropSpec[1].propid = PID_SUBJECT;

		PropSpec[2].ulKind = PRSPEC_PROPID;
		PropSpec[2].propid = PID_AUTHOR;

		PropSpec[3].ulKind = PRSPEC_PROPID;
		PropSpec[3].propid = PID_KEYWORDS;

		PropSpec[4].ulKind = PRSPEC_PROPID;
		PropSpec[4].propid = PID_COMMENTS;

		HRESULT hr = pSummaryInfoStorage->ReadMultiple(5, PropSpec, PropVar);
		
		Value* propVar;

		if (S_OK == hr) 
		{
			if (PropVar[0].vt == VT_LPSTR) 
			{
				propVar = new String (PropVar[0].pszVal);
				mProps->m_summaryInfo->append(propVar);
				mProps->m_title = PropVar[0].pszVal;
			}
			else
			{
				propVar = new String (_T(""));
				mProps->m_summaryInfo->append(propVar);
			}

			if (PropVar[1].vt == VT_LPSTR) 
			{
				propVar = new String (PropVar[1].pszVal);
				mProps->m_summaryInfo->append(propVar);
				mProps->m_subject = PropVar[1].pszVal;
			}
			else
			{
				propVar = new String (_T(""));
				mProps->m_summaryInfo->append(propVar);
			}

			if (PropVar[2].vt == VT_LPSTR) 
			{
				propVar = new String (PropVar[2].pszVal);
				mProps->m_summaryInfo->append(propVar);
				mProps->m_author = PropVar[2].pszVal;
			}
			else
			{
				propVar = new String (_T(""));
				mProps->m_summaryInfo->append(propVar);
			}
			
			if (PropVar[3].vt == VT_LPSTR) 
			{
				propVar = new String (PropVar[3].pszVal);
				mProps->m_summaryInfo->append(propVar);
				mProps->m_keywords = PropVar[3].pszVal;
			}
			else
			{
				propVar = new String (_T(""));
				mProps->m_summaryInfo->append(propVar);
			}

			if (PropVar[4].vt == VT_LPSTR) 
			{
				propVar = new String (PropVar[4].pszVal);
				mProps->m_summaryInfo->append(propVar);
				mProps->m_comments = PropVar[4].pszVal;
			}
			else
			{
				propVar = new String (_T(""));
				mProps->m_summaryInfo->append(propVar);
			}
		}
		
		FreePropVariantArray(5, PropVar);
		pSummaryInfoStorage->Release();

	}
	
	

	// Get the DocumentSummaryInfo property set interface
	if (S_OK == pPropertySetStorage->Open(FMTID_DocSummaryInformation, STGM_READ|STGM_SHARE_EXCLUSIVE, &pDocumentSummaryInfoStorage)) 
	{
		BOOL bFound = FALSE;

		PROPSPEC	PropSpec[5];
		PROPVARIANT	PropVar[5];

		PropSpec[0].ulKind = PRSPEC_PROPID;
		PropSpec[0].propid = PID_MANAGER;

		PropSpec[1].ulKind = PRSPEC_PROPID;
		PropSpec[1].propid = PID_COMPANY;

		PropSpec[2].ulKind = PRSPEC_PROPID;
		PropSpec[2].propid = PID_CATEGORY;

		PropSpec[3].ulKind = PRSPEC_PROPID;
		PropSpec[3].propid = PID_HEADINGPAIR;

		PropSpec[4].ulKind = PRSPEC_PROPID;
		PropSpec[4].propid = PID_DOCPARTS;
		
		Value* propVar;

		HRESULT hr = pDocumentSummaryInfoStorage->ReadMultiple(5, PropSpec, PropVar);
		if (S_OK == hr) 
		{
			if (PropVar[0].vt == VT_LPSTR) 
			{
				propVar = new String (PropVar[0].pszVal);
				mProps->m_documentSummaryInfo->append(propVar);
				mProps->m_manager = PropVar[0].pszVal;
			}
			else
			{
				propVar = new String (_T(""));
				mProps->m_documentSummaryInfo->append(propVar);
			}


			if (PropVar[1].vt == VT_LPSTR) 			
			{
				propVar = new String (PropVar[1].pszVal);
				mProps->m_documentSummaryInfo->append(propVar);
				mProps->m_company = PropVar[1].pszVal;
			}
			else
			{
				propVar = new String (_T(""));
				mProps->m_documentSummaryInfo->append(propVar);
			}

			if (PropVar[2].vt == VT_LPSTR) 			
			{
				propVar = new String (PropVar[2].pszVal);
				mProps->m_documentSummaryInfo->append(propVar);
				mProps->m_category = PropVar[2].pszVal;
			}
			else
			{
				propVar = new String (_T(""));
				mProps->m_documentSummaryInfo->append(propVar);
			}

			if ((PropVar[3].vt == (VT_VARIANT | VT_VECTOR)) && (PropVar[4].vt == (VT_LPSTR | VT_VECTOR))) 
			{
				CAPROPVARIANT*	pHeading = &PropVar[3].capropvar;
				CALPSTR*		pDocPart = &PropVar[4].calpstr;

				// Headings:
				// =========
				// 0  - General
				// 2  - Mesh Totals
				// 4  - Scene Totals
				// 6  - External Dependencies
				// 8  - Objects
				// 10 - Materials
				// 12 - Plug-Ins
				// 14 - Render Data

				int nDocPart = 0;
				for (UINT i=0; i<pHeading->cElems; i+=2) 
				{
					for (int j=0; j<pHeading->pElems[i+1].lVal; j++) 
					{
						sprintf(szBuf, "%s", pDocPart->pElems[nDocPart]);
						
						switch ( i ) 
						{
							case 0: // m_general
							{
								propVar = new String (szBuf);
								mProps->m_general->append(propVar);
							}
							break;

							case 2: // m_meshTotals
							{
								propVar = new String (szBuf);
								mProps->m_meshTotals->append(propVar);
							}
							break;

							case 4: // m_sceneTotals
							{
								propVar = new String (szBuf);
								mProps->m_sceneTotals->append(propVar);
							}
							break;

							case 6: // m_externalDependences
							{
								propVar = new String (szBuf);
								mProps->m_externalDependences->append(propVar);
							}
							break; 
							
							case 8: // m_objects
							{
								propVar = new String (szBuf);
								mProps->m_objects->append(propVar);
							}
							break;

							case 10: // m_materials
							{
								propVar = new String (szBuf);
								mProps->m_materials->append(propVar);
							}
							break;

							case 12: // m_usedPlugins
							{
								propVar = new String (szBuf);
								mProps->m_usedPlugins->append(propVar);
							}
							break;

							case 14: // m_renderData
							{
								propVar = new String (szBuf);
								mProps->m_renderData->append(propVar);
							}
							break;

						}

						nDocPart++;
					}
				}

			}

		}
		
		mProps->m_documentSummaryInfo->append(mProps->m_general);
		mProps->m_documentSummaryInfo->append(mProps->m_meshTotals);
		mProps->m_documentSummaryInfo->append(mProps->m_sceneTotals);
		mProps->m_documentSummaryInfo->append(mProps->m_externalDependences);
		mProps->m_documentSummaryInfo->append(mProps->m_objects);
		mProps->m_documentSummaryInfo->append(mProps->m_materials);
		mProps->m_documentSummaryInfo->append(mProps->m_usedPlugins);
		mProps->m_documentSummaryInfo->append(mProps->m_renderData);

		FreePropVariantArray(5, PropVar);
		pDocumentSummaryInfoStorage->Release();
	}

	if (S_OK == pPropertySetStorage->Open(FMTID_UserDefinedProperties, STGM_READ|STGM_SHARE_EXCLUSIVE, &pUserDefinedPropertyStorage)) 
	{
		Value* propVar;
		int		numUserProps = 0;

		// First we need to count the properties
		IEnumSTATPROPSTG*	pIPropertyEnum;
		if (S_OK == pUserDefinedPropertyStorage->Enum(&pIPropertyEnum)) 
		{
			STATPROPSTG property;
			while (pIPropertyEnum->Next(1, &property, NULL) == S_OK) 
			{
				if (property.lpwstrName) 
				{
					CoTaskMemFree(property.lpwstrName);
					property.lpwstrName = NULL;
					numUserProps++;
				}
			}

			PROPSPEC* pPropSpec = new PROPSPEC[numUserProps];
			PROPVARIANT* pPropVar = new PROPVARIANT[numUserProps];

			ZeroMemory(pPropVar, numUserProps*sizeof(PROPVARIANT));
			ZeroMemory(pPropSpec, numUserProps*sizeof(PROPSPEC));

			pIPropertyEnum->Reset();
			int idx = 0;
			while (pIPropertyEnum->Next(1, &property, NULL) == S_OK) 
			{
				if (property.lpwstrName) 
				{
					pPropSpec[idx].ulKind = PRSPEC_LPWSTR;
					pPropSpec[idx].lpwstr = (LPWSTR)CoTaskMemAlloc(sizeof(wchar_t)*(wcslen(property.lpwstrName)+1));
					wcscpy(pPropSpec[idx].lpwstr, property.lpwstrName);
					idx++;
					CoTaskMemFree(property.lpwstrName);
					property.lpwstrName = NULL;
				}
			}
			pIPropertyEnum->Release();

			HRESULT hr = pUserDefinedPropertyStorage->ReadMultiple(idx, pPropSpec, pPropVar);
			if (S_OK == hr) 
			{
				
				for (int i=0; i<idx; i++) 
				{
					Array* newProp = new Array(0);

					wcstombs(szBuf, pPropSpec[i].lpwstr, 255);
				
					propVar = new String (szBuf);
					newProp->append(propVar);

					VariantToString(&pPropVar[i], szBuf, 255);
					
					propVar = new String (szBuf);
					newProp->append(propVar);

					TypeNameFromVariant(&pPropVar[i], szBuf, 255);
					
					propVar = new String (szBuf);
					newProp->append(propVar);

					mProps->m_userDefinedProperties->append(newProp);
				}

				
			}

			for (int i=0; i<idx; i++) 
			{
				CoTaskMemFree(pPropSpec[i].lpwstr);
			}

			FreePropVariantArray(numUserProps, pPropVar);

			delete [] pPropSpec;
			delete [] pPropVar;
		}

		pUserDefinedPropertyStorage->Release();
	}

	pPropertySetStorage->Release();
	pStorage->Release();

	return mProps;
}

void MaxFilePropertiesInit()
{
	#ifdef __MAXSCRIPT_2012__
	#include "maxscript\macros\define_implementations.h"
	#else
	#include "defimpfn.h"
	#endif
		def_name ( summaryInfo )
		def_name ( documentSummaryInfo )
		def_name ( userDefinedProperties )
		def_name ( general )
		def_name ( meshTotals )
		def_name ( sceneTotals )
		def_name ( externalDependences )
	//	def_name ( objects )
		def_name ( materials )
		def_name ( usedPlugins )
		def_name ( renderData )

	//	def_name ( title )
		def_name ( subject )
		def_name ( author )
		def_name ( keywords )
		def_name ( comments )

		def_name ( manager )
		def_name ( company )
	//	def_name ( category )


}