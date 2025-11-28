// (c) 2014 Beem Media

#include "EGMenu.h"
#include "EGQueue.h"
#include "EGEntObj.h"
#include "EGInputTypes.h"
#include "EGDelegate.h"

class EGClient;

class EGMenuStack
{
public:

	EGMCDelegate<EGMenuStack*> MenuStackChangedDelegate;

public:

	EGMenuStack( EGClient* Owner );
	~EGMenuStack();

	EGMenu* Push( eg_string_crc MenuId );
	void Pop( void );
	EGMenu* SwitchTo( eg_string_crc MenuId );
	void Clear( void );
	void PopTo( EGMenu* PopToMenu );
	EGMenu* PopToSwitchTo( EGMenu* PopToMenu , eg_string_crc SwitchToMenuId );

	void AcquireInput( void );
	void UnacquireInput( void );

	eg_bool HasInput( void )const{ return m_HasInput; }

	class EGMenu* GetActiveMenu(){ return m_Stack.HasItems() ? m_Stack.Top() : nullptr; }
	EGMenu* GetMenuByIndex( eg_size_t Index ) const { return m_Stack.IsValidIndex( Index ) ? m_Stack[Index] : nullptr; }
	eg_size_t Len() const { return m_Stack.Len(); }
	EGMenu* GetParentMenu( const EGMenu* Menu ) const;
	template<typename T> T* FindMenuByClass()
	{
		for( EGMenu* Menu : m_Stack )
		{
			if( Menu && Menu->IsA( &T::GetStaticClass() ) )
			{
				return EGCast<T>(Menu);
			}
		}
		return nullptr;
	}

	eg_bool IsEmpty()const{ return !m_Stack.HasItems(); }

	void Update( eg_real SecondsElapsed , const struct egLockstepCmds* Input , eg_real AspectRatio );
	void Draw();
	void DrawCursor( eg_real AspectRatio , eg_real x , eg_real y );

	eg_bool HasStackChanged()const{ return m_StackChanged; }

	eg_bool IsMenuOnStack( EGMenu* Menu ) const;

	void SetMouseCursor( eg_string_crc NewCursor );

private:

	void Update_HandleMouse( EGMenu* Menu , const struct egLockstepCmds* Input );
	void CullDeletedMenus( void );

	EGMenu* InternalInitMenu( eg_string_crc MenuId );


private:

	EGArray<class EGMenu*> m_Stack;
	EGArray<class EGMenu*> m_StackDelete; //Menu deletions are deferred so that scripts may continue running till update is complete.
	EGClient*const         m_Owner;
	eg_real                m_LastKnownAspectRatio;
	EGEntObj               m_MouseObj;
	eg_vec2                m_v2LastMousePos;
	eg_bool                m_StackChanged:1;
	eg_bool                m_HasInput:1;
	eg_bool                m_bHasInitialMousePos:1;
	egLockstepCmds         m_HasSeenDown; // Keeps track if the current active menu has actually seen these keys down, otherwise OnReleased events won't be used on them.
};