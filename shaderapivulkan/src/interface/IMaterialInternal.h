#pragma once

#include <materialsystem/imaterial.h>

class IMaterialInternal : public IMaterial
{
public:
	virtual int GetReferenceCount() const = 0;
	virtual void SetEnumerationID(int) = 0;
	virtual void SetNeedsWhiteLightmap(bool) = 0;
	virtual bool GetNeedsWhiteLightmap() const = 0;
	virtual void Uncache(bool) = 0;
	virtual void Precache() = 0;
	virtual void PrecacheVars(KeyValues*, KeyValues*, CUtlVector<void*>*, int) = 0;
	virtual void ReloadTextures() = 0;
	virtual void SetMinLightmapPageID(int) = 0;
	virtual void SetMaxLightmapPageID(int) = 0;
	virtual int GetMinLightmapPageID() = 0;
	virtual int GetMaxLightmapPageID() = 0;
	virtual IShader* GetShader() = 0;
	virtual bool IsPrecachedVars() = 0;
	virtual void DrawMesh(VertexCompressionType_t compression) = 0;
	virtual VertexFormat_t GetVertexUsage() const = 0;
};
