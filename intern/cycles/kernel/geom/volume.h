/*
 * Copyright 2011-2013 Blender Foundation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/* Volume Primitive
 *
 * Volumes are just regions inside meshes with the mesh surface as boundaries.
 * There isn't as much data to access as for surfaces, there is only a position
 * to do lookups in 3D voxel or procedural textures.
 *
 * 3D voxel textures can be assigned as attributes per mesh, which means the
 * same shader can be used for volume objects with different densities, etc. */

#pragma once

CCL_NAMESPACE_BEGIN

#ifdef __VOLUME__

/* Return position normalized to 0..1 in mesh bounds */

ccl_device_inline float3 volume_normalized_position(KernelGlobals kg,
                                                    ccl_private const ShaderData *sd,
                                                    float3 P)
{
  /* todo: optimize this so it's just a single matrix multiplication when
   * possible (not motion blur), or perhaps even just translation + scale */
  const AttributeDescriptor desc = find_attribute(kg, sd, ATTR_STD_GENERATED_TRANSFORM);

  object_inverse_position_transform(kg, sd, &P);

  if (desc.offset != ATTR_STD_NOT_FOUND) {
    Transform tfm = primitive_attribute_matrix(kg, sd, desc);
    P = transform_point(&tfm, P);
  }

  return P;
}

ccl_device float volume_attribute_value_to_float(const float4 value)
{
  return average(float4_to_float3(value));
}

ccl_device float volume_attribute_value_to_alpha(const float4 value)
{
  return value.w;
}

ccl_device float3 volume_attribute_value_to_float3(const float4 value)
{
  if (value.w > 1e-6f && value.w != 1.0f) {
    /* For RGBA colors, unpremultiply after interpolation. */
    return float4_to_float3(value) / value.w;
  }
  else {
    return float4_to_float3(value);
  }
}

ccl_device float4 volume_attribute_float4(KernelGlobals kg,
                                          ccl_private const ShaderData *sd,
                                          const AttributeDescriptor desc)
{
  if (desc.element & (ATTR_ELEMENT_OBJECT | ATTR_ELEMENT_MESH)) {
    return kernel_tex_fetch(__attributes_float3, desc.offset);
  }
  else if (desc.element == ATTR_ELEMENT_VOXEL) {
    /* todo: optimize this so we don't have to transform both here and in
     * kernel_tex_image_interp_3d when possible. Also could optimize for the
     * common case where transform is translation/scale only. */
    float3 P = sd->P;
    object_inverse_position_transform(kg, sd, &P);
    InterpolationType interp = (sd->flag & SD_VOLUME_CUBIC) ? INTERPOLATION_CUBIC :
                                                              INTERPOLATION_NONE;
    return kernel_tex_image_interp_3d(kg, desc.offset, P, interp);
  }
  else {
    return make_float4(0.0f, 0.0f, 0.0f, 0.0f);
  }
}

#endif

CCL_NAMESPACE_END
