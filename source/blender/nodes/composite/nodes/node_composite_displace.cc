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
 */

/** \file
 * \ingroup cmpnodes
 */

#include "node_composite_util.hh"

/* **************** Displace  ******************** */

static bNodeSocketTemplate cmp_node_displace_in[] = {
    {SOCK_RGBA, N_("Image"), 1.0f, 1.0f, 1.0f, 1.0f},
    {SOCK_VECTOR, N_("Vector"), 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, PROP_TRANSLATION},
    {SOCK_FLOAT, N_("X Scale"), 0.0f, 0.0f, 0.0f, 0.0f, -1000.0f, 1000.0f, PROP_NONE},
    {SOCK_FLOAT, N_("Y Scale"), 0.0f, 0.0f, 0.0f, 0.0f, -1000.0f, 1000.0f, PROP_NONE},
    {-1, ""},
};
static bNodeSocketTemplate cmp_node_displace_out[] = {
    {SOCK_RGBA, N_("Image")},
    {-1, ""},
};

void register_node_type_cmp_displace(void)
{
  static bNodeType ntype;

  cmp_node_type_base(&ntype, CMP_NODE_DISPLACE, "Displace", NODE_CLASS_DISTORT, 0);
  node_type_socket_templates(&ntype, cmp_node_displace_in, cmp_node_displace_out);

  nodeRegisterType(&ntype);
}
