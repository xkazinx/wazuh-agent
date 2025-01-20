#include <agent_info.hpp>

#include <agent_info_persistance.hpp>

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <random>
#include <utility>

namespace
{
    constexpr size_t KEY_LENGTH = 32;
    const std::string AGENT_TYPE = "Endpoint";
    const std::string AGENT_VERSION = "5.0.0";
    const std::string PRODUCT_NAME = "WazuhXDR";
} // namespace

AgentInfo::AgentInfo(std::string dbFolderPath,
                     std::function<nlohmann::json()> getOSInfo,
                     std::function<nlohmann::json()> getNetworksInfo,
                     bool agentIsRegistering)
    : m_dataFolderPath(std::move(dbFolderPath))
    , m_agentIsRegistering(agentIsRegistering)
{
    if (!m_agentIsRegistering)
    {
        AgentInfoPersistance agentInfoPersistance(m_dataFolderPath);
        m_name = agentInfoPersistance.GetName();
        m_key = agentInfoPersistance.GetKey();
        m_uuid = agentInfoPersistance.GetUUID();
        m_groups = agentInfoPersistance.GetGroups();
    }

    if (m_uuid.empty())
    {
        m_uuid = boost::uuids::to_string(boost::uuids::random_generator()());
    }

    if (getOSInfo != nullptr)
    {
        m_getOSInfo = std::move(getOSInfo);
    }

    if (getNetworksInfo != nullptr)
    {
        m_getNetworksInfo = std::move(getNetworksInfo);
    }

    LoadEndpointInfo();
    LoadHeaderInfo();
}

std::string AgentInfo::GetName() const
{
    return m_name;
}

std::string AgentInfo::GetKey() const
{
    return m_key;
}

std::string AgentInfo::GetUUID() const
{
    return m_uuid;
}

std::vector<std::string> AgentInfo::GetGroups() const
{
    return m_groups;
}

bool AgentInfo::SetName(const std::string& name)
{
    if (!name.empty())
    {
        m_name = name;
    }
    else if (m_getOSInfo != nullptr)
    {
        m_name = m_getOSInfo().value("hostname", "Unknown");
    }
    else
    {
        return false;
    }

    return true;
}

bool AgentInfo::SetKey(const std::string& key)
{
    if (!key.empty())
    {
        if (!ValidateKey(key))
        {
            return false;
        }
        m_key = key;
    }
    else
    {
        m_key = CreateKey();
    }

    return true;
}

void AgentInfo::SetUUID(const std::string& uuid)
{
    m_uuid = uuid;
}

void AgentInfo::SetGroups(const std::vector<std::string>& groupList)
{
    m_groups = groupList;
}

std::string AgentInfo::CreateKey() const
{
    constexpr std::string_view charset = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";

    std::random_device rd;
    std::mt19937 generator(rd());
    std::uniform_int_distribution<size_t> distribution(0, charset.size() - 1);

    std::string key;
    for (size_t i = 0; i < KEY_LENGTH; ++i)
    {
        key += charset[distribution(generator)];
    }

    return key;
}

bool AgentInfo::ValidateKey(const std::string& key) const
{
    return key.length() == KEY_LENGTH && std::ranges::all_of(key, ::isalnum);
}

std::string AgentInfo::GetType() const
{
    return AGENT_TYPE;
}

std::string AgentInfo::GetVersion() const
{
    return AGENT_VERSION;
}

std::string AgentInfo::GetHeaderInfo() const
{
    return m_headerInfo;
}

std::string AgentInfo::GetMetadataInfo() const
{
    nlohmann::json agentMetadataInfo;
    auto& target = m_agentIsRegistering ? agentMetadataInfo : agentMetadataInfo["agent"];

    target["id"] = GetUUID();
    target["name"] = GetName();
    target["type"] = GetType();
    target["version"] = GetVersion();

    if (!m_endpointInfo.empty())
    {
        target["host"] = m_endpointInfo;
    }

    if (m_agentIsRegistering)
    {
        target["key"] = GetKey();
    }
    else
    {
        target["groups"] = GetGroups();
    }

    return agentMetadataInfo.dump();
}

void AgentInfo::Save() const
{
    AgentInfoPersistance agentInfoPersistance(m_dataFolderPath);
    agentInfoPersistance.ResetToDefault();
    agentInfoPersistance.SetName(m_name);
    agentInfoPersistance.SetKey(m_key);
    agentInfoPersistance.SetUUID(m_uuid);
    agentInfoPersistance.SetGroups(m_groups);
}

bool AgentInfo::SaveGroups() const
{
    AgentInfoPersistance agentInfoPersistance(m_dataFolderPath);
    return agentInfoPersistance.SetGroups(m_groups);
}

std::vector<std::string> AgentInfo::GetActiveIPAddresses(const nlohmann::json& networksJson) const
{
    std::vector<std::string> ipAddresses;

    if (networksJson.contains("iface"))
    {
        for (const auto& iface : networksJson["iface"])
        {
            if (iface.contains("state") && iface["state"] == "up")
            {
                if (iface.contains("IPv4") && !iface["IPv4"].empty())
                {
                    ipAddresses.emplace_back(iface["IPv4"][0].value("address", ""));
                }
                if (iface.contains("IPv6") && !iface["IPv6"].empty())
                {
                    ipAddresses.emplace_back(iface["IPv6"][0].value("address", ""));
                }
            }
        }
    }
    return ipAddresses;
}

void AgentInfo::LoadEndpointInfo()
{
    if (m_getOSInfo != nullptr)
    {
        nlohmann::json osInfo = m_getOSInfo();
        m_endpointInfo["hostname"] = osInfo.value("hostname", "Unknown");
        m_endpointInfo["architecture"] = osInfo.value("architecture", "Unknown");
        m_endpointInfo["os"] = nlohmann::json::object();
        m_endpointInfo["os"]["name"] = osInfo.value("os_name", "Unknown");
        m_endpointInfo["os"]["type"] = osInfo.value("sysname", "Unknown");
        m_endpointInfo["os"]["version"] = osInfo.value("os_version", "Unknown");
    }

    if (m_getNetworksInfo != nullptr)
    {
        nlohmann::json networksInfo = m_getNetworksInfo();
        m_endpointInfo["ip"] = GetActiveIPAddresses(networksInfo);
    }
}

void AgentInfo::LoadHeaderInfo()
{
    if (!m_endpointInfo.empty() && m_endpointInfo.contains("os"))
    {
        m_headerInfo = PRODUCT_NAME + "/" + GetVersion() + " (" + GetType() + "; " +
                       m_endpointInfo.value("architecture", "Unknown") + "; " +
                       m_endpointInfo["os"].value("type", "Unknown") + ")";
    }
    else
    {
        m_headerInfo = PRODUCT_NAME + "/" + GetVersion() + " (" + GetType() + "; Unknown; Unknown)";
    }
}
