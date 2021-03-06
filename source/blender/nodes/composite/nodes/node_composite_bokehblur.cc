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
 * The Original Code is Copyright (C) 2006 Blender Foundation.
 * All rights reserved.
 * Juho Vepsäläinen
 */

/** \file
 * \ingroup cmpnodes
 */

#include "../node_composite_util.hh"

/* **************** BLUR ******************** */
static bNodeSocketTemplate cmp_node_bokehblur_in[] = {
    {SOCK_RGBA, N_("Image"), 0.8f, 0.8f, 0.8f, 1.0f, 0.0f, 1.0f},
    {SOCK_RGBA, N_("Bokeh"), 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f},
    {SOCK_FLOAT, N_("Size"), 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 10.0f},
    {SOCK_FLOAT, N_("Bounding box"), 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f},
    {-1, ""}};

static bNodeSocketTemplate cmp_node_bokehblur_out[] = {
    {SOCK_RGBA, N_("Image"), 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f}, {-1, ""}};

static void node_composit_init_bokehblur(bNodeTree *UNUSED(ntree), bNode *node)
{
  node->custom3 = 4.0f;
  node->custom4 = 16.0f;
}

void register_node_type_cmp_bokehblur(void)
{
  static bNodeType ntype;

  cmp_node_type_base(&ntype, CMP_NODE_BOKEHBLUR, "Bokeh Blur", NODE_CLASS_OP_FILTER, 0);
  node_type_socket_templates(&ntype, cmp_node_bokehblur_in, cmp_node_bokehblur_out);
  node_type_init(&ntype, node_composit_init_bokehblur);

  nodeRegisterType(&ntype);
}
