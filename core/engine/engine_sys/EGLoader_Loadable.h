#pragma once 

class ILoadable
{
friend class EGLoader;
protected:
	//Loading thread functionality:
	virtual void DoLoad(eg_cpstr strFile, const eg_byte*const  pMem, const eg_size_t Size)=0;
	//Main thread functionality:
	virtual void OnLoadComplete(eg_cpstr strFile)=0;
	//Tells the loader that it can be loaded now. Only called on the loading thread.
	virtual eg_bool CanLoadNow()const{return true;}
};
