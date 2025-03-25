#ifndef HV_STATUS_H
#define HV_STATUS_H

namespace hv
{

// More detatied statuses will be added in future
enum class Status
{
    Success,
    ConnectionError,
    UnknownError
};

}

#endif // !HV_STATUS_H
