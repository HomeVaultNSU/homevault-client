#ifndef HV_STATUS_H
#define HV_STATUS_H

#include <string>

namespace hv
{

/**
 * @enum Status
 * @brief Represents status of any operation in homevault-core library
 */
enum class Status
{
    eSuccess,
    eConnectionError,
    eUnknownError
};

/**
 * @class ResultValue
 * @brief A class to return value or error with message.
 *
 * Inspired by struct ResultValue from Vulkan-Hpp. Is returned by methods
 * in homevault-core library.
 * Example usage:
 * @code
 * // on success
 * return ResultValue(new std::vector<int>());
 * // on failure
 * return ResultValue(Status::eConnectionError, "Failed to connect to server")
 * @endcode
 *
 * @tparam T The type of return value
 */
template <class T>
class ResultValue
{
public:
    ResultValue(T& value, Status status = Status::eSuccess,
                const std::string& message = "")
        : _value(value), _status(status), _message(message)
    {
    }

    ResultValue(Status status, const std::string& message = "")
        : _value(T()), _status(status), _message(message)
    {
    }

    T& value() { return _value; }
    Status status() { return _status; }
    std::string& message() { return _message; }

private:
    T _value;
    Status _status;
    std::string _message;
};

/**
 * @class Result
 * @brief A class to return status of operation with message.
 *
 * Is returned by methods in homevault-core library. Is used when method
 * doesn't need to return any value, only the status code.
 * Example usage:
 * @code
 * // on success
 * return Result(Status::eSuccess);
 * // on failure
 * return Result(Status::eConnectionError, "Failed to connect to server")
 * @endcode
 */
class Result
{
public:
    Result(Status status, const std::string& message = "")
        : _status(status), _message(message)
    {
    }

    Status status() { return _status; }
    std::string& message() { return _message; }

private:
    Status _status;
    std::string _message;
};

}  // namespace hv

#endif  // !HV_STATUS_H
