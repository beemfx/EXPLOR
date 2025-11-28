// (c) 2018 Beem Media

#pragma once

class EGMs3DFile;
class EGFileData;

class EGMs3DCompiler
{
protected:

	enum class eg_float_format_t
	{
		Text, 
		Base64,
	};

protected:

	EGMs3DFile* m_File = nullptr;
	eg_float_format_t m_FloatOutput = eg_float_format_t::Base64;
	eg_bool m_bSkelExportDisabled = false;
	eg_bool m_bMeshExportDisabled = false;

public:

	EGMs3DCompiler( const EGFileData& FileData );
	~EGMs3DCompiler();

	void SaveEMesh( EGFileData& FileDataOut ) const;
	void SaveESkel( EGFileData& FileDataOut ) const;

	eg_bool HasMesh() const;
	eg_bool HasSkel() const;

private:

	void LoadFile( const EGFileData& FileData );
	void CloseFile();

	eg_uint GetTextNodes( EGFileData& File ) const;
	eg_uint GetAnimations( EGFileData& File ) const;
	void ProcessScripts();
};
