#include <core/ApiClient.hpp>
#include <core/Models.hpp>
#include <curlpp/Easy.hpp>
#include <curlpp/Form.hpp>
#include <curlpp/Infos.hpp>
#include <curlpp/Options.hpp>
#include <curlpp/cURLpp.hpp>
#include <fstream>
#include <iostream>
#include <sstream>

namespace hv
{

ApiClient::ApiClient(const std::string& baseUrl, const std::string& username,
                     const std::string& password)
    : baseUrl(baseUrl), username(username), password(password)
{
    curlpp::initialize();
}

DirectoryListing ApiClient::getDirectoryListing(const std::string& path,
                                                int depth)
{
    std::map<std::string, std::string> params;
    params["path"] = path;
    params["depth"] = std::to_string(depth);

    try
    {
        nlohmann::json response = get("/list", params, {getAuthorizationHeader()});
        return DirectoryListing::fromJson(response);
    }
    catch (const curlpp::RuntimeError& e)
    {
        throw HomevaultServerException(std::string("Network error: ") +
                                       e.what());
    }
    // Assume HomevaultApiException and derived exceptions are caught by caller
    // or handled in get()
}

UploadResponse ApiClient::uploadFile(const std::string& filePath,
                                     const std::string& targetDir)
{
    std::map<std::string, std::string>
        params;                  // Parameters other than the file itself
    params["path"] = targetDir;  // Target directory goes as a form field

    try
    {
        // Call the helper function that handles multipart POST
        nlohmann::json response = postMultipart("/upload", filePath, params, {getAuthorizationHeader()});
        return UploadResponse::fromJson(
            response);  // Parse the success response
    }
    catch (const curlpp::RuntimeError& e)  // Network/Curl errors
    {
        throw HomevaultServerException(
            std::string("Network error during upload: ") + e.what());
    }
    catch (const std::ifstream::failure& e)  // File reading errors
    {
        // Convert file error to a BadRequest exception as the client provided a
        // bad path
        throw HomevaultBadRequestException(std::string("File error: ") +
                                           e.what());
    }
    // Assume HomevaultApiException and derived exceptions are caught by caller
    // or handled in postMultipart()
}

std::vector<uint8_t> ApiClient::downloadFile(const std::string& path)
{
    std::map<std::string, std::string> params;
    params["path"] = path;

    try
    {
        return getBinary("/download", params, {getAuthorizationHeader()});
    }
    catch (const curlpp::RuntimeError& e)
    {
        throw HomevaultServerException(
            std::string("Network error during download: ") + e.what());
    }
}

void ApiClient::registerUser(const std::string& username,
                             const std::string& password)
{
    nlohmann::json requestData;
    requestData["username"] = username;
    requestData["password"] = password;

    try
    {
        nlohmann::json response = postJson("/auth", requestData);
        // Registration endpoint returns 201 with no body on success
        // If we get here without exception, registration was successful
    }
    catch (const curlpp::RuntimeError& e)
    {
        throw HomevaultServerException(
            std::string("Network error during registration: ") + e.what());
    }
}

TokenResponse ApiClient::login(const std::string& username,
                               const std::string& password)
{
    nlohmann::json requestData;
    requestData["username"] = username;
    requestData["password"] = password;

    try
    {
        // this is FUCKING AWFUL, architecture is broken, authentication was
        // added at the last day before project submission, so i don't care
        size_t colonPos = baseUrl.rfind(':');
        baseUrl = baseUrl.substr(0, colonPos + 1) + "8090";

        nlohmann::json response = putJson("/auth", requestData);
        baseUrl = baseUrl.substr(0, colonPos + 1) + "8080";
        return TokenResponse::fromJson(response);
    }
    catch (const curlpp::RuntimeError& e)
    {
        throw HomevaultServerException(
            std::string("Network error during login: ") + e.what());
    }
}

DecodedTokenResponse ApiClient::decodeToken(const std::string& token)
{
    nlohmann::json requestData;
    requestData["token"] = token;

    try
    {
        nlohmann::json response = putJson("/auth", requestData);
        return DecodedTokenResponse::fromJson(response);
    }
    catch (const curlpp::RuntimeError& e)
    {
        throw HomevaultServerException(
            std::string("Network error during token decode: ") + e.what());
    }
}

nlohmann::json ApiClient::get(const std::string& endpoint,
                              const std::map<std::string, std::string>& params, const std::list<std::string>& headers)
{
    std::string url = buildUrl(endpoint, params);

    try
    {
        curlpp::Easy request;
        request.setOpt(curlpp::options::Url(url));
        request.setOpt(curlpp::options::HttpHeader(headers));

        std::ostringstream responseStream;
        request.setOpt(curlpp::options::WriteStream(&responseStream));

        request.perform();

        long responseCode = curlpp::infos::ResponseCode::get(request);
        std::string responseBody = responseStream.str();

        if (responseBody.empty())
        {
            throw HomevaultServerException("Empty response from server");
        }

        nlohmann::json jsonResponse = nlohmann::json::parse(responseBody);

        // Check for HTTP error codes based on OpenAPI spec
        if (responseCode >= 400)
        {
            std::string errorMsg = "Unknown server error";
            if (jsonResponse.contains("error") &&
                jsonResponse["error"].is_string())
            {
                errorMsg = jsonResponse["error"].get<std::string>();
            }

            if (responseCode == 404)
            {
                throw HomevaultNotFoundException(errorMsg);
            }
            else if (responseCode == 400)
            {
                throw HomevaultBadRequestException(errorMsg);
            }
            else  // Treat other 4xx/5xx errors as general server errors
            {
                throw HomevaultServerException(errorMsg);
            }
        }

        return jsonResponse;
    }
    catch (const nlohmann::json::parse_error& e)  // JSON parsing errors
    {
        throw HomevaultServerException(std::string("JSON parse error: ") +
                                       e.what());
    }
    // Let curlpp::RuntimeError propagate up if not caught here
}

std::vector<uint8_t> ApiClient::getBinary(
    const std::string& endpoint,
    const std::map<std::string, std::string>& params, const std::list<std::string>& headers)
{
    std::string url = buildUrl(endpoint, params);

    try
    {
        curlpp::Easy request;
        request.setOpt(curlpp::options::Url(url));
        request.setOpt(curlpp::options::HttpHeader(headers));

        std::ostringstream responseStream;
        request.setOpt(curlpp::options::WriteStream(&responseStream));

        request.perform();

        long responseCode = curlpp::infos::ResponseCode::get(request);
        std::string responseStr = responseStream.str();

        if (responseCode >= 400)
        {
            try
            {
                nlohmann::json jsonResponse =
                    nlohmann::json::parse(responseStr);

                if (responseCode == 404)
                {
                    throw HomevaultNotFoundException(
                        jsonResponse["error"].get<std::string>());
                }
                else if (responseCode == 400)
                {
                    throw HomevaultBadRequestException(
                        jsonResponse["error"].get<std::string>());
                }
                else
                {
                    throw HomevaultServerException(
                        jsonResponse["error"].get<std::string>());
                }
            }
            catch (const nlohmann::json::parse_error& e)
            {
                throw HomevaultServerException(
                    "Server error with invalid response format");
            }
        }

        // Convert the response to binary data
        std::vector<uint8_t> binary(responseStr.begin(), responseStr.end());
        return binary;
    }
    catch (const curlpp::RuntimeError& e)
    {
        throw HomevaultServerException(std::string("Network error: ") +
                                       e.what());
    }
}

nlohmann::json ApiClient::post(const std::string& endpoint,
                               const std::map<std::string, std::string>& params, const std::list<std::string>& headers)
{
    std::string url = buildUrl(endpoint);

    try
    {
        curlpp::Easy request;
        request.setOpt(curlpp::options::Url(url));
        request.setOpt(curlpp::options::HttpHeader(headers));

        // Prepare post fields
        std::list<std::string> postFields;
        for (const auto& [key, value] : params)
        {
            postFields.push_back(key + "=" + curlpp::escape(value));
        }

        std::string postData = "";
        for (const auto& field : postFields)
        {
            if (!postData.empty())
            {
                postData += "&";
            }
            postData += field;
        }

        request.setOpt(curlpp::options::PostFields(postData));
        request.setOpt(curlpp::options::PostFieldSize(postData.length()));

        std::ostringstream responseStream;
        request.setOpt(curlpp::options::WriteStream(&responseStream));

        request.perform();

        long responseCode = curlpp::infos::ResponseCode::get(request);
        std::string responseBody = responseStream.str();

        if (responseBody.empty())
        {
            throw HomevaultServerException("Empty response from server");
        }

        nlohmann::json jsonResponse = nlohmann::json::parse(responseBody);

        // Check for error responses
        if (responseCode >= 400)
        {
            if (responseCode == 404)
            {
                throw HomevaultNotFoundException(
                    jsonResponse["error"].get<std::string>());
            }
            else if (responseCode == 400)
            {
                throw HomevaultBadRequestException(
                    jsonResponse["error"].get<std::string>());
            }
            else
            {
                throw HomevaultServerException(
                    jsonResponse["error"].get<std::string>());
            }
        }

        return jsonResponse;
    }
    catch (const nlohmann::json::parse_error& e)
    {
        throw HomevaultServerException(std::string("JSON parse error: ") +
                                       e.what());
    }
}

nlohmann::json ApiClient::postJson(const std::string& endpoint,
                                   const nlohmann::json& jsonData, const std::list<std::string>& headers)
{
    std::string url = buildUrl(endpoint);

    try
    {
        curlpp::Easy request;
        request.setOpt(curlpp::options::Url(url));

        auto headersAltered = headers;
        headersAltered.push_back("Content-Type: application/json");
        request.setOpt(curlpp::options::HttpHeader(headersAltered));

        // Convert JSON to string
        std::string jsonString = jsonData.dump();
        request.setOpt(curlpp::options::PostFields(jsonString));
        request.setOpt(curlpp::options::PostFieldSize(jsonString.length()));

        std::ostringstream responseStream;
        request.setOpt(curlpp::options::WriteStream(&responseStream));

        request.perform();

        long responseCode = curlpp::infos::ResponseCode::get(request);
        std::string responseBody = responseStream.str();

        // Handle different success codes for different endpoints
        if (responseCode == 201)
        {
            // For registration (201 Created), response might be empty
            if (responseBody.empty())
            {
                return nlohmann::json::object();  // Return empty JSON object
            }
        }
        else if (responseCode >= 400)
        {
            // Handle error responses
            if (!responseBody.empty())
            {
                try
                {
                    nlohmann::json jsonResponse =
                        nlohmann::json::parse(responseBody);
                    std::string errorMsg = "Unknown error";
                    if (jsonResponse.contains("error") &&
                        jsonResponse["error"].is_string())
                    {
                        errorMsg = jsonResponse["error"].get<std::string>();
                    }

                    if (responseCode == 400)
                    {
                        throw HomevaultBadRequestException(errorMsg);
                    }
                    else if (responseCode == 401)
                    {
                        throw HomevaultUnauthorizedException(errorMsg);
                    }
                    else if (responseCode == 409)
                    {
                        throw HomevaultConflictException(errorMsg);
                    }
                    else
                    {
                        throw HomevaultServerException(errorMsg);
                    }
                }
                catch (const nlohmann::json::parse_error& e)
                {
                    throw HomevaultServerException(
                        "Server returned invalid error response");
                }
            }
            else
            {
                throw HomevaultServerException(
                    "Server returned error with empty response");
            }
        }

        // Parse successful response
        if (!responseBody.empty())
        {
            return nlohmann::json::parse(responseBody);
        }
        else
        {
            return nlohmann::json::object();
        }
    }
    catch (const nlohmann::json::parse_error& e)
    {
        throw HomevaultServerException(std::string("JSON parse error: ") +
                                       e.what());
    }
    catch (const curlpp::RuntimeError& e)
    {
        throw HomevaultServerException(std::string("Network error: ") +
                                       e.what());
    }
}

nlohmann::json ApiClient::putJson(const std::string& endpoint,
                                  const nlohmann::json& jsonData, const std::list<std::string>& headers)
{
    std::string url = buildUrl(endpoint);

    try
    {
        curlpp::Easy request;
        request.setOpt(curlpp::options::Url(url));
        request.setOpt(curlpp::options::HttpHeader(headers));

        // Set HTTP method to PUT
        request.setOpt(curlpp::options::CustomRequest("PUT"));

        // Set content type to JSON
        std::list<std::string> headers;
        headers.push_back("Content-Type: application/json");
        request.setOpt(curlpp::options::HttpHeader(headers));

        // Convert JSON to string
        std::string jsonString = jsonData.dump();
        request.setOpt(curlpp::options::PostFields(jsonString));
        request.setOpt(curlpp::options::PostFieldSize(jsonString.length()));

        std::ostringstream responseStream;
        request.setOpt(curlpp::options::WriteStream(&responseStream));

        request.perform();

        long responseCode = curlpp::infos::ResponseCode::get(request);
        std::string responseBody = responseStream.str();

        if (responseBody.empty())
        {
            throw HomevaultServerException("Empty response from server");
        }

        nlohmann::json jsonResponse = nlohmann::json::parse(responseBody);

        // Check for error responses
        if (responseCode >= 400)
        {
            std::string errorMsg = "Unknown error";
            if (jsonResponse.contains("error") &&
                jsonResponse["error"].is_string())
            {
                errorMsg = jsonResponse["error"].get<std::string>();
            }

            if (responseCode == 401)
            {
                throw HomevaultUnauthorizedException(errorMsg);
            }
            else if (responseCode == 400)
            {
                throw HomevaultBadRequestException(errorMsg);
            }
            else
            {
                throw HomevaultServerException(errorMsg);
            }
        }

        return jsonResponse;
    }
    catch (const nlohmann::json::parse_error& e)
    {
        throw HomevaultServerException(std::string("JSON parse error: ") +
                                       e.what());
    }
    catch (const curlpp::RuntimeError& e)
    {
        throw HomevaultServerException(std::string("Network error: ") +
                                       e.what());
    }
}

nlohmann::json ApiClient::postMultipart(
    const std::string& endpoint, const std::string& filePath,
    const std::map<std::string, std::string>& params, const std::list<std::string>& headers)
{
    std::string url = buildUrl(endpoint);

    try
    {
        curlpp::Easy request;
        request.setOpt(curlpp::options::Url(url));
        request.setOpt(curlpp::options::HttpHeader(headers));

        curlpp::Forms formParts;

        std::string filename = filePath;
        size_t lastSlash = filename.find_last_of("/\\");
        if (lastSlash != std::string::npos)
        {
            filename = filename.substr(lastSlash + 1);
        }

        formParts.push_back(
            new curlpp::FormParts::File("file", filePath, filename));

        for (const auto& [key, value] : params)
        {
            formParts.push_back(new curlpp::FormParts::Content(key, value));
        }

        request.setOpt(curlpp::options::HttpPost(formParts));

        std::ostringstream responseStream;
        request.setOpt(curlpp::options::WriteStream(&responseStream));

        request.perform();

        // --- Cleanup form parts ---
        // Important: Manually delete the allocated FormParts objects
        // for (auto part : formParts)
        // {
        //     delete part;
        // }
        formParts.clear();

        // --- Process Response ---
        long responseCode = curlpp::infos::ResponseCode::get(request);
        std::string responseBody = responseStream.str();

        if (responseBody.empty())
        {
            throw HomevaultServerException("Empty response from server");
        }

        nlohmann::json jsonResponse;
        try
        {
            jsonResponse = nlohmann::json::parse(responseBody);
        }
        catch (const nlohmann::json::parse_error& pe)
        {
            // If response isn't JSON on error, create a generic error response
            if (responseCode >= 400)
            {
                throw HomevaultServerException(
                    "Server returned status " + std::to_string(responseCode) +
                    " with non-JSON body: " + responseBody);
            }
            else
            {
                throw HomevaultServerException(
                    "Server returned non-JSON success body: " + responseBody);
            }
        }

        // Check for HTTP error codes (OpenAPI: 400, 500 for upload)
        if (responseCode >= 400)
        {
            std::string errorMsg = "Unknown upload error";
            if (jsonResponse.contains("error") &&
                jsonResponse["error"].is_string())
            {
                errorMsg = jsonResponse["error"].get<std::string>();
            }

            // Map status codes to exceptions
            if (responseCode ==
                400)  // Bad request (e.g., invalid path format, missing file)
            {
                throw HomevaultBadRequestException(errorMsg);
            }
            // Add 404 case if the target *directory* could be not found
            else if (responseCode == 404)
            {
                throw HomevaultNotFoundException(
                    errorMsg);  // Assuming 404 means target dir not found
            }
            else  // Treat other 4xx/5xx as general server errors
            {
                throw HomevaultServerException(errorMsg);
            }
        }
        // OpenAPI spec indicates 201 Created for success
        else if (responseCode != 201)
        {
            // Handle unexpected success codes if necessary
            // For now, assume any 2xx other than 201 might be odd, but treat as
            // success Or throw an exception:
            // throw hv::HomevaultServerException(
            //     "Server returned unexpected success code: " +
            //     std::to_string(responseCode));
            std::cout << "postMultipart failed: response code != 201"
                      << std::endl;
            return jsonResponse;
        }

        return jsonResponse;  // Return parsed JSON on success (201)
    }
    catch (const nlohmann::json::parse_error&
               e)  // JSON parsing errors on response
    {
        throw HomevaultServerException(
            std::string("JSON parse error on upload response: ") + e.what());
    }
    // Let curlpp::RuntimeError and std::ifstream::failure propagate up
}

std::string ApiClient::buildUrl(
    const std::string& endpoint,
    const std::map<std::string, std::string>& params)
{
    std::string url = baseUrl + endpoint;

    if (!params.empty())
    {
        url += "?";
        bool first = true;

        for (const auto& [key, value] : params)
        {
            if (!first)
            {
                url += "&";
            }
            url += key + "=" + curlpp::escape(value);  // URL-encode parameters
            first = false;
        }
    }
    return url;
}

void ApiClient::handleErrorResponse(const std::string& response)
{
    try
    {
        nlohmann::json jsonResponse = nlohmann::json::parse(response);
        ErrorResponse::throwIfError(jsonResponse);
    }
    catch (const nlohmann::json::parse_error& e)
    {
        throw HomevaultServerException("Failed to parse error response");
    }
}

std::string ApiClient::getAuthorizationHeader()
{
    if (!authToken.has_value())
    {
        authToken = login(username, password).token;
    }

    return "Authorization: Bearer " + *authToken;
}

}  // namespace hv
