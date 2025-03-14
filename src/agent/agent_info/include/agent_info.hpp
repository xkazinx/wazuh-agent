#pragma once

#include <iagent_info.hpp>
#include <iagent_info_persistence.hpp>

#include <nlohmann/json.hpp>

#include <functional>
#include <memory>
#include <string>
#include <vector>

/// @brief Stores and manages information about an agent.
///
/// This class provides methods for getting and setting the agent's name, key,
/// UUID, and groups. It also includes private methods for creating and
/// validating the key.
class AgentInfo : public IAgentInfo
{
public:
    /// @brief Constructs an AgentInfo object with OS and network information retrieval functions.
    ///
    /// This constructor initializes the AgentInfo object by setting up OS and network
    /// information retrieval functions. It also generates a UUID for the agent if one
    /// does not already exist, and loads endpoint, metadata, and header information.
    ///
    /// @param dbFolderPath Path to the database folder.
    /// @param getOSInfo Function to retrieve OS information in JSON format.
    /// @param getNetworksInfo Function to retrieve network information in JSON format.
    /// @param agentIsEnrolling True if the agent is enrolling, false otherwise.
    /// @param persistence Optional pointer to an IAgentInfoPersistence object.
    AgentInfo(const std::string& dbFolderPath,
              std::function<nlohmann::json()> getOSInfo = nullptr,
              std::function<nlohmann::json()> getNetworksInfo = nullptr,
              bool agentIsEnrolling = false,
              std::shared_ptr<IAgentInfoPersistence> persistence = nullptr);

    /// @copydoc IAgentInfo::GetName
    std::string GetName() const override;

    /// @copydoc IAgentInfo::GetKey
    std::string GetKey() const override;

    /// @copydoc IAgentInfo::GetUUID
    std::string GetUUID() const override;

    /// @copydoc IAgentInfo::GetGroups
    std::vector<std::string> GetGroups() const override;

    /// @copydoc IAgentInfo::SetName
    bool SetName(const std::string& name) override;

    /// @copydoc IAgentInfo::SetKey
    bool SetKey(const std::string& key) override;

    /// @copydoc IAgentInfo::SetUUID
    void SetUUID(const std::string& uuid) override;

    /// @copydoc IAgentInfo::SetGroups
    void SetGroups(const std::vector<std::string>& groupList) override;

    /// @copydoc IAgentInfo::GetType
    std::string GetType() const override;

    /// @copydoc IAgentInfo::GetVersion
    std::string GetVersion() const override;

    /// @copydoc IAgentInfo::GetHeaderInfo
    std::string GetHeaderInfo() const override;

    /// @copydoc IAgentInfo::GetMetadataInfo
    std::string GetMetadataInfo() const override;

    /// @copydoc IAgentInfo::Save
    void Save() const override;

    /// @copydoc IAgentInfo::SaveGroups
    bool SaveGroups() const override;

private:
    /// @brief Creates a random key for the agent.
    ///
    /// The key is 32 alphanumeric characters.
    ///
    /// @return A randomly generated key.
    std::string CreateKey() const;

    /// @brief Validates a given key.
    /// @param key The key to validate.
    /// @return True if the key is valid (32 alphanumeric characters), false otherwise.
    bool ValidateKey(const std::string& key) const;

    /// @brief Loads the endpoint information into `m_endpointInfo`.
    void LoadEndpointInfo();

    /// @brief Loads the header information into `m_headerInfo`.
    void LoadHeaderInfo();

    /// @brief Extracts the active IP address from the network JSON data.
    /// @param networksJson JSON object containing network interface information.
    /// @return Vector of strings with the active IP addresses.
    std::vector<std::string> GetActiveIPAddresses(const nlohmann::json& networksJson) const;

    /// @brief The agent's name.
    std::string m_name;

    /// @brief The agent's key.
    std::string m_key;

    /// @brief The agent's UUID.
    std::string m_uuid;

    /// @brief The agent's groups.
    std::vector<std::string> m_groups;

    /// @brief The agent's endpoint information.
    nlohmann::json m_endpointInfo;

    /// @brief The agent's header information.
    std::string m_headerInfo;

    /// @brief The OS information
    std::function<nlohmann::json()> m_getOSInfo;

    /// @brief The networks information
    std::function<nlohmann::json()> m_getNetworksInfo;

    /// @brief Specify if the agent is about to enroll.
    bool m_agentIsEnrolling;

    /// @brief Pointer to the agent info persistence instance.
    std::shared_ptr<IAgentInfoPersistence> m_persistence;
};
