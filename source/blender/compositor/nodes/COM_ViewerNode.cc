/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * Copyright 2011, Blender Foundation.
 */

#include "COM_ViewerNode.h"

#include "COM_ViewerOperation.h"

namespace blender::compositor {

ViewerNode::ViewerNode(bNode *editor_node) : Node(editor_node)
{
  /* pass */
}

void ViewerNode::convert_to_operations(NodeConverter &converter,
                                       const CompositorContext &context) const
{
  bNode *editor_node = this->get_bnode();
  bool do_output = (editor_node->flag & NODE_DO_OUTPUT_RECALC || context.is_rendering()) &&
                   (editor_node->flag & NODE_DO_OUTPUT);
  bool ignore_alpha = (editor_node->custom2 & CMP_NODE_OUTPUT_IGNORE_ALPHA) != 0;

  NodeInput *image_socket = this->get_input_socket(0);
  NodeInput *alpha_socket = this->get_input_socket(1);
  NodeInput *depth_socket = this->get_input_socket(2);
  Image *image = (Image *)this->get_bnode()->id;
  ImageUser *image_user = (ImageUser *)this->get_bnode()->storage;
  ViewerOperation *viewer_operation = new ViewerOperation();
  viewer_operation->set_bnodetree(context.get_bnodetree());
  viewer_operation->set_image(image);
  viewer_operation->set_image_user(image_user);
  viewer_operation->set_chunk_order((ChunkOrdering)editor_node->custom1);
  viewer_operation->setCenterX(editor_node->custom3);
  viewer_operation->setCenterY(editor_node->custom4);
  /* alpha socket gives either 1 or a custom alpha value if "use alpha" is enabled */
  viewer_operation->set_use_alpha_input(ignore_alpha || alpha_socket->is_linked());
  viewer_operation->set_render_data(context.get_render_data());
  viewer_operation->set_view_name(context.get_view_name());

  viewer_operation->set_view_settings(context.get_view_settings());
  viewer_operation->set_display_settings(context.get_display_settings());

  viewer_operation->set_canvas_input_index(0);
  if (!image_socket->is_linked()) {
    if (alpha_socket->is_linked()) {
      viewer_operation->set_canvas_input_index(1);
    }
  }

  converter.add_operation(viewer_operation);
  converter.map_input_socket(image_socket, viewer_operation->get_input_socket(0));
  /* only use alpha link if "use alpha" is enabled */
  if (ignore_alpha) {
    converter.add_input_value(viewer_operation->get_input_socket(1), 1.0f);
  }
  else {
    converter.map_input_socket(alpha_socket, viewer_operation->get_input_socket(1));
  }
  converter.map_input_socket(depth_socket, viewer_operation->get_input_socket(2));

  converter.add_node_input_preview(image_socket);

  if (do_output) {
    converter.register_viewer(viewer_operation);
  }
}

}  // namespace blender::compositor