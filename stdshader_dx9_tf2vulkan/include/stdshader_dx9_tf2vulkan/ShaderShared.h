#ifndef INCLUDE_GUARD_SHADER_SHARED_H
#define INCLUDE_GUARD_SHADER_SHARED_H

#ifdef __cplusplus
namespace TF2Vulkan{ namespace ShaderConstants
{
#pragma push_macro("DECLARE_LOCATION_TYPE_UNIQUE")
#pragma push_macro("DECLARE_LOCATION_TYPE_ARRAY")
#undef DECLARE_LOCATION_TYPE_UNIQUE
#undef DECLARE_LOCATION_TYPE_ARRAY

#endif // __cplusplus
	static const int _LOCATION_ARRAY_SIZE = 8;

	static const int LOCATIONTYPE_UNIQUE = 0;
	static const int LOCATIONTYPE_ARRAY = 1;

#define DECLARE_LOCATION_TYPE_UNIQUE(type, typeIndex) \
	static const int LOCATION_TYPEINDEX_ ## type = typeIndex; \
	static const int LOCATION_ ## type ## _TYPE = LOCATIONTYPE_UNIQUE; \
	static const int LOCATION_ ## type = (LOCATION_TYPEINDEX_ ## type) * _LOCATION_ARRAY_SIZE;

#define DECLARE_LOCATION_TYPE_ARRAY(type, typeIndex) \
	static const int LOCATION_TYPEINDEX_ ## type = typeIndex; \
	static const int LOCATION_ ## type ## _TYPE = LOCATIONTYPE_ARRAY; \
	static const int LOCATION_ ## type ## 0 = (LOCATION_TYPEINDEX_ ## type) * _LOCATION_ARRAY_SIZE; \
	static const int LOCATION_ ## type ## 1 = LOCATION_ ## type ## 0 + 1; \
	static const int LOCATION_ ## type ## 2 = LOCATION_ ## type ## 0 + 2; \
	static const int LOCATION_ ## type ## 3 = LOCATION_ ## type ## 0 + 3; \
	static const int LOCATION_ ## type ## 4 = LOCATION_ ## type ## 0 + 4; \
	static const int LOCATION_ ## type ## 5 = LOCATION_ ## type ## 0 + 5; \
	static const int LOCATION_ ## type ## 6 = LOCATION_ ## type ## 0 + 6; \
	static const int LOCATION_ ## type ## 7 = LOCATION_ ## type ## 0 + 7; \

	DECLARE_LOCATION_TYPE_ARRAY(POSITION, 0);
	DECLARE_LOCATION_TYPE_ARRAY(TEXCOORD, 1);
	DECLARE_LOCATION_TYPE_ARRAY(COLOR, 2);
	DECLARE_LOCATION_TYPE_ARRAY(NORMAL, 3);

	DECLARE_LOCATION_TYPE_UNIQUE(FOG, 4);
	DECLARE_LOCATION_TYPE_UNIQUE(BLENDWEIGHT, 5);
	DECLARE_LOCATION_TYPE_UNIQUE(BLENDINDICES, 6);

	static const int _LOCATION_TYPE_COUNT = 7; // KEEP THIS UPDATED!!!
	static const int _LOCATION_COUNT = _LOCATION_TYPE_COUNT * _LOCATION_ARRAY_SIZE;

#ifdef __cplusplus

#pragma pop_macro("DECLARE_LOCATION_TYPE_UNIQUE")
#pragma pop_macro("DECLARE_LOCATION_TYPE_ARRAY")

} }
#endif // __cplusplus
#endif // INCLUDE_GUARD_SHADER_SHARED_H
