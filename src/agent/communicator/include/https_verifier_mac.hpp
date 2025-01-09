#include <icertificate_utils.hpp>

#include <CoreFoundation/CoreFoundation.h>
#include <Security/Security.h>
#include <boost/asio/ssl.hpp>

#include <memory>
#include <string>

namespace https_socket_verify_utils
{
    class HttpsVerifier
    {
    public:
        /// @brief Constructor to initialize the verifier object.
        /// @param mode The verification mode to use
        /// @param host The hostname to verify against
        /// @param utils The certificate utilities object to use
        HttpsVerifier(const std::string& mode, const std::string& host, std::unique_ptr<ICertificateUtils>& utils)
            : m_mode(mode)
            , m_host(host)
            , m_utils(utils)
        {
        }

        /// @brief Verifies the certificate of the HTTPS connection
        /// @param ctx The verification context
        /// @return True if the certificate is valid, false otherwise
        bool Verify(boost::asio::ssl::verify_context& ctx);

    private:
        /// @brief Extracts the certificate from the verification context
        /// @param ctx The verification context
        /// @param certData The extracted certificate
        /// @return True if the certificate was extracted successfully, false otherwise
        bool ExtractCertificate(boost::asio::ssl::verify_context& ctx, CFDataRef& certData);

        /// @brief Creates a trust object from the certificate
        /// @param certData The certificate data
        /// @param trust The created trust object
        /// @return True if the trust object was created successfully, false otherwise
        bool CreateTrustObject(CFDataRef certData, SecTrustRef& trust);

        /// @brief Evaluates the trust status of the trust object
        /// @param trust The trust object to evaluate
        /// @return True if the trust evaluation succeeded, false otherwise
        bool EvaluateTrust(SecTrustRef trust);

        /// @brief Validates the hostname of the server certificate
        /// @param cert The server certificate
        /// @return True if the hostname is valid, false otherwise
        bool ValidateHostname(SecCertificateRef cert);

        /// @brief The verification mode to use
        std::string m_mode;

        /// @brief The hostname to verify against
        std::string m_host;

        /// @brief The certificate utilities object to use
        std::unique_ptr<ICertificateUtils>& m_utils;
    };
} // namespace https_socket_verify_utils
