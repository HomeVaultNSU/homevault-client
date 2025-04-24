#include <core/ApiClient.hpp>
#include <core/Models.hpp>

#include <curlpp/cURLpp.hpp>
#include <curlpp/Easy.hpp>
#include <curlpp/Infos.hpp>
#include <curlpp/Options.hpp>
#include <curlpp/Form.hpp>

#include <fstream>
#include <iostream>
#include <sstream>

namespace hv {

ApiClient::ApiClient(const std::string& baseUrl)
    : baseUrl(baseUrl)
{
    // Initialize curlpp
    curlpp::initialize();
}

DirectoryListing ApiClient::getDirectoryListing(
    const std::string& path, int depth)
{
    std::map<std::string, std::string> params;
    params["path"] = path;
    params["depth"] = std::to_string(depth);

    try
    {
        nlohmann::json response = get("/list", params);
        return DirectoryListing::fromJson(response);
    }
    catch (const curlpp::RuntimeError& e)
    {
        throw HomevaultServerException(std::string("Network error: ") +
                                       e.what());
    }
}

UploadResponse ApiClient::uploadFile(const std::string& filePath,
                                              const std::string& targetDir)
{
    std::map<std::string, std::string> params;
    params["path"] = targetDir;

    try
    {
        nlohmann::json response = postMultipart("/upload", filePath, params);
        return UploadResponse::fromJson(response);
    }
    catch (const curlpp::RuntimeError& e)
    {
        throw HomevaultServerException(
            std::string("Network error during upload: ") + e.what());
    }
    catch (const std::ifstream::failure& e)
    {
        throw HomevaultBadRequestException(std::string("File error: ") +
                                           e.what());
    }
}

std::vector<uint8_t> ApiClient::downloadFile(const std::string& path)
{
    std::map<std::string, std::string> params;
    params["path"] = path;

    try
    {
        return getBinary("/download", params);
    }
    catch (const curlpp::RuntimeError& e)
    {
        throw HomevaultServerException(
            std::string("Network error during download: ") + e.what());
    }
}

nlohmann::json ApiClient::get(
    const std::string& endpoint,
    const std::map<std::string, std::string>& params)
{
    std::string url = buildUrl(endpoint, params);

    try
    {
        curlpp::Easy request;
        request.setOpt(curlpp::options::Url(url));

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

std::vector<uint8_t> ApiClient::getBinary(
    const std::string& endpoint,
    const std::map<std::string, std::string>& params)
{
    std::string url = buildUrl(endpoint, params);

    try
    {
        curlpp::Easy request;
        request.setOpt(curlpp::options::Url(url));

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

nlohmann::json ApiClient::post(
    const std::string& endpoint,
    const std::map<std::string, std::string>& params)
{
    std::string url = buildUrl(endpoint);

    try
    {
        curlpp::Easy request;
        request.setOpt(curlpp::options::Url(url));

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

nlohmann::json ApiClient::postMultipart(
    const std::string& endpoint, const std::string& filePath,
    const std::map<std::string, std::string>& params)
{
    std::string url = buildUrl(endpoint);

    try
    {
        // Open the file to upload
        std::ifstream fileStream(filePath, std::ios::binary);
        if (!fileStream)
        {
            throw std::ifstream::failure("Could not open file: " + filePath);
        }

        // Get file size
        fileStream.seekg(0, std::ios::end);
        std::streampos fileSize = fileStream.tellg();
        fileStream.seekg(0, std::ios::beg);

        // Read file content
        std::vector<char> fileContent(fileSize);
        fileStream.read(fileContent.data(), fileSize);

        // Get the filename from path
        std::string filename = filePath;
        size_t lastSlash = filename.find_last_of("/\\");
        if (lastSlash != std::string::npos)
        {
            filename = filename.substr(lastSlash + 1);
        }

        // Setup curl for multipart form data
        curlpp::Easy request;
        request.setOpt(curlpp::options::Url(url));

        // Setup form
        curlpp::Forms formParts;

        // TODO: this part is not compiling because curlpp::Form does not exist
        //
        (void) params;

        // Add file part
        // curlpp::Form fileForm;
        // fileForm.add(curlpp::FormParts::Name("file"));
        // fileForm.add(
        //     curlpp::FormParts::ContentType("application/octet-stream"));
        // fileForm.add(
        //     curlpp::FormParts::Content(fileContent.data(), fileContent.size()));
        // fileForm.add(curlpp::FormParts::Filename(filename));
        // formParts.push_back(fileForm);

        // // Add other parameters
        // for (const auto& [key, value] : params)
        // {
        //     curlpp::Form paramForm;
        //     paramForm.add(curlpp::FormParts::Name(key));
        //     paramForm.add(curlpp::FormParts::Content(value));
        //     formParts.push_back(paramForm);
        // }

        request.setOpt(curlpp::options::HttpPost(formParts));

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
            url += key + "=" + curlpp::escape(value);
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

} // namespace hv
