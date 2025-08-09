#include "order.h"
using json = nlohmann::json;

nlohmann::json LimitOrder::toJson() const {
    json j;
    j["orderId"] = orderId;
    j["inputToken"] = inputToken;
    j["outputToken"] = outputToken;
    j["inputAmount"] = inputAmount;
    j["limitPrice"] = limitPrice;
    j["slippagePct"] = slippagePct;
    j["tif"] = (tif == TIF::GTC ? "GTC" : tif == TIF::GTT ? "GTT" : tif == TIF::IOC ? "IOC" : "FOK");
    j["status"] = (status == Status::PENDING ? "PENDING" :
                   status == Status::PARTIAL_FILLED ? "PARTIAL_FILLED" :
                   status == Status::FILLED ? "FILLED" :
                   status == Status::CANCELED ? "CANCELED" : "FAILED");
    j["filledAmount"] = filledAmount;
    if (tif == TIF::GTT) {
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(expiry.time_since_epoch()).count();
        j["expiry_ms"] = ms;
    }
    return j;
}
