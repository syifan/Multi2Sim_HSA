/*
 *  Multi2Sim
 *  Copyright (C) 2014  Amir Kavyan Ziabari (aziabari@ece.neu.edu)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef NETWORK_NETWORK_H
#define NETWORK_NETWORK_H

#include "System.h"

namespace net
{

class Node;
class Link;


class Network
{

	//
	// Class Members
	//

	// Network Name
	static std::string name;

	// Message ID counter
	static long long msg_id_counter;

	// Node list of the Network
	static const std::vector<std::unique_ptr<Node>> node_list;

	// Total number of End nodes in the Network
	static int end_node_count;

	// Link List of the Network
	static const std::vector<std::unique_ptr<Link>> link_list;

	// Unique instance of singleton
	static std::unique_ptr<Network> network;

	// Private constructor for singleton
	Network();

public:

	/// Get instance of singleton
	static Network *getInstance();


};
}  // namespace net

#endif

