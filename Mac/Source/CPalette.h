//
//	CPalette.h
//

#ifndef _H_CPALETTE
#define _H_CPALETTE

#include "THeader.h"
#include "TString.h"

#include "CResource.h"
#include "QTGraphic.h"

BEGIN_NAMESPACE_FIVEL

class CPalette : public CResource
{
	public:
					CPalette(TString &inName);
		virtual		~CPalette();
	
		virtual void	Load() { Load(false); }	
		virtual void	Load(bool firstTime);	

		virtual void	_Load(void);
		virtual void	_Purge(void);

		CTabHandle	GetColorTable(void);
		RGBColor	GetColor(int32 inIndex);
		
		void		SetSize(uint32 inNewSize);
		void		UpdatePriority(void);
		
	protected:
		CTabHandle	m_CTab;	
		QTGraphic	*m_Qtg;
		TString		m_FullPath;
		bool		m_ClutResource;	
		
		void		LoadClutResource();
		void		LoadClutQT();
};

class CPaletteManager : public CResourceManager
{
	public:
					CPaletteManager();
					~CPaletteManager();
					
		void		Init(void);
		
		CPalette	*GetPalette(TString &inName);
		CPalette	*GetCurrentPalette(void) { return (m_GraphicsPal); }
		void		SetPalette(CPalette *inPal, bool inGraphPal);
		RGBColor	GetColor(int32 inIndex);
		void		RemoveAll(void);
		
		void		ResetPalette(void);
		void		CheckPalette(void);
		
		bool		HaveNewPal(void) { return (m_HaveNewPal); }
		
	protected:
		CPalette	*m_GraphicsPal;
		CPalette	*m_VideoPal;
		
		bool		m_HaveNewPal;		
};	

extern CPaletteManager gPaletteManager;	

END_NAMESPACE_FIVEL
		
#endif
