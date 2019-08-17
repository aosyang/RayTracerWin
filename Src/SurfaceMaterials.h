//=============================================================================
// SurfaceMaterials.h by Shiyang Ao, 2019 All Rights Reserved.
//
// 
//=============================================================================

#pragma once

#include "RVector.h"
#include "RRay.h"

#include <memory>

/// The surface material interface
class ISurfaceMaterial
{
public:
	virtual ~ISurfaceMaterial() {}

	/// Bounce a view ray against a surface of current material. Returns remaining amount of light after surface absorption.
	virtual RVec3 BounceViewRay(const RRay& InViewRay, const RayHitResult& HitResult, RRay& OutViewRay) const = 0;

	/// Get a preview color for this material used when rendering the base color
	virtual RVec3 PreviewColor(const RayHitResult& HitResult) const = 0;
};

/// Diffuse material
class SurfaceMaterial_Diffuse : public ISurfaceMaterial
{
public:
	SurfaceMaterial_Diffuse(const RVec3 InAlbedo = RVec3(1.0f, 1.0f, 1.0f));

	virtual RVec3 BounceViewRay(const RRay& InViewRay, const RayHitResult& HitResult, RRay& OutViewRay) const override;
	virtual RVec3 PreviewColor(const RayHitResult& HitResult) const override;
private:
	RVec3 Albedo;
};

/// Reflective material
class SurfaceMaterial_Reflective : public ISurfaceMaterial
{
public:
	SurfaceMaterial_Reflective(const RVec3 InAlbedo = RVec3(1.0f, 1.0f, 1.0f));

	virtual RVec3 BounceViewRay(const RRay& InViewRay, const RayHitResult& HitResult, RRay& OutViewRay) const override;
	virtual RVec3 PreviewColor(const RayHitResult& HitResult) const override;
private:
	RVec3 Albedo;
};

/// A blend material is used to blend two types of surface materials
class SurfaceMaterial_Blend : public ISurfaceMaterial
{
public:
	SurfaceMaterial_Blend(std::unique_ptr<ISurfaceMaterial> InMaterialA, std::unique_ptr<ISurfaceMaterial> InMaterialB, float InBlendFactor);

	virtual RVec3 BounceViewRay(const RRay& InViewRay, const RayHitResult& HitResult, RRay& OutViewRay) const override;
	virtual RVec3 PreviewColor(const RayHitResult& HitResult) const override;
private:
	std::unique_ptr<ISurfaceMaterial> BlendMaterialA;
	std::unique_ptr<ISurfaceMaterial> BlendMaterialB;
	float BlendFactor;
};

/// Surface material that allows light to pass through
class SurfaceMaterial_Null : public ISurfaceMaterial
{
public:
	SurfaceMaterial_Null()
	{
	}

	virtual RVec3 BounceViewRay(const RRay& InViewRay, const RayHitResult& HitResult, RRay& OutViewRay) const override;
	virtual RVec3 PreviewColor(const RayHitResult& HitResult) const = 0;
};
