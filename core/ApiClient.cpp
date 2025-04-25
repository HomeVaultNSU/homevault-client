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

ApiClient::ApiClient(const std::string& baseUrl) : baseUrl(baseUrl)
{
    // Initialize curlpp
    curlpp::initialize();
}

// Method to get directory listing from the API
DirectoryListing ApiClient::getDirectoryListing(const std::string& path,
                                                int depth)  //
{
    std::map<std::string, std::string> params;  //
    params["path"] = path;                      //
    params["depth"] = std::to_string(depth);    //

    try
    {
        nlohmann::json response = get("/list", params);  // Make the GET request
        return DirectoryListing::fromJson(response);     // Parse JSON response
    }
    catch (const curlpp::RuntimeError& e)  // Network/Curl errors
    {
        throw HomevaultServerException(std::string("Network error: ") +
                                       e.what());  //
    }
    // Assume HomevaultApiException and derived exceptions are caught by caller
    // or handled in get()
}

UploadResponse ApiClient::uploadFile(const std::string& filePath,
                                     const std::string& targetDir)  //
{
    std::map<std::string, std::string>
        params;                  // Parameters other than the file itself
    params["path"] = targetDir;  // Target directory goes as a form field

    try
    {
        // Call the helper function that handles multipart POST
        nlohmann::json response =
            postMultipart("/upload", filePath, params);  //
        return UploadResponse::fromJson(
            response);  // Parse the success response
    }
    catch (const curlpp::RuntimeError& e)  // Network/Curl errors
    {
        throw HomevaultServerException(
            std::string("Network error during upload: ") + e.what());  //
    }
    catch (const std::ifstream::failure& e)  // File reading errors
    {
        // Convert file error to a BadRequest exception as the client provided a
        // bad path
        throw HomevaultBadRequestException(std::string("File error: ") +
                                           e.what());  //
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
    const std::map<std::string, std::string>& params)  //
{
    std::string url = buildUrl(endpoint, params);  //

    try
    {
        curlpp::Easy request;                       //
        request.setOpt(curlpp::options::Url(url));  //

        std::ostringstream responseStream;                              //
        request.setOpt(curlpp::options::WriteStream(&responseStream));  //

        request.perform();  //

        long responseCode = curlpp::infos::ResponseCode::get(request);  //
        std::string responseBody = responseStream.str();                //

        if (responseBody.empty())  //
        {
            throw HomevaultServerException("Empty response from server");  //
        }

        nlohmann::json jsonResponse = nlohmann::json::parse(responseBody);  //

        // Check for HTTP error codes based on OpenAPI spec
        if (responseCode >= 400)  //
        {
            std::string errorMsg = "Unknown server error";
            if (jsonResponse.contains("error") &&
                jsonResponse["error"].is_string())
            {
                errorMsg = jsonResponse["error"].get<std::string>();  //
            }

            if (responseCode == 404)  //
            {
                throw HomevaultNotFoundException(errorMsg);  //
            }
            else if (responseCode == 400)  //
            {
                throw HomevaultBadRequestException(errorMsg);  //
            }
            else  // Treat other 4xx/5xx errors as general server errors
            {
                throw HomevaultServerException(errorMsg);  //
            }
        }

        return jsonResponse;  //
    }
    catch (const nlohmann::json::parse_error& e)  // JSON parsing errors
    {
        throw HomevaultServerException(std::string("JSON parse error: ") +
                                       e.what());  //
    }
    // Let curlpp::RuntimeError propagate up if not caught here
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

nlohmann::json ApiClient::post(const std::string& endpoint,
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
    const std::map<std::string, std::string>& params)  //
{
    std::string url =
        buildUrl(endpoint);  // Build URL (no query params for POST body)

    try
    {
        // Setup curlpp request object
        curlpp::Easy request;                       //
        request.setOpt(curlpp::options::Url(url));  //

        // Prepare the multipart form data
        curlpp::Forms formParts;  // List to hold form parts

        // --- File Part ---
        // Get the filename from the full path
        std::string filename = filePath;                  //
        size_t lastSlash = filename.find_last_of("/\\");  //
        if (lastSlash != std::string::npos)
        {
            filename = filename.substr(lastSlash + 1);  //
        }
        // Add the file form part (using File approach which reads from disk)
        // The original code read the file into memory first, which is less
        // efficient for large files. Using FormParts::File directly is often
        // better.
        formParts.push_back(new curlpp::FormParts::File(
            "file", filePath,
            filename));  // "file" is the field name from OpenAPI
        // Optional: Set content type if needed, though libcurl often guesses
        // well formParts.back()->contentType("application/octet-stream"); //
        // Example

        // --- Text Parameters Part ---
        // Add other parameters from the map as form fields
        for (const auto& [key, value] : params)  //
        {
            formParts.push_back(new curlpp::FormParts::Content(key, value));  //
        }

        // Set the HttpPost option with the form parts
        request.setOpt(curlpp::options::HttpPost(formParts));  //

        // --- Execute Request and Handle Response ---
        std::ostringstream responseStream;                              //
        request.setOpt(curlpp::options::WriteStream(&responseStream));  //

        request.perform();  //

        // --- Cleanup form parts ---
        // Important: Manually delete the allocated FormParts objects
        // for (auto part : formParts)
        // {
        //     delete part;
        // }
        formParts.clear();

        // --- Process Response ---
        long responseCode = curlpp::infos::ResponseCode::get(request);  //
        std::string responseBody = responseStream.str();                //

        if (responseBody.empty())  //
        {
            throw HomevaultServerException("Empty response from server");  //
        }

        nlohmann::json jsonResponse;
        try
        {
            jsonResponse = nlohmann::json::parse(responseBody);  //
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
        if (responseCode >= 400)  //
        {
            std::string errorMsg = "Unknown upload error";
            if (jsonResponse.contains("error") &&
                jsonResponse["error"].is_string())
            {
                errorMsg = jsonResponse["error"].get<std::string>();  //
            }

            // Map status codes to exceptions
            if (responseCode ==
                400)  // Bad request (e.g., invalid path format, missing file)
            {
                throw HomevaultBadRequestException(errorMsg);  //
            }
            // Add 404 case if the target *directory* could be not found
            else if (responseCode == 404)
            {
                throw HomevaultNotFoundException(
                    errorMsg);  // Assuming 404 means target dir not found
            }
            else  // Treat other 4xx/5xx as general server errors
            {
                throw HomevaultServerException(errorMsg);  //
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
            std::string("JSON parse error on upload response: ") +
            e.what());  //
    }
    // Let curlpp::RuntimeError and std::ifstream::failure propagate up
}

std::string ApiClient::buildUrl(
    const std::string& endpoint,
    const std::map<std::string, std::string>& params)  //
{
    std::string url = baseUrl + endpoint;  //

    if (!params.empty())
    {
        url += "?";         //
        bool first = true;  //

        for (const auto& [key, value] : params)  //
        {
            if (!first)
            {
                url += "&";  //
            }
            url += key + "=" + curlpp::escape(value);  // URL-encode parameters
            first = false;                             //
        }
    }
    return url;  //
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

}  // namespace hv
