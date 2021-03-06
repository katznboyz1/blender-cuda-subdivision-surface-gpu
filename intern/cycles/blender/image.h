/*
 * Copyright 2011-2020 Blender Foundation
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

#ifndef __BLENDER_IMAGE_H__
#define __BLENDER_IMAGE_H__

#include "RNA_blender_cpp.h"

#include "scene/image.h"

CCL_NAMESPACE_BEGIN

class BlenderImageLoader : public ImageLoader {
 public:
  BlenderImageLoader(BL::Image b_image, int frame);

  bool load_metadata(const ImageDeviceFeatures &features, ImageMetaData &metadata) override;
  bool load_pixels(const ImageMetaData &metadata,
                   void *pixels,
                   const size_t pixels_size,
                   const bool associate_alpha) override;
  string name() const override;
  bool equals(const ImageLoader &other) const override;

  BL::Image b_image;
  int frame;
  bool free_cache;
};

class BlenderPointDensityLoader : public ImageLoader {
 public:
  BlenderPointDensityLoader(BL::Depsgraph depsgraph, BL::ShaderNodeTexPointDensity b_node);

  bool load_metadata(const ImageDeviceFeatures &features, ImageMetaData &metadata) override;
  bool load_pixels(const ImageMetaData &metadata,
                   void *pixels,
                   const size_t pixels_size,
                   const bool associate_alpha) override;
  string name() const override;
  bool equals(const ImageLoader &other) const override;

  BL::Depsgraph b_depsgraph;
  BL::ShaderNodeTexPointDensity b_node;
};

CCL_NAMESPACE_END

#endif /* __BLENDER_IMAGE_H__ */
