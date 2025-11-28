// (c) 2017 Beem Media

#pragma once

#include "EGWnd.h"
#include "EGEditProperty.h"
#include "EGWndChildBase.h"
#include "EGWndTextNode.h"

struct egPropControlInfo
{
	eg_string_small DisplayName = CT_Clear;
	egRflEditor*    Editor = nullptr;
	egRflEditor*    Parent = nullptr;
};


class IWndPropControlOwner
{
public:
	
	virtual EGWndBase* GetOwningWnd() = 0;

	virtual void OnPropChangedValue( const egRflEditor* Editor ) { unused( Editor ); }
	virtual void OnPropControlButtonPressed( const egRflEditor* Property ){ unused( Property ); }
	virtual void PropEditorRebuild(){ }
};

class EGWndPropControl : public EGWndChildBase
{
	EG_DECL_SUPER( EGWndChildBase )

public:

	const eg_int FONT_SIZE = EGWnd_GetFontSize( egw_font_t::DEFAULT_BOLD );

	EGWndPropControl( IWndPropControlOwner* Parent , const egPropControlInfo& Info );
	~EGWndPropControl();

	eg_int GetHeight() const { return m_Height; }
	void SetEditorWidth( eg_int InEditorWidth ) { m_EditorWidth = InEditorWidth; }
	eg_int GetEditorWidth() const { return m_EditorWidth; }

	virtual LRESULT WndProc( UINT message , WPARAM wParam , LPARAM lParam ) override;
	virtual void OnWmSize( const eg_ivec2& NewClientSize ) override;
	virtual void OnPaint( HDC hdc ) override;
	virtual void OnDrawBg( HDC hdc ) override;
	virtual void OnWmRButtonUp( const eg_ivec2& MousePos ) override;
	virtual void OnWmMenuCmd( eg_int CmdId, eg_bool bFromAccelerator ) override;
	virtual void RefreshFromSourceData(){ }
	virtual void CommitChangesToSourceData( HWND CommitControl ){ unused( CommitControl ); }

	void CopyToClipboard();
	void DeleteFromArray();
	void InsertBeforeArray();
	void PasteFromClipboard();

protected:

	HWND CreateStaticText( eg_int x , eg_int y , eg_int Width , eg_int Height , eg_cpstr16 Text, HFONT Font );
	HWND CreateButtonControl( eg_int x , eg_int y , eg_int Width , eg_int Height , eg_cpstr16 Text, HFONT Font );
	HWND CreateCheckboxControl( eg_int x , eg_int y , eg_int Width , eg_int Height , eg_cpstr16 Text, HFONT Font );
	HWND CreateComboBoxControl( eg_int x , eg_int y , eg_int Width , eg_int Height , eg_bool bCanManualEdit , HFONT Font ); 

	void SetupEditControl( EGWndTextNode& EditText , eg_int x , eg_int y , eg_int Width , eg_int Height , eg_bool bMultiline , HFONT Font );
	eg_bool IsArrayChild() const;

protected:

	IWndPropControlOwner*const m_Parent = nullptr;
	egPropControlInfo          m_ControlInfo;
	eg_int                     m_EditorWidth = 0;
	eg_int                     m_Height = 25;
	HBRUSH                     m_BkBrush = nullptr;
	HFONT                      m_Font = nullptr;
	eg_bool                    m_bIsDirty = false;
	eg_bool                    m_bDrawDirtyControls = false;

private:

	static LRESULT CALLBACK ComboBoxControlSubproc( HWND hWnd , UINT uMsg , WPARAM wParam , LPARAM lParam , UINT_PTR uIdSubclass , DWORD_PTR dwRefData );
	static LRESULT CALLBACK ComboBoxEditControlSubproc( HWND hWnd , UINT uMsg , WPARAM wParam , LPARAM lParam , UINT_PTR uIdSubclass , DWORD_PTR dwRefData );
	static LRESULT CALLBACK CheckboxControlSubproc( HWND hWnd , UINT uMsg , WPARAM wParam , LPARAM lParam , UINT_PTR uIdSubclass , DWORD_PTR dwRefData );
	static LRESULT CALLBACK ButtonControlSubproc( HWND hWnd , UINT uMsg , WPARAM wParam , LPARAM lParam , UINT_PTR uIdSubclass , DWORD_PTR dwRefData );
};

typedef EGWndPropControl EGWndPropControl_Label;
