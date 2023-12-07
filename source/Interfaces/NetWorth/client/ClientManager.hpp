/**
 * This file is part of the Fall 2023, CSE 491 course project.
 * @brief A networking interface that allows information to be sent across a  network
 * @note Status: PROTOTYPE
 **/

#pragma once
#include <map>
#include <sstream>
#include <vector>

namespace netWorth{
    using namespace sf;

    /**
     * The server that will be running and that allows clients to connect to
     */
    class ClientManager {
    private:
        sf::UdpSocket *m_socket = nullptr;                /// Socket shared with ClientInterface
        sf::UdpSocket *m_game_update_socket = nullptr;    /// Game update socket (for agent updates)
        std::optional<sf::IpAddress> m_ip;      /// Server IP address
		unsigned short m_update_port = 0; ///Port to update the game
        std::unordered_map<size_t, size_t> m_action_map;     ///Map of agent IDs to most recent action selected
        size_t m_client_id = 0;		///Id of client
    protected:

    public:

        /**
         * Default constructor (AgentBase)
         * @param id agent ID
         * @param name agent name
         */
        ClientManager()= default;

		void SetUpdatePort(unsigned short port) {m_update_port = port;}

        /**
         * Turn packet from server into action map for ControlledAgents
         * @param pkt received packet
         */
        void PacketToActionMap(sf::Packet pkt) {
            size_t data_size, agent_id, action_id;
            pkt >> data_size;
            for (size_t i = 0; i < data_size; i++) {
                pkt >> agent_id >> action_id;
                m_action_map[agent_id] = action_id;
            }
        }

        /**
         * Set receiving socket for action map and IP/port info
         * @param socket pointer to ClientInterface's socket
         * @param ip server IP
         * @param port server port
         */
        void SetupSocket(sf::UdpSocket *socket, std::optional<sf::IpAddress> ip) {
            m_socket = socket;
            m_ip = ip;
        }

        /**
         * Set socket for game updates
         * @param socket pointer to socket
         */
        void SetupGameUpdateSocket(sf::UdpSocket *socket) {
            m_game_update_socket = socket;
            m_game_update_socket->setBlocking(false);
			if (m_game_update_socket->bind(m_update_port) != Socket::Status::Done){
				std::cerr << "Failed to setup game update socket" << std::endl;
			}
        }

        /**
         * Check if Agent ID is present in agent action map
         * @param id Agent ID
         * @return true if ID is present
         */
        bool IdPresent(size_t id) {
            return m_action_map.contains(id);
        }

        /**
         * Return action ID correspoding to agent ID
         * @param id Agent ID
         * @return action ID
         */
        size_t GetActionID(size_t id) {
            return m_action_map[id];
        }

        /**
         * Clear action map after ClientInterface moves
         */
        void ClearActionMap() {
            m_action_map.clear();
        }

        /**
         * Receive serialized agent data for midgame updates
         * @return serialized data (or empty if no update)
         */
        std::string GetSerializedAgents() {
            sf::Packet recv_pkt;
            std::optional<sf::IpAddress> temp_ip;
            unsigned short temp_port;
			std::cout << m_game_update_socket->getLocalPort() << std::endl;
            if (m_game_update_socket->receive(recv_pkt, temp_ip, temp_port) == sf::Socket::Status::Done) {
                std::string data;
                recv_pkt >> data;
                std::cout << "pong!" << std::endl;
                return data;
            }
            return "";
        }


        void SetClientID(size_t id) {
            m_client_id = id;
        }

        size_t GetClientID() const{
            return m_client_id;
        }

    }; // End of class ClientManager
} // End of namespace netWorth