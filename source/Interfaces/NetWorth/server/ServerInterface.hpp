/**
 * This file is part of the Fall 2023, CSE 491 course project.
 * @brief A networking interface that allows information to be sent across a  network
 * @note Status: PROTOTYPE
 **/

#pragma once

#include "ServerManager.hpp"
#include <thread>
#include <sstream>
#include "Interfaces/NetWorth/NetworkInterface.hpp"
#include "Worlds/MazeWorld.hpp"

namespace netWorth
{
	using namespace sf;

/**
 * The server that will be running and that allows clients to connect to, acts as a sort of clone
 */
	class ServerInterface : public NetworkingInterface
	{
	private:
		ServerManager* m_manager = nullptr; ///Manager to handle updates of the world

		unsigned short m_world_update_port = 0;  ///Port used by servermanager to handle world updates
	protected:

	public:
		/**
		 * Default constructor (AgentBase)
		 * @param id agent ID
		 * @param name agent name
		 */
		ServerInterface(size_t id, const std::string& name)
			: cse491::InterfaceBase(id, name),
			  NetworkingInterface(id, name)
		{

		}

		bool Initialize() override
		{
			// resolve port and IP from entity properties
			m_ip = sf::IpAddress::resolve(NetworkingInterface::GetProperty<std::string>("client_ip"));
			m_world_update_port = NetworkingInterface::GetProperty<unsigned short>("client_port");
			m_manager = GetProperty<netWorth::ServerManager*>("server_manager");
			return InitialConnection(m_ip, m_port);
		}

		/**
		 * The initial connection for the server to a client
		 * @param sender address of the sender
		 * @param port port of the connection
		 * @return true if successful
		 */
		bool InitialConnection(std::optional<IpAddress>& sender, unsigned short& port)
		{
			Packet sendPkt, recvPkt;
			std::string str;

			bindSocket(m_socket, GetProperty<unsigned short>("server_port"));

			// Await client
			if (!receivePacket(recvPkt, sender, port))
				return false;

			std::cout << sender.value() << " has connected successfully." << std::endl;

			// Acknowledge client
			sendPkt << "Connection established.";
			if (!sendPacket(sendPkt, sender.value(), m_port))
				return false;

			recvPkt.clear();
			// await request for map
			if (!receivePacket(recvPkt, sender, m_port))
				return false;

            GetWorld().SetWorldRunning(true);
			return true;
		}

		/**
		 * The grid that will be sent to the client from the server after the connection
		 * so the client can start asking to make moves
		 * @param grid the grid to send to the server
		 * @param typeOptions different cell types of the world
		 * @param itemMap the items that may be apart of the grid
		 * @param agentMap the agents that may be apart of the grid
		 * @return the grid that will be sent to the client
		 */
		static Packet GridToPacket(const cse491::WorldGrid& grid,
			const cse491::type_options_t& typeOptions,
			const cse491::item_map_t& itemMap,
			const cse491::agent_map_t& agentMap)
		{
			std::vector<std::string> packetGrid(grid.GetHeight());

			// Load the world into the symbol_grid;
			for (size_t y = 0; y < grid.GetHeight(); ++y)
			{
				packetGrid[y].resize(grid.GetWidth());
				for (size_t x = 0; x < grid.GetWidth(); ++x)
				{
					packetGrid[y][x] = typeOptions[grid.At(x, y)].symbol;
				}
			}

			// Add in the agents / entities
			for (const auto& [id, entityPtr] : itemMap)
			{
				cse491::GridPosition pos = entityPtr->GetPosition();
				packetGrid[pos.CellY()][pos.CellX()] = '+';
			}

			for (const auto& [id, agent_ptr] : agentMap)
			{
				cse491::GridPosition pos = agent_ptr->GetPosition();
				char c = '*';
				if (agent_ptr->HasProperty("symbol"))
				{
					c = agent_ptr->GetProperty<char>("symbol");
				}
				packetGrid[pos.CellY()][pos.CellX()] = c;
			}

			// Print out the symbol_grid with a box around it.
			std::ostringstream oss;
			oss << '+' << std::string(grid.GetWidth(), '-') << "+\n";
			for (const auto& row : packetGrid)
			{
				oss << "|";
				for (char cell : row)
				{
					oss << cell;
				}
				oss << "|\n";
			}
			oss << '+' << std::string(grid.GetWidth(), '-') << "+\n";
			std::string gridString = oss.str();

			Packet gridPacket;
			gridPacket << gridString;

			return gridPacket;
		}

		/**
		 * Choose action for player agent (mirror client agent)
		 * @param grid the server-side grid
		 * @param typeOptions different cell types of the world
		 * @param itemMap the items that may be apart of the grid
		 * @param agentMap the agents that may be apart of the grid
		 * @return action ID of the interface
		 */
		size_t SelectAction(const cse491::WorldGrid& grid,
			const cse491::type_options_t& typeOptions,
			const cse491::item_map_t& itemMap,
			const cse491::agent_map_t& agentMap) override
		{
			// send action map to client
			sf::Packet sendPkt = m_manager->ActionMapToPacket();
			std::cout << "Sending action map to " << m_ip.value().toString() << " on port " << m_port << std::endl;
			sendPacket(sendPkt, m_ip.value(), m_port);

			// print server-side map (for test purposes)
			sf::Packet mapPkt = GridToPacket(grid, typeOptions, itemMap, agentMap);
			std::string map;
			mapPkt >> map;
			std::cout << map << std::endl;

			// receive player input
			sf::Packet recvPkt;
			size_t actionID;
            receivePacket(recvPkt, m_ip, m_port);
			recvPkt >> actionID;

            // handle leaving client
			if (actionID == 9999)
			{
                m_manager->JoinClient(GetID());
				m_manager->RemoveFromActionMap(GetID());
				m_manager->RemoveFromUpdatePairs(m_ip.value(), m_world_update_port);
			}

            // return actionId
			return actionID;
		}

	}; // End of class ServerInterface
// End of namespace netWorth
}